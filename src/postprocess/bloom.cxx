/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file bloom.cxx
 * @author brian
 * @date 2019-07-23
 */

#include "bloom.h"
#include "postProcess.h"
#include "postProcessPass.h"
#include "blurPasses.h"
#include "hdr.h"
#include "colorWriteAttrib.h"
#include "colorBlendAttrib.h"

#include "configVariableDouble.h"
#include "texture.h"

static ConfigVariableDouble bloom_strength("bloom-strength", 1.0);
static ConfigVariableInt bloom_blur_passes("bloom-blur-passes", 5);
static ConfigVariableBool bloom_remove_fireflies("bloom-remove-fireflies", true);

class RemoveFireflies : public PostProcessPass {
public:
	RemoveFireflies(PostProcess *pp, Texture *source_tex) :
		PostProcessPass(pp, "bloom-remove_fireflies"),
		_source_tex(source_tex)
	{
		_fbprops.set_rgba_bits(16, 16, 16, 0);
		_fbprops.set_alpha_bits(0);
	}

	virtual void setup() {
		PostProcessPass::setup();

		get_quad().set_shader(
			Shader::load(Shader::SL_GLSL,
				"shaders/postprocess/base.vert.glsl",
				"shaders/postprocess/remove_fireflies.frag.glsl"));

		get_quad().set_shader_input("sourceTexture", _source_tex);
	}

private:
	Texture *_source_tex;
};

class ExtractBrightSpots : public PostProcessPass {
public:
	ExtractBrightSpots(PostProcess *pp, Texture *source_tex, Texture *dest_tex) :
		PostProcessPass(pp, "bloom-extract_bright_spots"),
		_dest_tex(dest_tex),
		_source_tex(source_tex)
	{
		_fbprops.set_rgb_color(false);
		_fbprops.set_rgba_bits(0, 0, 0, 0);
	}

	virtual void setup() {
		PostProcessPass::setup();

		get_quad().set_attrib(ColorWriteAttrib::make(ColorWriteAttrib::C_off));
		get_quad().set_attrib(ColorBlendAttrib::make_off());

		get_quad().set_shader(
			Shader::load(Shader::SL_GLSL,
				"shaders/postprocess/base.vert.glsl",
				"shaders/postprocess/extract_bright_spots.frag.glsl"));

		get_quad().set_shader_input("sourceTexture", _source_tex);
		get_quad().set_shader_input("destTexture", _dest_tex, false, true, -1, 0);
		get_quad().set_shader_input("bloomStrengthVec",
			LVecBase2(bloom_strength.get_value()));
	}

private:
	Texture *_dest_tex;
	Texture *_source_tex;
};

class BloomDownsample : public PostProcessPass {
public:
	BloomDownsample(const std::string &name, PostProcess *pp, Texture *source_tex, Texture *dest_tex, int mip) :
		PostProcessPass(pp, name),
		_source_tex(source_tex),
		_dest_tex(dest_tex),
		_mip(mip)
	{
		_fbprops.set_rgb_color(false);
		_fbprops.set_rgba_bits(0, 0, 0, 0);
	}

	virtual void setup() {
		PostProcessPass::setup();

		get_quad().set_attrib(ColorWriteAttrib::make(ColorWriteAttrib::C_off));
		get_quad().set_attrib(ColorBlendAttrib::make_off());

		get_quad().set_shader(
			Shader::load(Shader::SL_GLSL,
				"shaders/postprocess/base.vert.glsl",
				"shaders/postprocess/bloom_downsample.frag.glsl"));

		get_quad().set_shader_input("sourceTexture", _source_tex);
		get_quad().set_shader_input("destTexture", _dest_tex, false, true, -1, _mip + 1);
		get_quad().set_shader_input("mipVec", LVecBase2i(_mip));
	}

private:
	Texture *_source_tex;
	Texture *_dest_tex;
	int _mip;
};

class BloomUpsample : public PostProcessPass {
public:
	BloomUpsample(const std::string &name, PostProcess *pp, Texture *source_tex,
							  Texture *dest_tex, int mip, bool first, float strength, float radius) :
		PostProcessPass(pp, name),
		_source_tex(source_tex),
		_dest_tex(dest_tex),
		_mip(mip),
		_first(first),
		_strength(strength),
		_radius(radius)
	{
		_fbprops.set_rgb_color(false);
		_fbprops.set_rgba_bits(0, 0, 0, 0);
	}

	virtual void setup() {
		PostProcessPass::setup();

		get_quad().set_attrib(ColorWriteAttrib::make(ColorWriteAttrib::C_off));
		get_quad().set_attrib(ColorBlendAttrib::make_off());

		get_quad().set_shader(
			Shader::load(Shader::SL_GLSL,
				"shaders/postprocess/base.vert.glsl",
				"shaders/postprocess/bloom_upsample.frag.glsl"));

		get_quad().set_shader_input("sourceTexture", _source_tex);
		get_quad().set_shader_input("destTexture", _dest_tex, false, true, -1, _mip - 1);
		get_quad().set_shader_input("mip_first", LVecBase2i(_mip, _first));
	}

private:
	Texture *_source_tex;
	Texture *_dest_tex;
	int _mip;
	bool _first;
	float _strength;
	float _radius;
};

class ApplyBloom : public PostProcessPass {
public:
	ApplyBloom(PostProcess *pp, Texture *scene_tex, Texture *bloom_tex) :
		PostProcessPass(pp, "bloom-apply"),
		_scene_tex(scene_tex),
		_bloom_tex(bloom_tex)
	{
		_fbprops.set_rgba_bits(16, 16, 16, 0);
		_fbprops.set_float_color(true);
	}

	virtual void setup() {
		PostProcessPass::setup();

		get_quad().set_shader(
			Shader::load(Shader::SL_GLSL,
				"shaders/postprocess/base.vert.glsl",
				"shaders/postprocess/apply_bloom.frag.glsl"));

		get_quad().set_shader_input("sceneTexture", _scene_tex);
		get_quad().set_shader_input("bloomTexture", _bloom_tex);
	}

private:
	Texture *_scene_tex;
	Texture *_bloom_tex;
};

IMPLEMENT_CLASS(BloomEffect);

BloomEffect::
BloomEffect(PostProcess *pp) :
	PostProcessEffect(pp, "bloom")
{
	// Create the bloom texture.  This will be read from and written to by our
	// intermediate passes.
	_bloom_texture = new Texture("bloom-final");
	_bloom_texture->setup_2d_texture(
		pp->get_output()->get_x_size(), pp->get_output()->get_y_size(),
		Texture::T_float, Texture::F_rgba32);
	_bloom_texture->set_minfilter(SamplerState::FT_linear_mipmap_linear);
	_bloom_texture->set_magfilter(SamplerState::FT_linear);
	_bloom_texture->set_wrap_u(SamplerState::WM_clamp);
	_bloom_texture->set_wrap_v(SamplerState::WM_clamp);
	_bloom_texture->set_clear_color(LColor(0.1, 0.0, 0.0, 1.0));
	_bloom_texture->clear_image();

	Texture *scene_texture;
	if (bloom_remove_fireflies) {
		PT(RemoveFireflies) fireflies = new RemoveFireflies(pp, _pp->get_output_pipe("scene_color"));
		fireflies->setup();
		fireflies->add_color_output();
		add_pass(fireflies);

		scene_texture = fireflies->get_color_texture();

	} else {
		scene_texture = _pp->get_output_pipe("scene_color");
	}

	PT(ExtractBrightSpots) extract = new ExtractBrightSpots(pp, scene_texture, _bloom_texture);
	extract->setup();
	add_pass(extract);

	// Downsample.
	for (int i = 0; i < bloom_blur_passes.get_value(); i++) {
		float scale_mult = std::pow(2.0f, (float)(i + 1));
		std::ostringstream ss;
		ss << "bloom-downsample-" << i << "\n";
		PT(BloomDownsample) ds = new BloomDownsample(ss.str(), _pp, _bloom_texture, _bloom_texture, i);
		ds->set_div_size(true, scale_mult);
		ds->setup();
		add_pass(ds);
	}

	// Upsample.
	for (int i = 0; i < bloom_blur_passes.get_value(); i++) {
		float scale_mult = std::pow(2.0f, (float)(bloom_blur_passes.get_value() - i - 1));
		std::ostringstream ss;
		ss << "bloom-upsample-" << i << "\n";
		float strength = bloom_strength.get_value();
		PT(BloomUpsample) us = new BloomUpsample(
			ss.str(), _pp, _bloom_texture, _bloom_texture,
			bloom_blur_passes.get_value() - i, i == 0, strength, 1.0f);
		us->set_div_size(true, scale_mult);
		us->setup();
		add_pass(us);
	}

	// Now create the pass that will add the bloom onto the scene color.
	// The output texture of this pass will replace the current scene color
	// output pipe.
	PT(ApplyBloom) apply = new ApplyBloom(pp, _pp->get_output_pipe("scene_color"), _bloom_texture);
	apply->setup();
	apply->add_color_output();
	add_pass(apply);

	_bloom_combine_texture = apply->get_color_texture();
	_pp->push_output_pipe("scene_color", _bloom_combine_texture);
}

/**
 *
 */
Texture *BloomEffect::
get_final_texture() {
	return _bloom_combine_texture;
}

/**
 *
 */
void BloomEffect::
window_event(GraphicsOutput *win) {
	PostProcessEffect::window_event(win);

	const LVecBase2i &size = win->get_size();
	_bloom_texture->set_x_size(size[0]);
	_bloom_texture->set_y_size(size[1]);
}
