/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file fmod_filesystem_hooks.cxx
 * @author brian
 * @date 2023-06-09
 */

#include "fmod_filesystem_hooks.h"
#include "virtualFileSystem.h"
#include "virtualFile.h"
#include "config_fmodAudio.h"
#include <istream>

/**
 * A hook into Panda's virtual file system.
 */
FMOD_RESULT F_CALLBACK
pfmod_open_callback(const char *name, unsigned int *file_size,
                    void **handle, void *user_data) {
  // We actually pass in the VirtualFile pointer as the "name".
  VirtualFile *file = (VirtualFile *)user_data;
  if (file == nullptr) {
    return FMOD_ERR_FILE_NOTFOUND;
  }
  if (fmodAudio_cat.is_spam()) {
    fmodAudio_cat.spam()
      << "open_callback(" << *file << ")\n";
  }

  std::istream *str = file->open_read_file(true);

  (*file_size) = file->get_file_size(str);
  (*handle) = (void *)str;

  // Explicitly ref the VirtualFile since we're storing it in a void pointer
  // instead of a PT(VirtualFile).
  file->ref();

  return FMOD_OK;
}

/**
 * A hook into Panda's virtual file system.
 */
FMOD_RESULT F_CALLBACK
pfmod_close_callback(void *handle, void *user_data) {
  VirtualFile *file = (VirtualFile *)user_data;
  if (fmodAudio_cat.is_spam()) {
    fmodAudio_cat.spam()
      << "close_callback(" << *file << ")\n";
  }

  VirtualFileSystem *vfs = VirtualFileSystem::get_global_ptr();

  std::istream *str = (std::istream *)handle;
  vfs->close_read_file(str);

  // Explicitly unref the VirtualFile pointer.
  unref_delete(file);

  return FMOD_OK;
}

/**
 * A hook into Panda's virtual file system.
 */
FMOD_RESULT F_CALLBACK
pfmod_read_callback(void *handle, void *buffer, unsigned int size_bytes,
                    unsigned int *bytes_read, void *user_data) {
  VirtualFile *file = (VirtualFile *)user_data;
  if (fmodAudio_cat.is_spam()) {
    fmodAudio_cat.spam()
      << "read_callback(" << *file << ", " << size_bytes << ")\n";
  }

  std::istream *str = (std::istream *)handle;
  str->read((char *)buffer, size_bytes);
  (*bytes_read) = str->gcount();

  // We can't yield here, since this callback is made within a sub-thread--an
  // OS-level sub-thread spawned by FMod, not a Panda thread.  But we will
  // only execute this code in the true-threads case anyway.
  // thread_consider_yield();

  if (str->eof()) {
    if ((*bytes_read) == 0) {
      return FMOD_ERR_FILE_EOF;
    } else {
      // Report the EOF next time.
      return FMOD_OK;
    }
  } if (str->fail()) {
    return FMOD_ERR_FILE_BAD;
  } else {
    return FMOD_OK;
  }
}

/**
 * A hook into Panda's virtual file system.
 */
FMOD_RESULT F_CALLBACK
pfmod_seek_callback(void *handle, unsigned int pos, void *user_data) {
  VirtualFile *file = (VirtualFile *)user_data;
  if (fmodAudio_cat.is_spam()) {
    fmodAudio_cat.spam()
      << "seek_callback(" << *file << ", " << pos << ")\n";
  }

  std::istream *str = (std::istream *)handle;
  str->clear();
  str->seekg(pos);

  if (str->fail() && !str->eof()) {
    return FMOD_ERR_FILE_COULDNOTSEEK;
  } else {
    return FMOD_OK;
  }
}
