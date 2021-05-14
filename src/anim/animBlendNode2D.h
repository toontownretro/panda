/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file animBlendNode2D.h
 * @author lachbr
 * @date 2021-02-18
 */

#ifndef ANIMBLENDNODE2D_H
#define ANIMBLENDNODE2D_H

#include "animGraphNode.h"
#include "vector_stdfloat.h"
#include "poseParameter.h"

/**
 * Animation graph node that assigns each input node to a 2D location on a
 * grid, and uses an input coordinate to blend between the 3 closest input
 * nodes.
 */
class EXPCL_PANDA_ANIM AnimBlendNode2D final : public AnimGraphNode {
PUBLISHED:
  AnimBlendNode2D(const std::string &name);

  void build_triangles();

  INLINE void add_input(AnimGraphNode *input, const LPoint2 &point);
  INLINE AnimGraphNode *get_input_node(int n) const;
  INLINE LPoint2 get_input_point(int n) const;

  void compute_weights();

  INLINE void set_input_x(PoseParameter *param);
  INLINE void set_input_y(PoseParameter *param);

public:
  virtual void evaluate(AnimGraphEvalContext &context) override;

private:
  void blend_triangle(const LPoint2 &a, const LPoint2 &b, const LPoint2 &c,
                      const LPoint2 &point, PN_stdfloat *weights);
  bool point_in_triangle(const LPoint2 &a, const LPoint2 &b,
                         const LPoint2 &c, const LPoint2 &point) const;
  PN_stdfloat triangle_sign(const LPoint2 &a, const LPoint2 &b,
                            const LPoint2 &c) const;
  LPoint2 closest_point_to_segment(const LPoint2 &point, const LPoint2 &a,
                                   const LPoint2 &b) const;

private:
  class Triangle {
  public:
    // Control point indices.
    int a;
    int b;
    int c;
  };
  typedef pvector<Triangle> Triangles;
  Triangles _triangles;

  class Input {
  public:
    PT(AnimGraphNode) _node;
    LPoint2 _point;
    PN_stdfloat _weight;
  };
  typedef pvector<Input> Inputs;
  Inputs _inputs;

  Triangle *_active_tri;

  PT(PoseParameter) _x_param;
  PT(PoseParameter) _y_param;
  LPoint2 _input_coord;

  bool _has_triangles;

public:
  virtual TypeHandle get_type() const {
    return get_class_type();
  }
  virtual TypeHandle force_init_type() {init_type(); return get_class_type();}

  static TypeHandle get_class_type() {
    return _type_handle;
  }
  static void init_type() {
    AnimGraphNode::init_type();
    register_type(_type_handle, "AnimBlendNode2D",
                  AnimGraphNode::get_class_type());
  }

private:
  static TypeHandle _type_handle;
};

#include "animBlendNode2D.I"

#endif // ANIMBLENDNODE2D_H
