/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file pdxList.h
 * @author brian
 * @date 2021-06-10
 */

#ifndef PDXLIST_H
#define PDXLIST_H

#include "referenceCount.h"
#include "pdxValue.h"

class TokenFile;

/**
 * A list of PDXValues.
 */
class EXPCL_PANDA_PDX PDXList : public ReferenceCount {
PUBLISHED:
  INLINE void append(const PDXValue &value);
  INLINE void prepend(const PDXValue &value);
  INLINE void insert(size_t n, const PDXValue &value);
  INLINE void remove(size_t n);
  INLINE void reserve(size_t count);
  INLINE void resize(size_t count);
  INLINE size_t size() const;
  INLINE const PDXValue &get(size_t n) const;
  INLINE PDXValue &get(size_t n);

  INLINE const PDXValue &operator [] (size_t n) const;
  INLINE PDXValue &operator [] (size_t n);

  MAKE_SEQ(get_values, size, get);
  MAKE_SEQ_PROPERTY(values, size, get);

  void to_datagram(Datagram &dg) const;
  void from_datagram(DatagramIterator &scan);

public:
  typedef pvector<PDXValue> Values;

  INLINE const Values &get_values() const;
  INLINE Values &get_values();

  void write(std::ostream &out, int indent_level) const;
  bool parse(TokenFile *tokens);

private:

  Values _values;
};

#include "pdxList.I"

#endif // PDXLIST_H
