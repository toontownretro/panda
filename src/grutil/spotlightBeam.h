/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file spotlightBeam.h
 * @author brian
 * @date 2022-09-28
 */

#ifndef SPOTLIGHTBEAM_H
#define SPOTLIGHTBEAM_H

#include "pandabase.h"
#include "pandaNode.h"
#include "luse.h"
#include "spriteGlow.h"

class Material;

/**
 * A node that modulates color scale based on the dot product between the
 * node's position and the camera.
 */
class EXPCL_PANDA_GRUTIL SpotlightBeam : public PandaNode {
  DECLARE_CLASS(SpotlightBeam, PandaNode);

PUBLISHED:
  SpotlightBeam(const std::string &name);

  INLINE void set_beam_color(const LColor &color);
  INLINE LColor get_beam_color() const;
  MAKE_PROPERTY(beam_color, get_beam_color, set_beam_color);

  INLINE void set_beam_size(PN_stdfloat length, PN_stdfloat width);
  INLINE PN_stdfloat get_beam_length() const;
  INLINE PN_stdfloat get_beam_width() const;

  INLINE void set_halo_color(const LColor &color);
  INLINE LColor get_halo_color() const;

  INLINE void set_halo_size(PN_stdfloat size);
  INLINE PN_stdfloat get_halo_size() const;

  //INLINE void set_beam_material(Material *mat);
  //INLINE Material *get_beam_material() const;

  //INLINE void set_halo_material(Material *mat);
  //INLINE Material *get_halo_material() const;

public:
  virtual bool cull_callback(CullTraverser *trav, CullTraverserData &data) override;
  virtual void add_for_draw(CullTraverser *trav, CullTraverserData &data) override;

private:
  LColor _beam_color;
  LColor _halo_color;
  PN_stdfloat _halo_size;

  // Renderable geometry for the beam and halo.
  //PT(PandaNode) _beam_node;
  //PT(PandaNode) _halo_node;

  // This issues occlusion queries to determine the halo visibility.
  PT(SpriteGlow) _halo_query;

  PN_stdfloat _beam_width, _beam_length;
};

#include "spotlightBeam.I"

#endif // SPOTLIGHTBEAM_H
