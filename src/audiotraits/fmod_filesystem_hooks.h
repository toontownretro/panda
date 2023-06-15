/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file fmod_filesystem_hooks.h
 * @author brian
 * @date 2023-06-09
 */

#ifndef FMOD_FILESYSTEM_HOOKS_H
#define FMOD_FILESYSTEM_HOOKS_H

#include <fmod.hpp>

extern FMOD_RESULT F_CALLBACK
pfmod_open_callback(const char *name, unsigned int *file_size,
                    void **handle, void *user_data);

extern FMOD_RESULT F_CALLBACK
pfmod_close_callback(void *handle, void *user_data);

extern FMOD_RESULT F_CALLBACK
pfmod_read_callback(void *handle, void *buffer, unsigned int size_bytes,
                    unsigned int *bytes_read, void *user_data);

extern FMOD_RESULT F_CALLBACK
pfmod_seek_callback(void *handle, unsigned int pos, void *user_data);

#endif // FMOD_FILESYSTEM_HOOKS_H
