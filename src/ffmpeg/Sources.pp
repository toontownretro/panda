#define BUILD_DIRECTORY $[HAVE_FFMPEG]

#define OTHER_LIBS p3dtoolutil:c p3dtoolbase:c p3dtool:m p3prc:m

#define BUILDING_DLL BUILDING_FFMPEG

#define USE_PACKAGES ffmpeg swscale swresample

#begin lib_target
  #define TARGET p3ffmpeg

  #define LOCAL_LIBS p3movies

  #define SOURCES \
    config_ffmpeg.h \
    ffmpegVideo.h ffmpegVideo.I \
    ffmpegVideoCursor.h ffmpegVideoCursor.I \
    ffmpegAudio.h ffmpegAudio.I \
    ffmpegAudioCursor.h ffmpegAudioCursor.I \
    ffmpegVirtualFile.h ffmpegVirtualFile.I

  #define COMPOSITE_SOURCES \
    config_ffmpeg.cxx \
    ffmpegVideo.cxx \
    ffmpegVideoCursor.cxx \
    ffmpegAudio.cxx \
    ffmpegAudioCursor.cxx \
    ffmpegVirtualFile.cxx

  #define INSTALL_HEADERS \
    config_ffmpeg.h \
    ffmpegVideo.h ffmpegVideo.I \
    ffmpegVideoCursor.h ffmpegVideoCursor.I \
    ffmpegAudio.h ffmpegAudio.I \
    ffmpegAudioCursor.h ffmpegAudioCursor.I \
    ffmpegVirtualFile.h ffmpegVirtualFile.I

#end lib_target
