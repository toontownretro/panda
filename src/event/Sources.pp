#define LOCAL_LIBS putil express pandabase pstatclient linmath
#define OTHER_LIBS dtoolutil:c dtoolbase:c dtool:m prc

#begin lib_target
  #define TARGET event

  #define BUILDING_DLL BUILDING_PANDA_EVENT

  #define SOURCES \
    asyncFuture.h asyncFuture.I \
    asyncTask.h asyncTask.I \
    asyncTaskChain.h asyncTaskChain.I \
    asyncTaskCollection.h asyncTaskCollection.I \
    asyncTaskManager.h asyncTaskManager.I \
    asyncTaskPause.h asyncTaskPause.I \
    asyncTaskSequence.h asyncTaskSequence.I \
    config_event.h \
    buttonEvent.I buttonEvent.h \
    buttonEventList.I buttonEventList.h \
    genericAsyncTask.h genericAsyncTask.I \
    pointerEvent.I pointerEvent.h \
    pointerEventList.I pointerEventList.h \
    event.I event.h eventHandler.h eventHandler.I \
    eventParameter.I eventParameter.h \
    eventQueue.I eventQueue.h eventReceiver.h \
    pt_Event.h throw_event.I throw_event.h

  #define COMPOSITE_SOURCES \
    asyncFuture.cxx \
    asyncTask.cxx \
    asyncTaskChain.cxx \
    asyncTaskCollection.cxx \
    asyncTaskManager.cxx \
    asyncTaskPause.cxx \
    asyncTaskSequence.cxx \
    buttonEvent.cxx \
    buttonEventList.cxx \
    genericAsyncTask.cxx \
    pointerEvent.cxx \
    pointerEventList.cxx \
    config_event.cxx event.cxx eventHandler.cxx \
    eventParameter.cxx eventQueue.cxx eventReceiver.cxx \
    pt_Event.cxx

  #define INSTALL_HEADERS \
    asyncFuture.h asyncFuture.I \
    asyncTask.h asyncTask.I \
    asyncTaskChain.h asyncTaskChain.I \
    asyncTaskCollection.h asyncTaskCollection.I \
    asyncTaskManager.h asyncTaskManager.I \
    asyncTaskPause.h asyncTaskPause.I \
    asyncTaskSequence.h asyncTaskSequence.I \
    buttonEvent.I buttonEvent.h \
    buttonEventList.I buttonEventList.h \
    genericAsyncTask.h genericAsyncTask.I \
    pointerEvent.I pointerEvent.h \
    pointerEventList.I pointerEventList.h \
    event.I event.h eventHandler.h eventHandler.I \
    eventParameter.I eventParameter.h \
    eventQueue.I eventQueue.h eventReceiver.h \
    pt_Event.h throw_event.I throw_event.h

  #define IGATESCAN all

  #define IGATEEXT \
    asyncFuture_ext.cxx \
    asyncFuture_ext.h \
    pythonTask.cxx \
    pythonTask.h \
    pythonTask.I

#end lib_target

#begin test_bin_target
  #define TARGET test_task
  #define LOCAL_LIBS $[LOCAL_LIBS] mathutil
  #define OTHER_LIBS \
   dtoolbase:c prc \
   dtoolutil:c dtool:m

  #define SOURCES \
    test_task.cxx

#end test_bin_target
