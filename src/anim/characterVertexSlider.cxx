/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file characterVertexSlider.cxx
 * @author drose
 * @date 2005-03-28
 */

#include "characterVertexSlider.h"
#include "datagram.h"
#include "datagramIterator.h"
#include "bamReader.h"
#include "bamWriter.h"
#include "characterSlider.h"
#include "character.h"

TypeHandle CharacterVertexSlider::_type_handle;

/**
 * Constructs an invalid object; used only by the bam loader.
 */
CharacterVertexSlider::
CharacterVertexSlider() :
  VertexSlider(InternalName::get_root())
{
}

/**
 * Constructs a new object that converts vertices from the indicated joint's
 * coordinate space, into the other indicated joint's space.
 */
CharacterVertexSlider::
CharacterVertexSlider(Character *character, int slider) :
  VertexSlider(InternalName::make(character->get_slider_name(slider))),
  _char(character),
  _slider(slider)
{
  // Tell the char_slider that we need to be informed when it moves.
  _char->set_vertex_slider(_slider, this);
}

/**
 *
 */
CharacterVertexSlider::
~CharacterVertexSlider() {
  // Tell the char_slider to stop informing us about its motion.
  if (_char.is_valid_pointer()) {
    _char->set_vertex_slider(_slider, nullptr);
  }
}

/**
 *
 */
PN_stdfloat CharacterVertexSlider::
get_slider(Thread *current_thread) const {
  return _char->get_slider_value(_slider, current_thread);
}

/**
 * Tells the BamReader how to create objects of type CharacterVertexSlider.
 */
void CharacterVertexSlider::
register_with_read_factory() {
  BamReader::get_factory()->register_factory(get_class_type(), make_from_bam);
}

/**
 * Writes the contents of this object to the datagram for shipping out to a
 * Bam file.
 */
void CharacterVertexSlider::
write_datagram(BamWriter *manager, Datagram &dg) {
  VertexSlider::write_datagram(manager, dg);

  manager->write_pointer(dg, _char.p());
  dg.add_int16(_slider);
}

/**
 * Receives an array of pointers, one for each time manager->read_pointer()
 * was called in fillin(). Returns the number of pointers processed.
 */
int CharacterVertexSlider::
complete_pointers(TypedWritable **p_list, BamReader *manager) {
  int pi = VertexSlider::complete_pointers(p_list, manager);

  _char = DCAST(Character, p_list[pi++]);
  _char->set_vertex_slider(_slider, this);
  _name = InternalName::make(_char->get_slider_name(_slider));

  return pi;
}

/**
 * This function is called by the BamReader's factory when a new object of
 * type CharacterVertexSlider is encountered in the Bam file.  It should
 * create the CharacterVertexSlider and extract its information from the file.
 */
TypedWritable *CharacterVertexSlider::
make_from_bam(const FactoryParams &params) {
  CharacterVertexSlider *object = new CharacterVertexSlider;
  DatagramIterator scan;
  BamReader *manager;

  parse_params(params, scan, manager);
  object->fillin(scan, manager);

  return object;
}

/**
 * This internal function is called by make_from_bam to read in all of the
 * relevant data from the BamFile for the new CharacterVertexSlider.
 */
void CharacterVertexSlider::
fillin(DatagramIterator &scan, BamReader *manager) {
  VertexSlider::fillin(scan, manager);

  manager->read_pointer(scan);
  _slider = scan.get_int16();
}
