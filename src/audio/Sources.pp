#define OTHER_LIBS \
     dtoolutil:c dtoolbase:c dtool:m prc
#define BUILD_DIRECTORY $[HAVE_AUDIO]

#begin lib_target
  #define TARGET audio
  #define LOCAL_LIBS putil event movies linmath

  #define BUILDING_DLL BUILDING_PANDA_AUDIO

  #define SOURCES \
    config_audio.h \
    filterProperties.h filterProperties.I \
    audioLoadRequest.h audioLoadRequest.I \
    audioManager.h audioManager.I \
    audioSound.h audioSound.I \
    chorusDSP.h chorusDSP.I \
    compressorDSP.h compressorDSP.I \
    distortionDSP.h distortionDSP.I \
    dsp.h dsp.I \
    echoDSP.h echoDSP.I \
    faderDSP.h faderDSP.I \
    flangeDSP.h flangeDSP.I \
    highpassDSP.h highpassDSP.I \
    limiterDSP.h limiterDSP.I \
    lowpassDSP.h lowpassDSP.I \
    normalizeDSP.h normalizeDSP.I \
    nullAudioManager.h \
    nullAudioSound.h \
    oscillatorDSP.h oscillatorDSP.I \
    paramEQDSP.h paramEQDSP.I \
    pitchShiftDSP.h pitchShiftDSP.I \
    sfxReverbDSP.h sfxReverbDSP.I

  #define COMPOSITE_SOURCES \
    config_audio.cxx \
    filterProperties.cxx \
    audioLoadRequest.cxx \
    audioManager.cxx \
    audioSound.cxx \
    chorusDSP.cxx \
    compressorDSP.cxx \
    distortionDSP.cxx \
    dsp.cxx \
    echoDSP.cxx \
    faderDSP.cxx \
    flangeDSP.cxx \
    highpassDSP.cxx \
    limiterDSP.cxx \
    lowpassDSP.cxx \
    normalizeDSP.cxx \
    nullAudioManager.cxx \
    nullAudioSound.cxx \
    oscillatorDSP.cxx \
    paramEQDSP.cxx \
    pitchShiftDSP.cxx \
    sfxReverbDSP.cxx

  #define INSTALL_HEADERS \
    config_audio.h \
    filterProperties.h filterProperties.I \
    audioLoadRequest.h audioLoadRequest.I \
    audioManager.h audioManager.I \
    audioSound.h audioSound.I \
    chorusDSP.h chorusDSP.I \
    compressorDSP.h compressorDSP.I \
    distortionDSP.h distortionDSP.I \
    dsp.h dsp.I \
    echoDSP.h echoDSP.I \
    faderDSP.h faderDSP.I \
    flangeDSP.h flangeDSP.I \
    highpassDSP.h highpassDSP.I \
    limiterDSP.h limiterDSP.I \
    lowpassDSP.h lowpassDSP.I \
    normalizeDSP.h normalizeDSP.I \
    nullAudioManager.h \
    nullAudioSound.h \
    oscillatorDSP.h oscillatorDSP.I \
    paramEQDSP.h paramEQDSP.I \
    pitchShiftDSP.h pitchShiftDSP.I \
    sfxReverbDSP.h sfxReverbDSP.I

  #define IGATESCAN all
#end lib_target

#begin test_bin_target
  #define TARGET test_audio
  #define LOCAL_LIBS \
    audio

  #define SOURCES \
    test_audio.cxx

#end test_bin_target
