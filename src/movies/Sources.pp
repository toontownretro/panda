#define OTHER_LIBS interrogatedb \
                   dtoolutil:c dtoolbase:c dtool:m prc


#begin lib_target
  #define TARGET movies
  #define LOCAL_LIBS gobj

  #define USE_PACKAGES vorbis opus
  #define WIN_SYS_LIBS $[WIN_SYS_LIBS] strmiids.lib winmm.lib

  #define BUILDING_DLL BUILDING_PANDA_MOVIES

  #define SOURCES \
    dr_flac.h \
    config_movies.h \
    flacAudio.h flacAudio.I \
    flacAudioCursor.h flacAudioCursor.I \
    inkblotVideo.h inkblotVideo.I \
    inkblotVideoCursor.h inkblotVideoCursor.I \
    microphoneAudio.h microphoneAudio.I \
    movieAudio.h movieAudio.I \
    movieAudioCursor.h movieAudioCursor.I \
    movieTypeRegistry.h movieTypeRegistry.I \
    movieVideo.h movieVideo.I \
    movieVideoCursor.h movieVideoCursor.I \
    opusAudio.h opusAudio.I \
    opusAudioCursor.h opusAudioCursor.I \
    userDataAudio.h userDataAudio.I \
    userDataAudioCursor.h userDataAudioCursor.I \
    vorbisAudio.h vorbisAudio.I \
    vorbisAudioCursor.h vorbisAudioCursor.I \
    wavAudio.h wavAudio.I \
    wavAudioCursor.h wavAudioCursor.I

  #define COMPOSITE_SOURCES \
    config_movies.cxx \
    flacAudio.cxx \
    flacAudioCursor.cxx \
    inkblotVideo.cxx \
    inkblotVideoCursor.cxx \
    microphoneAudio.cxx \
    microphoneAudioDS.cxx \
    movieAudio.cxx \
    movieAudioCursor.cxx \
    movieTypeRegistry.cxx \
    movieVideo.cxx \
    movieVideoCursor.cxx \
    opusAudio.cxx \
    opusAudioCursor.cxx \
    userDataAudio.cxx \
    userDataAudioCursor.cxx \
    vorbisAudio.cxx \
    vorbisAudioCursor.cxx \
    wavAudio.cxx \
    wavAudioCursor.cxx

  #define INSTALL_HEADERS \
    $[SOURCES]

  #define IGATESCAN all

#end lib_target
