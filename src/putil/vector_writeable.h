// Filename: vector_writeable.h
// Created by:  jason (14Jun00)
// 
////////////////////////////////////////////////////////////////////

#ifndef VECTOR_WRITEABLE_H
#define VECTOR_WRITEABLE_H

#include <pandabase.h>

#include <vector>

class Writeable;

////////////////////////////////////////////////////////////////////
//       Class : vector_writeable
// Description : A vector of TypedWriteable.  This class is defined once here,
//               and exported to PANDA.DLL; other packages that want
//               to use a vector of this type (whether they need to
//               export it or not) should include this header file,
//               rather than defining the vector again.
////////////////////////////////////////////////////////////////////

EXPORT_TEMPLATE_CLASS(EXPCL_PANDA, EXPTP_PANDA, std::vector<Writeable*>)
typedef vector<Writeable*> vector_writeable;

// Tell GCC that we'll take care of the instantiation explicitly here.
#ifdef __GNUC__
#pragma interface
#endif

#endif
