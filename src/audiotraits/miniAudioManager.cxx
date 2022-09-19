/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file miniAudioManager.cxx
 * @author brian
 * @date 2022-09-06
 */

#include "miniAudioManager.h"
#include "miniaudio.h"
#include "virtualFileSystem.h"
#include "config_putil.h"
#include "virtualFile.h"
#include "pointerTo.h"
#include "memoryBase.h"
#include "config_miniaudio.h"
#include "miniAudioSound.h"
#include "nullAudioSound.h"
#include "dcast.h"
IMPLEMENT_CLASS(MiniAudioManager);

bool MiniAudioManager::_ma_initialized = false;
ma_engine *MiniAudioManager::_ma_engine = nullptr;
ma_vfs_callbacks *MiniAudioManager::_ma_vfs = nullptr;
ma_resource_manager *MiniAudioManager::_ma_rsrc_mgr = nullptr;
ma_device *MiniAudioManager::_ma_playback_device = nullptr;

/**
 *
 */
static void *
panda_ma_malloc(size_t size, void *user_data) {
  return PANDA_MALLOC_ARRAY(size);
}

/**
 *
 */
static void
panda_ma_free(void *ptr, void *user_data) {
  PANDA_FREE_ARRAY(ptr);
}

/**
 *
 */
static void *
panda_ma_realloc(void *ptr, size_t size, void *user_data) {
  return PANDA_REALLOC_ARRAY(ptr, size);
}

/**
 *
 */
class PandaMiniAudioFileHandle : public MemoryBase {
public:
  PT(VirtualFile) _vfile;
  ma_uint32 _open_mode;
  void *_stream;
};

/**
 *
 */
static ma_result
panda_ma_vfs_open(ma_vfs *mvfs, const char *file_path, ma_uint32 open_mode, ma_vfs_file *file) {
  VirtualFileSystem *vfs = VirtualFileSystem::get_global_ptr();

  PT(VirtualFile) vfile = vfs->get_file(Filename(file_path));
  if (vfile == nullptr) {
    return MA_DOES_NOT_EXIST;
  }

  void *stream = nullptr;
  if (open_mode == MA_OPEN_MODE_READ) {
    stream = vfile->open_read_file(true);

  } else if (open_mode == MA_OPEN_MODE_WRITE) {
    stream = vfile->open_write_file(true, true);
  }

  if (stream == nullptr) {
    return MA_ERROR;
  }

  // Store "file" as a custom object containing the VirtualFile and associated
  // open stream.
  PandaMiniAudioFileHandle *handle = new PandaMiniAudioFileHandle;
  handle->_vfile = vfile;
  handle->_stream = stream;
  handle->_open_mode = open_mode;
  *file = handle;

  return MA_SUCCESS;
}

/**
 *
 */
static ma_result
panda_ma_vfs_close(ma_vfs *mvfs, ma_vfs_file file) {
  PandaMiniAudioFileHandle *handle = (PandaMiniAudioFileHandle *)file;
  nassertr(handle != nullptr, MA_ERROR);
  nassertr(handle->_vfile != nullptr, MA_ERROR);
  nassertr(handle->_stream != nullptr, MA_ERROR);

  if (handle->_open_mode == MA_OPEN_MODE_READ) {
    handle->_vfile->close_read_file((std::istream *)handle->_stream);

  } else if (handle->_open_mode == MA_OPEN_MODE_WRITE) {
    handle->_vfile->close_write_file((std::ostream *)handle->_stream);
  }

  delete handle;

  return MA_SUCCESS;
}

/**
 *
 */
static ma_result
panda_ma_vfs_seek(ma_vfs *mvfs, ma_vfs_file file, ma_int64 offset, ma_seek_origin origin) {
  PandaMiniAudioFileHandle *handle = (PandaMiniAudioFileHandle *)file;

  nassertr(handle != nullptr, MA_ERROR);
  nassertr(handle->_vfile != nullptr, MA_ERROR);
  nassertr(handle->_stream != nullptr, MA_ERROR);

  std::ios::seekdir dir;
  switch (origin) {
  case ma_seek_origin_current:
    dir = std::ios::cur;
    break;
  case ma_seek_origin_start:
    dir = std::ios::beg;
    break;
  case ma_seek_origin_end:
    dir = std::ios::end;
    break;
  }

  if (handle->_open_mode == MA_OPEN_MODE_READ) {
    std::istream *stream = (std::istream *)handle->_stream;
    stream->clear();
    stream->seekg((std::streamoff)offset, dir);
    if (stream->fail() && !stream->eof()) {
      return MA_BAD_SEEK;
    }

  } else if (handle->_open_mode == MA_OPEN_MODE_WRITE) {
    std::ostream *stream = (std::ostream *)handle->_stream;
    stream->clear();
    stream->seekp((std::streamoff)offset, dir);
    if (stream->fail() && !stream->eof()) {
      return MA_BAD_SEEK;
    }

  } else {
    return MA_ERROR;
  }

  return MA_SUCCESS;
}

/**
 *
 */
static ma_result
panda_ma_vfs_tell(ma_vfs *mvfs, ma_vfs_file file, ma_int64 *cursor) {
  PandaMiniAudioFileHandle *handle = (PandaMiniAudioFileHandle *)file;

  nassertr(handle != nullptr, MA_ERROR);
  nassertr(handle->_vfile != nullptr, MA_ERROR);
  nassertr(handle->_stream != nullptr, MA_ERROR);

  if (handle->_open_mode == MA_OPEN_MODE_READ) {
    std::istream *stream = (std::istream *)handle->_stream;
    *cursor = stream->tellg();

  } else if (handle->_open_mode == MA_OPEN_MODE_WRITE) {
    std::ostream *stream = (std::ostream *)handle->_stream;
    *cursor = stream->tellp();

  } else {
    return MA_ERROR;
  }

  return MA_SUCCESS;
}

/**
 *
 */
static ma_result
panda_ma_vfs_info(ma_vfs *mvfs, ma_vfs_file file, ma_file_info *info) {
  PandaMiniAudioFileHandle *handle = (PandaMiniAudioFileHandle *)file;
  nassertr(handle != nullptr, MA_ERROR);
  nassertr(handle->_open_mode == MA_OPEN_MODE_READ, MA_ERROR);
  nassertr(handle->_vfile != nullptr, MA_ERROR);
  nassertr(handle->_stream != nullptr, MA_ERROR);
  info->sizeInBytes = handle->_vfile->get_file_size((std::istream *)handle->_stream);
  return MA_SUCCESS;
}

/**
 *
 */
static ma_result
panda_ma_vfs_read(ma_vfs *mvfs, ma_vfs_file file, void *dst, size_t size, size_t *bytes_read) {
  PandaMiniAudioFileHandle *handle = (PandaMiniAudioFileHandle *)file;

  std::istream *stream = (std::istream *)handle->_stream;
  stream->read((char *)dst, size);
  (*bytes_read) = stream->gcount();

  if (stream->eof()) {
    if ((*bytes_read) == 0) {
      return MA_AT_END;
    } else {
      return MA_SUCCESS;
    }
  } else if (stream->fail()) {
    return MA_BAD_SEEK;
  } else {
    return MA_SUCCESS;
  }
}

/**
 *
 */
static ma_result
panda_ma_vfs_write(ma_vfs *mvfs, ma_vfs_file file, const void *src, size_t size, size_t *bytes_written) {
  PandaMiniAudioFileHandle *handle = (PandaMiniAudioFileHandle *)file;

  std::ostream *stream = (std::ostream *)handle->_stream;
  stream->clear();
  stream->write((const char *)src, size);

  if (stream->eof()) {
    return MA_ERROR;
  } else if (stream->fail()) {
    return MA_BAD_SEEK;
  }

  *bytes_written = size;

  return MA_SUCCESS;
}

/**
 *
 */
MiniAudioManager::
MiniAudioManager() :
  _stream_mode(SM_sample),
  _sound_group(nullptr),
  _preload_threshold(miniaudio_preload_threshold)
{
  initialize_ma();

  // The design of the AudioManager is for grouping/categorizing sounds,
  // so we will create a ma_sound_group for each MiniAudioManager.
  _sound_group = (ma_sound_group *)PANDA_MALLOC_SINGLE(sizeof(ma_sound_group));
  ma_result result = ma_sound_group_init(_ma_engine, 0, nullptr, _sound_group);
  nassertv(result == MA_SUCCESS);
  //result = ma_sound_group_start(_sound_group);
  //nassertv(result == MA_SUCCESS);
}

/**
 *
 */
MiniAudioManager::
~MiniAudioManager() {
  if (_sound_group != nullptr) {
    ma_sound_group_uninit(_sound_group);
    PANDA_FREE_SINGLE(_sound_group);
    _sound_group = nullptr;
  }
}

/**
 *
 */
PT(AudioSound) MiniAudioManager::
get_sound(const Filename &filename, bool positional, StreamMode mode) {
  VirtualFileSystem *vfs = VirtualFileSystem::get_global_ptr();

  Filename resolved = filename;
  if (!vfs->resolve_filename(resolved, get_model_path().get_value())) {
    miniaudio_cat.warning()
      << "get_sound(): Could not find sound file " << filename << " on model-path "
      << get_model_path().get_value() << "\n";
    return get_null_sound();
  }

  PT(VirtualFile) vfile = vfs->get_file(resolved);
  if (vfile == nullptr) {
    return get_null_sound();
  }

  if (mode == SM_default) {
    mode = _stream_mode;
  }

  return new MiniAudioSound(vfile, positional, this, mode);
}

/**
 *
 */
PT(AudioSound) MiniAudioManager::
get_sound(AudioSound *sound) {
  AudioSound *null_sound = get_null_sound();
  if (sound == null_sound) {
    return null_sound;
  }
  nassertr(sound->is_of_type(MiniAudioSound::get_class_type()), get_null_sound());
  return new MiniAudioSound(DCAST(MiniAudioSound, sound), this);
}

/**
 *
 */
PT(AudioSound) MiniAudioManager::
get_sound(MovieAudio *source, bool positional, StreamMode mode) {
  return get_null_sound();
}

/**
 *
 */
void MiniAudioManager::
uncache_sound(const Filename &filename) {
}

/**
 *
 */
void MiniAudioManager::
clear_cache() {
}

/**
 *
 */
void MiniAudioManager::
set_cache_limit(unsigned int count) {
}

/**
 *
 */
unsigned int MiniAudioManager::
get_cache_limit() const {
  return 0;
}

/**
 *
 */
void MiniAudioManager::
set_volume(PN_stdfloat volume) {
  ma_sound_group_set_volume(_sound_group, volume);
}

/**
 *
 */
PN_stdfloat MiniAudioManager::
get_volume() const {
  return ma_sound_group_get_volume(_sound_group);
}

/**
 *
 */
bool MiniAudioManager::
is_valid() {
  return _ma_initialized && _sound_group != nullptr;
}

/**
 *
 */
void MiniAudioManager::
set_active(bool flag) {
}

/**
 *
 */
bool MiniAudioManager::
get_active() const {
  return true;
}

/**
 *
 */
void MiniAudioManager::
set_concurrent_sound_limit(unsigned int limit) {
}

/**
 *
 */
unsigned int MiniAudioManager::
get_concurrent_sound_limit() const {
  return 0;
}

/**
 *
 */
void MiniAudioManager::
reduce_sounds_playing_to(unsigned int count) {
}

/**
 *
 */
void MiniAudioManager::
stop_all_sounds() {
}

/**
 * Specifies how sounds loaded through this audio manager should be
 * accessed from disk.  It can be overridden on a per-sound basis,
 * but this setting determines the default stream mode.
 */
void MiniAudioManager::
set_stream_mode(StreamMode mode) {
  _stream_mode = mode;
}

/**
 * Returns the default StreamMode of the audio manager.  Sounds loaded
 * through this manager will be streamed/preloaded according to this
 * setting, but it can be optionally overridden on a per-sound basis.
 */
AudioManager::StreamMode MiniAudioManager::
get_stream_mode() const {
  return _stream_mode;
}

/**
 * When a sound or audio manager is using SM_heuristic, this determines
 * how big a sound must be for it to be streamed from disk, rather than
 * preloaded.  -1 means to never stream, 0 means to always stream.
 *
 * Specified in bytes.
 */
void MiniAudioManager::
set_preload_threshold(int bytes) {
  _preload_threshold = bytes;
}

/**
 * Returns the preload threshold of the audio manager.  When a sound or
 * audio manager is using SM_heuristic, this determine how big a sound must
 * be for it to be streamed from disk, rather than preloaded.
 *
 * Specified in bytes.  -1 means to never stream, 0 means to always stream.
 */
int MiniAudioManager::
get_preload_threshold() const {
  return _preload_threshold;
}

/**
 *
 */
void MiniAudioManager::
audio_3d_set_listener_attributes(PN_stdfloat px, PN_stdfloat py, PN_stdfloat pz,
                                 PN_stdfloat vx, PN_stdfloat vy, PN_stdfloat vz,
                                 PN_stdfloat fx, PN_stdfloat fy, PN_stdfloat fz,
                                 PN_stdfloat ux, PN_stdfloat uy, PN_stdfloat uz) {
  _listener_pos.set(px, py, pz);
  _listener_forward.set(fx, fy, fz);
  _listener_up.set(ux, uy, uz);
  _listener_velocity.set(vx, vy, vz);

  ma_engine_listener_set_position(_ma_engine, 0, px, pz, -py);
  ma_engine_listener_set_velocity(_ma_engine, 0, vx, vz, -vy);
  ma_engine_listener_set_direction(_ma_engine, 0, fx, fz, -fy);
  ma_engine_listener_set_world_up(_ma_engine, 0, ux, uz, -uy);
}

/**
 *
 */
void MiniAudioManager::
audio_3d_get_listener_attributes(PN_stdfloat *px, PN_stdfloat *py, PN_stdfloat *pz,
                                 PN_stdfloat *vx, PN_stdfloat *vy, PN_stdfloat *vz,
                                 PN_stdfloat *fx, PN_stdfloat *fy, PN_stdfloat *fz,
                                 PN_stdfloat *ux, PN_stdfloat *uy, PN_stdfloat *uz) {
  *px = _listener_pos[0];
  *py = _listener_pos[1];
  *pz = _listener_pos[2];
  *vx = _listener_velocity[0];
  *vy = _listener_velocity[1];
  *vz = _listener_velocity[2];
  *fx = _listener_forward[0];
  *fy = _listener_forward[1];
  *fz = _listener_forward[2];
  *ux = _listener_up[0];
  *uy = _listener_up[1];
  *uz = _listener_up[2];
}

/**
 *
 */
bool MiniAudioManager::
initialize_ma() {
  if (_ma_initialized) {
    return true;
  }

  _ma_initialized = true;

  ma_result result;

  ma_allocation_callbacks alloc_callbacks;
  alloc_callbacks.onMalloc = panda_ma_malloc;
  alloc_callbacks.onFree = panda_ma_free;
  alloc_callbacks.onRealloc = panda_ma_realloc;
  alloc_callbacks.pUserData = nullptr;

  _ma_vfs = (ma_vfs_callbacks *)PANDA_MALLOC_SINGLE(sizeof(ma_vfs_callbacks));
  _ma_vfs->onOpen = panda_ma_vfs_open;
  _ma_vfs->onOpenW = nullptr;
  _ma_vfs->onRead = panda_ma_vfs_read;
  _ma_vfs->onWrite = panda_ma_vfs_write;
  _ma_vfs->onClose = panda_ma_vfs_close;
  _ma_vfs->onSeek = panda_ma_vfs_seek;
  _ma_vfs->onTell = panda_ma_vfs_tell;
  _ma_vfs->onInfo = panda_ma_vfs_info;

  _ma_playback_device = (ma_device *)PANDA_MALLOC_SINGLE(sizeof(ma_device));
  ma_device_config dev_cfg = ma_device_config_init(ma_device_type_playback);
  dev_cfg.playback.format = ma_format_unknown;
  // Set up the device to use the configured channel count and sample rate
  // of the user.  Note that setting the config variables to 0 will use the
  // device's default.
  dev_cfg.playback.channels = miniaudio_num_channels;
  dev_cfg.sampleRate = miniaudio_sample_rate;
  result = ma_device_init(nullptr, &dev_cfg, _ma_playback_device);
  if (result != MA_SUCCESS) {
    miniaudio_cat.error()
      << "Failed to init device: " << result << "\n";
    return false;
  }

  if (miniaudio_cat.is_info()) {
    ma_device_info dev_info;
    result = ma_device_get_info(_ma_playback_device, ma_device_type_playback, &dev_info);
    nassertr(result == MA_SUCCESS, false);
    miniaudio_cat.info()
      << "Using playback device: " << dev_info.name << "\n"
      << "isDefault: " << dev_info.isDefault << "\n";
    miniaudio_cat.info()
      << dev_info.nativeDataFormatCount << " native data formats:\n";
    for (int i = 0; i < dev_info.nativeDataFormatCount; ++i) {
      miniaudio_cat.info(false)
        << dev_info.nativeDataFormats[i].channels << " channels, "
        << dev_info.nativeDataFormats[i].sampleRate << " Hz, "
        << dev_info.nativeDataFormats[i].format << " format\n";
    }
    miniaudio_cat.info()
      << "Using " << dev_cfg.playback.channels << " channels at " << dev_cfg.sampleRate
      << " Hz, format " << dev_cfg.playback.format << "\n";
  }

  _ma_rsrc_mgr = (ma_resource_manager *)PANDA_MALLOC_SINGLE(sizeof(ma_resource_manager));
  ma_resource_manager_config rsrc_cfg = ma_resource_manager_config_init();
  if (miniaudio_decode_to_device_format) {
    rsrc_cfg.decodedChannels = dev_cfg.playback.channels;
    rsrc_cfg.decodedFormat = dev_cfg.playback.format;
    rsrc_cfg.decodedSampleRate = dev_cfg.sampleRate;
  }
  rsrc_cfg.pVFS = _ma_vfs;
  result = ma_resource_manager_init(&rsrc_cfg, _ma_rsrc_mgr);
  if (result != MA_SUCCESS) {
    miniaudio_cat.error()
      << "Failed to init resource manager: " << result << "\n";
    return false;
  }

  _ma_engine = (ma_engine *)PANDA_MALLOC_SINGLE(sizeof(ma_engine));
  ma_engine_config ma_eng_cfg = ma_engine_config_init();
  ma_eng_cfg.allocationCallbacks = alloc_callbacks;
  ma_eng_cfg.pResourceManager = _ma_rsrc_mgr;
  result = ma_engine_init(&ma_eng_cfg, _ma_engine);

  if (result == MA_SUCCESS) {
    miniaudio_cat.info()
      << "Successfully initialized miniaudio " << ma_version_string() << "\n";
    return true;

  } else {
    miniaudio_cat.error()
      << "Failed to initialize engine: " << result << "\n";
    return false;
  }
}

/**
 *
 */
AudioManager *
Create_MiniAudioManager() {
  return new MiniAudioManager;
}
