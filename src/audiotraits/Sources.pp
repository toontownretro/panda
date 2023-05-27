#define OTHER_LIBS dtool:m dtoolbase:c dtoolutil:c \
    putil:c prc

#define BUILD_DIRECTORY $[HAVE_AUDIO]

// FIXME: Could possibly restore Miles support
#begin lib_target
  #define TARGET miles_audio
  #define BUILD_TARGET $[HAVE_RAD_MSS]
  #define USE_PACKAGES rad_mss
  #define BUILDING_DLL BUILDING_MILES_AUDIO
  #define LOCAL_LIBS audio event pipeline
  #define WIN_SYS_LIBS $[WIN_SYS_LIBS] user32.lib advapi32.lib winmm.lib

  #define SOURCES \
      config_milesAudio.h \
      milesAudioManager.h \
      milesAudioSound.I milesAudioSound.h \
      milesAudioSample.I milesAudioSample.h \
      milesAudioSequence.I milesAudioSequence.h \
      milesAudioStream.I milesAudioStream.h \
      globalMilesManager.I globalMilesManager.h

  #define COMPOSITE_SOURCES \
      config_milesAudio.cxx milesAudioManager.cxx milesAudioSound.cxx \
      milesAudioStream.cxx globalMilesManager.cxx milesAudioSample.cxx \
      milesAudioSequence.cxx

#end lib_target

#begin lib_target
  #define TARGET fmod_audio
  #define BUILD_TARGET $[HAVE_FMOD]
  #define USE_PACKAGES fmod
  #define BUILDING_DLL BUILDING_FMOD_AUDIO
  #define LOCAL_LIBS audio event jobsystem
  #define WIN_SYS_LIBS $[WIN_SYS_LIBS] user32.lib advapi32.lib winmm.lib

  #if $[HAVE_STEAM_AUDIO]
    // Steam Audio integration compiled in.
    // High quality sound propagation, occlusion, spatial audio, etc.
    #define USE_PACKAGES $[USE_PACKAGES] steam_audio
    #define C++FLAGS $[C++FLAGS] -DHAVE_STEAM_AUDIO
  #endif

  #define SOURCES \
      config_fmodAudio.h \
      fmodAudioEngine.h fmodAudioEngine.I \
      fmodAudioManager.h \
      fmodAudioSound.I fmodAudioSound.h \
      fmodSoundCache.I fmodSoundCache.h

  #define COMPOSITE_SOURCES \
      config_fmodAudio.cxx \
      fmodAudioEngine.cxx \
      fmodAudioManager.cxx \
      fmodAudioSound.cxx \
      fmodSoundCache.cxx

#end lib_target

#begin lib_target
  #define TARGET openal_audio
  #define BUILD_TARGET
  #define USE_PACKAGES openal
  #define BUILDING_DLL BUILDING_OPENAL_AUDIO
  #define LOCAL_LIBS audio event
  #define WIN_SYS_LIBS $[WIN_SYS_LIBS] user32.lib advapi32.lib winmm.lib

  #define SOURCES \
      config_openalAudio.h \
      openalAudioManager.h \
      openalAudioSound.I openalAudioSound.h

  #define COMPOSITE_SOURCES \
      config_openalAudio.cxx openalAudioManager.cxx openalAudioSound.cxx

#end lib_target

#begin lib_target
  #define BUILD_TARGET
  #define TARGET pminiaudio

  #define LOCAL_LIBS audio event
  #define SOURCES \
    config_miniaudio.h config_miniaudio.cxx \
    miniaudio.h miniaudio.cxx \
    miniAudioManager.h miniAudioManager.I miniAudioManager.cxx \
    miniAudioSound.h miniAudioSound.I miniAudioSound.cxx

#end lib_target

//#begin lib_target
//  #define TARGET audio_linux
//  #define BUILDING_DLL BUILDING_MISC
//  //#define LOCAL_LIBS \
//  //  audio
//
//  #define SOURCES \
//    $[if $[HAVE_SYS_SOUNDCARD_H], \
//      linuxAudioManager.cxx linuxAudioManager.h \
//      linuxAudioSound.cxx linuxAudioSound.h \
//    , \
//      nullAudioManager.cxx nullAudioManager.h \
//      nullAudioSound.cxx nullAudioSound.h \
//    ]
//
//#end lib_target

#begin test_bin_target
  #define TARGET test_steam_audio_direct_fade
  #define BUILD_TESTS 1
  #define USE_PACKAGES steam_audio
  #define LOCAL_LIBS putil
  #define SOURCES test_steam_audio_direct_fade.cxx

#end test_bin_target
