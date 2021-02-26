/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file animIKLockNode.h
 * @author lachbr
 * @date 2021-02-21
 */

#if 0
#ifndef ANIMIKLOCKNODE_H
#define ANIMIKLOCKNODE_H

#include "animGraphNode.h"
#include "ikChain.h"

/**
 * Animation graph node that prevents one or more of the character's IK chains
 * from moving after applying a pose.
 */
class EXPCL_PANDA_ANIM AnimIKLockNode final : public AnimGraphNode {
PUBLISHED:
  AnimIKLockNode(const std::string &name);

  INLINE void set_pose(AnimGraphNode *pose);

  INLINE void add_chain(IKChain *chain);

public:
  virtual void evaluate(AnimGraphEvalContext &context) override;

private:
  PT(AnimGraphNode) _pose;

  typedef pvector<PT(IKChain)> IKChains;
  IKChains _ik_chains;
};

#include "animIKLockNode.I"

#endif // ANIMIKLOCKNODE_H

#endif
