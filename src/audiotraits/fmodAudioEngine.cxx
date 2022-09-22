/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file fmodAudioEngine.cxx
 * @author brian
 * @date 2022-09-19
 */

#include "fmodAudioEngine.h"
#include "config_fmodAudio.h"
#include "fmodAudioManager.h"
#include "dsp.h"
#include "chorusDSP.h"
#include "compressorDSP.h"
#include "distortionDSP.h"
#include "echoDSP.h"
#include "faderDSP.h"
#include "flangeDSP.h"
#include "highpassDSP.h"
#include "limiterDSP.h"
#include "lowpassDSP.h"
#include "normalizeDSP.h"
#include "oscillatorDSP.h"
#include "paramEQDSP.h"
#include "pitchShiftDSP.h"
#include "sfxReverbDSP.h"
#include "dcast.h"
#include "load_dso.h"

IMPLEMENT_CLASS(FMODAudioEngine);

#define FMOD_MIN_SAMPLE_RATE 8000
#define FMOD_MAX_SAMPLE_RATE 192000

#ifdef HAVE_STEAM_AUDIO

#include <phonon/phonon.h>

typedef void (*PFNIPLFMODINITIALIZE)(IPLContext);
typedef void (*PFNIPLFMODSETHRTF)(IPLHRTF);
typedef void (*PFNIPLFMODSETSIMULATIONSETTINGS)(IPLSimulationSettings);
typedef void (*PFNIPLFMODSETREVERBSOURCE)(IPLSource);

/**
 *
 */
std::string
_ipl_errstring(IPLerror err) {
  switch (err) {
  case IPL_STATUS_SUCCESS:
    return "The operation completed successfully.";
  case IPL_STATUS_FAILURE:
    return "An unspecified error occurred.";
  case IPL_STATUS_OUTOFMEMORY:
    return "The system ran out of memory.";
  case IPL_STATUS_INITIALIZATION:
    return "An error occurred while initializing an external dependency.";
  default:
    return "Unknown error code.";
  }
}

/**
 *
 */
bool _ipl_errcheck(const std::string &context, IPLerror err) {
  if (err != IPL_STATUS_SUCCESS) {
    fmodAudio_cat.error() << "IPL error, context: " << context << ", error: " << _ipl_errstring(err) << "\n";
    return false;
  }
  return true;
}
#define IPL_ERRCHECK(context, n) _ipl_errcheck(context, n)

#endif

/**
 * Redirects an FMOD log message to our notify category.
 */
static FMOD_RESULT
fmod_panda_log(FMOD_DEBUG_FLAGS flags, const char *file, int line, const char* func, const char* message) {
  std::ostream *str;
  if (flags & FMOD_DEBUG_LEVEL_ERROR) {
    str = &fmodAudio_cat.error();
  } else if (flags & FMOD_DEBUG_LEVEL_WARNING) {
    str = &fmodAudio_cat.warning();
  } else {
    str = &fmodAudio_cat.info();
  }

  (*str)
    << "FMOD: " << std::string(message) << " at line " << line << " of "
    << std::string(file) << ", " << std::string(func) << "\n";

  return FMOD_OK;
}

/**
 *
 */
static void *
fmod_panda_malloc(unsigned int size, FMOD_MEMORY_TYPE type, const char *sourcestr) {
  return FMODAudioEngine::get_class_type().allocate_array(size);
}

/**
 *
 */
static void *
fmod_panda_realloc(void *ptr, unsigned int size, FMOD_MEMORY_TYPE type, const char *sourcestr) {
  return FMODAudioEngine::get_class_type().reallocate_array(ptr, size);
}

/**
 *
 */
static void
fmod_panda_free(void *ptr, FMOD_MEMORY_TYPE type, const char *sourcestr) {
  FMODAudioEngine::get_class_type().deallocate_array(ptr);
}

/**
 *
 */
FMOD_VECTOR
lvec_to_fmod(const LVecBase3 &vec) {
  FMOD_VECTOR ret;
  ret.x = vec[0];
  ret.y = vec[2];
  ret.z = vec[1];
  return ret;
}

/**
 *
 */
std::string
fmod_output_type_string(FMOD_OUTPUTTYPE type) {
  switch (type) {
  case FMOD_OUTPUTTYPE_AUTODETECT:
    return "auto detect";
  case FMOD_OUTPUTTYPE_NOSOUND:
    return "no sound";
  case FMOD_OUTPUTTYPE_WAVWRITER:
    return "wav writer";
  case FMOD_OUTPUTTYPE_NOSOUND_NRT:
    return "no sound (nrt)";
  case FMOD_OUTPUTTYPE_WAVWRITER_NRT:
    return "wav writer (nrt)";
  case FMOD_OUTPUTTYPE_WASAPI:
    return "WASAPI";
  case FMOD_OUTPUTTYPE_ASIO:
    return "ASIO";
  case FMOD_OUTPUTTYPE_PULSEAUDIO:
    return "PulseAudio";
  case FMOD_OUTPUTTYPE_ALSA:
    return "ALSA";
  case FMOD_OUTPUTTYPE_COREAUDIO:
    return "CoreAudio";
  case FMOD_OUTPUTTYPE_AUDIOTRACK:
    return "AudioTrack";
  case FMOD_OUTPUTTYPE_OPENSL:
    return "OpenSL";
  case FMOD_OUTPUTTYPE_AUDIOOUT:
    return "AudioOut";
  case FMOD_OUTPUTTYPE_AUDIO3D:
    return "Audio3D";
  case FMOD_OUTPUTTYPE_WEBAUDIO:
    return "WebAudio";
  case FMOD_OUTPUTTYPE_NNAUDIO:
    return "NNAudio";
  case FMOD_OUTPUTTYPE_WINSONIC:
    return "WinSonic";
  case FMOD_OUTPUTTYPE_AAUDIO:
    return "AAudio";
  case FMOD_OUTPUTTYPE_AUDIOWORKLET:
    return "AudioWorklet";
  case FMOD_OUTPUTTYPE_UNKNOWN:
    return "unknown";
  default:
    return "invalid";
  }
}

/**
 *
 */
std::string
fmod_speaker_mode_string(FMOD_SPEAKERMODE mode) {
  switch (mode) {
  case FMOD_SPEAKERMODE_DEFAULT:
    return "default";
  case FMOD_SPEAKERMODE_RAW:
    return "raw";
  case FMOD_SPEAKERMODE_MONO:
    return "mono";
  case FMOD_SPEAKERMODE_STEREO:
    return "stereo";
  case FMOD_SPEAKERMODE_QUAD:
    return "quad";
  case FMOD_SPEAKERMODE_SURROUND:
    return "surround";
  case FMOD_SPEAKERMODE_5POINT1:
    return "5.1";
  case FMOD_SPEAKERMODE_7POINT1:
    return "7.1";
  case FMOD_SPEAKERMODE_7POINT1POINT4:
    return "7.1.4";
  default:
    return "invalid";
  }
}

/**
 *
 */
FMODAudioEngine::
FMODAudioEngine() :
  _system(nullptr),
  _master_channel_group(nullptr),
  _unit_scale(1.0f)
{
}

/**
 *
 */
FMODAudioEngine::
~FMODAudioEngine() {
#ifdef HAVE_STEAM_AUDIO
  shutdown_steam_audio();
#endif
  _master_channel_group = nullptr;
  if (_system != nullptr) {
    _system->close();
    _system->release();
  }
}

/**
 *
 */
bool FMODAudioEngine::
initialize() {
  if (_system != nullptr) {
    return true;
  }

  // Create the global FMOD System object.  This one object must be shared
  // by all FmodAudioManagers (this is particularly true on OSX, but the
  // FMOD documentation is unclear as to whether this is the intended design
  // on all systems).

  FMOD_RESULT result;

  result = FMOD::Memory_Initialize(nullptr, 0, fmod_panda_malloc, fmod_panda_realloc,
                                   fmod_panda_free, FMOD_MEMORY_ALL);
  if (!fmod_audio_errcheck("FMOD::Memory_Initialize", result)) {
    return false;
  }

  result = FMOD::System_Create(&_system);
  if (!fmod_audio_errcheck("FMOD::System_Create", result)) {
    return false;
  }

  if (fmod_debug) {
    fmodAudio_cat.info()
      << "Enabling FMOD debugging (will only take effect if you linked with libfmodL)\n";
    unsigned int debug_flags =
      FMOD_DEBUG_LEVEL_LOG | FMOD_DEBUG_LEVEL_WARNING | FMOD_DEBUG_LEVEL_ERROR | FMOD_DEBUG_TYPE_TRACE |
      FMOD_DEBUG_TYPE_FILE | FMOD_DEBUG_DISPLAY_LINENUMBERS;
    result = FMOD::Debug_Initialize(debug_flags, FMOD_DEBUG_MODE_CALLBACK, fmod_panda_log, nullptr);
    if (!fmod_audio_errcheck("FMOD::Debug_Initialize", result)) {
      return false;
    }
  }

  // Lets check the version of FMOD to make sure the headers and libraries
  // are correct.
  unsigned int version;
  result = _system->getVersion(&version);
  if (!fmod_audio_errcheck("_system->getVersion()", result)) {
    return false;
  }

  if (version < FMOD_VERSION) {
    fmodAudio_cat.error()
      << "You are using an old version of FMOD.  This program requires: " << FMOD_VERSION << "\n";
    return false;
  }

  // Determine the sample rate and speaker mode for the system.  We will use
  // the default configuration that FMOD chooses unless the user specifies
  // custom values via config variables.

  int sample_rate;
  FMOD_SPEAKERMODE speaker_mode;
  int num_raw_speakers;
  _system->getSoftwareFormat(&sample_rate,
                             &speaker_mode,
                             &num_raw_speakers);

  audio_debug("fmod-mixer-sample-rate: " << fmod_mixer_sample_rate);
  if (fmod_mixer_sample_rate.get_value() != -1) {
    if (fmod_mixer_sample_rate.get_value() >= FMOD_MIN_SAMPLE_RATE &&
        fmod_mixer_sample_rate.get_value() <= FMOD_MAX_SAMPLE_RATE) {
        sample_rate = fmod_mixer_sample_rate;
        audio_debug("Using user specified sample rate");
    } else {
      fmodAudio_cat.warning()
        << "fmod-mixer-sample-rate had an out-of-range value: "
        << fmod_mixer_sample_rate
        << ". Valid range is [" << FMOD_MIN_SAMPLE_RATE << ", "
        << FMOD_MAX_SAMPLE_RATE << "]\n";
    }
  }

  if (fmod_speaker_mode == FSM_unspecified) {
    if (fmod_use_surround_sound) {
      // fmod-use-surround-sound is the old variable, now replaced by fmod-
      // speaker-mode.  This is for backward compatibility.
      speaker_mode = FMOD_SPEAKERMODE_5POINT1;
    }
  } else {
    speaker_mode = (FMOD_SPEAKERMODE)fmod_speaker_mode.get_value();
  }

  fmodAudio_cat.info()
    << "Using software format: " << sample_rate << " Hz, " << fmod_speaker_mode_string(speaker_mode)
    << " speaker mode\n";

  // Set the mixer and speaker format.
  result = _system->setSoftwareFormat(sample_rate, speaker_mode,
                                      num_raw_speakers);
  if (!fmod_audio_errcheck("_system->setSoftwareFormat()", result)) {
    return false;
  }

  fmodAudio_cat.info()
    << "Using DSP buffer size " << fmod_dsp_buffer_size << " * " << fmod_dsp_buffer_count << "\n";

  result = _system->setDSPBufferSize(fmod_dsp_buffer_size, fmod_dsp_buffer_count);
  if (!fmod_audio_errcheck("_system->setDSPBufferSize()", result)) {
    return false;
  }

  // Now initialize the system.
  int nchan = fmod_number_of_sound_channels;
  int flags = FMOD_INIT_NORMAL;
  if (fmod_profile) {
    fmodAudio_cat.info()
      << "Enabling FMOD profiling, connect to application with FMOD "
      << "profiling tool\n";
    flags |= FMOD_INIT_PROFILE_ENABLE | FMOD_INIT_PROFILE_METER_ALL;
  }

  result = _system->init(nchan, flags, 0);
  if (result == FMOD_ERR_TOOMANYCHANNELS) {
    fmodAudio_cat.error()
      << "Value too large for fmod-number-of-sound-channels: " << nchan
      << "\n";
    return false;

  } else if (!fmod_audio_errcheck("_system->init()", result)) {
    return false;
  }

  fmodAudio_cat.info()
    << "FMOD initialized successfully\n";

  // Query default output device for logging purposes.
  int driver;
  result = _system->getDriver(&driver);
  if (!fmod_audio_errcheck("_system->getDriver()", result)) {
    return false;
  }
  char driver_name[256];
  int driver_rate;
  int driver_num_channels;
  FMOD_SPEAKERMODE driver_speaker_mode;
  result = _system->getDriverInfo(driver, driver_name, 256, nullptr, &driver_rate,
                                  &driver_speaker_mode, &driver_num_channels);
  if (!fmod_audio_errcheck("_system->getDriverInfo()", result)) {
    return false;
  }

  fmodAudio_cat.info()
    << "Using default output device " << std::string(driver_name) << ":\n"
    << "\tNative sample rate: " << driver_rate << " Hz\n"
    << "\tNative speaker mode: " << fmod_speaker_mode_string(driver_speaker_mode) << "\n"
    << "\tNative channel count: " << driver_num_channels << "\n";

  FMOD_OUTPUTTYPE output_type;
  result = _system->getOutput(&output_type);
  if (!fmod_audio_errcheck("_system->getOutput()", result)) {
    return false;
  }
  fmodAudio_cat.info()
    << "Using output type: " << fmod_output_type_string(output_type) << "\n";

  // Cache the pathname to the DLS file for the software MIDI synth.
  Filename dls_pathname = AudioManager::get_dls_pathname();

#ifdef IS_OSX
  // Here's a big kludge.  Don't ever let FMOD try to load this OSX-provided
  // file; it crashes messily if you do.
  // FIXME: Is this still true on FMOD Core?
  if (dls_pathname == "/System/Library/Components/CoreAudio.component/Contents/Resources/gs_instruments.dls") {
    dls_pathname = "";
  }
#endif  // IS_OSX

  if (!dls_pathname.empty()) {
    _dls_name = dls_pathname.to_os_specific();
  }

  fmodAudio_cat.info()
    << "Software MIDI DLS file: " << _dls_name << "\n";

  _sound_cache = new FMODSoundCache(this);
  _sound_cache->initialize();

  result = _system->getMasterChannelGroup(&_master_channel_group);
  if (!fmod_audio_errcheck("_system->getMasterChannelGroup()", result)) {
    return false;
  }

#ifdef HAVE_STEAM_AUDIO
  if (fmod_use_steam_audio) {
    if (!init_steam_audio()) {
      fmodAudio_cat.error()
        << "Failed to initialize Steam Audio\n";
      shutdown_steam_audio();
    }
  }
#endif

  return (result == FMOD_OK);
}

#ifdef HAVE_STEAM_AUDIO
/**
 *
 */
bool FMODAudioEngine::
init_steam_audio() {

  IPLContextSettings ctx_settings{};
  ctx_settings.version = STEAMAUDIO_VERSION;
  ctx_settings.simdLevel = IPL_SIMDLEVEL_AVX2;
  //ctx_settings.allocateCallback = ipl_panda_malloc;
  //ctx_settings.freeCallback = ipl_panda_free;
  //ctx_settings.logCallback = ipl_panda_log;
  IPLerror err;
  err = iplContextCreate(&ctx_settings, &_ipl_context);
  if (!IPL_ERRCHECK("create context", err)) {
    return false;
  }

  IPLAudioSettings audio_settings{};
  audio_settings.frameSize = fmod_dsp_buffer_size;
  audio_settings.samplingRate = fmod_mixer_sample_rate;

  IPLHRTFSettings hrtf_settings{};
  hrtf_settings.type = IPL_HRTFTYPE_DEFAULT;
  err = iplHRTFCreate(_ipl_context, &audio_settings, &hrtf_settings, &_ipl_hrtf);
  if (!IPL_ERRCHECK("create HRTF", err)) {
    return false;
  }
  IPLSimulationSettings sim_settings{};
  sim_settings.samplingRate = fmod_mixer_sample_rate;
  sim_settings.frameSize = fmod_dsp_buffer_size;
  sim_settings.flags = (IPLSimulationFlags)(IPL_SIMULATIONFLAGS_DIRECT | IPL_SIMULATIONFLAGS_REFLECTIONS);
  sim_settings.sceneType = IPL_SCENETYPE_DEFAULT;
  sim_settings.reflectionType = IPL_REFLECTIONEFFECTTYPE_CONVOLUTION;
  sim_settings.maxOrder = 2;
  sim_settings.numThreads = 1;
  sim_settings.maxNumSources = 8;
  sim_settings.maxDuration = 2.0f;
  sim_settings.maxNumRays = 16384;
  err = iplSimulatorCreate(_ipl_context, &sim_settings, &_ipl_simulator);
  if (!IPL_ERRCHECK("create simulator", err)) {
    return false;
  }

  // Create an IPLSource representing the listener, solely for simulating
  // reflections for listener-centric reverb.
  IPLSourceSettings listener_source_settings{};
  listener_source_settings.flags = IPL_SIMULATIONFLAGS_REFLECTIONS;
  err = iplSourceCreate(_ipl_simulator, &listener_source_settings, &_ipl_listener_source);
  if (!IPL_ERRCHECK("create listener source", err)) {
    return false;
  }

  // Now initialize the Steam Audio FMOD plugin.  This implements custom FMOD
  // DSPs that render the results of our simulations.

  Filename plugin_filename = Filename::dso_filename("libphonon_fmod.so");
  std::string plugin_filename_os = plugin_filename.to_os_specific();
  FMOD_RESULT result = _system->loadPlugin(plugin_filename_os.c_str(), &_ipl_plugin_handle);
  if (!fmod_audio_errcheck("Load Steam Audio FMOD plugin", result)) {
    return false;
  }

  result = _system->getNestedPlugin(_ipl_plugin_handle, 0, &_ipl_spatialize_handle);
  if (!fmod_audio_errcheck("Get SA spatialize DSP handle", result)) {
    return false;
  }
  result = _system->getNestedPlugin(_ipl_plugin_handle, 1, &_ipl_mixer_return_handle);
  if (!fmod_audio_errcheck("Get SA mixer return DSP handle", result)) {
    return false;
  }
  result = _system->getNestedPlugin(_ipl_plugin_handle, 2, &_ipl_reverb_handle);
  if (!fmod_audio_errcheck("Get SA reverb DSP handle", result)) {
    return false;
  }

  // We have to load the plugin ourselves to call certain functions in the
  // library... bleh.
  void *dso_handle = load_dso(get_plugin_path().get_value(), plugin_filename);
  if (dso_handle == nullptr) {
    fmodAudio_cat.error()
      << "Could not load Steam Audio FMOD plugin " << plugin_filename << " on plugin-path "
      << get_plugin_path().get_value() << "\n";
    return false;
  }
  void *init_func = get_dso_symbol(dso_handle, "iplFMODInitialize");
  nassertr(init_func != nullptr, false);
  void *hrtf_func = get_dso_symbol(dso_handle, "iplFMODSetHRTF");
  nassertr(hrtf_func != nullptr, false);
  void *sim_func = get_dso_symbol(dso_handle, "iplFMODSetSimulationSettings");
  nassertr(sim_func != nullptr, false);
  void *reverb_source_func = get_dso_symbol(dso_handle, "iplFMODSetReverbSource");
  nassertr(reverb_source_func != nullptr, false);
  ((PFNIPLFMODINITIALIZE)init_func)(_ipl_context);
  ((PFNIPLFMODSETHRTF)hrtf_func)(_ipl_hrtf);
  ((PFNIPLFMODSETSIMULATIONSETTINGS)sim_func)(sim_settings);
  ((PFNIPLFMODSETREVERBSOURCE)reverb_source_func)(_ipl_listener_source);

  fmodAudio_cat.info()
    << "Steam Audio initialized successfully\n";

  return true;
}

/**
 *
 */
void FMODAudioEngine::
shutdown_steam_audio() {
  if (_ipl_listener_source != nullptr) {
    iplSourceRelease(&_ipl_listener_source);
    _ipl_listener_source = nullptr;
  }

  if (_ipl_simulator != nullptr) {
    iplSimulatorRelease(&_ipl_simulator);
    _ipl_simulator = nullptr;
  }

  if (_ipl_hrtf != nullptr) {
    iplHRTFRelease(&_ipl_hrtf);
    _ipl_hrtf = nullptr;
  }

  if (_ipl_context != nullptr) {
    iplContextRelease(&_ipl_context);
    _ipl_context = nullptr;
  }
}

#endif

/**
 * Specifies the position, orientation, and velocity of the listener in
 * world-space.  Positional sounds will be calculated relative to the
 * listener transform.
 */
void FMODAudioEngine::
set_3d_listener_attributes(const LPoint3 &pos, const LQuaternion &quat, const LVector3 &vel) {
  // Convert from game units to meters.
  _listener_pos = pos / _unit_scale;
  _listener_vel = vel / _unit_scale;
  _listener_quat = quat;

  LVector3 fwd = quat.get_forward();
  LVector3 up = quat.get_up();

  FMOD_RESULT result;
  FMOD_VECTOR fpos, fvel, fup, ffwd;
  fpos = lvec_to_fmod(_listener_pos);
  fvel = lvec_to_fmod(_listener_vel);
  fup = lvec_to_fmod(up);
  ffwd = lvec_to_fmod(fwd);
  result = _system->set3DListenerAttributes(0, &fpos, &fvel, &ffwd, &fup);
  fmod_audio_errcheck("_system->set3DListenerAttributes()", result);
}

/**
 *
 */
LPoint3 FMODAudioEngine::
get_3d_listener_pos() const {
  return _listener_pos * _unit_scale;
}

/**
 *
 */
LQuaternion FMODAudioEngine::
get_3d_listener_quat() const {
  return _listener_quat;
}

/**
 *
 */
LVector3 FMODAudioEngine::
get_3d_listener_velocity() const {
  return _listener_vel * _unit_scale;
}

/**
 * Specifies the units-per-meter of the game's coordinate system.  The
 * audio system internally uses meters for 3-D audio calculations, so
 * coordinates passed into the interface will have this scale applied
 * to convert them to meters (and back).
 */
void FMODAudioEngine::
set_3d_unit_scale(PN_stdfloat factor) {
  _unit_scale = factor;
}

/**
 *
 */
PN_stdfloat FMODAudioEngine::
get_3d_unit_scale() const {
  return _unit_scale;
}

/**
 *
 */
PT(AudioManager) FMODAudioEngine::
make_manager(const std::string &name, AudioManager *parent) {
  return new FMODAudioManager(name, DCAST(FMODAudioManager, parent), this);
}

/**
 *
 */
void FMODAudioEngine::
update() {
  update_dirty_dsps();

  for (ManagerList::const_iterator it = _managers.begin(); it != _managers.end(); ++it) {
    (*it)->update();
  }

  FMOD_RESULT result;
  result = _system->update();
  fmod_audio_errcheck("_system->update()", result);
}

/**
 * Adds the specified audio manager as having the specified DSP applied to it.
 */
void FMODAudioEngine::
add_manager_to_dsp(DSP *dsp, FMODAudioManager *mgr) {
  auto itr = _dsp_managers.find(dsp);
  if (itr == _dsp_managers.end()) {
    audio_debug("Adding first manager to DSP");
    _dsp_managers[dsp] = { mgr };
    return;
  }

  audio_debug("Adding new manager to DSP");

  itr->second.insert(mgr);
}

/**
 * Removes the specified audio manager from the list of audio managers with
 * this DSP applied to it.  Destructs the DSP if there are no more managers
 * with this DSP.
 */
void FMODAudioEngine::
remove_manager_from_dsp(DSP *dsp, FMODAudioManager *mgr) {
  auto itr = _dsp_managers.find(dsp);
  if (itr == _dsp_managers.end()) {
    return;
  }

  itr->second.erase(mgr);

  audio_debug("Removed manager from DSP");

  if (itr->second.size() == 0) {
    audio_debug("DSP has no more managers");
    _dsp_managers.erase(itr);
  }
}

/**
 * Updates all DSPs with the dirty flag set.
 */
void FMODAudioEngine::
update_dirty_dsps() {
  for (auto itr = _dsp_managers.begin(); itr != _dsp_managers.end(); itr++) {
    DSP *panda_dsp = itr->first;
    const ManagerList &managers = itr->second;

    if (panda_dsp->is_dirty()) {
      audio_debug("Updating dirty " << panda_dsp->get_type().get_name());

      for (auto mitr = managers.begin(); mitr != managers.end(); mitr++) {
        FMODAudioManager *manager = *mitr;
        FMOD::DSP *fmod_dsp = manager->get_fmod_dsp(panda_dsp);
        if (!fmod_dsp) {
          continue;
        }

        configure_dsp(panda_dsp, fmod_dsp);
      }

      panda_dsp->clear_dirty();
    }
  }
}

/**
 * Returns the FMOD DSP type from the Panda DSP type.
 */
FMOD_DSP_TYPE FMODAudioEngine::
get_fmod_dsp_type(DSP::DSPType panda_type) {
  switch (panda_type) {
  case DSP::DT_chorus:
    return FMOD_DSP_TYPE_CHORUS;
  case DSP::DT_compressor:
    return FMOD_DSP_TYPE_COMPRESSOR;
  case DSP::DT_distortion:
    return FMOD_DSP_TYPE_DISTORTION;
  case DSP::DT_echo:
    return FMOD_DSP_TYPE_ECHO;
  case DSP::DT_fader:
    return FMOD_DSP_TYPE_FADER;
  case DSP::DT_flange:
    return FMOD_DSP_TYPE_FLANGE;
  case DSP::DT_highpass:
    return FMOD_DSP_TYPE_HIGHPASS;
  case DSP::DT_lowpass:
    return FMOD_DSP_TYPE_LOWPASS;
  case DSP::DT_limiter:
    return FMOD_DSP_TYPE_LIMITER;
  case DSP::DT_oscillator:
    return FMOD_DSP_TYPE_OSCILLATOR;
  case DSP::DT_parameq:
    return FMOD_DSP_TYPE_PARAMEQ;
  case DSP::DT_pitchshift:
    return FMOD_DSP_TYPE_PITCHSHIFT;
  case DSP::DT_sfxreverb:
    return FMOD_DSP_TYPE_SFXREVERB;
  case DSP::DT_normalize:
    return FMOD_DSP_TYPE_NORMALIZE;
  default:
    return FMOD_DSP_TYPE_UNKNOWN;
  }
}

/**
 * Creates and returns a new FMOD DSP instance for the provided Panda DSP,
 * or returns nullptr if the DSP type is unsupported.
 */
FMOD::DSP *FMODAudioEngine::
create_fmod_dsp(DSP *panda_dsp) {
  FMOD_DSP_TYPE type = get_fmod_dsp_type(panda_dsp->get_dsp_type());
  if (type == FMOD_DSP_TYPE_UNKNOWN) {
    // We don't support this DSP type.
    return nullptr;
  }

  audio_debug("Creating new DSP instance of type "
              << panda_dsp->get_type().get_name());

  FMOD_RESULT result;
  FMOD::DSP *dsp;
  result = _system->createDSPByType(type, &dsp);
  fmod_audio_errcheck("_sytem->createDSPByType()", result);

  dsp->setUserData(panda_dsp);

  // Apply the current parameters while we're at it.
  configure_dsp(panda_dsp, dsp);
  panda_dsp->clear_dirty();

  return dsp;
}

/**
 * Configures the FMOD DSP based on the parameters in the Panda DSP.
 */
void FMODAudioEngine::
configure_dsp(DSP *dsp_conf, FMOD::DSP *dsp) {
  switch(dsp_conf->get_dsp_type()) {
  default:
    fmodAudio_cat.warning()
      << "Don't know how to configure "
      << dsp_conf->get_type().get_name() << "\n";
    break;
  case DSP::DT_chorus:
    {
      ChorusDSP *chorus_conf = DCAST(ChorusDSP, dsp_conf);
      dsp->setParameterFloat(FMOD_DSP_CHORUS_MIX, chorus_conf->get_mix());
      dsp->setParameterFloat(FMOD_DSP_CHORUS_RATE, chorus_conf->get_rate());
      dsp->setParameterFloat(FMOD_DSP_CHORUS_DEPTH, chorus_conf->get_depth());
    }
    break;
  case DSP::DT_compressor:
    {
      CompressorDSP *comp_conf = DCAST(CompressorDSP, dsp_conf);
      dsp->setParameterFloat(FMOD_DSP_COMPRESSOR_THRESHOLD, comp_conf->get_threshold());
      dsp->setParameterFloat(FMOD_DSP_COMPRESSOR_RATIO, comp_conf->get_ratio());
      dsp->setParameterFloat(FMOD_DSP_COMPRESSOR_ATTACK, comp_conf->get_attack());
      dsp->setParameterFloat(FMOD_DSP_COMPRESSOR_RELEASE, comp_conf->get_release());
      dsp->setParameterFloat(FMOD_DSP_COMPRESSOR_GAINMAKEUP, comp_conf->get_gainmakeup());
    }
    break;
  case DSP::DT_distortion:
    {
      DistortionDSP *dist_conf = DCAST(DistortionDSP, dsp_conf);
      dsp->setParameterFloat(FMOD_DSP_DISTORTION_LEVEL, dist_conf->get_level());
    }
    break;
  case DSP::DT_echo:
    {
      EchoDSP *echo_conf = DCAST(EchoDSP, dsp_conf);
      dsp->setParameterFloat(FMOD_DSP_ECHO_DELAY, echo_conf->get_delay());
      dsp->setParameterFloat(FMOD_DSP_ECHO_FEEDBACK, echo_conf->get_feedback());
      dsp->setParameterFloat(FMOD_DSP_ECHO_DRYLEVEL, echo_conf->get_drylevel());
      dsp->setParameterFloat(FMOD_DSP_ECHO_WETLEVEL, echo_conf->get_wetlevel());
    }
    break;
  case DSP::DT_fader:
    {
      FaderDSP *fader_conf = DCAST(FaderDSP, dsp_conf);
      dsp->setParameterFloat(FMOD_DSP_FADER_GAIN, fader_conf->get_gain());
    }
    break;
  case DSP::DT_flange:
    {
      FlangeDSP *flange_conf = DCAST(FlangeDSP, dsp_conf);
      dsp->setParameterFloat(FMOD_DSP_FLANGE_MIX, flange_conf->get_mix());
      dsp->setParameterFloat(FMOD_DSP_FLANGE_DEPTH, flange_conf->get_depth());
      dsp->setParameterFloat(FMOD_DSP_FLANGE_RATE, flange_conf->get_rate());
    }
    break;
  case DSP::DT_highpass:
    {
      HighpassDSP *hp_conf = DCAST(HighpassDSP, dsp_conf);
      dsp->setParameterFloat(FMOD_DSP_HIGHPASS_CUTOFF, hp_conf->get_cutoff());
      dsp->setParameterFloat(FMOD_DSP_HIGHPASS_RESONANCE, hp_conf->get_resonance());
    }
    break;
  case DSP::DT_limiter:
    {
      LimiterDSP *lim_conf = DCAST(LimiterDSP, dsp_conf);
      dsp->setParameterFloat(FMOD_DSP_LIMITER_RELEASETIME, lim_conf->get_release_time());
      dsp->setParameterFloat(FMOD_DSP_LIMITER_CEILING, lim_conf->get_ceiling());
      dsp->setParameterFloat(FMOD_DSP_LIMITER_MAXIMIZERGAIN, lim_conf->get_maximizer_gain());
    }
    break;
  case DSP::DT_lowpass:
    {
      LowpassDSP *lp_conf = DCAST(LowpassDSP, dsp_conf);
      dsp->setParameterFloat(FMOD_DSP_LOWPASS_CUTOFF, lp_conf->get_cutoff());
      dsp->setParameterFloat(FMOD_DSP_LOWPASS_RESONANCE, lp_conf->get_resonance());
    }
    break;
  case DSP::DT_normalize:
    {
      NormalizeDSP *norm_conf = DCAST(NormalizeDSP, dsp_conf);
      dsp->setParameterFloat(FMOD_DSP_NORMALIZE_FADETIME, norm_conf->get_fade_time());
      dsp->setParameterFloat(FMOD_DSP_NORMALIZE_THRESHOLD, norm_conf->get_threshold());
      dsp->setParameterFloat(FMOD_DSP_NORMALIZE_MAXAMP, norm_conf->get_max_amp());
    }
    break;
  case DSP::DT_oscillator:
    {
      OscillatorDSP *osc_conf = DCAST(OscillatorDSP, dsp_conf);
      dsp->setParameterInt(FMOD_DSP_OSCILLATOR_TYPE, osc_conf->get_oscillator_type());
      dsp->setParameterFloat(FMOD_DSP_OSCILLATOR_RATE, osc_conf->get_rate());
    }
    break;
  case DSP::DT_parameq:
    {
      ParamEQDSP *peq_conf = DCAST(ParamEQDSP, dsp_conf);
      dsp->setParameterFloat(FMOD_DSP_PARAMEQ_CENTER, peq_conf->get_center());
      dsp->setParameterFloat(FMOD_DSP_PARAMEQ_BANDWIDTH, peq_conf->get_bandwith());
      dsp->setParameterFloat(FMOD_DSP_PARAMEQ_GAIN, peq_conf->get_gain());
    }
    break;
  case DSP::DT_pitchshift:
    {
      PitchShiftDSP *ps_conf = DCAST(PitchShiftDSP, dsp_conf);
      dsp->setParameterFloat(FMOD_DSP_PITCHSHIFT_PITCH, ps_conf->get_pitch());
      dsp->setParameterFloat(FMOD_DSP_PITCHSHIFT_FFTSIZE, (float)ps_conf->get_fft_size());
    }
    break;
  case DSP::DT_sfxreverb:
    {
      SFXReverbDSP *sfx_conf = DCAST(SFXReverbDSP, dsp_conf);
      dsp->setParameterFloat(FMOD_DSP_SFXREVERB_DECAYTIME, sfx_conf->get_decay_time());
      dsp->setParameterFloat(FMOD_DSP_SFXREVERB_EARLYDELAY, sfx_conf->get_early_delay());
      dsp->setParameterFloat(FMOD_DSP_SFXREVERB_LATEDELAY, sfx_conf->get_late_delay());
      dsp->setParameterFloat(FMOD_DSP_SFXREVERB_HFREFERENCE, sfx_conf->get_hf_reference());
      dsp->setParameterFloat(FMOD_DSP_SFXREVERB_HFDECAYRATIO, sfx_conf->get_hf_decay_ratio());
      dsp->setParameterFloat(FMOD_DSP_SFXREVERB_DIFFUSION, sfx_conf->get_diffusion());
      dsp->setParameterFloat(FMOD_DSP_SFXREVERB_DENSITY, sfx_conf->get_density());
      dsp->setParameterFloat(FMOD_DSP_SFXREVERB_LOWSHELFFREQUENCY, sfx_conf->get_low_shelf_frequency());
      dsp->setParameterFloat(FMOD_DSP_SFXREVERB_LOWSHELFGAIN, sfx_conf->get_low_shelf_gain());
      dsp->setParameterFloat(FMOD_DSP_SFXREVERB_HIGHCUT, sfx_conf->get_highcut());
      dsp->setParameterFloat(FMOD_DSP_SFXREVERB_EARLYLATEMIX, sfx_conf->get_early_late_mix());
      dsp->setParameterFloat(FMOD_DSP_SFXREVERB_WETLEVEL, sfx_conf->get_wetlevel());
      dsp->setParameterFloat(FMOD_DSP_SFXREVERB_DRYLEVEL, sfx_conf->get_drylevel());
    }
    break;
  }
}

/**
 *
 */
void FMODAudioEngine::
add_manager(FMODAudioManager *mgr) {
  _managers.insert(mgr);
}

/**
 *
 */
void FMODAudioEngine::
remove_manager(FMODAudioManager *mgr) {
  _managers.erase(mgr);
}

/**
 *
 */
PT(AudioEngine) FMODAudioEngineProxy::
make_engine() const {
  return new FMODAudioEngine;
}
