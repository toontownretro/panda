/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file pmdlData.cxx
 * @author lachbr
 * @date 2021-02-13
 */

#include "pmdlData.h"
#include "tokenFile.h"
#include "config_egg.h"
#include "string_utils.h"
#include "virtualFileSystem.h"

/**
 * Reads the indicated .pmdl file and fills in the object with the data from
 * the file.
 *
 * Returns true on success, or false if the file couldn't be found or read.
 */
bool PMDLData::
read(const Filename &filename, const DSearchPath &search_path) {
  PT(TokenFile) tokens = new TokenFile;
  if (!tokens->read(filename, search_path)) {
    pmdl_cat.error()
      << "Couldn't parse pmdl tokens\n";
    return false;
  }

  DSearchPath read_search_path = search_path;
  read_search_path.append_directory(tokens->get_fullpath().get_dirname());

  _filename = filename;
  _fullpath = tokens->get_fullpath();

  return do_read(tokens, read_search_path);
}

/**
 * Internal implementation of read().
 */
bool PMDLData::
do_read(TokenFile *tokens, const DSearchPath &search_path) {
  if (pmdl_cat.is_debug()) {
    pmdl_cat.debug()
      << "Parsing pmdl tokens\n";
  }

  while(tokens->token_available(true)) {
    tokens->next_token(true);
    std::string token = downcase(tokens->get_token());

    std::cout << token << "\n";

    if (token == "$model") {
      if (!process_model(tokens)) {
        return false;
      }

    } else if (token == "$materialgroup") {
      if (!process_texturegroup(tokens)) {
        return false;
      }

    } else if (token == "$lod") {
      if (!process_lod(tokens)) {
        return false;
      }

    } else if (token == "$ikchain") {
      if (!process_ik_chain(tokens)) {
        return false;
      }

    } else if (token == "$sequence") {
      if (!process_sequence(tokens)) {
        return false;
      }

    } else if (token == "$scale") {
      if (!process_scale(tokens)) {
        return false;
      }

    } else if (token == "$attachment") {
      if (!process_attachment(tokens)) {
        return false;
      }

    } else if (token == "$expose") {
      if (!process_expose(tokens)) {
        return false;
      }

    } else {
      pmdl_cat.error()
        << "Unknown command: " << token << "\n";
      return false;
    }
  }

  return true;
}

/**
 * Processes a $model command.
 */
bool PMDLData::
process_model(TokenFile *tokens) {
  if (!tokens->token_available()) {
    pmdl_cat.error()
      << "$model: missing filename\n";
    return false;
  }

  tokens->next_token();

  _model_filename = tokens->get_token();
  return true;
}

/**
 * Processes a $lod command.
 */
bool PMDLData::
process_lod(TokenFile *tokens) {
  PT(PMDLSwitch) lod = new PMDLSwitch;
  if (!lod->parse(tokens)) {
    return false;
  }
  _lod_switches.push_back(lod);
  return true;
}

/**
 * Processes a $texturegroup command.
 */
bool PMDLData::
process_texturegroup(TokenFile *tokens) {
  if (!tokens->next_token(true)) {
    return false;
  }

  if (tokens->get_token() != "{") {
    pmdl_cat.error()
      << "$materialgroup: expected { after $materialgroup\n";
    return false;
  }

  bool in_group = false;
  PT(PMDLTextureGroup) curr_group;

  while (1) {
    if (!tokens->next_token(true)) {
      return false;
    }

    std::string token = tokens->get_token();

    if (token == "}") {
      if (!in_group) {
        break;

      } else {
        _texture_groups.push_back(curr_group);
        in_group = false;
      }

    } else if (token == "{") {
      if (in_group) {
        pmdl_cat.error()
          << "Unclosed material group\n";
        return false;
      }

      in_group = true;
      curr_group = new PMDLTextureGroup;

    } else if (in_group) {
      pmdl_cat.debug()
        << "Added material to group: " << token << "\n";
      curr_group->add_material(token);

    } else {
      pmdl_cat.error()
        << "$materialgroup: invalid token: " << token << "\n";
      return false;
    }
  }

  return true;
}

/**
 * Processes an $ikchain command.
 */
bool PMDLData::
process_ik_chain(TokenFile *tokens) {
  tokens->next_token();
  std::string name = tokens->get_token();

  if (_ik_chains.find(name) != _ik_chains.end()) {
    pmdl_cat.error()
      << "duplicated ik chain name: " << name << "\n";
    return false;
  }

  PT(PMDLIKChain) chain = new PMDLIKChain(name);

  tokens->next_token();
  chain->set_foot_joint(tokens->get_token());

  int depth = 0;

  while (1) {
    if (depth > 0) {
      if (!tokens->token_available(true)) {
        return false;
      }
    } else {
      if (!tokens->token_available()) {
        break;
      }
    }

    tokens->next_token(depth > 0);

    std::string token = tokens->get_token();

    if (token == "{") {
      depth++;

    } else if (token == "}") {
      depth--;

      if (depth == 0) {
        break;
      }

    } else if (token == "knee") {
      LVector3 dir;

      tokens->next_token();
      string_to_stdfloat(tokens->get_token(), dir[0]);
      tokens->next_token();
      string_to_stdfloat(tokens->get_token(), dir[1]);
      tokens->next_token();
      string_to_stdfloat(tokens->get_token(), dir[2]);

      chain->set_knee_direction(dir);

    } else if (token == "center") {
      LPoint3 center;

      tokens->next_token();
      string_to_stdfloat(tokens->get_token(), center[0]);
      tokens->next_token();
      string_to_stdfloat(tokens->get_token(), center[1]);
      tokens->next_token();
      string_to_stdfloat(tokens->get_token(), center[2]);

      chain->set_center(center);

    } else if (token == "height") {
      PN_stdfloat height;

      tokens->next_token();
      string_to_stdfloat(tokens->get_token(), height);

      chain->set_height(height);

    } else if (token == "pad") {
      PN_stdfloat pad;
      tokens->next_token();
      string_to_stdfloat(tokens->get_token(), pad);
      chain->set_pad(pad);

    } else if (token == "floor") {
      PN_stdfloat floor;
      tokens->next_token();
      string_to_stdfloat(tokens->get_token(), floor);
      chain->set_floor(floor);

    } else {
      pmdl_cat.error()
        << "$ikchain: invalid token: " << tokens->get_token() << "\n";
      return false;
    }
  }

  _ik_chains[name] = chain;

  return true;
}

/**
 * Processes a $sequence command.
 */
bool PMDLData::
process_sequence(TokenFile *tokens) {
  tokens->next_token();
  std::string seq_name = tokens->get_token();

  if (_sequences.find(seq_name) != _sequences.end()) {
    pmdl_cat.error()
      << "duplicated sequence name: " << seq_name << "\n";
    return false;
  }

  PT(PMDLSequence) seq = new PMDLSequence(seq_name);

  int depth = 0;

  while (1) {
    if (depth > 0) {
      if (!tokens->token_available(true)) {
        return false;
      }
    } else {
      if (!tokens->token_available()) {
        break;
      }
    }

    tokens->next_token(depth > 0);

    std::string token = tokens->get_token();

    if (token == "{") {
      depth++;

    } else if (token == "}") {
      depth--;

      if (depth == 0) {
        break;
      }

    } else if (token == "fps") {
      tokens->next_token();
      int fps;
      string_to_int(tokens->get_token(), fps);
      seq->set_fps(fps);

    } else if (token == "fadein") {
      tokens->next_token();
      PN_stdfloat fadein;
      string_to_stdfloat(tokens->get_token(), fadein);
      seq->set_fade_in(fadein);

    } else if (token == "fadeout") {
      tokens->next_token();
      PN_stdfloat fadeout;
      string_to_stdfloat(tokens->get_token(), fadeout);
      seq->set_fade_out(fadeout);

    } else if (token == "snap") {
      tokens->next_token();
      int snap;
      string_to_int(tokens->get_token(), snap);
      seq->set_snap(snap > 0);

    } else {
      // Assume it's the anim filename.
      seq->set_anim_filename(token);

    }

  }

  _sequences[seq_name] = seq;

  return true;
}

/**
 * Processes a $scale command.
 */
bool PMDLData::
process_scale(TokenFile *tokens) {
  tokens->next_token();
  string_to_stdfloat(tokens->get_token(), _scale);
  return true;
}

/**
 * Processes an $attachment command.
 */
bool PMDLData::
process_attachment(TokenFile *tokens) {
  tokens->next_token();
  std::string name = tokens->get_token();

  if (_attachments.find(name) != _attachments.end()) {
    pmdl_cat.error()
      << "duplicated attachment name: " << name << "\n";
    return false;
  }

  PT(PMDLAttachment) attach = new PMDLAttachment(name);

  tokens->next_token();
  attach->set_parent_joint(tokens->get_token());

  while (tokens->token_available()) {
    tokens->next_token();
    std::string token = tokens->get_token();

    if (token == "pos") {
      LPoint3 pos;

      tokens->next_token();
      string_to_stdfloat(tokens->get_token(), pos[0]);
      tokens->next_token();
      string_to_stdfloat(tokens->get_token(), pos[1]);
      tokens->next_token();
      string_to_stdfloat(tokens->get_token(), pos[2]);

      attach->set_pos(pos);

    } else if (token == "hpr") {
      LVector3 hpr;

      tokens->next_token();
      string_to_stdfloat(tokens->get_token(), hpr[0]);
      tokens->next_token();
      string_to_stdfloat(tokens->get_token(), hpr[1]);
      tokens->next_token();
      string_to_stdfloat(tokens->get_token(), hpr[2]);

      attach->set_hpr(hpr);

    } else {
      pmdl_cat.error()
        << "$attachment: invalid token: " << token << "\n";
      return false;
    }
  }

  _attachments[name] = attach;

  return true;
}

/**
 * Processes an $expose command.
 */
bool PMDLData::
process_expose(TokenFile *tokens) {
  tokens->next_token();
  std::string joint_name = tokens->get_token();
  std::string expose_name = joint_name;

  if (tokens->token_available()) {
    tokens->next_token();
    expose_name = tokens->get_token();
  }

  if (tokens->token_available()) {
    pmdl_cat.error()
      << "Too many tokens in $expose command.\n";
    return false;
  }

  _exposes[joint_name] = expose_name;

  return true;
}
