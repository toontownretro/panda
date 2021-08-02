/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file characterMaker.h
 * @author drose
 * @date 2002-03-06
 */

#ifndef CHARACTERMAKER_H
#define CHARACTERMAKER_H

#include "pandabase.h"

#include "vertexTransform.h"
#include "vertexSlider.h"
#include "characterNode.h"
#include "typedef.h"
#include "pmap.h"
#include "modelNode.h"

class EggNode;
class EggGroup;
class EggGroupNode;
class EggPrimitive;
class EggBin;
class Character;
class GeomNode;
class CharacterSlider;
class EggLoader;
class PandaNode;

/**
 * Converts an EggGroup hierarchy, beginning with a group with <Dart> set, to
 * a character node with joints.
 */
class EXPCL_PANDA_EGG2PG CharacterMaker {
public:
  CharacterMaker(EggGroup *root, EggLoader &loader, bool structured = false);

  CharacterNode *make_node();

  std::string get_name() const;

  int egg_to_joint(EggNode *egg_node) const;

  VertexTransform *egg_to_transform(EggNode *egg_node);

  PandaNode *part_to_node(const std::string &name) const;

  int create_slider(const std::string &name);
  VertexSlider *egg_to_vertex_slider(const std::string &name);

private:
  Character *make_bundle();
  void build_joint_hierarchy(EggNode *egg_node, int parent);
  void parent_joint_nodes();

  void make_geometry(EggNode *egg_node);

  VertexTransform *get_identity_transform();

  typedef pmap<EggNode *, int> NodeMap;
  NodeMap _slider_map;
  NodeMap _joint_map;

  typedef pmap<int, PT(ModelNode)> JointDCS;
  JointDCS _joint_dcs;

  typedef pmap<int, PT(VertexTransform) > VertexTransforms;
  VertexTransforms _vertex_transforms;
  PT(VertexTransform) _identity_transform;

  typedef pmap<std::string, PT(VertexSlider) > VertexSliders;
  VertexSliders _vertex_sliders;

  EggLoader &_loader;
  EggGroup *_egg_root;
  PT(CharacterNode) _character_node;
  Character *_bundle;

  bool _structured;

};

#endif
