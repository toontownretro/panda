/**
 * @file pandaexpress.cxx
 * @author drose
 * @date 2000-05-15
 */

#include "dconfig.h"
#include "config_express.h"
#include "config_downloader.h"

#if defined(_WIN32) && !defined(CPPPARSER) && !defined(LINK_ALL_STATIC)
__declspec(dllexport)
#endif
void
init_libpandaexpress() {
  static bool initialized = false;
  if (initialized) {
    return;
  }
  initialized = true;

  init_libexpress();
  init_libdownloader();
}

Configure(config_pandaexpress);
ConfigureFn(config_pandaexpress) {
  init_libpandaexpress();
}
