/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file ikSolver.cxx
 * @author lachbr
 * @date 2021-02-12
 */

#include "ikSolver.h"

#include "animGraphNode.h"
#include "mathutil_misc.h"

/**
 *
 */
bool IKSolver::
solve(PN_stdfloat A, PN_stdfloat B, const LPoint3 &P, const LPoint3 &D, LPoint3 &Q) {
  LPoint3 R;
  define_m(P, D);
  R = _inverse.xform_vec(P);
  PN_stdfloat r = R.length();
  PN_stdfloat d = find_d(A, B, r);
  PN_stdfloat e = find_e(A, d);
  LPoint3 S(d, e, 0);
  Q = _forward.xform_vec(S);
  return d > (r - B) && d < A;
}

/**
 * If "knee" position Q needs to be as close as possible to some point D,
 * then choose M such that M(D) is in the y>0 half of the z=0 plane.
 *
 * Given that constraint, define the forward and inverse of M as follows:
 */
void IKSolver::
define_m(const LPoint3 &P, const LPoint3 &D) {
  // Minv defines a coordinate system whose x axis contains P, so X = unit(P).
  LPoint3 X = P.normalized();

  // Its Y axis is perpendicular to P, so Y = unit(E - X(EdotX)).
  PN_stdfloat d_dot_x = D.dot(X);
  LPoint3 Y = (D - d_dot_x * X).normalized();

  // Its Z axis is perpendicular to both X and Y, so Z = cross(X,Y).
  LPoint3 Z = X.cross(Y);

  _inverse = LMatrix3(X, Y, Z);
  _forward.transpose_from(_inverse);
}

/**
 *
 */
PN_stdfloat IKSolver::
find_d(PN_stdfloat a, PN_stdfloat b, PN_stdfloat c) {
  return (c + (a * a - b * b) / c) * 0.5;
}

/**
 *
 */
PN_stdfloat IKSolver::
find_e(PN_stdfloat a, PN_stdfloat d) {
  return std::sqrt(a * a - d * d);
}

void
find_best_axis_vectors(const LVector3 &me, LVector3 &axis1, LVector3 &axis2) {
  const PN_stdfloat nx = std::abs(me[0]);
  const PN_stdfloat ny = std::abs(me[1]);
  const PN_stdfloat nz = std::abs(me[2]);

  // Find best basis vectors.
  if (nz > nx && nz > ny) {
    axis1 = LVector3::right();
  } else {
    axis1 = LVector3::up();
  }

  axis1 = (axis1 - me * (axis1.dot(me))).normalized();
  axis2 = axis1.cross(me);
}

void IKSolver::
solve_ik(const LPoint3 &root_pos, const LPoint3 &joint_pos, const LPoint3 &end_pos,
         const LPoint3 &target, const LPoint3 &effector, LPoint3 &out_joint_pos,
         LPoint3 &out_end_pos, PN_stdfloat upper_length, PN_stdfloat lower_length,
         bool allow_stretching, PN_stdfloat start_stretch_ratio,
         PN_stdfloat max_stretch_scale) {

  // This is our reach goal.
  LPoint3 desired_pos = effector;
  LVector3 desired_delta = desired_pos - root_pos;
  PN_stdfloat desired_length = desired_delta.length();

  // Find lengths of upper and lower limb in the ref skeleton.
  // Use actual sizes instead of ref skeleton, so we take into account
  // translation and scaling from other bone controllers.
  PN_stdfloat max_limb_length = lower_length + upper_length;

  // Check to handle case where desired_pos is the same as root_pos.
  LVector3 desired_dir;
  if (desired_length < 0.01f) {
    desired_length = 0.01f;
    desired_dir = LVector3::right();

  } else {
    desired_dir = desired_delta.normalized();
  }

  // Get joint target (used for defining plane that joint should be in.)
  LVector3 joint_target_delta = target - root_pos;
  const PN_stdfloat joint_target_length_sqr = joint_target_delta.length_squared();

  // Same check as above, to cover case when target position is the same as root_pos.
  LVector3 joint_plane_normal, joint_bend_dir;
  if (joint_target_length_sqr < 0.01f*0.01f) {
    joint_bend_dir = LVector3::forward();
    joint_plane_normal = LVector3::up();

  } else {
    joint_plane_normal = desired_dir.cross(joint_target_delta);

    // If we are trying to point the limb in the same direction that we are
    // supposed to displace the joint in, we have to just pick 2 random vector
    // perp to desired_dir and each other.
    if (joint_plane_normal.length_squared() < 0.01f*0.01f) {
      find_best_axis_vectors(desired_dir, joint_plane_normal, joint_bend_dir);

    } else {
      joint_plane_normal.normalize();

      // Find the final member of the reference frame by removing any component
      // of joint_target_delta along desired_dir.  This should never leave a
      // zero vector, because we've checked desired_dir and joint_target_delta
      // are not parallel.
      joint_bend_dir = joint_target_delta - (joint_target_delta.dot(desired_dir) * desired_dir);
      joint_bend_dir.normalize();
    }
  }

  if (allow_stretching) {
    const PN_stdfloat scale_range = max_stretch_scale - start_stretch_ratio;
    if (scale_range > 0.01f && max_limb_length > 0.01f) {
      const PN_stdfloat reach_ratio = desired_length / max_limb_length;
      const PN_stdfloat scaling_factor = (max_stretch_scale - 1.0f) * std::clamp(
        (reach_ratio - start_stretch_ratio) / scale_range, 0.0f, 1.0f);

      if (scaling_factor > 0.01f) {
        lower_length *= (1.0f + scaling_factor);
        upper_length *= (1.0f + scaling_factor);
        max_limb_length *= (1.0f + scaling_factor);
      }
    }
  }

  out_end_pos = desired_pos;
  out_joint_pos = joint_pos;

  // If we are trying to reach a goal beyond the length of the limb, clamp it
  // to something solvable and extend limb fully.
  if (desired_length >= max_limb_length) {
    out_end_pos = root_pos + (max_limb_length * desired_dir);
    out_joint_pos = root_pos + (upper_length * desired_dir);

  } else {
    // So we have a triangle we know the side lengths of.  We can work out the
    // angle between desired_dir and the direction of the upper limb using the
    // sin rule:
    const PN_stdfloat two_ab = 2.0f * upper_length * desired_length;

    const PN_stdfloat cos_angle = (two_ab != 0.0f) ?
      ((upper_length * upper_length) + (desired_length * desired_length) - (lower_length * lower_length)) / two_ab
      : 0.0f;

    // If cos_angle is less than 0, the upper arm actually points the opposite
    // way to desired_dir, so we handle that.
    const bool reverse_upper_bone = (cos_angle < 0.0f);

    // Angle between upper limb and desired dir.
    const PN_stdfloat angle = std::acos(cos_angle);

    // Now we calculate the distance of the joint from the root -> effector
    // line.  This forms a right-angle triangle, with the upper limb as the
    // hypotenuse.
    const PN_stdfloat joint_line_dist = upper_length * std::sin(angle);

    // And the final side of that triangle - distance along desired_dir of
    // perpendicular.  proj_joint_dist_sqr can't be neg, because
    // joint_line_dist must be <= upper_length because sin(angle) is <= 1.
    const PN_stdfloat proj_joint_dist_sqr = (upper_length * upper_length) - (joint_line_dist * joint_line_dist);
    PN_stdfloat proj_joint_dist = (proj_joint_dist_sqr > 0.0f) ? std::sqrt(proj_joint_dist_sqr) : 0.0f;
    if (reverse_upper_bone) {
      proj_joint_dist *= -1.0f;
    }

    // So now we can work out where to put the joint!
    out_joint_pos = root_pos + (proj_joint_dist * desired_dir) + (joint_line_dist * joint_bend_dir);
  }
}

void IKSolver::
solve_ik(JointTransform &root, JointTransform &joint, JointTransform &end,
         const LPoint3 &target, const LPoint3 &effector, PN_stdfloat upper_length,
         PN_stdfloat lower_length, bool allow_stretching,
         PN_stdfloat start_stretch_ratio, PN_stdfloat max_stretch_scale) {
  LPoint3 out_joint_pos, out_end_pos;

  LPoint3 root_pos = root._position;
  LPoint3 joint_pos = joint._position;
  LPoint3 end_pos = end._position;

  // IK solver.
  solve_ik(root_pos, joint_pos, end_pos, target, effector, out_joint_pos, out_end_pos,
           upper_length, lower_length, allow_stretching, start_stretch_ratio,
           max_stretch_scale);

  // Update transform for upper joint.
  {
    // Get difference in direction for old and new joint orientations.
    LVector3 old_dir = (joint_pos - root_pos).normalized();
    LVector3 new_dir = (out_joint_pos - root_pos).normalized();
    // Find delta rotation, takes us from old to new dir.
    LQuaternion delta_rotation = LQuaternion::find_between_normals(old_dir, new_dir);
    // Rotate our joint quaternion by this delta rotation.
    root._rotation = delta_rotation * root._rotation;
    // And put the joint where it should be.
    root._position = root_pos;
  }

  // Update transform for middle joint.
  {
    // Get difference in direction for old and new joint orientations.
    LVector3 old_dir = (end_pos - joint_pos).normalized();
    LVector3 new_dir = (out_end_pos - out_joint_pos).normalized();
    // Find delta rotation, takes use from old to new dir.
    LQuaternion delta_rotation = LQuaternion::find_between_normals(old_dir, new_dir);
    // Rotate our joint quaternion by this delta rotation.
    joint._rotation = delta_rotation * joint._rotation;
    // And put the joint where it should be.
    joint._position = out_joint_pos;
  }

  // Update transform for end joint.
  // Currently not doing anything to rotation.
  // Keeping input rotation.
  // Set correct location for end joint.
  end._position = out_end_pos;
}

void IKSolver::
solve_ik(const LPoint3 &root, const LPoint3 &joint, const LPoint3 &end, const LPoint3 &target,
         const LPoint3 &effector, LPoint3 &out_joint_pos, LPoint3 &out_end_pos,
         bool allow_stretching, PN_stdfloat start_stretch_ratio, PN_stdfloat max_stretch_scale) {
  PN_stdfloat lower_length = (end - joint).length();
  PN_stdfloat upper_length = (joint - root).length();
  solve_ik(root, joint, end, target, effector, out_joint_pos, out_end_pos,
           upper_length, lower_length, allow_stretching, start_stretch_ratio,
           max_stretch_scale);
}

void IKSolver::
solve_ik(JointTransform &root, JointTransform &joint, JointTransform &end,
         const LPoint3 &target, const LPoint3 &effector, bool allow_stretching,
         PN_stdfloat start_stretch_ratio, PN_stdfloat max_stretch_scale) {
  PN_stdfloat lower_length = (end._position - joint._position).length();
  PN_stdfloat upper_length = (joint._position - root._position).length();
  solve_ik(root, joint, end, target, effector, upper_length, lower_length, allow_stretching,
           start_stretch_ratio, max_stretch_scale);
}
