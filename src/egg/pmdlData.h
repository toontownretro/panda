/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file pmdlData.h
 * @author lachbr
 * @date 2021-02-13
 */

#ifndef PMDLDATA_H
#define PMDLDATA_H

#include "pandabase.h"
#include "referenceCount.h"
#include "pmdlTextureGroup.h"
#include "pmdlSequence.h"
#include "pmdlSwitch.h"
#include "pmdlIKChain.h"
#include "pmdlAttachment.h"
#include "pointerTo.h"
#include "keyValues.h"
#include "pmap.h"
#include "config_putil.h"

class TokenFile;

/**
 * This class represents a .pmdl file and all of the data it contains.
 */
class EXPCL_PANDA_EGG PMDLData : public ReferenceCount {
PUBLISHED:
  INLINE PMDLData();

  bool read(const Filename &filename, const DSearchPath &search_path = get_model_path());

private:
  bool do_read(TokenFile *file, const DSearchPath &search_path);

  bool process_model(TokenFile *tokens);
  bool process_lod(TokenFile *tokens);
  bool process_texturegroup(TokenFile *tokens);
  bool process_ik_chain(TokenFile *tokens);
  bool process_sequence(TokenFile *tokens);
  bool process_scale(TokenFile *tokens);
  bool process_attachment(TokenFile *tokens);
  bool process_expose(TokenFile *tokens);

public:
  // The different skins of the model.
  typedef pvector<PT(PMDLTextureGroup)> TextureGroups;
  TextureGroups _texture_groups;

  // Animation sequences.
  typedef pmap<std::string, PT(PMDLSequence)> Sequences;
  Sequences _sequences;

  // IK chains.
  typedef pmap<std::string, PT(PMDLIKChain)> IKChains;
  IKChains _ik_chains;

  // LOD switches.
  typedef pvector<PT(PMDLSwitch)> LODSwitches;
  LODSwitches _lod_switches;

  typedef pmap<std::string, PT(PMDLAttachment)> Attachments;
  Attachments _attachments;

  typedef pmap<std::string, std::string> StringMap;
  StringMap _exposes;

  PN_stdfloat _scale;

  // Miscellaneous key-values assigned to the model.
  PT(KeyValues) _misc_kv;

  // The egg file containing the actual geometry and joint structure.
  Filename _model_filename;

  Filename _filename;
  Filename _fullpath;
};

#include "pmdlData.I"

#endif // PMDLDATA_H
