/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file collisionHandlerFluidPusher.cxx
 * @author drose
 * @date 2002-03-16
 */

#include "collisionHandlerFluidPusher.h"
#include "collisionNode.h"
#include "collisionEntry.h"
#include "collisionPolygon.h"
#include "collisionSphere.h"
#include "config_collide.h"
#include "dcast.h"

TypeHandle CollisionHandlerFluidPusher::_type_handle;

/**
 *
 */
CollisionHandlerFluidPusher::
CollisionHandlerFluidPusher() {
  _wants_all_potential_collidees = true;
}

/**
 * Called between a begin_group() .. end_group() sequence for each collision
 * that is detected.
 */
void CollisionHandlerFluidPusher::
add_entry(CollisionEntry *entry) {
  nassertv(entry != nullptr);
  // skip over CollisionHandlerPhysical::add_entry, since it filters out
  // collidees by orientation; our collider can change direction mid-frame, so
  // it may collide with something that would have been filtered out
  CollisionHandlerEvent::add_entry(entry);

  // filter out non-tangibles
  if (entry->get_from()->is_tangible() &&
      (!entry->has_into() || entry->get_into()->is_tangible())) {

    _from_entries[entry->get_from_node_path()].push_back(entry);
    if (entry->collided()) {
      _has_contact = true;
    }
  }
}

/**
 * Calculates a reasonable final position for a collider given a set of
 * collidees
 */
bool CollisionHandlerFluidPusher::
handle_entries() {
  /*
    This pusher repeatedly calculates the earliest collision, calculates a new
    trajectory based on that collision, and repeats until the original motion is
    exhausted or the collider becomes "stuck". This solves the "acute collisions"
    problem where colliders could bounce their way through to the other side
    of a wall.

    Pseudocode:

    INPUTS
    PosA = collider's previous position
    PosB = collider's current position
    M = movement vector (PosB - PosA)
    BV = bounding sphere that includes collider at PosA and PosB
    CS = 'collision set', all 'collidables' within BV (collision polys, capsules, etc)

    **** need to track a 'composite collision plane' at each vetted collision event that
    represents the appropriate deflection given M and what's been collided with
    no angle of rotation should ever increase relative to plane perpendicular to M
    2D case is easy: angle relative to perpendicular should never move farther from zero, or cross zero

    VARIABLES
    N = movement vector since most recent collision (or start of frame)
	P = set of collision planes that have already been collided against
    SCS = 'sub collision set', all collidables that could still be collided with
    C = single collider currently being collided with
    PosX = new position given movement along N interrupted by collision with C

    OUTPUTS
    final position is PosX

    1. N = M, SCS = CS, PosX = PosB
    2. compute, using SCS and N, which collidable C is the first collision
    3. if no collision found, DONE
    4. if movement in direction M is now completely  blocked, then
       PosX = initial point of contact with C along N, DONE
    5. calculate PosX (and new N) assuming that there will be no more collisions
    6. calculate new CP based on current CP and C's collision plane
    7. clip PosX/N against CP
    8. add C's collision plane to P
    9. remove C from SCS (assumes that you can't collide against a solid more than once per frame)
    10. go to 2
  */
  bool okflag = true;

  // if all we got was potential collisions, don't bother
  if (!_has_contact) {
    return okflag;
  }

  // for every fluid mover being pushed...
  FromEntries::iterator fei;
  for (fei = _from_entries.begin(); fei != _from_entries.end(); ++fei) {
    NodePath from_node_path = fei->first;
    Entries *orig_entries = &fei->second;

    Colliders::iterator ci;
    ci = _colliders.find(from_node_path);
    if (ci == _colliders.end()) {
      // Hmm, someone added a CollisionNode to a traverser and gave it this
      // CollisionHandler pointer--but they didn't tell us about the node.
      collide_cat.error()
        << "CollisionHandlerFluidPusher doesn't know about "
        << from_node_path << ", disabling.\n";
      okflag = false;
    } else {
      ColliderDef &def = (*ci).second;

      // we do our math in this node's space
      NodePath wrt_node(*_root);

      // extract the collision entries into a vector that we can safely modify
      Entries entries(*orig_entries);

      // this is the original position delta for the entire frame, before
      // collision response
      LVector3 M(from_node_path.get_pos_delta(wrt_node));
      // this is used to track position deltas every time we collide against a
      // solid
      LVector3 N(M);

      const LPoint3 orig_pos(from_node_path.get_pos(wrt_node));
      CPT(TransformState) prev_trans(from_node_path.get_prev_transform(wrt_node));
      const LPoint3 orig_prev_pos(prev_trans->get_pos());

      // currently we only support spheres as the collider
      const CollisionSphere *sphere;
      DCAST_INTO_R(sphere, entries.front()->get_from(), false);

      from_node_path.set_pos(wrt_node, 0,0,0);
      LPoint3 sphere_offset = (sphere->get_center() *
                                from_node_path.get_transform(wrt_node)->get_mat());
      // TODO: what if scale is non-uniform?
      const float sphere_radius = sphere->get_radius() * from_node_path.get_scale(wrt_node)[0];
      from_node_path.set_pos(wrt_node, orig_pos);

      // this will hold the final calculated position at each iteration
      LPoint3 candidate_final_pos(orig_pos);
      // this holds the position before reacting to collisions
      LPoint3 uncollided_pos(candidate_final_pos);

      // unit vector facing back into original direction of motion
      LVector3 reverse_vec(-M);
      reverse_vec.normalize();

      // unit vector pointing out to the right relative to the direction of
      // motion, looking into the direction of motion
      //const LVector3 right_unit(LVector3::up().cross(reverse_vec));

      int numCollisions = 0;

      PlaneList P;

      // iterate until the mover runs out of movement or gets stuck
      while (true) {
        const CollisionEntry *C = nullptr;
        // find the first (earliest) collision
        Entries::const_iterator cei;
        for (cei = entries.begin(); cei != entries.end(); ++cei) {
          const CollisionEntry *entry = (*cei);
          nassertr(entry != nullptr, false);
          if (entry->collided() && ((C == nullptr) || (entry->get_t() < C->get_t()))) {
            nassertr(from_node_path == entry->get_from_node_path(), false);
            C = entry;
          }
        }

        // if no collisions, we're done
        if (C == nullptr) {
          break;
        }


        numCollisions++;

        // move back to initial contact position
        LPoint3 contact_pos;
        LVector3 contact_normal;

        if (!C->get_all_contact_info(wrt_node, contact_pos, contact_normal)) {
          collide_cat.warning()
            << "Cannot shove on " << from_node_path << " for collision into "
            << C->get_into_node_path() << "; no contact pos/normal information.\n";
          break;
        }
        // calculate the position of the target node at the point of contact
        contact_pos -= sphere_offset;

        uncollided_pos = candidate_final_pos;
        candidate_final_pos = contact_pos;

        LVector3 proj_surface_normal(contact_normal);

        LVector3 norm_proj_surface_normal(proj_surface_normal);
        norm_proj_surface_normal.normalize();

        LVector3 blocked_movement(uncollided_pos - contact_pos);

        PN_stdfloat push_magnitude(-blocked_movement.dot(proj_surface_normal));
        if (push_magnitude < 0.0f) {
          // don't ever push into plane
          candidate_final_pos = contact_pos;
        } else {
          // calculate new position given that you collided with this thing
          // project the final position onto the plane of the obstruction
          candidate_final_pos = uncollided_pos + (norm_proj_surface_normal * push_magnitude);
          LPoint3f fp(uncollided_pos + (norm_proj_surface_normal * push_magnitude));
          // make sure we don't push through any of the established collision planes
          // this should be more clever, this will probably make the collider 'stick' instead of sliding
          PlaneList::iterator pli = P.begin();
          while (pli != P.end()) {
            float t;
            LVector3f delta(fp - orig_pos);
            if ((*pli).intersects_line(t, orig_pos, delta)) {
              fp = orig_pos + (t * delta);
            }
            pli++;
          }
          candidate_final_pos = fp;
        }

        from_node_path.set_pos(wrt_node, candidate_final_pos);
        CPT(TransformState) prev_trans(from_node_path.get_prev_transform(wrt_node));
        prev_trans = prev_trans->set_pos(contact_pos);
        from_node_path.set_prev_transform(wrt_node, prev_trans);

        /*{
          const LPoint3 new_pos(from_node_path.get_pos(wrt_node));
          CPT(TransformState) new_prev_trans(from_node_path.get_prev_transform(wrt_node));
          const LPoint3 new_prev_pos(new_prev_trans->get_pos());
        }*/

        // recalculate the position delta
        N = from_node_path.get_pos_delta(wrt_node);

        LPoint3f norm_contact_normal(contact_normal);
        norm_contact_normal.normalize();
        P.push_back(LPlanef(contact_normal, contact_pos + (norm_contact_normal * sphere_radius)));

        // calculate new collisions given new movement vector
        Entries::iterator ei;
        Entries new_entries;
        for (ei = entries.begin(); ei != entries.end(); ++ei) {
          CollisionEntry *entry = (*ei);
          nassertr(entry != nullptr, false);
          // skip the one we just collided against
          if (entry != C) {
            entry->_from_node_path = from_node_path;
            entry->reset_collided();
            PT(CollisionEntry) result = entry->get_from()->test_intersection(**ei);
            if (result != nullptr && result != nullptr) {
              new_entries.push_back(result);
            }
          }
        }
        entries.swap(new_entries);
      }

      // put things back where they were
      from_node_path.set_pos(wrt_node, orig_pos);
      // restore the appropriate previous position
      prev_trans = from_node_path.get_prev_transform(wrt_node);
      prev_trans = prev_trans->set_pos(orig_prev_pos);
      from_node_path.set_prev_transform(wrt_node, prev_trans);

      LVector3 net_shove(candidate_final_pos - orig_pos);
      LVector3 force_normal(net_shove);
      force_normal.normalize();

      // This is the part where the node actually gets moved:
      def._target.set_pos(wrt_node, candidate_final_pos);

      // We call this to allow derived classes to do other fix-ups as they see
      // fit:
      apply_net_shove(def, net_shove, force_normal);
      apply_linear_force(def, force_normal);
    }
  }

  return okflag;
}
