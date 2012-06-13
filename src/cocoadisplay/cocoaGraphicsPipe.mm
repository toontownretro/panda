// Filename: cocoaGraphicsPipe.mm
// Created by:  rdb (14May12)
//
////////////////////////////////////////////////////////////////////
//
// PANDA 3D SOFTWARE
// Copyright (c) Carnegie Mellon University.  All rights reserved.
//
// All use of this software is subject to the terms of the revised BSD
// license.  You should have received a copy of this license along
// with this source code in a file named "LICENSE."
//
////////////////////////////////////////////////////////////////////

#include "cocoaGraphicsPipe.h"
//#include "cocoaGraphicsBuffer.h"
#include "cocoaGraphicsWindow.h"
#include "cocoaGraphicsStateGuardian.h"
#include "config_cocoadisplay.h"
#include "frameBufferProperties.h"

#import <Foundation/NSAutoreleasePool.h>
#import <AppKit/NSApplication.h>

#include <mach-o/arch.h>

TypeHandle CocoaGraphicsPipe::_type_handle;

static void init_app() {
  if (NSApp == nil) {
    NSAutoreleasePool *pool = [[NSAutoreleasePool alloc] init];
    [NSApplication sharedApplication];

    [NSApp setActivationPolicy:nil];
    [NSApp finishLaunching];
    [NSApp activateIgnoringOtherApps:YES];
  }
}

////////////////////////////////////////////////////////////////////
//     Function: CocoaGraphicsPipe::Constructor
//       Access: Public
//  Description: Uses the main screen (the one the user is most
//               likely to be working in at the moment).
////////////////////////////////////////////////////////////////////
CocoaGraphicsPipe::
CocoaGraphicsPipe() {
  _supported_types = OT_window | OT_buffer | OT_texture_buffer;
  _is_valid = true;

  init_app();

  _screen = [NSScreen mainScreen];
  NSNumber *num = [[_screen deviceDescription] objectForKey: @"NSScreenNumber"];
  _display = (CGDirectDisplayID) [num longValue];

  _display_width = CGDisplayPixelsWide(_display);
  _display_height = CGDisplayPixelsHigh(_display);
  load_display_information();

  cocoadisplay_cat.debug()
    << "Creating CocoaGraphicsPipe for main screen "
    << _screen << " with display ID " << _display << "\n";
}

////////////////////////////////////////////////////////////////////
//     Function: CocoaGraphicsPipe::Constructor
//       Access: Public
//  Description: Takes a CoreGraphics display ID.
////////////////////////////////////////////////////////////////////
CocoaGraphicsPipe::
CocoaGraphicsPipe(CGDirectDisplayID display) {
  _supported_types = OT_window | OT_buffer | OT_texture_buffer;
  _is_valid = true;
  _display = display;

  init_app();

  // Iterate over the screens to find the one with our display ID.
  NSEnumerator *e = [[NSScreen screens] objectEnumerator];
  while (NSScreen *screen = (NSScreen *) [e nextObject]) {
    NSNumber *num = [[screen deviceDescription] objectForKey: @"NSScreenNumber"];
    if (display == (CGDirectDisplayID) [num longValue]) {
      _screen = screen;
      break;
    }
  }

  _display_width = CGDisplayPixelsWide(_display);
  _display_height = CGDisplayPixelsHigh(_display);
  load_display_information();

  cocoadisplay_cat.debug()
    << "Creating CocoaGraphicsPipe for screen "
    << _screen << " with display ID " << _display << "\n";
}

////////////////////////////////////////////////////////////////////
//     Function: CocoaGraphicsPipe::Constructor
//       Access: Public
//  Description: Takes an NSScreen pointer.
////////////////////////////////////////////////////////////////////
CocoaGraphicsPipe::
CocoaGraphicsPipe(NSScreen *screen) {
  _supported_types = OT_window | OT_buffer | OT_texture_buffer;
  _is_valid = true;

  init_app();

  if (screen == nil) {
    _screen = [NSScreen mainScreen];
  } else {
    _screen = screen;
  }
  NSNumber *num = [[_screen deviceDescription] objectForKey: @"NSScreenNumber"];
  _display = (CGDirectDisplayID) [num longValue];

  _display_width = CGDisplayPixelsWide(_display);
  _display_height = CGDisplayPixelsHigh(_display);
  load_display_information();

  cocoadisplay_cat.debug()
    << "Creating CocoaGraphicsPipe for screen "
    << _screen << " with display ID " << _display << "\n";
}

////////////////////////////////////////////////////////////////////
//     Function: CocoaGraphicsPipe::load_display_information
//       Access: Private
//  Description: Fills in _display_information.
////////////////////////////////////////////////////////////////////
void CocoaGraphicsPipe::
load_display_information() {
  _display_information->_vendor_id = CGDisplayVendorNumber(_display);
  //_display_information->_device_id = CGDisplayUnitNumber(_display);
  //_display_information->_device_id = CGDisplaySerialNumber(_display);

  // Display modes
#if defined(MAC_OS_X_VERSION_10_6) && MAC_OS_X_VERSION_MAX_ALLOWED >= MAC_OS_X_VERSION_10_6
  CFArrayRef modes = CGDisplayCopyAllDisplayModes(_display, NULL);
  size_t num_modes = CFArrayGetCount(modes);
  _display_information->_total_display_modes = num_modes;
  _display_information->_display_mode_array = new DisplayMode[num_modes];

  for (size_t i = 0; i < num_modes; ++i) {
    CGDisplayModeRef mode = (CGDisplayModeRef) CFArrayGetValueAtIndex(modes, i);

    _display_information->_display_mode_array[i].width = CGDisplayModeGetWidth(mode);
    _display_information->_display_mode_array[i].height = CGDisplayModeGetHeight(mode);
    _display_information->_display_mode_array[i].refresh_rate = CGDisplayModeGetRefreshRate(mode);
    _display_information->_display_mode_array[i].fullscreen_only = false;

    // Read number of bits per pixels from the pixel encoding
    CFStringRef encoding = CGDisplayModeCopyPixelEncoding(mode);
    if (CFStringCompare(encoding, CFSTR(kIO64BitDirectPixels),
                              kCFCompareCaseInsensitive) == kCFCompareEqualTo) {
      _display_information->_display_mode_array[i].bits_per_pixel = 64;

    } else if (CFStringCompare(encoding, CFSTR(kIO32BitFloatPixels),
                              kCFCompareCaseInsensitive) == kCFCompareEqualTo) {
      _display_information->_display_mode_array[i].bits_per_pixel = 32;

    } else if (CFStringCompare(encoding, CFSTR(kIO16BitFloatPixels),
                              kCFCompareCaseInsensitive) == kCFCompareEqualTo) {
      _display_information->_display_mode_array[i].bits_per_pixel = 16;

    } else if (CFStringCompare(encoding, CFSTR(IOYUV422Pixels),
                              kCFCompareCaseInsensitive) == kCFCompareEqualTo ||
               CFStringCompare(encoding, CFSTR(IO8BitOverlayPixels),
                              kCFCompareCaseInsensitive) == kCFCompareEqualTo) {
      _display_information->_display_mode_array[i].bits_per_pixel = 8;
    } else {
      // The other possible pixel formats in IOKit/IOGraphicsTypes.h
      // have strings like "PPPP" or "-RRRRRGGGGGBBBBB", so the number
      // of bits per pixel can be deduced from the string length.  Nifty!
      _display_information->_display_mode_array[i].bits_per_pixel = CFStringGetLength(encoding);
    }
    CFRelease(encoding);
  }
  CFRelease(modes);

#else
  CFArrayRef modes = CGDisplayAvailableModes(_display);
  size_t num_modes = CFArrayGetCount(modes);
  _display_information->_total_display_modes = num_modes;
  _display_information->_display_mode_array = new DisplayMode[num_modes];

  for (size_t i = 0; i < num_modes; ++i) {
    CFDictionaryRef mode = (CFDictionaryRef) CFArrayGetValueAtIndex(modes, i);

    CFNumberGetValue((CFNumberRef) CFDictionaryGetValue(mode, kCGDisplayWidth),
      kCFNumberIntType, &_display_information->_display_mode_array[i].width);

    CFNumberGetValue((CFNumberRef) CFDictionaryGetValue(mode, kCGDisplayHeight),
      kCFNumberIntType, &_display_information->_display_mode_array[i].height);

    CFNumberGetValue((CFNumberRef) CFDictionaryGetValue(mode, kCGDisplayBitsPerPixel),
      kCFNumberIntType, &_display_information->_display_mode_array[i].bits_per_pixel);

    CFNumberGetValue((CFNumberRef) CFDictionaryGetValue(mode, kCGDisplayRefreshRate),
      kCFNumberIntType, &_display_information->_display_mode_array[i].refresh_rate);

    _display_information->_display_mode_array[i].fullscreen_only = false;
  }
#endif

  // Get processor information
  const NXArchInfo *ainfo = NXGetLocalArchInfo();
  _display_information->_cpu_brand_string = strdup(ainfo->description);

  // Get version of Mac OS X
  SInt32 major, minor, bugfix;
  Gestalt(gestaltSystemVersionMajor, &major);
  Gestalt(gestaltSystemVersionMinor, &minor);
  Gestalt(gestaltSystemVersionBugFix, &bugfix);
  _display_information->_os_version_major = major;
  _display_information->_os_version_minor = minor;
  _display_information->_os_version_build = bugfix;
}

////////////////////////////////////////////////////////////////////
//     Function: CocoaGraphicsPipe::Destructor
//       Access: Public, Virtual
//  Description:
////////////////////////////////////////////////////////////////////
CocoaGraphicsPipe::
~CocoaGraphicsPipe() {
}

////////////////////////////////////////////////////////////////////
//     Function: CocoaGraphicsPipe::get_interface_name
//       Access: Published, Virtual
//  Description: Returns the name of the rendering interface
//               associated with this GraphicsPipe.  This is used to
//               present to the user to allow him/her to choose
//               between several possible GraphicsPipes available on a
//               particular platform, so the name should be meaningful
//               and unique for a given platform.
////////////////////////////////////////////////////////////////////
string CocoaGraphicsPipe::
get_interface_name() const {
  return "OpenGL";
}

////////////////////////////////////////////////////////////////////
//     Function: CocoaGraphicsPipe::pipe_constructor
//       Access: Public, Static
//  Description: This function is passed to the GraphicsPipeSelection
//               object to allow the user to make a default
//               CocoaGraphicsPipe.
////////////////////////////////////////////////////////////////////
PT(GraphicsPipe) CocoaGraphicsPipe::
pipe_constructor() {
  return new CocoaGraphicsPipe;
}

////////////////////////////////////////////////////////////////////
//     Function: CocoaGraphicsPipe::get_preferred_window_thread
//       Access: Public, Virtual
//  Description: Returns an indication of the thread in which this
//               GraphicsPipe requires its window processing to be
//               performed: typically either the app thread (e.g. X)
//               or the draw thread (Windows).
////////////////////////////////////////////////////////////////////
GraphicsPipe::PreferredWindowThread
CocoaGraphicsPipe::get_preferred_window_thread() const {
  // The NSView and NSWindow classes are not thread-safe,
  // they can only be called from the main thread!
  return PWT_app;
}

////////////////////////////////////////////////////////////////////
//     Function: CocoaGraphicsPipe::make_output
//       Access: Protected, Virtual
//  Description: Creates a new window on the pipe, if possible.
////////////////////////////////////////////////////////////////////
PT(GraphicsOutput) CocoaGraphicsPipe::
make_output(const string &name,
            const FrameBufferProperties &fb_prop,
            const WindowProperties &win_prop,
            int flags,
            GraphicsEngine *engine,
            GraphicsStateGuardian *gsg,
            GraphicsOutput *host,
            int retry,
            bool &precertify) {

  if (!_is_valid) {
    return NULL;
  }

  CocoaGraphicsStateGuardian *cocoagsg = 0;
  if (gsg != 0) {
    DCAST_INTO_R(cocoagsg, gsg, NULL);
  }

  // First thing to try: a CocoaGraphicsWindow

  if (retry == 0) {
    if (((flags&BF_require_parasite)!=0)||
        ((flags&BF_refuse_window)!=0)||
        ((flags&BF_resizeable)!=0)||
        ((flags&BF_size_track_host)!=0)||
        ((flags&BF_rtt_cumulative)!=0)||
        ((flags&BF_can_bind_color)!=0)||
        ((flags&BF_can_bind_every)!=0)) {
      return NULL;
    }
    return new CocoaGraphicsWindow(engine, this, name, fb_prop, win_prop,
                                   flags, gsg, host);
  }
/*
  // Second thing to try: a GLES(2)GraphicsBuffer
  if (retry == 1) {
    if ((host==0)||
  //        (!gl_support_fbo)||
        ((flags&BF_require_parasite)!=0)||
        ((flags&BF_require_window)!=0)) {
      return NULL;
    }
    // Early failure - if we are sure that this buffer WONT
    // meet specs, we can bail out early.
    if ((flags & BF_fb_props_optional)==0) {
      if ((fb_prop.get_indexed_color() > 0)||
          (fb_prop.get_back_buffers() > 0)||
          (fb_prop.get_accum_bits() > 0)||
          (fb_prop.get_multisamples() > 0)) {
        return NULL;
      }
    }
    // Early success - if we are sure that this buffer WILL
    // meet specs, we can precertify it.
    if ((cocoagsg != 0) &&
        (cocoagsg->is_valid()) &&
        (!cocoagsg->needs_reset()) &&
        (cocoagsg->_supports_framebuffer_object) &&
        (cocoagsg->_glDrawBuffers != 0)&&
        (fb_prop.is_basic())) {
      precertify = true;
    }

    return new GLGraphicsBuffer(engine, this, name, fb_prop, win_prop,
                                flags, gsg, host);
  }

  // Third thing to try: a CocoaGraphicsBuffer
  if (retry == 2) {
    if (((flags&BF_require_parasite)!=0)||
        ((flags&BF_require_window)!=0)||
        ((flags&BF_resizeable)!=0)||
        ((flags&BF_size_track_host)!=0)) {
      return NULL;
    }

    if (!support_rtt) {
      if (((flags&BF_rtt_cumulative)!=0)||
          ((flags&BF_can_bind_every)!=0)) {
        // If we require Render-to-Texture, but can't be sure we
        // support it, bail.
        return NULL;
      }
    }

    return new CocoaGraphicsBuffer(engine, this, name, fb_prop, win_prop,
                                 flags, gsg, host);
  }
*/

  // Nothing else left to try.
  return NULL;
}
