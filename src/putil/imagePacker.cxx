/**
 * PANDA3D BSP LIBRARY
 *
 * Copyright (c) Brian Lach <brianlach72@gmail.com>
 * All rights reserved.
 *
 * @file imagePacker.cxx
 * @author Brian Lach
 * @date September 1, 2020
 */

#include "imagePacker.h"

/**
 *
 */
int
ceil_pow_2(int in) {
	int retval;

	retval = 1;
	while( retval < in ) {
		retval <<= 1;
  }

	return retval;
}

float ImagePacker::
get_efficiency() {
  return (float)_area_used / (float)(_max_width * ceil_pow_2(_minimum_height));
}

bool ImagePacker::
reset(int sort_id, int max_width, int max_height, int border) {
  int i;

  nassertr(max_width <= MAX_MAX_IMAGE_WIDTH, false);

  _max_width = max_width;
  _max_height = max_height;

  _max_block_width = _max_width + 1;
  _max_block_height = _max_height + 1;

  _sort_id = sort_id;

  _area_used = 0;
  _minimum_height = -1;
  _minimum_width = -1;

  _border = border;

  for (i = 0; i < _max_width; i++) {
    _image_wavefront[i] = -1;
  }

  return true;
}

int ImagePacker::
get_max_y_index(int first_x, int width) {
  int max_y = -1;
  int max_y_index = 0;

  for (int x = first_x; x < first_x + width; x++) {
    // NOTE: Want the equals here since we'll never be able to fit
    // in between multiple instances of max_y
    if (_image_wavefront[x] >= max_y) {
      max_y = _image_wavefront[x];
      max_y_index = x;
    }
  }

  return max_y_index;
}

#define ADD_ONE_TEXEL_BORDER

LVecBase2i ImagePacker::
add_block(int width, int height) {
#ifdef ADD_ONE_TEXEL_BORDER
  width += _border * 2;
  height += _border * 2;
  width = std::clamp(width, 0, _max_width);
  height = std::clamp(height, 0, _max_height);
#endif

  // If we've already determined that a block this big couldn't fit
  // then blow off checking again...
  if ((width >= _max_block_width) && (height >= _max_block_height)) {
    return LVecBase2i(-1);
  }

  int best_x = -1;
  int max_y_idx;
  int outer_x = 0;
  int outer_min_y = _max_height;
  int last_x = _max_width - width;
  int last_max_y_val = -2;

  while (outer_x <= last_x) {
    // Skip all tiles that have the last Y value, these
    // aren't going to change our min Y value
    if (_image_wavefront[outer_x] == last_max_y_val) {
      outer_x++;
      continue;
    }

    max_y_idx = get_max_y_index(outer_x, width);
    last_max_y_val = _image_wavefront[max_y_idx];
    if (outer_min_y > last_max_y_val) {
      outer_min_y = last_max_y_val;
      best_x = outer_x;
    }
    outer_x = max_y_idx + 1;
  }

  if (best_x == -1) {
    // If we failed to add it, remember the block size that failed
		// *only if both dimensions are smaller*!!
		// Just because a 1x10 block failed, doesn't mean a 10x1 block will fail
    if ((width <= _max_block_width) && (height <= _max_block_height)) {
      _max_block_width = width;
      _max_block_height = height;
    }

    return LVecBase2i(-1);
  }

  // Set the return positions for the block.
  LVecBase2i offset(best_x, outer_min_y + 1);

  // Check if it actually fits height-wise.
  if (offset[1] + height >= _max_height - 1) {
    if ((width <= _max_block_width) && (height <= _max_block_height)) {
      _max_block_width = width;
      _max_block_height = height;
    }

    return LVecBase2i(-1);
  }

  // It fit!
  // Keep up with the smallest possible size for the image so far.
  if (offset[1] + height > _minimum_height) {
    _minimum_height = offset[1] + height;
  }
  if (offset[0] + width > _minimum_width) {
    _minimum_width = offset[0] + width;
  }

  // Update the wavefront info.
  int x;
  for (x = best_x; x < best_x + width; x++) {
    _image_wavefront[x] = outer_min_y + height;
  }

  _area_used += width * height;

#ifdef ADD_ONE_TEXEL_BORDER
  offset[0] += _border;
  offset[1] += _border;
#endif

  return offset;
}

LVecBase2i ImagePacker::
get_minimum_dimensions() {
  return LVecBase2i(
    ceil_pow_2(_minimum_width),
    ceil_pow_2(_minimum_height)
  );
}

LVecBase2i ImagePacker::
get_minimum_dimensions_npot() {
  return LVecBase2i(
    _minimum_width,
    _minimum_height
  );
}
