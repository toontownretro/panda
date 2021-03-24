/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file pmdlSwitch.cxx
 * @author lachbr
 * @date 2021-02-13
 */

#include "pmdlSwitch.h"
#include "tokenFile.h"
#include "string_utils.h"
#include "config_egg.h"

/**
 * Parses a $lod description.
 */
bool PMDLSwitch::
parse(TokenFile *tokens) {
  if (!tokens->token_available()) {
    pmdl_cat.error()
      << "$lod: missing in distance\n";
    return false;
  }
  tokens->next_token();
  if (!string_to_stdfloat(tokens->get_token(), _in_distance)) {
    pmdl_cat.error()
      << "$lod: invalid in distance: " << tokens->get_token() << "\n";
    return false;
  }

  if (!tokens->token_available()) {
    pmdl_cat.error()
      << "$lod: missing out distance\n";
    return false;
  }
  tokens->next_token();
  if (!string_to_stdfloat(tokens->get_token(), _out_distance)) {
    pmdl_cat.error()
      << "$lod: invalid out distance: " << tokens->get_token() << "\n";
    return false;
  }

  tokens->next_token(true);

  if (tokens->get_token() != "{") {
    pmdl_cat.error()
      << "'{' expected while processing $lod\n";
    return false;
  }

  while (1) {
    if (!tokens->next_token(true)) {
      pmdl_cat.error()
        << "Unexpected EOF while processing $lod\n";
      return false;
    }

    std::string token = downcase(tokens->get_token());

    if (token == "}") {
      if (get_num_groups() == 0) {
        pmdl_cat.error()
          << "$lod: no groups specified\n";
        return false;
      }

      break;

    } else if (token == "group") {
      if (!tokens->token_available()) {
        pmdl_cat.error()
          << "$lod: group command missing group name(s)\n";
        return false;
      }

      while (tokens->token_available()) {
        tokens->next_token();

        add_group(tokens->get_token());
      }

    } else if (token == "center") {
      tokens->next_token();
      string_to_stdfloat(tokens->get_token(), _center[0]);
      tokens->next_token();
      string_to_stdfloat(tokens->get_token(), _center[1]);
      tokens->next_token();
      string_to_stdfloat(tokens->get_token(), _center[2]);

    } else if (token == "fade") {
      tokens->next_token();
      string_to_stdfloat(tokens->get_token(), _fade);

    } else {
      pmdl_cat.error()
        << "Unknown $lod command: " << token << "\n";
      return false;
    }
  }

  return true;
}
