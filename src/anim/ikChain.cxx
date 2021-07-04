/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file ikChain.cxx
 * @author lachbr
 * @date 2021-02-11
 */

#include "ikChain.h"
#include "config_anim.h"

/**
 *
 */
IKChain::
IKChain(const std::string &name, int top_joint, int middle_joint, int end_joint) :
  Namable(name) {

  _top_joint = top_joint;
  _middle_joint = middle_joint;
  _end_joint = end_joint;

  _height = 0.0;
  _floor = 0.0;
  _pad = 0.0;
}

/**
 *
 */
IKChain::
IKChain(const IKChain &copy) :
  Namable(copy),
  _top_joint(copy._top_joint),
  _middle_joint(copy._middle_joint),
  _end_joint(copy._end_joint),
  _height(copy._height),
  _floor(copy._floor),
  _pad(copy._pad)
{
}

/**
 *
 */
IKChain::
IKChain(IKChain &&other) :
  Namable(std::move(other)),
  _top_joint(std::move(other._top_joint)),
  _middle_joint(std::move(other._middle_joint)),
  _end_joint(std::move(other._end_joint)),
  _height(std::move(other._height)),
  _floor(std::move(other._floor)),
  _pad(std::move(other._pad))
{
}

/**
 *
 */
void IKChain::
operator = (const IKChain &copy) {
  Namable::operator = (copy);
  _top_joint = copy._top_joint;
  _middle_joint = copy._middle_joint;
  _end_joint = copy._middle_joint;
  _height = copy._height;
  _floor = copy._floor;
  _pad = copy._pad;
}

/**
 *
 */
void IKChain::
operator = (IKChain &&other) {
  Namable::operator = (std::move(other));
  _top_joint = std::move(other._top_joint);
  _middle_joint = std::move(other._middle_joint);
  _end_joint = std::move(other._end_joint);
  _height = std::move(other._height);
  _floor = std::move(other._floor);
  _pad = std::move(other._pad);
}

/**
 * Function to write the important information in the particular object to a
 * Datagram
 */
void IKChain::
write_datagram(BamWriter *manager, Datagram &me) {
  me.add_string(get_name());
  me.add_int16(_top_joint);
  me.add_int16(_middle_joint);
  me.add_int16(_end_joint);
  _middle_direction.write_datagram(me);
  _center.write_datagram(me);
  me.add_stdfloat(_height);
  me.add_stdfloat(_floor);
  me.add_stdfloat(_pad);
}

/**
 * Function that reads out of the datagram (or asks manager to read) all of
 * the data that is needed to re-create this object and stores it in the
 * appropiate place
 */
void IKChain::
fillin(DatagramIterator &scan, BamReader *manager) {
  set_name(scan.get_string());
  _top_joint = scan.get_int16();
  _middle_joint = scan.get_int16();
  _end_joint = scan.get_int16();
  _middle_direction.read_datagram(scan);
  _center.read_datagram(scan);
  _height = scan.get_stdfloat();
  _floor = scan.get_stdfloat();
  _pad = scan.get_stdfloat();
}
