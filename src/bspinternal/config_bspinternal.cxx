#include "config_bspinternal.h"
#include "dconfig.h"

#include "bspMaterial.h"
#include "bspMaterialAttrib.h"

Configure(config_bspinternal)
ConfigureFn(config_bspinternal) {
  init_libbspinternal();
}

void
init_libbspinternal() {
  static bool initialized = false;
  if (initialized) {
    return;
  }

  initialized = true;

  BSPMaterial::init_type();
  BSPMaterialAttrib::init_type();
  BSPMaterialAttrib::register_with_read_factory();
}
