#include "bspMaterialAttrib.h"
#include "bspMaterial.h"

#include "bamReader.h"

TypeHandle BSPMaterialAttrib::_type_handle;
int BSPMaterialAttrib::_attrib_slot;

CPT(RenderAttrib) BSPMaterialAttrib::make(const BSPMaterial *mat) {
  PT(BSPMaterialAttrib) bma = new BSPMaterialAttrib;
  bma->_mat = mat;
  bma->_has_override_shader = false;
  return return_new(bma);
}

/**
 * Creates a new BSPMaterialAttrib that says the shader name on the attrib
 * should override any other material's shader it composes with, but keep the keyvalues.
 *
 * This is useful for something like a shadow render pass, where all objects are rendered
 * using the shadow pass shader, but need access to the $basetexture of each material
 * for transparency and possibly other effects.
 *
 * If this didn't exist, as the RenderState composes, each BSPMaterialAttrib would
 * completely override each other, voiding the shadow render shader.
 */
CPT(RenderAttrib) BSPMaterialAttrib::make_override_shader(const BSPMaterial *mat) {
  PT(BSPMaterialAttrib) bma = new BSPMaterialAttrib;
  bma->_mat = mat;
  bma->_has_override_shader = true;
  bma->_override_shader = mat->get_shader();
  return return_new(bma);
}

CPT(RenderAttrib) BSPMaterialAttrib::make_default() {
  PT(BSPMaterial) mat = new BSPMaterial;
  PT(BSPMaterialAttrib) bma = new BSPMaterialAttrib;
  bma->_mat = mat;
  bma->_has_override_shader = false;
  return return_new(bma);
}

CPT(RenderAttrib) BSPMaterialAttrib::compose_impl(const RenderAttrib *other) const {
  const BSPMaterialAttrib *bma = (const BSPMaterialAttrib *)other;

  if (_has_override_shader) {
    // We're going to override the other material's shader,
    // but keep their keyvalues.
    BSPMaterialAttrib *nbma = new BSPMaterialAttrib;
    nbma->_mat = bma->_mat;
    nbma->_has_override_shader = true;
    nbma->_override_shader = _override_shader;
    return return_new(nbma);
  }

  return other;
}

CPT(RenderAttrib) BSPMaterialAttrib::invert_compose_impl(const RenderAttrib *other) const {
  const BSPMaterialAttrib *bma = (const BSPMaterialAttrib *)other;

  if (bma->_has_override_shader) {
    // The other material is going to override our shader.
    BSPMaterialAttrib *nbma = new BSPMaterialAttrib;
    nbma->_mat = bma->_mat;
    nbma->_has_override_shader = true;
    nbma->_override_shader = bma->_override_shader;
    return return_new(nbma);
  }

  return other;
}

/**
 * BSPMaterials are compared solely by their source filename.
 * We could also compare all of the keyvalues, but whatever.
 * You shouldn't really be creating BSPMaterials on the fly,
 * they should always be in a file.
 */
int BSPMaterialAttrib::compare_to_impl(const RenderAttrib *other) const {
  const BSPMaterialAttrib *bma = (const BSPMaterialAttrib *)other;

  if (_mat != bma->_mat) {
    return _mat < bma->_mat ? -1 : 1;
  }
  if (_has_override_shader != bma->_has_override_shader) {
    return (int)_has_override_shader < (int)bma->_has_override_shader ? -1 : 1;
  }

  return _override_shader.compare(bma->_override_shader);
}

size_t BSPMaterialAttrib::get_hash_impl() const {
  size_t hash = 0;
  hash = pointer_hash::add_hash(hash, _mat);
  hash = int_hash::add_hash(hash, _has_override_shader);
  hash = string_hash::add_hash(hash, _override_shader);
  return hash;
}

void BSPMaterialAttrib::register_with_read_factory() {
  BamReader::get_factory()->register_factory(get_class_type(), make_from_bam);
}

void BSPMaterialAttrib::write_datagram(BamWriter *manager, Datagram &dg) {
  RenderAttrib::write_datagram(manager, dg);
  dg.add_string(_mat->get_file().get_fullpath());
}

TypedWritable *BSPMaterialAttrib::make_from_bam(const FactoryParams &params) {
  BSPMaterialAttrib *bma = new BSPMaterialAttrib;
  DatagramIterator scan;
  BamReader *manager;

  parse_params(params, scan, manager);
  bma->fillin(scan, manager);

  return bma;
}

void BSPMaterialAttrib::fillin(DatagramIterator &scan, BamReader *manager) {
  RenderAttrib::fillin(scan, manager);

  _mat = BSPMaterial::get_from_file(scan.get_string());
}
