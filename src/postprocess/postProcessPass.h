/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file postProcessPass.h
 * @author lachbr
 * @date 2019-07-24
 */

#ifndef POSTPROCESSPASS_H
#define POSTPROCESSPASS_H

#include "config_postprocess.h"
#include "postProcessDefines.h"

#include "typedObject.h"
#include "referenceCount.h"
#include "namable.h"
#include "frameBufferProperties.h"
#include "nodePath.h"
#include "aa_luse.h"
#include "graphicsBuffer.h"
#include "camera.h"

class PostProcess;
class DisplayRegionDrawCallbackData;

/**
 * Base class for an onscreen/offscreen render layer.  This object doesn't
 * necessarily have to draw anything, it can also be used to issue certain
 * GSG commands at a specific point in the render pipeline.
 */
class EXPCL_PANDA_POSTPROCESS PostProcessPass : public ReferenceCount, public Namable {
	DECLARE_CLASS2(PostProcessPass, ReferenceCount, Namable);

PUBLISHED:
	PostProcessPass( PostProcess *pp, const std::string &name = "pass",
			 const FrameBufferProperties &fbprops = PostProcessPass::get_default_fbprops(),
			 bool force_size = false, const LVector2i &forced_size = LVector2i::zero(), bool div_size = false, int div = 1 );

	virtual void add_color_output();
	virtual void add_depth_output();
	virtual void add_aux_output( int n );

	INLINE NodePath get_quad() const
	{
		return _quad_np;
	}

	LVector2i get_back_buffer_dimensions() const;
	Lens *get_scene_lens() const;

	INLINE NodePath get_camera() const
	{
		return _camera_np;
	}

	INLINE void set_div_size( bool div_size, int div )
	{
		_div_size = div_size;
		_div = div;
	}

	INLINE void set_forced_size( bool force_size, const LVector2i &forced_size )
	{
		_force_size = force_size;
		_forced_size = forced_size;
	}

	INLINE void set_framebuffer_properties( const FrameBufferProperties &fbprops )
	{
		_fbprops = fbprops;
	}

	INLINE const FrameBufferProperties &get_framebuffer_properties() const {
		return _fbprops;
	}

	LVector2i get_corrected_size( const LVector2i &size );

	INLINE Texture *get_aux_texture( int n )
	{
		return _aux_textures[n];
	}

	INLINE Texture *get_color_texture()
	{
		return _color_texture;
	}
	INLINE Texture *get_depth_texture()
	{
		return _depth_texture;
	}

	INLINE GraphicsOutput *get_buffer() const {
		return _buffer;
	}

	INLINE DisplayRegion *get_display_region() const {
		return _region;
	}

	INLINE void set_window_layer(bool flag, GraphicsOutput *window = nullptr, int sort = 0) {
		_window_layer = flag;
		_layer_window = window;
		_layer_sort = sort;
	}

	INLINE bool is_window_layer() const {
		return _window_layer;
	}

	PT( Texture ) make_texture( Texture::Format format, const std::string &suffix );

	virtual bool setup_buffer();
	virtual void setup_quad();
	virtual void setup_camera();
	virtual void setup_region();
	virtual void setup();

	virtual void update();
	virtual void window_event(GraphicsOutput *output);

	virtual void on_draw(DisplayRegionDrawCallbackData *cbdata, GraphicsStateGuardian *gsg);

	virtual void shutdown();

	static FrameBufferProperties get_default_fbprops();

protected:
	PostProcess *_pp;

	// True if the pass is a layer/display region on the main window, rather
	// than an offscreen buffer.
	bool _window_layer;
	PT(GraphicsOutput) _layer_window;
	int _layer_sort;

	PT( GraphicsOutput ) _buffer;
	PT( DisplayRegion ) _region;
	NodePath _camera_np;
	PT( Camera ) _camera;
	NodePath _quad_np;

	bool _force_size;
	LVector2i _forced_size;
	FrameBufferProperties _fbprops;
	bool _div_size;
	int _div;

	// Output textures from this pass
	PT( Texture ) _color_texture;
	PT( Texture ) _depth_texture;
	pvector<PT( Texture )> _aux_textures;

	static FrameBufferProperties *_default_fbprops;
};

#endif // POSTPROCESSPASS_H
