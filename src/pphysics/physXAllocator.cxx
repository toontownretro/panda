/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file physXAllocator.cxx
 * @author brian
 * @date 2021-04-13
 */

#include "config_pphysics.h"
#include "physXAllocator.h"
#include "dtoolbase.h"
#include "memoryHook.h"
#include "memoryUsage.h"

IMPLEMENT_CLASS(PhysXAllocator);

/**
 * Allocates some memory for PhysX.  Calls into Panda's allocator.
 */
void *PhysXAllocator::
allocate(size_t size, const char *type_name, const char *filename, int line) {
  //std::cerr << "Alloc physx: " << size << ", " << std::string(type_name) << "\n";
  void *ptr = get_class_type().allocate_array(size);
#ifndef NDEBUG
  if (ptr == nullptr) {
    pphysics_cat.error()
      << "Failed to allocate " << size << " bytes for PhysX object "
      << std::string(type_name) << "! (Requested from "
      << std::string(filename) << " at line " << line << ".)\n";
  }
#endif
  return ptr;
}

/**
 * Deallocates some PhysX memory.  Calls into Panda's allocator.
 */
void PhysXAllocator::
deallocate(void *ptr) {
  get_class_type().deallocate_array(ptr);
}
