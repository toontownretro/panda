/**
 * PANDA3D BSP LIBRARY
 *
 * Copyright (c) Brian Lach <brianlach72@gmail.com>
 * All rights reserved.
 *
 * @file keyValues.h
 * @author Brian Lach
 * @date April 15, 2020
 *
 * @desc The new and much cleaner interface for dealing with Valve's KeyValue
 *	     format. KeyValues can either be created procedurally or loaded from
 *       disk.
 */

#pragma once

#include "config_putil.h"

#include "typedWritableReferenceCount.h"
#include "factoryParams.h"
#include "pointerTo.h"
#include "simpleHashMap.h"
#include "luse.h"
#include "vector_int.h"
#include "vector_float.h"

class TokenFile;

static const std::string root_block_name = "__root";
static const std::string not_found = "not found";

/**
 * Represents a single block from a key-values file.
 * Has a list of string key-value pairs, and can have a list of child blocks.
 */
class EXPCL_PANDA_PUTIL KeyValues : public TypedWritableReferenceCount {
PUBLISHED:
	class Pair {
	PUBLISHED:
		std::string key;
		std::string value;
	};

	KeyValues(const std::string &name = root_block_name, KeyValues *parent = nullptr);

	//void set_parent( KeyValues *parent );
	KeyValues *get_parent() const;

	void set_name(const std::string &name);
	const std::string &get_name() const;

	void add_child(KeyValues *child);
	KeyValues *get_child(size_t n) const;
	int find_child(const std::string &name) const;
	pvector<KeyValues *> get_children_with_name(const std::string &name) const;
	size_t get_num_children() const;

	std::string &operator [](const std::string &key);

	void set_key_value(const std::string &key, const std::string &value);

	size_t get_num_keys() const;
	bool has_key(const std::string &name) const;
	int find_key(const std::string &name) const;
	const std::string &get_key(size_t n) const;
	const std::string &get_value(size_t n) const;
	const std::string &get_value(const std::string &key) const;
	void add_key_value(const std::string &key, const std::string &value);

	const Filename &get_filename() const;

	void write(const Filename &filename, int indent = 4);

	Pair *find_pair(const std::string &key);
	const Pair *find_pair(const std::string &key) const;
	const Pair *get_pair(size_t n) const;

private:
	void parse(TokenFile *tokens);
	void do_write(std::ostringstream &out, int indent, int &curr_indent);
	void do_indent(std::ostringstream &out, int curr_indent);

PUBLISHED:
	static PT(KeyValues) load(const Filename &filename);
	static PT(KeyValues) from_string(const std::string &data);

  static vector_int parse_int_list(const std::string &str);
	static vector_float parse_float_list(const std::string &str);
	static pvector<vector_float> parse_float_tuple_list(const std::string &str);
	static void parse_material_axis(const std::string &str, LVector3 &axis, LVector2 &shift_scale);
	static void parse_plane_points(const std::string &str, LPoint3 &p0, LPoint3 &p1, LPoint3 &p2);
	static LVecBase3f to_3f(const std::string &str);
	static LVecBase2f to_2f(const std::string &str);
	static LVecBase4f to_4f(const std::string &str);

	EXTENSION(PyObject *as_int_list(const std::string &str));
	EXTENSION(PyObject *as_float_list(const std::string &str));
  EXTENSION(PyObject *as_float_tuple_list(const std::string &str));

	template<class T>
	INLINE static std::string to_string(const pvector<T> &v);

	INLINE static std::string to_string(unsigned int v);
	INLINE static std::string to_string(int v);
	INLINE static std::string to_string(float v);
	INLINE static std::string to_string(double v);
	static std::string to_string(const LVecBase3f &v);
	static std::string to_string(const LVecBase4f &v);
	static std::string to_string(const LVecBase2f &v);

private:
	PT(KeyValues) _parent;
	Filename _filename;
	std::string _name;
	pvector<Pair> _keyvalues;
	pvector<PT(KeyValues)> _children;

public:
  static void register_with_read_factory();
  virtual void write_datagram(BamWriter *manager, Datagram &dg);
	virtual int complete_pointers(TypedWritable **p_list,
                                BamReader *manager);

protected:
  static TypedWritable *make_from_bam(const FactoryParams &params);
  virtual void fillin(DatagramIterator &scan, BamReader *manager);

public:
  static TypeHandle get_class_type() {
    return _type_handle;
  }
  static void init_type() {
    TypedWritableReferenceCount::init_type();
    register_type(_type_handle, "KeyValues",
                  TypedWritableReferenceCount::get_class_type());
  }
  virtual TypeHandle get_type() const {
    return get_class_type();
  }
  virtual TypeHandle force_init_type() {init_type(); return get_class_type();}

private:
  static TypeHandle _type_handle;
};

INLINE KeyValues::KeyValues(const std::string &name, KeyValues *parent) {
	_name = name;
	_parent = parent;
	if (parent) {
		parent->add_child(this);
	}
}

//inline void KeyValues::set_parent( KeyValues *parent )
//{
//	_parent = parent;
//}

INLINE KeyValues *KeyValues::get_parent() const {
	return _parent;
}

INLINE void KeyValues::set_name(const std::string &name) {
	_name = name;
}

INLINE const std::string &KeyValues::get_name() const {
	return _name;
}

INLINE void KeyValues::add_child(KeyValues *child) {
	child->_parent = this;
	_children.push_back(child);
}

INLINE KeyValues *KeyValues::get_child(size_t n) const {
	return _children[n];
}

INLINE size_t KeyValues::get_num_children() const {
	return _children.size();
}

INLINE std::string &KeyValues::operator[](const std::string &key) {
	Pair *pair = find_pair(key);
	if (pair) {
		return pair->value;

	} else {
		Pair newpair;
		newpair.key = key;
		size_t index = _keyvalues.size();
		_keyvalues.push_back(newpair);
		return _keyvalues[index].value;
	}
}

INLINE void KeyValues::
set_key_value(const std::string &key, const std::string &value) {
	Pair *pair = find_pair(key);
	if (!pair) {
		add_key_value(key, value);
	} else {
		pair->value = value;
	}
}

INLINE void KeyValues::
add_key_value(const std::string &key, const std::string &value) {
	Pair newpair;
	newpair.key = key;
	newpair.value = value;
	_keyvalues.push_back(newpair);
}

INLINE const std::string &KeyValues::
get_value(const std::string &key) const {
	int itr = find_key(key);
	if (itr != -1) {
		return get_value(itr);
	}

	return not_found;
}

INLINE size_t KeyValues::get_num_keys() const {
	return _keyvalues.size();
}

INLINE bool KeyValues::has_key(const std::string &key) const {
	return find_pair(key) != nullptr;
}

INLINE int KeyValues::find_key(const std::string &key) const {
	return find_pair(key) - _keyvalues.data();
}

INLINE const std::string &KeyValues::get_key(size_t n) const {
	return _keyvalues[n].key;
}

INLINE const std::string &KeyValues::get_value(size_t n) const {
	return _keyvalues[n].value;
}

INLINE const Filename &KeyValues::get_filename() const {
	return _filename;
}

template <class T>
INLINE std::string KeyValues::
to_string(const pvector<T> &v) {
  std::string res = "";
  for (size_t i = 0; i < v.size(); i++) {
    res += to_string(v[i]);
  }

  return res;
}

INLINE std::string KeyValues::
to_string(unsigned int v) {
	std::ostringstream ss;
	ss << v;
	return ss.str();
}

INLINE std::string KeyValues::
to_string(int v) {
	std::ostringstream ss;
	ss << v;
	return ss.str();
}

INLINE std::string KeyValues::
to_string(float v) {
	std::ostringstream ss;
	ss << v;
	return ss.str();
}

INLINE std::string KeyValues::
to_string(double v) {
	std::ostringstream ss;
	ss << v;
	return ss.str();
}
