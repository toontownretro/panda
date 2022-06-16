/**
 * PANDA3D BSP LIBRARY
 *
 * Copyright (c) Brian Lach <brianlach72@gmail.com>
 * All rights reserved.
 *
 * @file imagePacker.h
 * @author Brian Lach
 * @date September 1, 2020
 */

#ifndef IMAGEPACKER_H
#define IMAGEPACKER_H

#include "pandabase.h"
#include "config_putil.h"
#include "luse.h"

#define MAX_MAX_IMAGE_WIDTH 8192

/**
 * This class attempts to pack several small images onto a single large image.
 * It is currently used for creating lightmap palettes.
 */
class EXPCL_PANDA_PUTIL ImagePacker {
PUBLISHED:
  bool reset(int sort_id, int max_width, int max_height, int border = 2);
  LVecBase2i add_block(int width, int height);
  LVecBase2i get_minimum_dimensions();
  LVecBase2i get_minimum_dimensions_npot();
  float get_efficiency();
  int get_sort_id() const;
  void increment_sort_id();

protected:
  int get_max_y_index(int first_x, int width);

  int _max_width;
  int _max_height;
  int _image_wavefront[MAX_MAX_IMAGE_WIDTH];
  int _area_used;
  int _minimum_height;
  int _minimum_width;

  int _border;

  // For optimization purposes:
  // These store the width + height of the first image
  // that was unable to be stored in this image
  int _max_block_width;
  int _max_block_height;
  int _sort_id;
};

INLINE int ImagePacker::
get_sort_id() const {
  return _sort_id;
}

INLINE void ImagePacker::
increment_sort_id() {
  _sort_id++;
}

#endif // IMAGEPACKER_H
