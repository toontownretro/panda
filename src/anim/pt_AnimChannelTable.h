/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file pt_AnimChannelTable.h
 * @author drose
 * @date 2001-05-01
 */

#ifndef PT_ANIMCHANNELTABLE_H
#define PT_ANIMCHANNELTABLE_H

#include "pandabase.h"

#include "animChannelTable.h"
#include "pointerTo.h"

/**
 * A PT(AnimChannelTable).  This is defined here solely we can explicitly export the
 * template class.  It's not strictly necessary, but it doesn't hurt.
 */

EXPORT_TEMPLATE_CLASS(EXPCL_PANDA_ANIM, EXPTP_PANDA_ANIM, PointerToBase<AnimChannelTable>)
EXPORT_TEMPLATE_CLASS(EXPCL_PANDA_ANIM, EXPTP_PANDA_ANIM, PointerTo<AnimChannelTable>)
EXPORT_TEMPLATE_CLASS(EXPCL_PANDA_ANIM, EXPTP_PANDA_ANIM, ConstPointerTo<AnimChannelTable>)

typedef PointerTo<AnimChannelTable> PT_AnimChannelTable;
typedef ConstPointerTo<AnimChannelTable> CPT_AnimChannelTable;

#endif // PT_ANIMCHANNELTABLE_H
