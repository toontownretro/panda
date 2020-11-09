#include "config_keyvalues.h"
#include "dconfig.h"
#include "keyValues.h"

Configure(config_keyvalues)

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

  CKeyValues::init_type();
  CKeyValues::register_with_read_factory();
}
