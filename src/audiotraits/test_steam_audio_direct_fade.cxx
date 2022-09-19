/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file test_steam_audio_direct_fade.cxx
 * @author brian
 * @date 2022-09-16
 */

#include <phonon.h>
#include <iostream>
#include <assert.h>

int
main(int argc, char *argv[]) {
  IPLContextSettings ctx_settings{};
  ctx_settings.version = STEAMAUDIO_VERSION;
  ctx_settings.simdLevel = IPL_SIMDLEVEL_AVX2;
  IPLContext ctx = nullptr;
  IPLerror err = iplContextCreate(&ctx_settings, &ctx);
  assert(!err);

  IPLAudioSettings audio_settings{};
  audio_settings.samplingRate = 44100;
  audio_settings.frameSize = 1024;

  IPLDirectEffectSettings effect_settings{};
  effect_settings.numChannels = 1;
  IPLDirectEffect effect = nullptr;
  err = iplDirectEffectCreate(ctx, &audio_settings, &effect_settings, &effect);
  assert(!err);

  IPLAudioBuffer in_buffer;
  IPLAudioBuffer out_buffer;

  err = iplAudioBufferAllocate(ctx, 1, 1024, &in_buffer);
  assert(!err);
  err = iplAudioBufferAllocate(ctx, 1, 1024, &out_buffer);
  assert(!err);

  for (int i = 0; i < 1024; ++i) {
    in_buffer.data[0][i] = 100.0f;
  }

  IPLDirectEffectParams params{};
  params.flags = IPL_DIRECTEFFECTFLAGS_APPLYDISTANCEATTENUATION;
  params.distanceAttenuation = 1.0f;

  for (int i = 0; i < 20; ++i) {
    iplDirectEffectApply(effect, &params, &in_buffer, &out_buffer);
    std::cout << "First sample in out buffer: " << out_buffer.data[0][0] << "\n";
  }

  return 0;
}
