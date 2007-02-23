// Filename: graphicsOutput.h
// Created by:  drose (06Feb04)
//
////////////////////////////////////////////////////////////////////
//
// PANDA 3D SOFTWARE
// Copyright (c) 2001 - 2004, Disney Enterprises, Inc.  All rights reserved
//
// All use of this software is subject to the terms of the Panda 3d
// Software license.  You should have received a copy of this license
// along with this source code; you will also find a current copy of
// the license at http://etc.cmu.edu/panda3d/docs/license/ .
//
// To contact the maintainers of this program write to
// panda3d-general@lists.sourceforge.net .
//
////////////////////////////////////////////////////////////////////

#ifndef GRAPHICSOUTPUT_H
#define GRAPHICSOUTPUT_H

#include "pandabase.h"

#include "graphicsPipe.h"
#include "displayRegion.h"
#include "graphicsStateGuardian.h"
#include "drawableRegion.h"
#include "renderBuffer.h"

#include "typedWritableReferenceCount.h"
#include "pandaNode.h"
#include "pStatCollector.h"
#include "pnotify.h"
#include "pmutex.h"
#include "filename.h"
#include "drawMask.h"
#include "pvector.h"
#include "weakPointerTo.h"
#include "nodePath.h"

class PNMImage;

////////////////////////////////////////////////////////////////////
//       Class : GraphicsOutput
// Description : This is a base class for the various different
//               classes that represent the result of a frame of
//               rendering.  The most common kind of GraphicsOutput is
//               a GraphicsWindow, which is a real-time window on the
//               desktop, but another example is GraphicsBuffer, which
//               is an offscreen buffer.
//
//               The actual rendering, and anything associated with
//               the graphics context itself, is managed by the
//               associated GraphicsStateGuardian (which might output
//               to multiple GraphicsOutput objects).
//
//               GraphicsOutputs are not actually writable to bam
//               files, of course, but they may be passed as event
//               parameters, so they inherit from
//               TypedWritableReferenceCount instead of
//               TypedReferenceCount for that convenience.
////////////////////////////////////////////////////////////////////
class EXPCL_PANDA GraphicsOutput : public TypedWritableReferenceCount, public DrawableRegion {
protected:
  GraphicsOutput(GraphicsPipe *pipe, 
                 const string &name,
                 const FrameBufferProperties &fb_prop,
                 const WindowProperties &win_prop, int flags,
                 GraphicsStateGuardian *gsg,
                 GraphicsOutput *host);

private:
  GraphicsOutput(const GraphicsOutput &copy);
  void operator = (const GraphicsOutput &copy);

PUBLISHED:
  enum RenderTextureMode {
    RTM_none,
    RTM_bind_or_copy,
    RTM_copy_texture,
    RTM_copy_ram,
    RTM_triggered_copy_texture,
    RTM_triggered_copy_ram,
  };

  // It seems awkward to have this type, and also
  // RenderBuffer::Type.  However, the fact that RenderBuffer::Type
  // is a bitmask makes it awfully awkward to work with.
  enum RenderTexturePlane {
    RTP_depth_stencil=1,
    RTP_depth=1,
    RTP_color,
    RTP_aux_rgba_0,
    RTP_aux_rgba_1,
    RTP_aux_rgba_2,
    RTP_aux_rgba_3,
    RTP_aux_hrgba_0,
    RTP_aux_hrgba_1,
    RTP_aux_hrgba_2,
    RTP_aux_hrgba_3,
    RTP_aux_float_0,
    RTP_aux_float_1,
    RTP_aux_float_2,
    RTP_aux_float_3,
    RTP_COUNT
  };

  // There are many reasons to call begin_frame/end_frame.
  enum FrameMode {
    FM_render,   // We are rendering a frame.
    FM_parasite, // We are rendering a frame of a parasite.
    FM_refresh,  // We are just refreshing the display or exposing the window.
  };

  virtual ~GraphicsOutput();

  INLINE GraphicsStateGuardian *get_gsg() const;
  INLINE GraphicsPipe *get_pipe() const;
  INLINE const string &get_name() const;

  INLINE int count_textures() const;
  INLINE bool has_texture() const;
  INLINE Texture *get_texture(int i=0) const;
  INLINE RenderTexturePlane get_texture_plane(int i=0) const;
  INLINE RenderTextureMode get_rtm_mode(int i=0) const;
  void clear_render_textures();
  void add_render_texture(Texture *tex, RenderTextureMode mode, 
                          RenderTexturePlane bitplane=RTP_COUNT);
  void setup_render_texture(Texture *tex, bool allow_bind, bool to_ram);

  INLINE int get_x_size() const;
  INLINE int get_y_size() const;
  INLINE bool has_size() const;
  INLINE bool is_valid() const;

  void set_active(bool active);
  virtual bool is_active() const;

  INLINE void set_one_shot(bool one_shot);
  INLINE bool get_one_shot() const;

  void set_inverted(bool inverted);
  INLINE bool get_inverted() const;

  INLINE void set_red_blue_stereo(bool red_blue_stereo,
                                  unsigned int left_eye_color_mask,
                                  unsigned int right_eye_color_mask);
  INLINE bool get_red_blue_stereo() const;
  INLINE unsigned int get_left_eye_color_mask() const;
  INLINE unsigned int get_right_eye_color_mask() const;

  INLINE FrameBufferProperties &get_fb_properties();
  INLINE bool is_stereo() const;

  INLINE void clear_delete_flag();
  INLINE bool get_delete_flag() const;

  void set_sort(int sort);
  INLINE int get_sort() const;

  INLINE void trigger_copy();
  
  INLINE DisplayRegion *make_display_region();
  INLINE DisplayRegion *make_display_region(float l, float r,
                                            float b, float t);
  bool remove_display_region(DisplayRegion *display_region);
  void remove_all_display_regions();

  int get_num_display_regions() const;
  PT(DisplayRegion) get_display_region(int n) const;

  int get_num_active_display_regions() const;
  PT(DisplayRegion) get_active_display_region(int n) const;

  GraphicsOutput *make_texture_buffer(
      const string &name, int x_size, int y_size,
      Texture *tex = NULL, bool to_ram = false);
  GraphicsOutput *make_cube_map(const string &name, int size,
                                NodePath &camera_rig,
                                DrawMask camera_mask = PandaNode::get_all_camera_mask(),
                                bool to_ram = false);

  INLINE static Filename make_screenshot_filename(
      const string &prefix = "screenshot");
  INLINE Filename save_screenshot_default(
      const string &prefix = "screenshot");
  INLINE bool save_screenshot(
      const Filename &filename, const string &image_comment = "");
  INLINE bool get_screenshot(PNMImage &image);

  NodePath get_texture_card();

public:
  // These are not intended to be called directly by the user.
  INLINE bool flip_ready() const;

  INLINE bool operator < (const GraphicsOutput &other) const;

  virtual GraphicsOutput *get_host();

  virtual void request_open();
  virtual void request_close();

  virtual void set_close_now();
  virtual void reset_window(bool swapchain);

  void set_size_and_recalc(int x, int y);
  
  // It is an error to call any of the following methods from any
  // thread other than the draw thread.  These methods are normally
  // called by the GraphicsEngine.
  void clear(Thread *current_thread);
  virtual bool begin_frame(FrameMode mode, Thread *current_thread);
  virtual void end_frame(FrameMode mode, Thread *current_thread);

  // These entry points have been removed. Use begin_frame/end_frame instead.
  //  virtual void begin_render_texture();
  //  virtual void end_render_texture();
  //  INLINE bool needs_context() const;
  //  bool _needs_context;
  //  virtual bool make_context();
  //  virtual void make_current();
  //  virtual void auto_resize();

  void change_scenes(DisplayRegionPipelineReader *new_dr);
  virtual void select_cube_map(int cube_map_index);

  // These methods will be called within the app (main) thread.
  virtual void begin_flip();
  virtual void end_flip();

  // It is an error to call any of the following methods from any
  // thread other than the window thread.  These methods are normally
  // called by the GraphicsEngine.
  virtual void process_events();

  INLINE PStatCollector &get_cull_window_pcollector();
  INLINE PStatCollector &get_draw_window_pcollector();

protected:

  void prepare_for_deletion();
  void copy_to_textures();
  
  INLINE void begin_frame_spam();
  INLINE void end_frame_spam();
  INLINE void clear_cube_map_selection();
  INLINE void trigger_flip();

protected:

  class RenderTexture {
  public:
    PT(Texture) _texture;
    RenderTexturePlane _plane;
    RenderTextureMode _rtm_mode;
  };
  PT(GraphicsStateGuardian) _gsg;
  PT(GraphicsPipe) _pipe;
  PT(GraphicsOutput) _host;
  FrameBufferProperties _fb_properties;
  bool _stereo;
  string _name;
  pvector<RenderTexture> _textures;
  bool _flip_ready;
  int _cube_map_index;
  DisplayRegion *_cube_map_dr;
  PT(Geom) _texture_card;
  bool _trigger_copy;
  
private:
  PT(GeomVertexData) create_texture_card_vdata(int x, int y);
  
  DisplayRegion *add_display_region(DisplayRegion *display_region);

  INLINE void win_display_regions_changed();

  INLINE void determine_display_regions() const;
  void do_determine_display_regions();
  
  int _sort;
  unsigned int _internal_sort_index;

protected:
  bool _active;
  bool _one_shot;
  bool _inverted;
  bool _red_blue_stereo;
  unsigned int _left_eye_color_mask;
  unsigned int _right_eye_color_mask;
  bool _delete_flag;

  // These weak pointers are used to keep track of whether the
  // buffer's bound Texture has been deleted or not.  Until they have,
  // we don't auto-close the buffer (since that would deallocate the
  // memory associated with the texture).
  pvector<WPT(Texture)> _hold_textures;
  
protected:
  Mutex _lock; 
  // protects _display_regions.
  PT(DisplayRegion) _default_display_region;
  typedef pvector< PT(DisplayRegion) > TotalDisplayRegions;
  TotalDisplayRegions _total_display_regions;
  typedef pvector<DisplayRegion *> ActiveDisplayRegions;
  ActiveDisplayRegions _active_display_regions;
  bool _display_regions_stale;

protected:
  int _creation_flags;
  int _x_size;
  int _y_size;
  bool _has_size;
  bool _is_valid;

  static PStatCollector _make_current_pcollector;
  static PStatCollector _copy_texture_pcollector;
  static PStatCollector _cull_pcollector;
  static PStatCollector _draw_pcollector;
  PStatCollector _cull_window_pcollector;
  PStatCollector _draw_window_pcollector;
  
public:
  static TypeHandle get_class_type() {
    return _type_handle;
  }
  static void init_type() {
    TypedWritableReferenceCount::init_type();
    register_type(_type_handle, "GraphicsOutput",
                  TypedWritableReferenceCount::get_class_type());
  }
  virtual TypeHandle get_type() const {
    return get_class_type();
  }
  virtual TypeHandle force_init_type() {init_type(); return get_class_type();}

private:
  static TypeHandle _type_handle;

  friend class GraphicsPipe;
  friend class GraphicsEngine;
  friend class DisplayRegion;
};

#include "graphicsOutput.I"

#endif /* GRAPHICSOUTPUT_H */
