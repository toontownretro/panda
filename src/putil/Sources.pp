#define OTHER_LIBS interrogatedb \
                  dtoolutil:c dtoolbase:c dtool:m prc
#define LOCAL_LIBS pipeline linmath express pandabase
#define USE_PACKAGES zlib

#begin lib_target
  #define TARGET putil

  #define BUILDING_DLL BUILDING_PANDA_PUTIL

  #define SOURCES \
    animInterface.h animInterface.I \
    assetBase.h assetBase.I \
    assetRegistry.h assetRegistry.I \
    autoTextureScale.h \
    bam.h \
    bamCache.h bamCache.I \
    bamCacheIndex.h bamCacheIndex.I \
    bamCacheRecord.h bamCacheRecord.I \
    bamEnums.h \
    bamReader.I bamReader.h bamReaderParam.I \
    bamReaderParam.h \
    bamWriter.I bamWriter.h \
    bitArray.I bitArray.h \
    bitMask.I bitMask.h \
    buttonHandle.I buttonHandle.h \
    buttonMap.I buttonMap.h \
    buttonRegistry.I buttonRegistry.h \
    cachedTypedWritableReferenceCount.h cachedTypedWritableReferenceCount.I \
    callbackData.h callbackData.I \
    callbackObject.h callbackObject.I \
    clockObject.h clockObject.I \
    colorSpace.h collideMask.h \
    copyOnWriteObject.h copyOnWriteObject.I \
    copyOnWritePointer.h copyOnWritePointer.I \
    compareTo.I compareTo.h \
    config_putil.N config_putil.h configurable.h \
    cPointerCallbackObject.h cPointerCallbackObject.I \
    datagramBuffer.I datagramBuffer.h \
    datagramInputFile.I datagramInputFile.h \
    datagramOutputFile.I datagramOutputFile.h \
    doubleBitMask.I doubleBitMask.h \
    drawMask.h \
    factory.I factory.h \
    factoryBase.I factoryBase.h \
    factoryParam.I factoryParam.h factoryParams.I \
    factoryParams.h \
    firstOfPairCompare.I firstOfPairCompare.h \
    firstOfPairLess.I firstOfPairLess.h \
    gamepadButton.h \
    globalPointerRegistry.I globalPointerRegistry.h \
    indirectCompareNames.I indirectCompareNames.h \
    indirectCompareSort.I indirectCompareSort.h \
    indirectCompareTo.I indirectCompareTo.h \
    ioPtaDatagramChar.h \
    ioPtaDatagramFloat.h ioPtaDatagramInt.h \
    ioPtaDatagramShort.h keyboardButton.h \
    keyValues.h \
    iterator_types.h \
    linkedListNode.I linkedListNode.h \
    load_prc_file.h \
    loaderOptions.I loaderOptions.h \
    modelIndex.I modelIndex.h \
    modifierButtons.I modifierButtons.h mouseButton.h \
    mouseData.h nameUniquifier.I nameUniquifier.h \
    nodeCachedReferenceCount.h nodeCachedReferenceCount.I \
    paramValue.I paramValue.h \
    pbitops.I pbitops.h \
    pointerData.h pointerData.I \
    portalMask.h \
    pta_ushort.h \
    simpleHashMap.I simpleHashMap.h \
    sparseArray.I sparseArray.h \
    timedCycle.I timedCycle.h \
    tokenFile.I tokenFile.h \
    typedWritable.I typedWritable.h \
    typedWritableReferenceCount.I \
    typedWritableReferenceCount.h updateSeq.I updateSeq.h \
    uniqueIdAllocator.h \
    vector_typedWritable.h \
    vector_ulong.h \
    vector_ushort.h vector_writable.h \
    writableConfigurable.h \
    writableParam.I writableParam.h

 #define COMPOSITE_SOURCES \
    animInterface.cxx \
    assetBase.cxx \
    assetRegistry.cxx \
    autoTextureScale.cxx \
    bamCache.cxx \
    bamCacheIndex.cxx \
    bamCacheRecord.cxx \
    bamEnums.cxx \
    bamReader.cxx bamReaderParam.cxx \
    bamWriter.cxx \
    bitArray.cxx \
    bitMask.cxx \
    buttonHandle.cxx buttonMap.cxx buttonRegistry.cxx \
    cachedTypedWritableReferenceCount.cxx \
    callbackData.cxx \
    callbackObject.cxx \
    clockObject.cxx \
    colorSpace.cxx \
    config_putil.cxx configurable.cxx \
    copyOnWriteObject.cxx \
    copyOnWritePointer.cxx \
    cPointerCallbackObject.cxx \
    datagramBuffer.cxx \
    datagramInputFile.cxx datagramOutputFile.cxx \
    doubleBitMask.cxx \
    factoryBase.cxx \
    factoryParam.cxx factoryParams.cxx \
    gamepadButton.cxx \
    globalPointerRegistry.cxx \
    ioPtaDatagramChar.cxx \
    ioPtaDatagramFloat.cxx \
    ioPtaDatagramInt.cxx ioPtaDatagramShort.cxx \
    keyboardButton.cxx \
    keyValues.cxx \
    linkedListNode.cxx \
    load_prc_file.cxx \
    loaderOptions.cxx \
    modelIndex.cxx \
    modifierButtons.cxx mouseButton.cxx \
    nameUniquifier.cxx \
    nodeCachedReferenceCount.cxx \
    paramValue.cxx \
    pbitops.cxx \
    pointerData.cxx \
    pta_ushort.cxx \
    simpleHashMap.cxx \
    sparseArray.cxx \
    timedCycle.cxx \
    tokenFile.cxx typedWritable.cxx \
    typedWritableReferenceCount.cxx updateSeq.cxx \
    uniqueIdAllocator.cxx \
    vector_typedWritable.cxx \
    vector_ushort.cxx vector_writable.cxx \
    writableConfigurable.cxx writableParam.cxx

  #define INSTALL_HEADERS \
    animInterface.h animInterface.I \
    assetBase.h assetBase.I \
    assetRegistry.h assetRegistry.I \
    autoTextureScale.h \
    bam.h \
    bamCache.h bamCache.I \
    bamCacheIndex.h bamCacheIndex.I \
    bamCacheRecord.h bamCacheRecord.I \
    bamEnums.h \
    bamReader.I bamReader.h bamReaderParam.I bamReaderParam.h \
    bamWriter.I bamWriter.h \
    bitArray.I bitArray.h \
    bitMask.I bitMask.h \
    buttonHandle.I buttonHandle.h \
    buttonMap.I buttonMap.h \
    buttonRegistry.I buttonRegistry.h \
    cachedTypedWritableReferenceCount.h cachedTypedWritableReferenceCount.I \
    callbackData.h callbackData.I \
    callbackObject.h callbackObject.I \
    clockObject.h clockObject.I \
    colorSpace.h collideMask.h \
    copyOnWriteObject.h copyOnWriteObject.I \
    copyOnWritePointer.h copyOnWritePointer.I \
    compareTo.I compareTo.h \
    config_putil.h configurable.h \
    cPointerCallbackObject.h cPointerCallbackObject.I \
    datagramBuffer.I datagramBuffer.h \
    datagramInputFile.I datagramInputFile.h \
    datagramOutputFile.I datagramOutputFile.h \
    doubleBitMask.I doubleBitMask.h \
    drawMask.h \
    factory.I factory.h \
    factoryBase.I factoryBase.h factoryParam.I factoryParam.h \
    factoryParams.I factoryParams.h \
    firstOfPairCompare.I firstOfPairCompare.h \
    firstOfPairLess.I firstOfPairLess.h \
    gamepadButton.h \
    globalPointerRegistry.I globalPointerRegistry.h \
    indirectCompareNames.I indirectCompareNames.h \
    indirectCompareSort.I indirectCompareSort.h \
    indirectCompareTo.I indirectCompareTo.h \
    ioPtaDatagramChar.h \
    ioPtaDatagramFloat.h ioPtaDatagramInt.h \
    ioPtaDatagramShort.h keyboardButton.h \
    keyValues.h \
    iterator_types.h \
    linkedListNode.I linkedListNode.h \
    load_prc_file.h \
    loaderOptions.I loaderOptions.h \
    modelIndex.I modelIndex.h \
    modifierButtons.I \
    modifierButtons.h mouseButton.h mouseData.h \
    nameUniquifier.I nameUniquifier.h \
    nodeCachedReferenceCount.h nodeCachedReferenceCount.I \
    paramValue.I paramValue.h \
    pbitops.I pbitops.h \
    pointerData.h pointerData.I \
    portalMask.h \
    pta_ushort.h \
    simpleHashMap.I simpleHashMap.h \
    sparseArray.I sparseArray.h \
    timedCycle.I timedCycle.h \
    tokenFile.I tokenFile.h typedWritable.I \
    typedWritable.h typedWritableReferenceCount.I \
    typedWritableReferenceCount.h updateSeq.I updateSeq.h \
    uniqueIdAllocator.h \
    vector_typedWritable.h \
    vector_ulong.h \
    vector_ushort.h vector_writable.h \
    writableConfigurable.h writableParam.I \
    writableParam.h

  #define IGATESCAN all

  #define IGATEEXT \
    bamReader_ext.cxx \
    bamReader_ext.h \
    bitArray_ext.cxx \
    bitArray_ext.h \
    bitArray_ext.I \
    bitMask_ext.h \
    bitMask_ext.I \
    callbackObject_ext.h \
    doubleBitMask_ext.h \
    doubleBitMask_ext.I \
    keyValues_ext.cxx \
    keyValues_ext.h \
    paramPyObject.h paramPyObject.I paramPyObject.cxx \
    pythonCallbackObject.cxx \
    pythonCallbackObject.h \
    sparseArray_ext.cxx \
    sparseArray_ext.h \
    sparseArray_ext.I \
    typedWritable_ext.cxx \
    typedWritable_ext.h

#end lib_target

#begin test_bin_target
  #define TARGET test_bamRead
  #define LOCAL_LIBS \
    putil pgraph

  #define SOURCES \
    test_bam.cxx test_bam.h test_bamRead.cxx

#end test_bin_target

#begin test_bin_target
  #define TARGET test_bamWrite
  #define LOCAL_LIBS \
    putil pgraph

  #define SOURCES \
    test_bam.cxx test_bam.h test_bamWrite.cxx

#end test_bin_target

#begin test_bin_target
  #define TARGET test_filename

  #define SOURCES \
    test_filename.cxx

#end test_bin_target

#begin test_bin_target
  #define TARGET test_glob

  #define SOURCES \
    test_glob.cxx

  #define LOCAL_LIBS $[LOCAL_LIBS] putil

#end test_bin_target

#begin test_bin_target
  #define TARGET test_linestream

  #define SOURCES \
    test_linestream.cxx

  #define LOCAL_LIBS $[LOCAL_LIBS] putil

#end test_bin_target

#begin test_bin_target
  #define TARGET test_tokenfile

  #define SOURCES \
    test_tokenfile.cxx

  #define LOCAL_LIBS $[LOCAL_LIBS] putil

#end test_bin_target
