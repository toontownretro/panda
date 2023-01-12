/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file bspData.T
 * @author brian
 * @date 2021-01-01
 */

/**
 * Extracts a single field from the datagram.
 */
template <class T>
INLINE void BSPData::
extract_data(T &dest, BSPData::Field field, DatagramIterator &dgi) {
  switch (field) {
  case F_uint8:
    dest = dgi.get_uint8();
    break;
  case F_uint16:
    dest = dgi.get_uint16();
    break;
  case F_uint32:
    dest = dgi.get_uint32();
    break;
  case F_uint64:
    dest = dgi.get_uint64();
    break;
  case F_int8:
    dest = dgi.get_int8();
    break;
  case F_int16:
    dest = dgi.get_int16();
    break;
  case F_int32:
    dest = dgi.get_int32();
    break;
  case F_int64:
    dest = dgi.get_int64();
    break;
  case F_float32:
    dest = dgi.get_float32();
    break;
  case F_float64:
    dest = dgi.get_float64();
    break;
  default:
    break;
  }
}

/**
 * Extracts a single field from the datagram.
 */
template <>
INLINE void BSPData::
extract_data(LVector3f &out, BSPData::Field field, DatagramIterator &dgi) {
  out.read_datagram_fixed(dgi);
}

/**
 * Extracts a single field from the datagram.
 */
template <>
INLINE void BSPData::
extract_data(LVector4f &out, BSPData::Field field, DatagramIterator &dgi) {
  out.read_datagram_fixed(dgi);
}

/**
 * Copies the given structured lump into the destination vector from the
 * datagram.
 *
 * Returns true on success, false if something went wrong.
 */
template <class T>
INLINE bool BSPData::
copy_lump(int lump, pvector<T> &dest, const Datagram &dg, int force_version) {
  if (!has_lump(lump)) {
    bsp_cat.debug()
      << "Lump " << lump << " does not exist.\n";
    return true;
  }

  bsp_cat.debug()
    << "Reading lump " << lump << "\n";

  int version = _lumps[lump].version;
  if (force_version > -1) {
    version = force_version;
  }

  size_t size = T::get_size(version);
  int length = _lumps[lump].file_length;
  int offset = _lumps[lump].file_offset;

  const Datagram *the_dg = &dg;

  Datagram decoded_dg;

  if (_lumps[lump].uncompressed_size) {
    // Lump is LZMA compressed.  Need to uncompress it into a temp buffer and
    // read from that.
    unsigned char *input = (unsigned char *)dg.get_data() + _lumps[lump].file_offset;
    CLZMA lzma;
    if (lzma.IsCompressed(input)) {
      if (bsp_cat.is_debug()) {
        bsp_cat.debug()
          << "Uncompressing compressed lump " << lump << "\n";
      }
      unsigned char *output = new unsigned char[lzma.GetActualSize(input)];
      if (!lzma.Uncompress(input, output)) {
        bsp_cat.error()
          << "Failed to uncompress lump " << lump << "\n";
        delete[] output;
        return false;
      }

      decoded_dg.assign(output, lzma.GetActualSize(input));

      the_dg = &decoded_dg;

      offset = 0;
      length = _lumps[lump].uncompressed_size;

      delete[] output;
    }
  }

  size_t count = length / size;

  // The byte length should be evenly divisible by the number of lump entries.
  if ((length % size) != 0) {
    bsp_cat.error()
      << "odd lump size for lump " << lump << ", length " << length
      << ", size " << size << ", version " << version << "\n";
    return false;
  }

  dest.resize(count);

  DatagramIterator dgi(*the_dg, offset);
  for (size_t i = 0; i < count; i++) {
    dest[i].read_datagram(dgi, version);
  }

  return true;
}

/**
 * Copies the given single-field lump into the destination vector from the
 * datagram.
 *
 * Returns true on success, false if something went wrong.
 */
template <class T>
INLINE bool BSPData::
copy_lump(BSPData::Field field, int lump, pvector<T> &dest, const Datagram &dg) {
  if (!has_lump(lump)) {
    bsp_cat.debug()
      << "Lump " << lump << " does not exist.\n";
    return true;
  }

  bsp_cat.debug()
    << "Reading lump " << lump << "\n";

  size_t size = sizeof(T);
  int length = _lumps[lump].file_length;
  int offset = _lumps[lump].file_offset;

  const Datagram *the_dg = &dg;

  Datagram decoded_dg;

  if (_lumps[lump].uncompressed_size) {
    if (!uncompress_lump(lump, dg, decoded_dg)) {
      return false;
    }

    the_dg = &decoded_dg;
    offset = 0;
    length = _lumps[lump].uncompressed_size;
  }

  size_t count = length / size;

  // The byte length should be evenly divisible by the number of lump entries.
  if ((length % size) != 0) {
    bsp_cat.error()
      << "odd lump size for lump " << lump << ", length " << length
      << ", size " << size << "\n";
    return false;
  }

  dest.resize(count);

  DatagramIterator dgi(*the_dg, offset);
  for (size_t i = 0; i < count; i++) {
    extract_data(dest[i], field, dgi);
  }

  return true;
}
