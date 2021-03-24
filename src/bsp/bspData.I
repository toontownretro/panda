/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file bspData.I
 * @author lachbr
 * @date 2020-12-30
 */

/**
 *
 */
INLINE BSPData::
BSPData() {
  _header = 0;
  _version = 0;
  _map_revision = 0;
  _valid = false;
}

/**
 * Returns the magic number of this BSP file.
 */
INLINE int BSPData::
get_header() const {
  return _header;
}

/**
 * Returns the magic number of this BSP file, formatted as a string.
 */
INLINE std::string BSPData::
get_header_string() const {
  return std::string((const char *)&_header, 4);
}

/**
 * Returns the version of this BSP file.
 */
INLINE int BSPData::
get_version() const {
  return _version;
}

/**
 * Returns true if this is a Source Engine VBSP file, false otherwise.
 */
INLINE bool BSPData::
is_source() const {
  return _is_source;
}

/**
 * Returns the number of lump entries in the BSP file.
 */
INLINE size_t BSPData::
get_num_lumps() const {
  return _lumps.size();
}

/**
 * Returns the lump entry at the indicated index.
 */
INLINE const BSPData::Lump &BSPData::
get_lump(size_t n) const {
  return _lumps[n];
}

/**
 * Returns the revision number of this BSP file.
 */
INLINE int BSPData::
get_map_revision() const {
  return _map_revision;
}

/**
 * Returns true if this is a valid BSP file, false otherwise.
 */
INLINE bool BSPData::
is_valid() const {
  return _valid;
}

/**
 * Returns the version of the indicated lump.
 */
INLINE int BSPData::
get_lump_version(int lump) const {
  return _lumps[lump].version;
}

/**
 * Returns true if the indicated lump exists in the BSP file, false otherwise.
 */
INLINE bool BSPData::
has_lump(int lump) const {
  return _lumps[lump].file_length > 0;
}
