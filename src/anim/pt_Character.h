/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file pt_Character.h
 * @author drose
 * @date 2001-05-01
 */

#ifndef PT_CHARACTER_H
#define PT_CHARACTER_H

#include "pandabase.h"

#include "character.h"
#include "pointerTo.h"

/**
 * A PT(Character).  This is defined here solely we can explicitly export the
 * template class.  It's not strictly necessary, but it doesn't hurt.
 */

EXPORT_TEMPLATE_CLASS(EXPCL_PANDA_ANIM, EXPTP_PANDA_ANIM, PointerToBase<Character>)
EXPORT_TEMPLATE_CLASS(EXPCL_PANDA_ANIM, EXPTP_PANDA_ANIM, PointerTo<Character>)
EXPORT_TEMPLATE_CLASS(EXPCL_PANDA_ANIM, EXPTP_PANDA_ANIM, ConstPointerTo<Character>)

typedef PointerTo<Character> PT_Character;
typedef ConstPointerTo<Character> CPT_Character;

#endif
