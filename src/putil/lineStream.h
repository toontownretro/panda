// Filename: lineStream.h
// Created by:  drose (26Feb00)
// 
////////////////////////////////////////////////////////////////////

#ifndef LINESTREAM_H
#define LINESTREAM_H

#include <pandabase.h>

#include "lineStreamBuf.h"

////////////////////////////////////////////////////////////////////
//       Class : LineStream
// Description : This is a special ostream that writes to a memory
//               buffer, like ostrstream.  However, its contents can
//               be continuously extracted as a sequence of lines of
//               text.
//
//               Unlike ostrstream, which can only be extracted from
//               once (and then the buffer freezes and it can no
//               longer be written to), the LineStream is not
//               otherwise affected when a line of text is extracted.
//               More text can still be written to it and continuously
//               extracted.
////////////////////////////////////////////////////////////////////
class EXPCL_PANDA LineStream : public ostream {
public:
  INLINE LineStream();

  INLINE bool is_text_available() const;
  INLINE string get_line();
  INLINE bool has_newline() const;

private:
  LineStreamBuf _lsb;
};

#include "lineStream.I"

#endif
