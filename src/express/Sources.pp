#define LOCAL_LIBS pandabase
#define OTHER_LIBS interrogatedb dtoolutil:c dtoolbase:c dtool:m prc

#begin lib_target
  #define TARGET express
  #define USE_PACKAGES zlib openssl

  #define BUILDING_DLL BUILDING_PANDA_EXPRESS

  #define SOURCES \
    buffer.I buffer.h \
    checksumHashGenerator.I checksumHashGenerator.h circBuffer.I \
    circBuffer.h \
    compress_string.h \
    config_express.h \
    copy_stream.h \
    datagram.I datagram.h datagramGenerator.I \
    datagramGenerator.h \
    datagramIterator.I datagramIterator.h datagramSink.I datagramSink.h \
    dcast.h dcast.T \
    encrypt_string.h \
    error_utils.h \
    export_dtool.h \
    fileReference.h fileReference.I \
    hashGeneratorBase.I hashGeneratorBase.h \
    hashVal.I hashVal.h \
    indirectLess.I indirectLess.h \
    lzmaDecoder.h \
    memoryInfo.I memoryInfo.h \
    memoryUsage.I memoryUsage.h \
    memoryUsagePointerCounts.I memoryUsagePointerCounts.h \
    memoryUsagePointers.I memoryUsagePointers.h \
    multifile.I multifile.h \
    namable.I \
    namable.h \
    nodePointerTo.h nodePointerTo.I \
    nodePointerToBase.h nodePointerToBase.I \
    nodeReferenceCount.h nodeReferenceCount.I \
    openSSLWrapper.h openSSLWrapper.I \
    ordered_vector.h ordered_vector.I ordered_vector.T \
    pStatCollectorForwardBase.h \
    password_hash.h \
    patchfile.I patchfile.h \
    pointerTo.I pointerTo.h \
    pointerToArray.I pointerToArray.h \
    pointerToArrayBase.I pointerToArrayBase.h \
    pointerToBase.I pointerToBase.h \
    pointerToVoid.I pointerToVoid.h \
    pta_int.h \
    pta_uchar.h pta_double.h pta_float.h \
    pta_stdfloat.h \
    ramfile.I ramfile.h \
    referenceCount.I referenceCount.h \
    stringStreamBuf.I stringStreamBuf.h \
    stringStream.I stringStream.h \
    subStream.I subStream.h subStreamBuf.h \
    subfileInfo.h subfileInfo.I \
    temporaryFile.h temporaryFile.I \
    trueClock.I trueClock.h \
    typedReferenceCount.I typedReferenceCount.h \
    virtualFile.I virtualFileList.I virtualFileList.h virtualFileMount.h \
    virtualFileComposite.h virtualFileComposite.I virtualFile.h \
    virtualFileMount.I virtualFileMountMultifile.h \
    virtualFileMountAndroidAsset.h virtualFileMountAndroidAsset.I \
    virtualFileMountMultifile.I \
    virtualFileMountRamdisk.h virtualFileMountRamdisk.I \
    virtualFileMountSystem.h virtualFileMountSystem.I \
    virtualFileMountZip.h virtualFileMountZip.I \
    virtualFileSimple.h virtualFileSimple.I \
    virtualFileSystem.h virtualFileSystem.I \
    weakPointerCallback.I weakPointerCallback.h \
    weakPointerTo.I weakPointerTo.h \
    weakPointerToBase.I weakPointerToBase.h \
    weakPointerToVoid.I weakPointerToVoid.h \
    weakReferenceList.I weakReferenceList.h \
    windowsRegistry.h \
    zipArchive.I zipArchive.h \
    zStream.I zStream.h zStreamBuf.h

  #define COMPOSITE_SOURCES  \
    buffer.cxx checksumHashGenerator.cxx \
    compress_string.cxx \
    config_express.cxx \
    copy_stream.cxx \
    datagram.cxx datagramGenerator.cxx \
    datagramIterator.cxx \
    datagramSink.cxx \
    dcast.cxx \
    encrypt_string.cxx \
    error_utils.cxx \
    fileReference.cxx \
    hashGeneratorBase.cxx hashVal.cxx \
    lzmaDecoder.cxx \
    memoryInfo.cxx memoryUsage.cxx memoryUsagePointerCounts.cxx \
    memoryUsagePointers.cxx multifile.cxx \
    namable.cxx \
    nodePointerTo.cxx \
    nodePointerToBase.cxx \
    nodeReferenceCount.cxx \
    openSSLWrapper.cxx \
    ordered_vector.cxx \
    pStatCollectorForwardBase.cxx \
    password_hash.cxx \
    patchfile.cxx \
    pointerTo.cxx \
    pointerToArray.cxx \
    pointerToBase.cxx \
    pointerToVoid.cxx \
    pta_int.cxx \
    pta_uchar.cxx pta_double.cxx pta_float.cxx \
    ramfile.cxx \
    referenceCount.cxx \
    stringStreamBuf.cxx \
    stringStream.cxx \
    subStream.cxx subStreamBuf.cxx \
    subfileInfo.cxx \
    temporaryFile.cxx \
    trueClock.cxx \
    typedReferenceCount.cxx \
    virtualFileComposite.cxx virtualFile.cxx virtualFileList.cxx \
    virtualFileMount.cxx \
    $[if $[ANDROID_PLATFORM], virtualFileMountAndroidAsset.cxx] \
    virtualFileMountMultifile.cxx \
    virtualFileMountRamdisk.cxx \
    virtualFileMountSystem.cxx \
    virtualFileMountZip.cxx \
    virtualFileSimple.cxx virtualFileSystem.cxx \
    weakPointerCallback.cxx \
    weakPointerTo.cxx \
    weakPointerToBase.cxx \
    weakPointerToVoid.cxx \
    weakReferenceList.cxx \
    windowsRegistry.cxx \
    zipArchive.cxx \
    zStream.cxx zStreamBuf.cxx

  #define INSTALL_HEADERS  \
    buffer.I buffer.h \
    checksumHashGenerator.I checksumHashGenerator.h circBuffer.I \
    circBuffer.h \
    compress_string.h \
    config_express.h \
    copy_stream.h \
    datagram.I datagram.h datagramGenerator.I \
    datagramGenerator.h \
    datagramIterator.I datagramIterator.h datagramSink.I datagramSink.h \
    dcast.h dcast.T \
    encrypt_string.h \
    error_utils.h \
    fileReference.h fileReference.I \
    hashGeneratorBase.I hashGeneratorBase.h \
    hashVal.I hashVal.h \
    indirectLess.I indirectLess.h \
    lzmaDecoder.h \
    memoryInfo.I memoryInfo.h \
    memoryUsage.I memoryUsage.h \
    memoryUsagePointerCounts.I memoryUsagePointerCounts.h \
    memoryUsagePointers.I memoryUsagePointers.h \
    multifile.I multifile.h \
    namable.I \
    namable.h \
    nodePointerTo.h nodePointerTo.I \
    nodePointerToBase.h nodePointerToBase.I \
    nodeReferenceCount.h nodeReferenceCount.I \
    openSSLWrapper.h openSSLWrapper.I \
    ordered_vector.h ordered_vector.I ordered_vector.T \
    pStatCollectorForwardBase.h \
    password_hash.h \
    patchfile.I patchfile.h \
    pointerTo.I pointerTo.h \
    pointerToArray.I pointerToArray.h \
    pointerToArrayBase.I pointerToArrayBase.h \
    pointerToBase.I pointerToBase.h \
    pointerToVoid.I pointerToVoid.h \
    pta_int.h \
    pta_uchar.h pta_double.h pta_float.h \
    pta_stdfloat.h \
    ramfile.I ramfile.h \
    referenceCount.I referenceCount.h \
    stringStreamBuf.I stringStreamBuf.h \
    stringStream.I stringStream.h \
    subStream.I subStream.h subStreamBuf.h \
    subfileInfo.h subfileInfo.I \
    temporaryFile.h temporaryFile.I \
    trueClock.I trueClock.h \
    typedReferenceCount.I typedReferenceCount.h \
    virtualFile.I virtualFileList.I virtualFileList.h virtualFileMount.h \
    virtualFileComposite.h virtualFileComposite.I virtualFile.h \
    virtualFileMount.I virtualFileMountMultifile.h \
    virtualFileMountMultifile.I \
    virtualFileMountRamdisk.h virtualFileMountRamdisk.I \
    virtualFileMountSystem.h virtualFileMountSystem.I \
    virtualFileMountZip.h virtualFileMountZip.I \
    virtualFileSimple.h virtualFileSimple.I \
    virtualFileSystem.h virtualFileSystem.I \
    weakPointerCallback.I weakPointerCallback.h \
    weakPointerTo.I weakPointerTo.h \
    weakPointerToBase.I weakPointerToBase.h \
    weakPointerToVoid.I weakPointerToVoid.h \
    weakReferenceList.I weakReferenceList.h \
    windowsRegistry.h \
    zipArchive.I zipArchive.h \
    zStream.I zStream.h zStreamBuf.h

  #define IGATESCAN all

  #define IGATEEXT \
    datagram_ext.h \
    memoryUsagePointers_ext.cxx \
    memoryUsagePointers_ext.h \
    pointerToArray_ext.h \
    ramfile_ext.cxx \
    ramfile_ext.h \
    stringStream_ext.cxx \
    stringStream_ext.h \
    virtualFileSystem_ext.cxx \
    virtualFileSystem_ext.h \
    virtualFile_ext.cxx \
    virtualFile_ext.h \
    multifile_ext.h \
    multifile_ext.I

  // These are extensions of classes in dtool.  We define them here because we
  // can't directly run interrogate on dtool.  See config_express.N for more
  // information on this.
  #define IGATEEXT $[IGATEEXT] \
    filename_ext.cxx \
    filename_ext.h \
    globPattern_ext.cxx \
    globPattern_ext.h \
    iostream_ext.cxx \
    iostream_ext.h \
    streamReader_ext.cxx \
    streamReader_ext.h \
    streamWriter_ext.cxx \
    streamWriter_ext.h \
    textEncoder_ext.cxx \
    textEncoder_ext.h \
    textEncoder_ext.I \
    typeHandle_ext.cxx \
    typeHandle_ext.h \
    configVariable_ext.cxx \
    configVariable_ext.h

  #define WIN_SYS_LIBS \
    advapi32.lib ws2_32.lib shell32.lib user32.lib crypt32.lib $[WIN_SYS_LIBS]

  // These libraries and frameworks are used by dtoolutil; we redefine
  // them here so they get into the panda build system.
  #if $[ne $[PLATFORM], FreeBSD]
    #define UNIX_SYS_LIBS dl
  #endif
  #if $[ANDROID_PLATFORM]
    #define UNIX_SYS_LIBS android
  #endif
  #define OSX_SYS_FRAMEWORKS Foundation $[if $[not $[BUILD_IPHONE]],AppKit]

#end lib_target

#begin test_bin_target
  // Not really a "test" program; this program is used to regenerate
  // ca_bundle_data_src.c.

  #define TARGET make_ca_bundle
  #define LOCAL_LIBS $[LOCAL_LIBS] express
  #define OTHER_LIBS dtoolutil:c dtool:m

  #define SOURCES \
    make_ca_bundle.cxx

#end test_bin_target


#begin test_bin_target
  #define TARGET test_types
  #define LOCAL_LIBS $[LOCAL_LIBS] express
  #define OTHER_LIBS dtoolutil:c dtool:m prc

  #define SOURCES \
    test_types.cxx

#end test_bin_target


#begin test_bin_target
  #define TARGET test_ordered_vector

  #define SOURCES \
    test_ordered_vector.cxx

  #define LOCAL_LIBS $[LOCAL_LIBS] putil
  #define OTHER_LIBS dtoolutil:c dtool:m prc

#end test_bin_target


#if $[HAVE_ZLIB]
#begin test_bin_target
  #define TARGET test_zstream
  #define USE_PACKAGES zlib
  #define LOCAL_LIBS $[LOCAL_LIBS] express
  #define OTHER_LIBS dtoolutil:c dtool:m prc

  #define SOURCES \
    test_zstream.cxx

#end test_bin_target
#endif
