/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file dynamicVisNode.h
 * @author brian
 * @date 2021-12-03
 */

#ifndef DYNAMICVISNODE_H
#define DYNAMICVISNODE_H

#include "pandabase.h"
#include "pandaNode.h"
#include "pmap.h"
#include "pset.h"
#include "transformState.h"
#include "boundingVolume.h"
#include "pointerTo.h"
#include "memoryBase.h"
#include "lightMutex.h"
#include "cycleData.h"
#include "cycleDataReader.h"
#include "cycleDataWriter.h"
#include "cycleDataStageWriter.h"
#include "pipelineCycler.h"
#include "simpleHashMap.h"

class CullTraverser;
class CullTraverserData;
class MapData;
class SpatialPartition;

/**
 * This is a special type of node that partitions its list of direct children
 * into buckets that correspond to visgroups in the map.  When this node is
 * visited during the Cull traversal, only the children in buckets of
 * visgroups that are in the PVS are traversed.
 *
 * It is used for culling dynamic entities in the game world against the
 * precomputed potentially visible set of the level, such as players and
 * projectiles.
 *
 * Note that only *direct* children of the node are grouped into the buckets.
 * If an entity parented to this node has children entities, the entire
 * hierarchy will be treated as a single unit for PVS culling.
 *
 * It is likely that a child's bounding volume spans multiple visgroups.  The
 * node ensures that each visible child is traversed once.
 */
class EXPCL_PANDA_MAP DynamicVisNode : public PandaNode {
  DECLARE_CLASS(DynamicVisNode, PandaNode);

PUBLISHED:
  DynamicVisNode(const std::string &name);
  DynamicVisNode(const DynamicVisNode &copy) = delete;

  void level_init(int num_clusters, const SpatialPartition *tree);
  void level_shutdown();

  void set_culling_enabled(bool flag);
  bool get_culling_enabled() const;

  void update_dirty_children();

public:
  virtual void child_added(PandaNode *node, int pipeline_stage) override;
  virtual void child_removed(PandaNode *node, int pipeline_stage) override;
  virtual void child_bounds_stale(PandaNode *node, int pipeline_stage) override;

  virtual bool cull_callback(CullTraverser *trav, CullTraverserData &data) override;

  class ChildInfo : public ReferenceCount {
  public:
    // This counter is used to check if we've already traversed this child if
    // the child spans multiple visgroups.
    // The node maintains its own counter that increments every time the node
    // is visited.
    int _last_trav_counter;

    bool _dirty;

    PandaNode *_node;

    // The set of visgroups the node is in.  This is only needed so we can
    // remove the node from all of the buckets its in when it gets removed.
    ov_set<int> _visgroups;
  };

private:
  typedef SimpleHashMap<ChildInfo *, std::nullptr_t, pointer_hash> ChildSet;

  // This maps visgroups to a list of children for quick iteration over the
  // children in a visgroup.
  typedef pvector<ChildSet> VisGroupChildren;

  typedef pflat_hash_map<PandaNode *, PT(ChildInfo), pointer_hash> ChildInfos;

  ChildInfos _children;
  pvector<ChildInfo *> _dirty_children;

  /**
   * We require cycling the set of nodes in each visgroup.
   *
   * It is updated by the App stage and read by the Cull stage
   * to determine the list of nodes to traverse.
   */
  class CData : public CycleData {
  public:
    CData();
    CData(const CData &copy);
    virtual CycleData *make_copy() const override;

    VisGroupChildren _visgroups;
    bool _enabled;
  };

  // To avoid copying this potentially large set for cycling,
  // we pre-allocate 2 copies and swap between them.
  //VisGroupChildren _stage_visgroups[2];
  //char _stage_cycle;

  //PipelineCycler<CData> _cycler;
  CData _cdata;
  typedef CycleDataReader<CData> CDReader;
  typedef CycleDataWriter<CData> CDWriter;
  typedef CycleDataStageWriter<CData> CDStageWriter;

  int _trav_counter;

  const SpatialPartition *_tree;

  friend class CData;

public:
  void remove_from_tree(ChildInfo *child, CData *cdata);
  void insert_into_tree(ChildInfo *child, const GeometricBoundingVolume *bounds, const SpatialPartition *tree);
};

#include "dynamicVisNode.I"

#endif // DYNAMICVISNODE_H
