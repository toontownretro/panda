// Filename: pointerTo.h
// Created by:  drose (23Oct98)
//
////////////////////////////////////////////////////////////////////

#ifndef POINTERTO_H
#define POINTERTO_H

////////////////////////////////////////////////////////////////////
//
// This file defines the classes PointerTo and ConstPointerTo (and
// their abbreviations, PT and CPT).  These should be used in place of
// traditional C-style pointers wherever implicit reference counting
// is desired.
//
// The syntax is:                     instead of:
//
//    PointerTo<MyClass> p;            MyClass *p;
//    PT(MyClass) p;
//
//    ConstPointerTo<MyClass> p;       const MyClass *p;
//    CPT(MyClass) p;
//
// PointerTo and ConstPointerTo will automatically increment the
// object's reference count while the pointer is kept.  When the
// PointerTo object is reassigned or goes out of scope, the reference
// count is automatically decremented.  If the reference count reaches
// zero, the object is freed.
//
// Note that const PointerTo<MyClass> is different from
// ConstPointerTo<MyClass>.  A const PointerTo may not reassign its
// pointer, but it may still modify the contents at that address.  On
// the other hand, a ConstPointerTo may reassign its pointer at will,
// but may not modify the contents.  It is like the difference between
// (MyClass * const) and (const MyClass *).
//
// In order to use PointerTo, it is necessary that the thing pointed
// to--MyClass in the above example--either inherits from
// ReferenceCount, or is a proxy built with RefCountProxy or
// RefCountObj (see referenceCount.h).  However, also see
// PointerToArray, which does not have this restriction.
//
// It is crucial that the PointerTo object is only used to refer to
// objects allocated from the free store, for which delete is a
// sensible thing to do.  If you assign a PointerTo to an automatic
// variable (allocated from the stack, for instance), bad things will
// certainly happen when the reference count reaches zero and it tries
// to delete it.
//
// It's also important to remember that, as always, a virtual
// destructor is required if you plan to support polymorphism.  That
// is, if you define a PointerTo to some base type, and assign to it
// instances of a class derived from that base class, the base class
// must have a virtual destructor in order to properly destruct the
// derived object when it is deleted.
//
////////////////////////////////////////////////////////////////////

#include <pandabase.h>

#include "referenceCount.h"
#include "typedef.h"
#include "memoryUsage.h"
#include "config_express.h"


////////////////////////////////////////////////////////////////////
// 	 Class : PointerToBase
// Description : This is the base class for PointerTo and
//               ConstPointerTo.  Don't try to use it directly; use
//               either derived class instead.
////////////////////////////////////////////////////////////////////
template <class T>
class PointerToBase {
public:
  typedef T To;

protected:
  INLINE PointerToBase(To *ptr);
  INLINE PointerToBase(const PointerToBase<T> &copy);
  INLINE ~PointerToBase();

  void reassign(To *ptr);

  INLINE void reassign(const PointerToBase<To> &copy);

  To *_ptr;

  // No assignment or retrieval functions are declared in
  // PointerToBase, because we will have to specialize on const
  // vs. non-const later.

public:
  // These comparison functions are common to all things PointerTo, so
  // they're defined up here.
#ifndef CPPPARSER
#ifndef WIN32_VC
  INLINE bool operator == (const To *other) const;
  INLINE bool operator != (const To *other) const;
  INLINE bool operator > (const To *other) const;
  INLINE bool operator <= (const To *other) const;
  INLINE bool operator >= (const To *other) const;

  INLINE bool operator == (const PointerToBase<To> &other) const;
  INLINE bool operator != (const PointerToBase<To> &other) const;
  INLINE bool operator > (const PointerToBase<To> &other) const;
  INLINE bool operator <= (const PointerToBase<To> &other) const;
  INLINE bool operator >= (const PointerToBase<To> &other) const;
#endif  // WIN32_VC
  INLINE bool operator < (const To *other) const;
  INLINE bool operator < (const PointerToBase<To> &other) const;
#endif  // CPPPARSER

  INLINE bool is_null() const;
  INLINE void clear();

  void output(ostream &out) const;
};

template<class T>
INLINE ostream &operator <<(ostream &out, const PointerToBase<T> &pointer) {
  pointer.output(out);
  return out;
}

////////////////////////////////////////////////////////////////////
// 	 Class : PointerTo
// Description : PointerTo is a template class which implements a
//               smart pointer to an object derived from
//               ReferenceCount.
////////////////////////////////////////////////////////////////////
template <class T>
class PointerTo : public PointerToBase<T> {
public:
  INLINE PointerTo(To *ptr = (To *)NULL);
  INLINE PointerTo(const PointerTo<T> &copy);

  INLINE To &operator *() const;
  INLINE To *operator -> () const;
  INLINE operator PointerToBase<T>::To *() const;

  // When downcasting to a derived class from a PointerTo<BaseClass>,
  // C++ would normally require you to cast twice: once to an actual
  // BaseClass pointer, and then again to your desired pointer.  You
  // can use the handy function p() to avoid this first cast and make
  // your code look a bit cleaner.

  // e.g. instead of (MyType *)(BaseClass *)ptr, use (MyType *)ptr.p()
 
  // If your base class is a derivative of TypedObject, you might want
  // to use the DCAST macro defined in typeHandle.h instead,
  // e.g. DCAST(MyType, ptr).  This provides a clean downcast that
  // doesn't require .p() or any double-casting, and it can be
  // run-time checked for correctness.
  INLINE To *p() const;

  INLINE PointerTo<T> &operator = (To *ptr);
  INLINE PointerTo<T> &operator = (const PointerTo<T> &copy);

  // These functions normally wouldn't need to be redefined here, but
  // we do so anyway just to help out interrogate (which doesn't seem
  // to want to automatically export the PointerToBase class).  When
  // this works again in interrogate, we can remove these.
  INLINE bool is_null() const { return PointerToBase<T>::is_null(); }
  INLINE void clear() { PointerToBase<T>::clear(); }
};


////////////////////////////////////////////////////////////////////
// 	 Class : ConstPointerTo
// Description : A ConstPointerTo is similar to a PointerTo, except it
//               keeps a const pointer to the thing.
//
//               (Actually, it keeps a non-const pointer, because it
//               must be allowed to adjust the reference counts, and
//               it must be able to delete it when the reference count
//               goes to zero.  But it presents only a const pointer
//               to the outside world.)
//
//               Notice that a PointerTo may be assigned to a
//               ConstPointerTo, but a ConstPointerTo may not be
//               assigned to a PointerTo.
////////////////////////////////////////////////////////////////////
template <class T>
class ConstPointerTo : public PointerToBase<T> {
public:
  INLINE ConstPointerTo(const To *ptr = (const To *)NULL);
  INLINE ConstPointerTo(const PointerTo<T> &copy);
  INLINE ConstPointerTo(const ConstPointerTo<T> &copy);

  INLINE const To &operator *() const;
  INLINE const To *operator -> () const;
  INLINE operator const PointerToBase<T>::To *() const;

  INLINE const To *p() const;

  INLINE ConstPointerTo<T> &operator = (const To *ptr);
  INLINE ConstPointerTo<T> &operator = (const ConstPointerTo<T> &copy);
  INLINE ConstPointerTo<T> &operator = (const PointerTo<T> &copy);

  // These functions normally wouldn't need to be redefined here, but
  // we do so anyway just to help out interrogate (which doesn't seem
  // to want to automatically export the PointerToBase class).  When
  // this works again in interrogate, we can remove these.
  INLINE bool is_null() const { return PointerToBase<T>::is_null(); }
  INLINE void clear() { PointerToBase<T>::clear(); }
};


// Finally, we'll define a couple of handy abbreviations to save on
// all that wasted typing time.

#define PT(type) PointerTo< type >
#define CPT(type) ConstPointerTo< type >

#include "pointerTo.I"

#endif
