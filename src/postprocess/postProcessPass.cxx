/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file postProcessPass.cxx
 * @author lachbr
 * @date 2019-07-24
 */

#include "postProcessPass.h"
#include "postProcess.h"

#include "graphicsEngine.h"
#include "cardMaker.h"
#include "orthographicLens.h"
#include "omniBoundingVolume.h"
#include "renderState.h"
#include "depthWriteAttrib.h"
#include "depthTestAttrib.h"
#include "callbackObject.h"
#include "displayRegionDrawCallbackData.h"

/**
 * Callback that is executed whenever the display region of a pass should be
 * drawn.  Calls the on_draw() method to allow the pass to implement specific
 * draw behavior if necessary.
 */
class PostProcessPassDrawCallback : public CallbackObject {
public:
	ALLOC_DELETED_CHAIN(PostProcessPassDrawCallback);

	PostProcessPassDrawCallback(PostProcessPass *pass);

	virtual void do_callback(CallbackData *cbdata) override;

private:
	PostProcessPass *_pass;
};

/**
 *
 */
PostProcessPassDrawCallback::
PostProcessPassDrawCallback(PostProcessPass *pass) :
	_pass(pass)
{
}

/**
 *
 */
void PostProcessPassDrawCallback::
do_callback(CallbackData *cbdata) {
	DisplayRegionDrawCallbackData *drd_cbdata;
	DCAST_INTO_V(drd_cbdata, cbdata);

	GraphicsOutput *output = _pass->get_buffer();
	nassertv(output != nullptr);

	GraphicsStateGuardian *gsg = output->get_gsg();
	nassertv(gsg != nullptr);

	// We are only making Panda GSG calls, so the state will remain safe.
	drd_cbdata->set_lost_state(false);

	_pass->on_draw(drd_cbdata, gsg);
}

/////////////////////////////////////////////////////////////////////////////////////////
// PostProcessPass

IMPLEMENT_CLASS(PostProcessPass);

FrameBufferProperties *PostProcessPass::_default_fbprops = nullptr;

/**
 *
 */
FrameBufferProperties PostProcessPass::
get_default_fbprops() {
	if (!_default_fbprops) {
		_default_fbprops = new FrameBufferProperties;
		_default_fbprops->clear();
		_default_fbprops->set_srgb_color(false);
		_default_fbprops->set_float_depth(false);
		_default_fbprops->set_depth_bits(0);
		_default_fbprops->set_back_buffers(0);
		_default_fbprops->set_multisamples(0);
		_default_fbprops->set_accum_bits(0);
		_default_fbprops->set_aux_float(0);
		_default_fbprops->set_aux_rgba(0);
		_default_fbprops->set_aux_hrgba(0);
		_default_fbprops->set_coverage_samples(0);
		_default_fbprops->set_rgb_color(true);
		_default_fbprops->set_float_color(true);
	}

	return *_default_fbprops;
}

/**
 *
 */
PostProcessPass::
PostProcessPass(PostProcess *pp, const std::string &name,
				  const FrameBufferProperties &fbprops,
				  bool force_size, const LVector2i &forced_size,
				  bool div_size, int div) :
	Namable(name),
	_pp(pp),
	_fbprops(fbprops),
	_force_size(force_size),
	_forced_size(forced_size),
	_buffer(nullptr),
	_camera(nullptr),
	_region(nullptr),
	_div_size(div_size),
	_div(div),
	_window_layer(false),
	_layer_window(nullptr),
	_layer_sort(0),
	_color_texture(nullptr),
	_depth_texture(nullptr)
{
	for (int i = 0; i < AUXTEXTURE_COUNT; i++) {
		_aux_textures.push_back(nullptr);
	}
}

/**
 *
 */
LVector2i PostProcessPass::
get_back_buffer_dimensions() const {
	return _pp->get_output()->get_size();
}

/**
 *
 */
Lens *PostProcessPass::
get_scene_lens() const {
	return DCAST(Camera, _pp->get_camera(0).node())->get_lens();
}

/**
 *
 */
LVector2i PostProcessPass::
get_corrected_size(const LVector2i &size) {
	if (_force_size) {
		if (_div_size) {
			return _forced_size / _div;
		}

		return _forced_size;
	} else if (_div_size) {
		return size / _div;
	}

	return size;
}

/**
 *
 */
void PostProcessPass::
add_color_output() {
	nassertv(!is_window_layer());
	nassertv(_buffer != nullptr);
	if (!_color_texture) {
		_color_texture = make_texture(Texture::F_rgba16, "color");
		_color_texture->set_clear_color(LColor(0, 0, 0, 0));
		_color_texture->clear_image();
		_buffer->add_render_texture(_color_texture, GraphicsOutput::RTM_bind_or_copy, GraphicsOutput::RTP_color);
	}
}

/**
 *
 */
void PostProcessPass::
add_depth_output() {
	nassertv(!is_window_layer());
	nassertv(_buffer != nullptr);
	if (!_depth_texture) {
		_depth_texture = make_texture(Texture::F_depth_component, "depth");
		_buffer->add_render_texture(_depth_texture, GraphicsOutput::RTM_bind_or_copy, GraphicsOutput::RTP_depth);
	}
}

/**
 *
 */
void PostProcessPass::
add_aux_output(int n) {
	nassertv(!is_window_layer());
	nassertv(_buffer != nullptr);
	if (!_aux_textures[n]) {
		char name[10];
		sprintf(name, "aux%i", n);
		_aux_textures[n] = make_texture(Texture::F_rgba, name);
		_buffer->add_render_texture(_aux_textures[n], GraphicsOutput::RTM_bind_or_copy,
					     (GraphicsBuffer::RenderTexturePlane)(GraphicsOutput::RTP_aux_rgba_0 + n));
	}
}

/**
 *
 */
PT(Texture) PostProcessPass::
make_texture(Texture::Format format, const std::string &suffix) {
	PT(Texture) tex = new Texture(get_name() + "-" + suffix);
	tex->set_wrap_u(SamplerState::WM_clamp);
	tex->set_wrap_v(SamplerState::WM_clamp);
	tex->set_minfilter(SamplerState::FT_linear);
	tex->set_magfilter(SamplerState::FT_linear);
	tex->set_anisotropic_degree(1);
	return tex;
}

/**
 * Creates an offscreen buffer for the pass that will be used to apply effects
 * and output a result.  If the pass is a window layer, no offscreen buffer is
 * created, and the output window will be used instead.  Note that texture
 * outputs can only be added if the pass uses an offscreen buffer.
 */
bool PostProcessPass::
setup_buffer() {
	GraphicsOutput *window = _pp->get_output();

	if (is_window_layer()) {
		// We're a display region of the main window, no offscreen buffer needed.
		if (_layer_window == nullptr) {
			_layer_window = window;
		}
		_buffer = _layer_window;
		return true;
	}

	WindowProperties winprops;
	winprops.set_size(get_corrected_size(window->get_size()));

	FrameBufferProperties fbprops = _fbprops;
	fbprops.set_back_buffers(0);
	fbprops.set_stereo(window->is_stereo());

	int flags = GraphicsPipe::BF_refuse_window;
	if (!_force_size) {
		flags |= GraphicsPipe::BF_resizeable;
	}

	PT(GraphicsOutput) output = window->get_engine()->make_output(
		window->get_pipe(), get_name(), _pp->next_sort(),
		fbprops, winprops, flags, window->get_gsg(), window);
	nassertr(output != nullptr, false);

	_buffer = DCAST(GraphicsBuffer, output);
	_buffer->set_clear_color_active(true);

	return true;
}

/**
 * Creates a screen-space quad for the pass.  If needed, a shader can be
 * applied to the quad to apply effects and have them outputted to the display
 * region/output textures of the pass.
 */
void PostProcessPass::
setup_quad() {
	CardMaker cm(get_name() + "-quad");
	cm.set_frame(-1, 1, -1, 1);
	_quad_np = NodePath(cm.generate());
	_quad_np.set_depth_test(false);
	_quad_np.set_depth_write(false);
	//_quad_np.set_transparency(TransparencyAttrib::M_alpha);
}

/**
 * Creates the camera that will be used to render any geometry into the display
 * region of the pass.
 */
void PostProcessPass::
setup_camera() {
	PT(OrthographicLens) lens = new OrthographicLens;
	lens->set_film_size(2, 2);
	lens->set_film_offset(0, 0);
	lens->set_near_far(-1000, 1000);

	PT(Camera) cam = new Camera(get_name() + "-camera");
	cam->set_bounds(new OmniBoundingVolume);
	cam->set_lens(lens);

	// Disable z-testing and z-writing, we only render a single 2D quad.
	CPT(RenderState) state = RenderState::make(
		DepthTestAttrib::make(DepthTestAttrib::M_none),
		DepthWriteAttrib::make(DepthWriteAttrib::M_off));
	cam->set_initial_state(state);

	_camera = cam;
	if (!_quad_np.is_empty()) {
		_camera_np = _quad_np.attach_new_node(cam);
	} else {
		_camera_np = NodePath(cam);
	}
}

/**
 * Creates the display region that the pass will be rendered into.  If the pass
 * is a window layer, the display region is created on the main window.
 * Otherwise, it is created on the offscreen buffer of the pass.
 */
void PostProcessPass::
setup_region() {
	PT(DisplayRegion) dr = _buffer->make_display_region(0, 1, 0, 1);
	dr->set_draw_callback(new PostProcessPassDrawCallback(this));
	dr->disable_clears();
	dr->set_camera(_camera_np);
	dr->set_active(true);
	dr->set_scissor_enabled(is_window_layer());
	if (is_window_layer()) {
		dr->set_sort(_layer_sort);
	}
	_region = dr;
}

/**
 * Creates all the elements of the pass.  This creates an offscreen buffer (if
 * the pass is not a window layer), a screen-space quad, a camera/lens, and a
 * display region.  You may or may not need all of these elements for your
 * pass.  If that is the case, you may individually create only the elements
 * you need via setup_buffer/quad/camera/region().  If the pass is a window
 * layer, the display region is created on the output window instead of an
 * offscreen buffer.
 */
void PostProcessPass::
setup() {
	if (!setup_buffer()) {
		return;
	}

	setup_quad();
	setup_camera();
	setup_region();
}

/**
 * Called every frame to update the pass as necessary.
 */
void PostProcessPass::
update() {
}

/**
 * Called when a window event is thrown for the output window.  If the window
 * size changed, resizes the offscreen buffer accordingly.
 */
void PostProcessPass::
window_event(GraphicsOutput *output) {
	if (is_window_layer()) {
		return;
	}

	if (!_force_size && _buffer != nullptr) {
		GraphicsBuffer *buffer;
		DCAST_INTO_V(buffer, _buffer);

		LVector2i size = output->get_size();
		get_corrected_size(size);
		if (size != buffer->get_size()) {
			buffer->set_size(size[0], size[1]);
		}
	}
}

/**
 * Called when the display region of the pass should be drawn.
 */
void PostProcessPass::
on_draw(DisplayRegionDrawCallbackData *cbdata, GraphicsStateGuardian *gsg) {
	// Default behavior is to just draw it.  Derived classes might start a query
	// or perform a copy before drawing.
	cbdata->upcall();
}

/**
 *
 */
void PostProcessPass::
shutdown() {
	if (_buffer == nullptr) {
		return;
	}

	if (_region != nullptr) {
		_buffer->remove_display_region(_region);
	}
	_region = nullptr;
	if (!is_window_layer()) {
		// Only do this if we are an offscreen buffer and not a layer on the main
		// window.  We don't want to remove the main window.
		_buffer->clear_render_textures();
		_buffer->get_engine()->remove_window(_buffer);
	}
	_buffer = nullptr;
	if (!_camera_np.is_empty()) {
		_camera_np.remove_node();
	}
	_camera = nullptr;
	if (!_quad_np.is_empty()) {
		_quad_np.remove_node();
	}

	_color_texture = nullptr;
	_depth_texture = nullptr;
	_aux_textures.clear();

	_pp = nullptr;
}
