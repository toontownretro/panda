// Filename: bamReader.h
// Created by:  jason (12Jun00)
//
////////////////////////////////////////////////////////////////////
//
// PANDA 3D SOFTWARE
// Copyright (c) 2001, Disney Enterprises, Inc.  All rights reserved
//
// All use of this software is subject to the terms of the Panda 3d
// Software license.  You should have received a copy of this license
// along with this source code; you will also find a current copy of
// the license at http://www.panda3d.org/license.txt .
//
// To contact the maintainers of this program write to
// panda3d@yahoogroups.com .
//
////////////////////////////////////////////////////////////////////

#ifndef __BAM_READER_
#define __BAM_READER_

#include "pandabase.h"
#include "notify.h"

#include "typedWritable.h"
#include "datagramGenerator.h"
#include "datagramIterator.h"
#include "bamReaderParam.h"
#include "factory.h"
#include "vector_int.h"
#include "pset.h"
#include "pmap.h"
#include "dcast.h"

#include <algorithm>

struct PipelineCyclerBase;


// A handy macro for reading PointerToArrays.
#define READ_PTA(Manager, source, Read_func, array)   \
{                                                     \
  void *t;                                            \
  if ((t = Manager->get_pta(source)) == (void*)NULL)  \
  {                                                   \
    array = Read_func(source);                        \
    Manager->register_pta(array.get_void_ptr());      \
  }                                                   \
  else                                                \
  {                                                   \
    array.set_void_ptr(t);                            \
  }                                                   \
}

////////////////////////////////////////////////////////////////////
//       Class : BamReader
// Description : This is the fundamental interface for extracting
//               binary objects from a Bam file, as generated by a
//               BamWriter.
//
//               A Bam file can be thought of as a linear collection
//               of objects.  Each object is an instance of a class
//               that inherits, directly or indirectly, from
//               TypedWritable.  The objects may include pointers to
//               other objects within the Bam file; the BamReader
//               automatically manages these (with help from code
//               within each class) and restores the pointers
//               correctly.
//
//               This is the abstract interface and does not
//               specifically deal with disk files, but rather with a
//               DatagramGenerator of some kind, which is simply a
//               linear source of Datagrams.  It is probably from a
//               disk file, but it might conceivably be streamed
//               directly from a network or some such nonsense.
//
//               Bam files are most often used to store scene graphs
//               or subgraphs, and by convention they are given
//               filenames ending in the extension ".bam" when they
//               are used for this purpose.  However, a Bam file may
//               store any arbitrary list of TypedWritable objects;
//               in this more general usage, they are given filenames
//               ending in ".boo" to differentiate them from the more
//               common scene graph files.
//
//               See also BamFile, which defines a higher-level
//               interface to read and write Bam files on disk.
////////////////////////////////////////////////////////////////////
class EXPCL_PANDA BamReader {
public:
  typedef Factory<TypedWritable> WritableFactory;
  static BamReader *const Null;
  static WritableFactory *const NullFactory;

  // The primary interface for a caller.
  BamReader(DatagramGenerator *generator);
  ~BamReader();

  bool init();

  void set_aux_data(const string &name, void *data);
  void *get_aux_data(const string &name) const;

  TypedWritable *read_object();
  INLINE bool is_eof() const;
  bool resolve();

  INLINE int get_file_major_ver() const;
  INLINE int get_file_minor_ver() const;

  INLINE int get_current_major_ver() const;
  INLINE int get_current_minor_ver() const;

  // This special TypeHandle is written to the bam file to indicate an
  // object id is no longer needed.
  static TypeHandle _remove_flag;

public:
  // Functions to support classes that read themselves from the Bam.

  void read_pointer(DatagramIterator &scan);
  void read_pointers(DatagramIterator &scan, int count);
  void skip_pointer(DatagramIterator &scan);

  void read_cdata(DatagramIterator &scan, PipelineCyclerBase &cycler);

  void register_finalize(TypedWritable *whom);

  typedef TypedWritable *(*ChangeThisFunc)(TypedWritable *object, BamReader *manager);
  void register_change_this(ChangeThisFunc func, TypedWritable *whom);

  void finalize_now(TypedWritable *whom);

  void *get_pta(DatagramIterator &scan);
  void register_pta(void *ptr);

  TypeHandle read_handle(DatagramIterator &scan);


public:
  INLINE static WritableFactory *get_factory();
private:
  INLINE static void create_factory();

private:
  void free_object_ids(DatagramIterator &scan);
  int read_object_id(DatagramIterator &scan);
  int read_pta_id(DatagramIterator &scan);
  int p_read_object();
  bool resolve_object_pointers(TypedWritable *object, const vector_int &pointer_ids);
  bool resolve_cycler_pointers(PipelineCyclerBase *cycler, const vector_int &pointer_ids);
  void finalize();

  bool get_datagram(Datagram &datagram);

private:
  static WritableFactory *_factory;

  DatagramGenerator *_source;
  
  bool _long_object_id;
  bool _long_pta_id;

  // This maps the type index numbers encountered within the Bam file
  // to actual TypeHandles.
  typedef pmap<int, TypeHandle> IndexMap;
  IndexMap _index_map;

  // This maps the object ID numbers encountered within the Bam file
  // to the actual pointers of the corresponding generated objects.
  class CreatedObj {
  public:
    TypedWritable *_ptr;
    ChangeThisFunc _change_this;
  };
  typedef pmap<int, CreatedObj> CreatedObjs;
  CreatedObjs _created_objs;
  // This is the iterator into the above map for the object we are
  // currently reading in p_read_object().  It is carefully maintained
  // during recursion.  We need this so we can associate
  // read_pointer() calls with the proper objects.
  CreatedObjs::iterator _now_creating;
  // This is the pointer to the current PipelineCycler we are reading,
  // if we are within a read_cdata() call.
  PipelineCyclerBase *_reading_cycler;

  // This records all the objects that still need their pointers
  // completed, along with the object ID's of the pointers they need,
  // in the order in which read_pointer() was called, so that we may
  // call the appropriate complete_pointers() later.
  typedef pmap<int, vector_int> ObjectPointers;
  ObjectPointers _object_pointers;

  // Ditto, for the PiplineCycler objects.
  typedef pmap<PipelineCyclerBase *, vector_int> CyclerPointers;
  CyclerPointers _cycler_pointers;

  // This is the number of extra objects that must still be read (and
  // saved in the _created_objs map) before returning from
  // read_object().
  int _num_extra_objects;

  // This is the set of all objects that registered themselves for
  // finalization.
  typedef pset<TypedWritable *> Finalize;
  Finalize _finalize_list;

  // These are used by get_pta() and register_pta() to unify multiple
  // references to the same PointerToArray.
  typedef pmap<int, void *> PTAMap;
  PTAMap _pta_map;
  int _pta_id;

  // This is used internally to record all of the new types created
  // on-the-fly to satisfy bam requirements.  We keep track of this
  // just so we can suppress warning messages from attempts to create
  // objects of these types.
  typedef pset<TypeHandle> NewTypes;
  static NewTypes _new_types;

  // This is used in support of set_aux_data() and get_aux_data().
  typedef pmap<string, void *> AuxData;
  AuxData _aux_data;

  int _file_major, _file_minor;
  static const int _cur_major;
  static const int _cur_minor;
};

typedef BamReader::WritableFactory WritableFactory;

// Useful function for taking apart the Factory Params in the static
// functions that need to be defined in each writable class that will
// be generated by a factory.  Sets the DatagramIterator and the
// BamReader pointers.
INLINE void
parse_params(const FactoryParams &params,
             DatagramIterator &scan, BamReader *&manager);

#include "bamReader.I"

#endif
