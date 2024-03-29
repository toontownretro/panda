/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file postProcess.h
 * @author brian
 * @date 2019-07-22
 */

#ifndef POSTPROCESS_H
#define POSTPROCESS_H

#include "config_postprocess.h"

#include "referenceCount.h"
#include "pointerTo.h"
#include "namable.h"
#include "simpleHashMap.h"
#include "nodePath.h"
#include "aa_luse.h"
#include "frameBufferProperties.h"
#include "graphicsBuffer.h"
#include "genericAsyncTask.h"
#include "camera.h"
#include "renderState.h"

#include "postProcessEffect.h"
#include "postProcessScenePass.h"

class GraphicsOutput;

class EXPCL_PANDA_POSTPROCESS PostProcess : public ReferenceCount
{
public:
	struct clearinfo_t
	{
		clearinfo_t()
		{
			active = false;
			value = LColor( 0 );
		}
		bool active;
		LColor value;
	};
	typedef pvector<clearinfo_t> ClearInfoArray;

	struct camerainfo_t : public ReferenceCount
	{
		camerainfo_t()
		{
			region_clears.resize( GraphicsOutput::RTP_COUNT );
			original_state = nullptr;
			state = nullptr;
			new_region = nullptr;
			lens = 0;
		}

		NodePath camera;
		int lens;
		CPT( RenderState ) original_state;
		CPT( RenderState ) state;
		ClearInfoArray region_clears;
		PT( DisplayRegion ) new_region;
	};
	typedef pvector<PT( camerainfo_t )> CameraInfoArray;

	INLINE size_t get_num_camera_infos() const
	{
		return _camera_info.size();
	}
	INLINE camerainfo_t *get_camera_info( int n )
	{
		return _camera_info[n];
	}

PUBLISHED:
	PostProcess();

	void startup( GraphicsOutput *output );

	void add_camera( const NodePath &camera, int lens, int sort = 0 );
	void remove_camera( const NodePath &camera, int lens = -1 );

	INLINE void push_output_pipe(const std::string &identifier, Texture *texture) {
		_output_pipes[identifier] = texture;
	}

	INLINE Texture *get_output_pipe(const std::string &identifier) const {
		int itr = _output_pipes.find(identifier);
		if (itr != -1) {
			return _output_pipes.get_data(itr);
		}
		return nullptr;
	}

	void add_effect(PostProcessEffect *effect);
	void remove_effect(PostProcessEffect *effect);

	INLINE PostProcessEffect *get_effect( const std::string &name )
	{
		return _effects[name];
	}

	INLINE PostProcessScenePass *get_scene_pass() const
	{
		return _scene_pass;
	}

	INLINE GraphicsOutput *get_output() const
	{
		return _output;
	}

	INLINE NodePath get_camera( int n ) const
	{
		return _camera_info[n]->camera;
	}

	INLINE int get_camera_lens(int n) const
	{
		return _camera_info[n]->lens;
	}

	INLINE int next_sort()
	{
		return _buffer_sort++;
	}

	void set_scene_aux_bits( int bits );
	void set_stacked_clears( int n, DrawableRegion *region );
	void set_window_clears( DrawableRegion *region );
	void set_clears( int n, DrawableRegion *region );

	void shutdown();

	void update();
	void window_event();

private:
	ClearInfoArray _window_clears;
	CameraInfoArray _camera_info;

	void get_clears( DrawableRegion *region, ClearInfoArray &info );
	void set_clears( DrawableRegion *region, const ClearInfoArray &info );
	void set_stacked_clears( DrawableRegion *region, const ClearInfoArray &a, const ClearInfoArray &b );

	typedef SimpleHashMap<std::string, PT(PostProcessEffect), string_hash> PPEffects;
	PPEffects _effects;

	GraphicsOutput *_output;

	SimpleHashMap<std::string, PT(Texture), string_hash> _output_pipes;

	// The main scene is rendered into the textures on this pass.
	PT(PostProcessScenePass) _scene_pass;

	int _buffer_sort;
};

#endif // POSTPROCESS_H
