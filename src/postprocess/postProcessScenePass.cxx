/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file postProcessScenePass.cxx
 * @author lachbr
 * @date 2019-07-24
 */

#include "postProcessScenePass.h"
#include "postProcess.h"

#include "cardMaker.h"
#include "auxBitplaneAttrib.h"
#include "orthographicLens.h"

/////////////////////////////////////////////////////////////////////////////////////////
// PostProcessScenePass

PostProcessScenePass::PostProcessScenePass( PostProcess *pp ) :
	PostProcessPass( pp, "scene_pass", pp->get_output()->get_fb_properties() ),
	_aux_bits( 0 ),
	_cam_state( nullptr )
{
	// Inherit the same framebuffer properties as the main window.  This makes
	// sure that we respect things like MSAA, etc, from the PRC file when
	// rendering the main scene.  We have to override some things, though.

	// We can't use backbuffers, we're not presenting anything.
	_fbprops.set_back_buffers(0);
	//_fbprops.set_float_depth(false);
	_fbprops.set_depth_bits(32);
	// We don't need more than 8 bits for RGB, and we don't need alpha.
	//_fbprops.set_rgba_bits(16, 16, 16, 0);
	_fbprops.set_rgb_color(true);
	_fbprops.set_float_color(true);
	// Always render with hardware.
	_fbprops.set_force_hardware(true);
	// This stuff is not needed.
	_fbprops.set_accum_bits(0);
	_fbprops.set_coverage_samples(0);
	_fbprops.set_stencil_bits(0);
	_fbprops.set_srgb_color(false);

	// Enable our auxiliary framebuffer attachments.  Some of them are needed as
	// inputs to post processing passes.
	_fbprops.set_aux_rgba(AUXTEXTURE_COUNT);
}

void PostProcessScenePass::add_aux_output( int n )
{
	PostProcessPass::add_aux_output( n );

	int last_aux_bits = _aux_bits;

	_aux_bits = 0;
	for ( size_t i = 0; i < _aux_textures.size(); i++ )
	{
		if ( _aux_textures[i] )
		{
			_aux_bits |= ( 1 << i );
		}
	}

	if ( _aux_bits != last_aux_bits )
	{
		if (needs_aux_clear(n)) {
			// Make sure the bitplane gets cleared.
			_buffer->set_clear_active(GraphicsBuffer::RTP_aux_rgba_0 + n, true);
			_buffer->set_clear_value(GraphicsBuffer::RTP_aux_rgba_0 + n, get_aux_clear_value(n));
		}

		_cam_state = _cam_state->remove_attrib( AuxBitplaneAttrib::get_class_slot() );
		if ( _aux_bits )
		{
			_cam_state = _cam_state->set_attrib( AuxBitplaneAttrib::make( _aux_bits ) );
		}
		set_camera_state( _cam_state );
	}
}

/**
 * Some of the auxiliary bitplanes need to be cleared to a specific value based
 * on the data that the bitplane represents.  This method returns a appropriate
 * clear value for an auxiliary bitplane.
 */
LColor PostProcessScenePass::
get_aux_clear_value(int n) const {
	switch (n) {
	case AUXTEXTURE_NORMAL:
		return LColor(0, 1, 0, 0);
	case AUXTEXTURE_ARME:
		return LColor(1, 1, 0, 0);
	default:
		return LColor(0, 0, 0, 0);
	}
}

/**
 * Returns true if the indicated bitplane must be cleared before being rendered
 * to.
 */
bool PostProcessScenePass::
needs_aux_clear(int n) const {
	switch (n) {
	case AUXTEXTURE_NORMAL:
	case AUXTEXTURE_ARME:
		return true;
	default:
		return false;
	}
}

bool PostProcessScenePass::setup_buffer()
{
	if ( !PostProcessPass::setup_buffer() )
		return false;

	_buffer->disable_clears();
	// Use the clears from the original window in our offscreen buffer.
	// The window is no longer performing clears.
	_pp->set_window_clears( _buffer );

	return true;
}

void PostProcessScenePass::setup_quad()
{
	CardMaker cm( get_name() + "-quad" );
	cm.set_frame_fullscreen_quad();
	_quad_np = NodePath( cm.generate() );
	_quad_np.set_depth_test( false );
	_quad_np.set_depth_write( false );
	if ( _color_texture )
	{
		_quad_np.set_texture( _color_texture );
	}
	_quad_np.set_color( 1.0f, 0.5f, 0.5, 1.0f );
}

void PostProcessScenePass::setup_camera()
{
	CPT( RenderState ) cam_state = RenderState::make_empty();
	if ( _aux_bits )
	{
		cam_state = cam_state->set_attrib( AuxBitplaneAttrib::make( _aux_bits ) );
	}
	set_camera_state( cam_state );

	PT( Camera ) cam = new Camera( get_name() + "-camera" );
	PT( OrthographicLens ) lens = new OrthographicLens;
	lens->set_film_size( 2, 2 );
	lens->set_film_offset( 0, 0 );
	lens->set_near_far( -1000, 1000 );
	cam->set_lens( lens );
	_camera_np = _quad_np.attach_new_node( cam );
	_camera = cam;
}

void PostProcessScenePass::set_camera_state( const RenderState *state )
{
	_cam_state = state;
	for ( size_t i = 0; i < _pp->get_num_camera_infos(); i++ )
	{
		PostProcess::camerainfo_t *info = _pp->get_camera_info( i );
		DCAST( Camera, info->camera.node() )->set_initial_state( _cam_state );
		info->state = _cam_state;
	}
}

/**
 * Sets up the scene camera to render its contents into our output textures.
 * This is used to have multiple cameras/display regions render into the same output.
 */
void PostProcessScenePass::setup_scene_camera( int i, int sort )
{
	PostProcess::camerainfo_t *info = _pp->get_camera_info( i );
	DCAST( Camera, info->camera.node() )->set_initial_state( _cam_state );
	info->state = _cam_state;

	PT( DisplayRegion ) dr = _buffer->make_display_region();
	dr->disable_clears();
	_pp->set_clears( i, dr );
	dr->set_camera( info->camera );
	dr->set_lens_index(info->lens);
	dr->set_active( true );
	dr->set_sort(sort);
	info->new_region = dr;
}

void PostProcessScenePass::setup_region()
{
	// Render into the final output display region
	_pp->get_output_display_region()->set_camera( _camera_np );
}
