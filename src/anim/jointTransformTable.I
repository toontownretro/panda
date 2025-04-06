/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file jointTransformTable.I
 * @author theclashingfritz
 * @date 2025-03-18
 */
 
 /**
 * Registers a JointTransformTable for use.  This is similar to
 * GeomVertexFormat::register_format().  Once registered, a JointTransformTable may
 * no longer be modified (although the individual VertexTransform objects may
 * modify their reported transforms).
 *
 * This must be called before a table may be used in a Geom.  After this call,
 * you should discard the original pointer you passed in (which may or may not
 * now be invalid) and let its reference count decrement normally; you should
 * use only the returned value from this point on.
 */
INLINE CPT(JointTransformTable) JointTransformTable::
register_table(const JointTransformTable *table) {
  TransformTable::register_table(table);
  return table;
}
 
/**
 *
 */
INLINE const Character *JointTransformTable::
get_character() const {
  return _char.is_valid_pointer() ? _char.p() : nullptr;
}

/**
 *
 */
INLINE void JointTransformTable::
set_joint_count(int32_t count) {
  _joint_count = count;
}

/**
 *
 */
INLINE const int32_t JointTransformTable::
get_joint_count() const {
  return _joint_count;
}