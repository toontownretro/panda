/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file test_tokenfile.cxx
 * @author lachbr
 * @date 2021-02-12
 */

#include "pandabase.h"
#include "tokenFile.h"

void
indent(std::ostream &stream, int indent_level) {
  for (int i = 0; i < indent_level; i++) {
    stream << " ";
  }
}

void
r_list_tokens(TokenBlock *block, int indent_level) {
  indent(std::cout, indent_level);
  std::cout << "{\n";

  TokenBase *token = block->next();
  while (token) {
    if (token->as_block()) {
      r_list_tokens(token->as_block(), indent_level + 2);

    } else {
      TokenString *str = token->as_string();
      indent(std::cout, indent_level + 2);
      std::cout << str->get_token() << "\n";
    }

    token = block->next();
  }

  indent(std::cout, indent_level);
  std::cout << "}\n";
}

int
main(int argc, char *argv[]) {
  std::cout << "Hello\n";

  Filename filename("/home/lachbr/player/test.tok");
  std::cout << "init filename\n";

  PT(TokenFile) tokenfile = new TokenFile;
  std::cout << "Made tokenfile\n";

  if (!tokenfile->read(filename)) {
    std::cerr << "Failed to read the token file.\n";
    return 1;
  }

  TokenBlock *root = tokenfile->get_root();
  r_list_tokens(root, 0);

  return 0;
}
