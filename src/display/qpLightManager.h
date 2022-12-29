/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file qpLightManager.h
 * @author brian
 * @date 2022-12-21
 */

#ifndef QPLIGHTMANAGER_H
#define QPLIGHTMANAGER_H

#include "pandabase.h"
#include "referenceCount.h"
#include "pvector.h"
#include "lens.h"
#include "pointerTo.h"
#include "vector_int.h"
#include "nodePath.h"
#include "camera.h"
#include "texture.h"
#include "qpLight.h"
#include "ordered_vector.h"
#include "cycleData.h"
#include "pipelineCycler.h"
#include "cycleDataReader.h"
#include "cycleDataWriter.h"

/**
 *
 */
class EXPCL_PANDA_DISPLAY qpLightManager : public ReferenceCount {
PUBLISHED:
  qpLightManager();

  void initialize();

  void add_static_light(qpLight *light);
  void clear_static_lights();

  void add_dynamic_light(qpLight *light);
  void remove_dynamic_light(qpLight *light);
  void clear_dynamic_lights();

  void update();

  INLINE Texture *get_static_light_buffer() const { return _static_light_buffer; }
  INLINE Texture *get_dynamic_light_buffer() const;

  INLINE int get_num_static_lights() const { return (int)_static_lights.size(); }
  INLINE qpLight *get_static_light(int n) const { return _static_lights[n]; }

  INLINE int get_num_dynamic_lights() const { return (int)_dynamic_lights.size(); }
  INLINE qpLight *get_dynamic_light(int n) const { return _dynamic_lights[n]; }

public:
  void update_light_buffer(Texture *buffer, PT(qpLight) *lights, int num_lights);

private:
  PT(Texture) _static_light_buffer;

  static constexpr int num_buffers = 2;
  PT(Texture) _dynamic_light_buffers[num_buffers];
  unsigned char _dynamic_buffer_index;

  class CData : public CycleData {
  public:
    CData();
    CData(const CData &copy);
    virtual CycleData *make_copy() const override;

    Texture *_dynamic_light_buffer;
  };
  PipelineCycler<CData> _cycler;
  typedef CycleDataReader<CData> CDReader;
  typedef CycleDataWriter<CData> CDWriter;

  typedef pvector<PT(qpLight)> LightList;
  typedef ov_set<PT(qpLight)> LightSet;

  LightList _static_lights;
  LightSet _dynamic_lights;
};

#include "qpLightManager.I"

#endif // QPLIGHTMANAGER_H
