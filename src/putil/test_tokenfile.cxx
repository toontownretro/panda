/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file test_tokenfile.cxx
 * @author brian
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

int
main(int argc, char *argv[]) {
  std::cout << "Hello\n";

  if (argc < 2) {
    std::cerr << "You must specify the token file.\n";
    return 1;
  }

  Filename filename(argv[1]);
  std::cout << "init filename\n";

  PT(TokenFile) tokenfile = new TokenFile;
  std::cout << "Made tokenfile\n";

  if (!tokenfile->read(filename)) {
    std::cerr << "Failed to read the token file.\n";
    return 1;
  }

  while (tokenfile->next_token(true)) {
    std::cout << tokenfile->get_token_type() << " : " << tokenfile->get_token() << "\n";
  }

  return 0;
}
