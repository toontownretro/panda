#define LOCAL_LIBS express pandabase
#define OTHER_LIBS interrogatedb \
                   dtoolutil:c dtoolbase:c dtool:m prc

#define SELECT_TAU select.tau

#begin lib_target
  #define TARGET pipeline
  #define USE_PACKAGES threads

  #define BUILDING_DLL BUILDING_PANDA_PIPELINE

  #define HEADERS \
    contextSwitch.h \
    blockerSimple.h blockerSimple.I \
    conditionVar.h conditionVar.I \
    conditionVarDebug.h conditionVarDebug.I \
    conditionVarDirect.h conditionVarDirect.I \
    conditionVarDummyImpl.h conditionVarDummyImpl.I \
    conditionVarFull.h \
    conditionVarImpl.h \
    conditionVarPosixImpl.h conditionVarPosixImpl.I \
    $[if $[WINDOWS_PLATFORM], conditionVarWin32Impl.h conditionVarWin32Impl.I] \
    conditionVarSimpleImpl.h conditionVarSimpleImpl.I \
    conditionVarSpinlockImpl.h conditionVarSpinlockImpl.I \
    config_pipeline.h \
    cycleData.h cycleData.I \
    cycleDataLockedReader.h cycleDataLockedReader.I \
    cycleDataLockedStageReader.h cycleDataLockedStageReader.I \
    cycleDataReader.h cycleDataReader.I \
    cycleDataStageReader.h cycleDataStageReader.I \
    cycleDataStageWriter.h cycleDataStageWriter.I \
    cycleDataWriter.h cycleDataWriter.I \
    cyclerHolder.h cyclerHolder.I \
    externalThread.h \
    genericThread.h genericThread.I \
    lightMutex.I lightMutex.h \
    lightMutexDirect.h lightMutexDirect.I \
    lightMutexHolder.I lightMutexHolder.h \
    lightReMutex.I lightReMutex.h \
    lightReMutexDirect.h lightReMutexDirect.I \
    lightReMutexHolder.I lightReMutexHolder.h \
    mainThread.h \
    mutexDebug.h mutexDebug.I \
    mutexDirect.h mutexDirect.I \
    mutexHolder.h mutexHolder.I \
    mutexSimpleImpl.h mutexSimpleImpl.I \
    mutexTrueImpl.h \
    pipeline.h pipeline.I \
    pipelineCycler.h pipelineCycler.I \
    pipelineCyclerLinks.h pipelineCyclerLinks.I \
    pipelineCyclerBase.h  \
    pipelineCyclerDummyImpl.h pipelineCyclerDummyImpl.I \
    pipelineCyclerTrivialImpl.h pipelineCyclerTrivialImpl.I \
    pipelineCyclerTrueImpl.h pipelineCyclerTrueImpl.I \
    pmutex.h pmutex.I \
    reMutex.I reMutex.h \
    reMutexDirect.h reMutexDirect.I \
    reMutexHolder.I reMutexHolder.h \
    reMutexSpinlockImpl.h reMutexSpinlockImpl.I \
    psemaphore.h psemaphore.I \
    thread.h thread.I threadImpl.h \
    threadDummyImpl.h threadDummyImpl.I \
    threadPosixImpl.h threadPosixImpl.I \
    threadSimpleImpl.h threadSimpleImpl.I  \
    threadSimpleManager.h threadSimpleManager.I  \
    $[if $[WINDOWS_PLATFORM], threadWin32Impl.h threadWin32Impl.I] \
    threadPriority.h

  #define SOURCES \
    $[HEADERS]
    contextSwitch.c

  #define COMPOSITE_SOURCES  \
    conditionVar.cxx \
    conditionVarDebug.cxx \
    conditionVarDirect.cxx \
    conditionVarDummyImpl.cxx \
    conditionVarPosixImpl.cxx \
    $[if $[WINDOWS_PLATFORM], conditionVarWin32Impl.cxx] \
    conditionVarSimpleImpl.cxx \
    conditionVarSpinlockImpl.cxx \
    config_pipeline.cxx \
    cycleData.cxx \
    cycleDataLockedReader.cxx \
    cycleDataLockedStageReader.cxx \
    cycleDataReader.cxx \
    cycleDataStageReader.cxx \
    cycleDataStageWriter.cxx \
    cycleDataWriter.cxx \
    cyclerHolder.cxx \
    externalThread.cxx \
    genericThread.cxx \
    lightMutex.cxx \
    lightMutexDirect.cxx \
    lightMutexHolder.cxx \
    lightReMutex.cxx \
    lightReMutexDirect.cxx \
    lightReMutexHolder.cxx \
    mainThread.cxx \
    mutexDebug.cxx \
    mutexDirect.cxx \
    mutexHolder.cxx \
    mutexSimpleImpl.cxx \
    pipeline.cxx \
    pipelineCycler.cxx \
    pipelineCyclerDummyImpl.cxx \
    pipelineCyclerTrivialImpl.cxx \
    pipelineCyclerTrueImpl.cxx \
    pmutex.cxx \
    reMutex.cxx \
    reMutexDirect.cxx \
    reMutexHolder.cxx \
    psemaphore.cxx \
    thread.cxx \
    threadDummyImpl.cxx \
    threadPosixImpl.cxx \
    threadSimpleImpl.cxx \
    threadSimpleManager.cxx \
    $[if $[WINDOWS_PLATFORM], threadWin32Impl.cxx] \
    threadPriority.cxx

  #define INSTALL_HEADERS  \
    $[HEADERS]

  #define IGATESCAN all

  #define IGATEEXT \
    pmutex_ext.h \
    pmutex_ext.I \
    pythonThread.cxx \
    pythonThread.h \
    reMutex_ext.h \
    reMutex_ext.I

#end lib_target


#begin test_bin_target
  #define TARGET test_threaddata
  #define LOCAL_LIBS $[LOCAL_LIBS] pipeline
  #define OTHER_LIBS \
   interrogatedb dtoolbase:c prc \
   dtoolutil:c dtool:m

  #define SOURCES \
    test_threaddata.cxx

#end test_bin_target


#begin test_bin_target
  #define TARGET test_diners
  #define LOCAL_LIBS $[LOCAL_LIBS] pipeline
  #define OTHER_LIBS \
   interrogatedb dtoolbase:c prc \
   dtoolutil:c dtool:m

  #define SOURCES \
    test_diners.cxx

#end test_bin_target


#begin test_bin_target
  #define TARGET test_mutex
  #define LOCAL_LIBS $[LOCAL_LIBS] pipeline
  #define OTHER_LIBS \
   interrogatedb dtoolbase:c prc \
   dtoolutil:c dtool:m

  #define SOURCES \
    test_mutex.cxx

#end test_bin_target


#begin test_bin_target
  #define TARGET test_concurrency
  #define LOCAL_LIBS $[LOCAL_LIBS] pipeline
  #define OTHER_LIBS \
   interrogatedb dtoolbase:c prc \
   dtoolutil:c dtool:m

  #define SOURCES \
    test_concurrency.cxx

#end test_bin_target


#begin test_bin_target
  #define TARGET test_delete
  #define LOCAL_LIBS $[LOCAL_LIBS] pipeline
  #define OTHER_LIBS \
   interrogatedb dtoolbase:c prc \
   dtoolutil:c dtool:m

  #define SOURCES \
    test_delete.cxx

#end test_bin_target


#begin test_bin_target
  #define TARGET test_atomic
  #define LOCAL_LIBS $[LOCAL_LIBS] pipeline
  #define OTHER_LIBS \
   interrogatedb dtoolbase:c prc \
   dtoolutil:c dtool:m

  #define SOURCES \
    test_atomic.cxx

#end test_bin_target



#begin test_bin_target
  #define TARGET test_setjmp
  #define LOCAL_LIBS $[LOCAL_LIBS] pipeline
  #define OTHER_LIBS \
   interrogatedb dtoolbase:c prc \
   dtoolutil:c dtool:m

  #define SOURCES \
    test_setjmp.cxx

#end test_bin_target
