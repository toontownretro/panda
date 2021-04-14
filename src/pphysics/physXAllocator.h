/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file physXAllocator.h
 * @author brian
 * @date 2021-04-13
 */

#ifndef PHYSXALLOCATOR_H
#define PHYSXALLOCATOR_H

#include "foundation/PxAllocatorCallback.h"

/**
 * Allocator implementation for PhysX usage.  Calls into Panda's allocator.
 */
class PhysXAllocator : public physx::PxAllocatorCallback {
public:
  virtual void *allocate(size_t size, const char *type_name, const char *filename, int line) override;
  virtual void deallocate(void *ptr) override;
};

#endif // PHYSXALLOCATOR_H
