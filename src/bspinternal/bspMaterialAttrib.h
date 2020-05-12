#pragma once

#include "config_bspinternal.h"
#include "renderAttrib.h"
#include "pointerTo.h"
#include "factoryParams.h"

class BSPMaterial;

class EXPCL_BSPINTERNAL BSPMaterialAttrib : public RenderAttrib
{
private:
  INLINE BSPMaterialAttrib() :
    RenderAttrib(),
    _mat(nullptr),
    _has_override_shader(false) {
  }

PUBLISHED:

  static CPT(RenderAttrib) make(const BSPMaterial *mat);
  static CPT(RenderAttrib) make_override_shader(const BSPMaterial *mat);
  static CPT(RenderAttrib) make_default();

  INLINE std::string get_override_shader() const {
    return _override_shader;
  }
  INLINE bool has_override_shader() const {
    return _has_override_shader;
  }

  INLINE const BSPMaterial *get_material() const {
    return _mat;
  }

private:
  const BSPMaterial *_mat;
  bool _has_override_shader;
  std::string _override_shader;

public:
  static void register_with_read_factory();
  virtual void write_datagram(BamWriter *manager, Datagram &dg);

protected:
  virtual CPT(RenderAttrib) compose_impl(const RenderAttrib *other) const;
  virtual CPT(RenderAttrib) invert_compose_impl(const RenderAttrib *other) const;
  virtual int compare_to_impl(const RenderAttrib *other) const;
  virtual size_t get_hash_impl() const;
  static TypedWritable *make_from_bam(const FactoryParams &params);
  void fillin(DatagramIterator &scan, BamReader *manager);

PUBLISHED:
  static int get_class_slot() {
    return _attrib_slot;
  }
  virtual int get_slot() const {
    return get_class_slot();
  }
  MAKE_PROPERTY(class_slot, get_class_slot);

public:
  static TypeHandle get_class_type() {
    return _type_handle;
  }
  static void init_type() {
    RenderAttrib::init_type();
    register_type(_type_handle, "BSPMaterialAttrib",
                  RenderAttrib::get_class_type());
    _attrib_slot = register_slot(_type_handle, -1, new BSPMaterialAttrib);
  }
  virtual TypeHandle get_type() const {
    return get_class_type();
  }
  virtual TypeHandle force_init_type() {
    init_type(); return get_class_type();
  }

private:
  static TypeHandle _type_handle;
  static int _attrib_slot;
};
