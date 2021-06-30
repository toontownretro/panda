/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file physXStreams.h
 * @author brian
 * @date 2021-06-22
 */

#ifndef PHYSXSTREAMS_H
#define PHYSXSTREAMS_H

#include "physx_includes.h"
#include "streamWrapper.h"

/**
 *
 */
class PhysXInputStream : public physx::PxInputStream {
public:
  PhysXInputStream(IStreamWrapper &wrapper);

  virtual uint32_t read(void *dest, uint32_t count) override;

private:
  IStreamWrapper &_stream;
};

/**
 *
 */
class PhysXInputData : public physx::PxInputData {
public:
  PhysXInputData(IStreamWrapper &wrapper);

  virtual uint32_t read(void *dest, uint32_t count) override;
  virtual uint32_t getLength() const override;
  virtual void seek(uint32_t offset) override;
  virtual uint32_t tell() const override;

private:
  IStreamWrapper &_stream;
};

/**
 *
 */
class PhysXOutputStream : public physx::PxOutputStream {
public:
  PhysXOutputStream(OStreamWrapper &wrapper);

  virtual uint32_t write(const void *data, uint32_t count) override;

private:
  OStreamWrapper &_stream;
};

#endif // PHYSXSTREAMS_H
