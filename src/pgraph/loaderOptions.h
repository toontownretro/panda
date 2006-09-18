// Filename: loaderOptions.h
// Created by:  drose (05Oct05)
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

#ifndef LOADEROPTIONS_H
#define LOADEROPTIONS_H

#include "pandabase.h"

////////////////////////////////////////////////////////////////////
//       Class : LoaderOptions
// Description : Specifies parameters that may be passed to the
//               loader.
////////////////////////////////////////////////////////////////////
class EXPCL_PANDA LoaderOptions {
PUBLISHED:
  // At the moment, we only have this one set of flags.  Maybe one day
  // there will be more options.
  enum LoaderFlags {
    LF_search            = 0x0001,
    LF_report_errors     = 0x0002,
    LF_convert_skeleton  = 0x0004,
    LF_convert_channels  = 0x0008,
    LF_convert_anim      = 0x000c,  // skeleton + channels
    LF_no_disk_cache     = 0x0010,  // disallow BamCache
    LF_no_ram_cache      = 0x0020,  // disallow ModelPool
    LF_no_cache          = 0x0030,  // no_disk + no_ram
  };

  INLINE LoaderOptions(int flags = LF_search | LF_report_errors);
  INLINE LoaderOptions(const LoaderOptions &copy);
  INLINE void operator = (const LoaderOptions &copy);

  INLINE void set_flags(int flags);
  INLINE int get_flags() const;

  INLINE bool get_allow_disk_cache() const;
  INLINE bool get_allow_ram_cache() const;

  void output(ostream &out) const;

private:
  void write_flag(ostream &out, string &sep, 
                  const string &flag_name, int flag) const;
  int _flags;
};

INLINE ostream &operator << (ostream &out, const LoaderOptions &opts) {
  opts.output(out);
  return out;
}

#include "loaderOptions.I"

#endif
