#include "config_keyvalues.h"

#include "dconfig.h"

ConfigureDef(config_keyvalues)

ConfigureFn(config_keyvalues) {
  init_libkeyvalues();
}

void
init_libkeyvalues() {
  static bool initialized = false;
  if (initialized) {
    return;
  }

  initialized = true;
}
