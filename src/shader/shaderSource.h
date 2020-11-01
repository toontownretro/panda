/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file shaderSource.h
 * @author lachbr
 * @date 2020-10-18
 */

#ifndef SHADERSOURCE_H
#define SHADERSOURCE_H

#include "config_shader.h"
#include "referenceCount.h"
#include "pointerTo.h"
#include "filename.h"
#include "pmap.h"

/**
 * This class represents the raw source code of a shader, either loaded from
 * disk or created procedurally.
 */
class EXPCL_PANDA_SHADER ShaderSource : public ReferenceCount {
public:
  enum SourceFormat {
    SF_unknown,
    SF_file,
    SF_raw,
  };

  INLINE const std::string &get_source() const;
  INLINE const std::string &get_before_defines() const;
  INLINE const std::string &get_after_defines() const;

  INLINE SourceFormat get_format() const;

  static CPT(ShaderSource) from_filename(const Filename &filename);
  static CPT(ShaderSource) from_raw(const std::string &source);

private:
  INLINE ShaderSource();

  std::string _source;
  std::string _before_defines;
  std::string _after_defines;
  SourceFormat _format;

  typedef phash_map<Filename, CPT(ShaderSource), string_hash> SourceCache;
  typedef phash_map<std::string, CPT(ShaderSource), string_hash> RawSourceCache;
  static SourceCache _cache;
  static RawSourceCache _raw_cache;
};

#include "shaderSource.I"

#endif // SHADERSOURCE_H
