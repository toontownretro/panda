// Filename: memoryInfo.cxx
// Created by:  drose (04Jun01)
//
////////////////////////////////////////////////////////////////////
//
// PANDA 3D SOFTWARE
// Copyright (c) 2001 - 2004, Disney Enterprises, Inc.  All rights reserved
//
// All use of this software is subject to the terms of the Panda 3d
// Software license.  You should have received a copy of this license
// along with this source code; you will also find a current copy of
// the license at http://etc.cmu.edu/panda3d/docs/license/ .
//
// To contact the maintainers of this program write to
// panda3d-general@lists.sourceforge.net .
//
////////////////////////////////////////////////////////////////////

#ifdef DO_MEMORY_USAGE

#include "memoryInfo.h"
#include "typedReferenceCount.h"
#include "typeHandle.h"

////////////////////////////////////////////////////////////////////
//     Function: MemoryInfo::Constructor
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
MemoryInfo::
MemoryInfo() {
  _void_ptr = (void *)NULL;
  _ref_ptr = (ReferenceCount *)NULL;
  _typed_ptr = (TypedObject *)NULL;
  _size = 0;
  _static_type = TypeHandle::none();
  _dynamic_type = TypeHandle::none();

  _flags = 0;
}

////////////////////////////////////////////////////////////////////
//     Function: MemoryInfo::get_type
//       Access: Public
//  Description: Returns the best known type, dynamic or static, of
//               the pointer.
////////////////////////////////////////////////////////////////////
TypeHandle MemoryInfo::
get_type() {
  // If we don't want to consider the dynamic type any further, use
  // what we've got.
  if ((_flags & F_reconsider_dynamic_type) == 0) {
    if (_dynamic_type == TypeHandle::none()) {
      return _static_type;
    }
    return _dynamic_type;
  }

  // Otherwise, examine the pointer again and make sure it's still the
  // best information we have.  We have to do this each time because
  // if we happen to be examining the pointer from within the
  // constructor or destructor, its dynamic type will appear to be
  // less-specific than it actually is, so our idea of what type this
  // thing is could change from time to time.
  determine_dynamic_type();

  // Now return the more specific of the two.
  TypeHandle type = _static_type;
  update_type_handle(type, _dynamic_type);

  if (type != _static_type) {
    if (express_cat.is_spam()) {
      express_cat.spam()
        << "Pointer " << (void *)_ref_ptr << " has static type "
        << _static_type << " and dynamic type " << _dynamic_type << "\n";
    }
  }

  return type;
}

////////////////////////////////////////////////////////////////////
//     Function: MemoryInfo::determine_dynamic_type
//       Access: Private
//  Description: Tries to determine the actual type of the object to
//               which this thing is pointed, if possible.
////////////////////////////////////////////////////////////////////
void MemoryInfo::
determine_dynamic_type() {
  if ((_flags & F_reconsider_dynamic_type) != 0 &&
      _static_type != TypeHandle::none()) {
    // See if we know enough now to infer the dynamic type from the
    // pointer.

    if (_typed_ptr == (TypedObject *)NULL) {
      // If our static type is known to inherit from
      // TypedReferenceCount, then we can directly downcast to get the
      // TypedObject pointer.
      if (_static_type.is_derived_from(TypedReferenceCount::get_class_type())) {
        _typed_ptr = (TypedReferenceCount *)_ref_ptr;
      }
    }

    if (_typed_ptr != (TypedObject *)NULL) {
      // If we have a TypedObject pointer, we can determine the type.
      // This might still not return the exact type, particularly if
      // we are being called within the destructor or constructor of
      // this object.
      TypeHandle got_type = _typed_ptr->get_type();

      if (got_type == TypeHandle::none()) {
        express_cat.warning()
          << "Found an unregistered type in a " << _static_type
          << " pointer:\n"
          << "Check derived types of " << _static_type
          << " and make sure that all are being initialized.\n";
        _dynamic_type = _static_type;
        _flags &= ~F_reconsider_dynamic_type;

        nassert_raise("Unregistered type.");
        return;
      }

      TypeHandle orig_type = _dynamic_type;
      if (update_type_handle(_dynamic_type, got_type)) {
        if (orig_type != _dynamic_type) {
          if (express_cat.is_spam()) {
            express_cat.spam()
              << "Updating " << (void *)_ref_ptr << " from type "
              << orig_type << " to type " << _dynamic_type << "\n";
          }
        }

      } else {
        express_cat.error()
          << "Pointer " << (void *)_ref_ptr << " previously indicated as type "
          << orig_type << " is now type " << got_type << "!\n";
      }
    }    
  }
}


////////////////////////////////////////////////////////////////////
//     Function: MemoryInfo::update_type_handle
//       Access: Private
//  Description: Updates the given destination TypeHandle with the
//               refined TypeHandle, if it is in fact more specific
//               than the original value for the destination.  Returns
//               true if the update was trouble-free, or false if the
//               two types were not apparently related.
////////////////////////////////////////////////////////////////////
bool MemoryInfo::
update_type_handle(TypeHandle &destination, TypeHandle refined) {
  if (refined == TypeHandle::none()) {
    express_cat.error()
      << "Attempt to update type of " << (void *)_ref_ptr
      << "(type is " << get_type()
      << ") to an undefined type!\n";

  } else if (destination == refined) {
    // Updating with the same type, no problem.

  } else if (destination.is_derived_from(refined)) {
    // Updating with a less-specific type, no problem.

  } else if (refined.is_derived_from(destination)) {
    // Updating with a more-specific type, no problem.
    destination = refined;

  } else {
    // Unrelated types, which might or might not be a problem.
    destination = refined;
    return false;
  }
  
  return true;
}

#endif  // DO_MEMORY_USAGE
