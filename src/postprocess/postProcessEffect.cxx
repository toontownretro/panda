/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file postProcessEffect.cxx
 * @author lachbr
 * @date 2019-07-24
 */

#include "postProcessEffect.h"
#include "postProcessPass.h"

/////////////////////////////////////////////////////////////////////////////////////////
// PostProcessEffect

IMPLEMENT_CLASS( PostProcessEffect );

void PostProcessEffect::setup()
{
	for ( size_t i = 0; i < _passes.size(); i++ )
	{
		_passes.get_data( i )->setup();
	}
}

void PostProcessEffect::shutdown()
{
	for ( size_t i = 0; i < _passes.size(); i++ )
	{
		_passes.get_data( i )->shutdown();
	}
	//_passes.clear();

	_pp = nullptr;
}

void PostProcessEffect::update()
{
	size_t passes = _passes.size();
	for ( size_t i = 0; i < passes; i++ )
	{
		_passes.get_data( i )->update();
	}
}

void PostProcessEffect::window_event( GraphicsOutput *output )
{
	size_t passes = _passes.size();
	for ( size_t i = 0; i < passes; i++ )
	{
		_passes.get_data( i )->window_event( output );
	}
}

void PostProcessEffect::add_pass( PostProcessPass *pass )
{
	_passes[pass->get_name()] = pass;
}

void PostProcessEffect::remove_pass( PostProcessPass *pass )
{
	int itr = _passes.find( pass->get_name() );
	if ( itr != -1 )
		_passes.remove_element( itr );
}

PostProcessPass *PostProcessEffect::get_pass( const std::string &name )
{
	return _passes[name];
}

/**
 * Returns the number of passes present in the effect.
 */
size_t PostProcessEffect::
get_num_passes() const {
	return _passes.size();
}

/**
 * Returns the nth pass in the effect
 */
PostProcessPass *PostProcessEffect::
get_pass(size_t n) const {
	return _passes.get_data(n);
}
