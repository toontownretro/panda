/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file physXStreams.cxx
 * @author brian
 * @date 2021-06-22
 */

#include "physXStreams.h"

/**
 *
 */
PhysXInputStream::
PhysXInputStream(IStreamWrapper &wrapper) :
  _stream(wrapper)
{
}

/**
 *
 */
uint32_t PhysXInputStream::
read(void *dest, uint32_t count) {
  std::streamsize read_bytes;
  _stream.read((char *)dest, (std::streamsize)count, read_bytes);
  return (uint32_t)read_bytes;
}

/**
 *
 */
PhysXInputData::
PhysXInputData(IStreamWrapper &wrapper) :
  _stream(wrapper)
{
}

/**
 *
 */
uint32_t PhysXInputData::
read(void *dest, uint32_t count) {
  std::streamsize read_bytes;
  _stream.read((char *)dest, (std::streamsize)count, read_bytes);
  return (uint32_t)read_bytes;
}

/**
 *
 */
uint32_t PhysXInputData::
getLength() const {
  std::streampos curr = _stream.get_istream()->tellg();
  _stream.get_istream()->seekg(0, std::ios::end);
  std::streampos len = _stream.get_istream()->tellg();
  _stream.get_istream()->seekg(curr);
  return (uint32_t)len;
}

/**
 *
 */
void PhysXInputData::
seek(uint32_t offset) {
  _stream.get_istream()->seekg((std::streampos)offset);
}

/**
 *
 */
uint32_t PhysXInputData::
tell() const {
  return (uint32_t)(_stream.get_istream()->tellg());
}

/**
 *
 */
PhysXOutputStream::
PhysXOutputStream(OStreamWrapper &wrapper) :
  _stream(wrapper) {
}

/**
 *
 */
uint32_t PhysXOutputStream::
write(const void *data, uint32_t count) {
  std::streampos curr = _stream.get_ostream()->tellp();
  _stream.write((const char *)data, (std::streamsize)count);
  return (uint32_t)(_stream.get_ostream()->tellp() - curr);
}
