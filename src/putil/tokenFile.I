/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file tokenFile.I
 * @author lachbr
 * @date 2021-02-12
 */

/**
 *
 */
INLINE TokenFile::
TokenFile() {
  _token_index = -1;
  _token = nullptr;
}

/**
 * Returns the filename of the TokenFile.
 */
INLINE const Filename &TokenFile::
get_filename() const {
  return _filename;
}

/**
 * Returns the fullpath of the TokenFile.
 */
INLINE const Filename &TokenFile::
get_fullpath() const {
  return _fullpath;
}
