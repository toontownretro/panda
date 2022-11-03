/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file freezeFrame.cxx
 * @author brian
 * @date 2021-06-07
 */

#include "freezeFrame.h"
#include "postProcessPass.h"
#include "postProcess.h"
#include "frameBufferProperties.h"
#include "clockObject.h"

/**
 * Does the job of capturing a freeze frame when requested and rendering the
 * frozen frame to the screen while frozen.
 */
class FreezeFrameLayer : public PostProcessPass {
public:
  FreezeFrameLayer(PostProcess *pp, FreezeFrameEffect *effect);

  virtual void on_draw(DisplayRegionDrawCallbackData *cbdata, GraphicsStateGuardian *gsg) override;
  virtual void setup();
  virtual void update();

private:
  FreezeFrameEffect *_effect;
};

/**
 *
 */
FreezeFrameLayer::
FreezeFrameLayer(PostProcess *pp, FreezeFrameEffect *effect) :
  PostProcessPass(pp, "freeze-frame-layer"),
  _effect(effect)
{
}

/**
 *
 */
void FreezeFrameLayer::
on_draw(DisplayRegionDrawCallbackData *cbdata, GraphicsStateGuardian *gsg) {
  FreezeFrameEffect::CDReader cdata(_effect->_cycler);

  double now = ClockObject::get_global_clock()->get_frame_time();

  if (now < cdata->_freeze_frame_until) {
    if (cdata->_take_freeze_frame) {
      // Capture a freeze frame.
      gsg->framebuffer_copy_to_texture(_effect->_freeze_frame_texture, 0, -1,
        gsg->get_current_display_region(), RenderBuffer(gsg, RenderBuffer::T_color));
    }

    // Draw frozen frame.
    PostProcessPass::on_draw(cbdata, gsg);
  }

  // Otherwise draw nothing, we're not freeze framing.
}

/**
 *
 */
void FreezeFrameLayer::
setup() {
  PostProcessPass::setup();

  NodePath quad = get_quad();
  quad.set_shader(
    Shader::load(Shader::SL_GLSL,
      "shaders/postprocess/base.vert.glsl",
      "shaders/postprocess/freeze_frame.frag.glsl"));
  quad.set_shader_input("freezeFrameSampler", _effect->_freeze_frame_texture);

  // Not active until we actually need to freeze frame.
  //_region->set_active(false);
}

/**
 *
 */
void FreezeFrameLayer::
update() {
  PostProcessPass::update();

  bool turn_off_capture = false;
  bool turn_on_capture = false;

  {
    FreezeFrameEffect::CDReader cdata(_effect->_cycler);
    double now = ClockObject::get_global_clock()->get_frame_time();
    if (now < cdata->_freeze_frame_until) {
      if (_effect->_took_freeze_frame) {
        // We took a freeze frame last frame, so disable the
        // freeze capture and render the frozen frame until
        // time expires.
        turn_off_capture = true;
        _effect->_took_freeze_frame = false;
        assert(cdata->_take_freeze_frame);

      } else if (_effect->_request_freeze_frame) {
        // The user requested a freeze frame.  Enable freeze frame
        // capturing and note that we took a freeze frame this frame.
        turn_on_capture = true;
        _effect->_request_freeze_frame = false;
        _effect->_took_freeze_frame = true;
      }
    }
  }

  if (turn_off_capture) {
    //std::cout << "Turning off capture\n";
    FreezeFrameEffect::CDWriter cdata(_effect->_cycler);
    cdata->_take_freeze_frame = false;

  } else if (turn_on_capture) {
    //std::cout << "Turning on capture\n";
    FreezeFrameEffect::CDWriter cdata(_effect->_cycler);
    cdata->_take_freeze_frame = true;
  }
}

IMPLEMENT_CLASS(FreezeFrameEffect);

/**
 *
 */
FreezeFrameEffect::
FreezeFrameEffect(PostProcess *pp) :
  PostProcessEffect(pp, "freeze-frame-render"),
  _took_freeze_frame(false),
  _request_freeze_frame(false)
{
  _freeze_frame_texture = new Texture("freeze-frame");
  _freeze_frame_texture->set_match_framebuffer_format(true);
  _freeze_frame_texture->set_minfilter(SamplerState::FT_linear);
  _freeze_frame_texture->set_magfilter(SamplerState::FT_linear);

  PT(FreezeFrameLayer) layer = new FreezeFrameLayer(pp, this);
  // We render directly to the window, not offscreen.
  // Sort of -999 to render directly after the final output, which is sort -1000.
  layer->set_window_layer(true, pp->get_output(), -999);
  layer->setup();
  add_pass(layer);
}

/**
 * Freezes the frame for the specified duration.
 */
void FreezeFrameEffect::
freeze_frame(double duration) {
  CDWriter cdata(_cycler);

	if (duration == 0.0) {
		cdata->_freeze_frame_until = 0.0;
		cdata->_take_freeze_frame = false;
    _took_freeze_frame = false;
    _request_freeze_frame = false;

	} else {
		double now = ClockObject::get_global_clock()->get_frame_time();
		if (cdata->_freeze_frame_until > now) {
			cdata->_freeze_frame_until += duration;

		} else {
			cdata->_freeze_frame_until = now + duration;
      _took_freeze_frame = false;
      _request_freeze_frame = true;
		}
	}
}

/**
 *
 */
FreezeFrameEffect::CData::
CData() :
  _take_freeze_frame(false),
  _freeze_frame_until(0.0)
{
}

/**
 *
 */
FreezeFrameEffect::CData::
CData(const CData &copy) :
  _take_freeze_frame(copy._take_freeze_frame),
  _freeze_frame_until(copy._freeze_frame_until)
{
}

/**
 *
 */
CycleData *FreezeFrameEffect::CData::
make_copy() const {
  return new CData(*this);
}
