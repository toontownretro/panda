#begin lib_target
  #define TARGET jobsystem

  #define BUILDING_DLL BUILDING_PANDA_JOBSYSTEM

  #define LOCAL_LIBS pipeline pstatclient mathutil

  #define HEADERS \
    config_jobsystem.h \
    job.h job.I \
    jobSystem.h jobSystem.I \
    jobWorkerThread.h jobWorkerThread.I

  #define SOURCES \
    $[HEADERS]

  #define COMPOSITE_SOURCES \
    config_jobsystem.cxx \
    job.cxx \
    jobSystem.cxx \
    jobWorkerThread.cxx

  #define INSTALL_HEADERS $[HEADERS]

#end lib_target

#begin test_bin_target
  #define TARGET test_jobs
  #define LOCAL_LIBS jobsystem
  #define OTHER_LIBS \
   interrogatedb dtoolbase:c prc \
   dtoolutil:c dtool:m

  #define SOURCES \
    test_jobs.cxx

#end test_bin_target
