/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file test_anim_threads.cxx
 * @author brian
 * @date 2022-05-01
 */

#include "pandabase.h"
#include "jobSystem.h"
#include "job.h"
#include "pandaFramework.h"
#include "windowFramework.h"
#include "pvector.h"
#include "asyncTaskManager.h"
#include "loader.h"
#include "character.h"
#include "nodePath.h"
#include "characterNode.h"
#include "dcast.h"
#include "genericAsyncTask.h"
#include "pStatClient.h"

pvector<PT(CharacterNode)> char_list;

AsyncTask::DoneStatus
animate_characters(GenericAsyncTask *task, void *data) {
  JobSystem *sys = JobSystem::get_global_ptr();
  sys->parallel_process(char_list.size(), [] (int i) {
    char_list[i]->update();
  });
  //for (CharacterNode *node : char_list) {
  //  node->update();
  //}
  return AsyncTask::DS_cont;
}

int
main(int argc, char *argv[]) {
  //JobSystem *sys = JobSystem::get_global_ptr();
  //sys->initialize();

  PandaFramework framework;
  framework.open_framework(argc, argv);

  WindowFramework *win_framework = framework.open_window();
  win_framework->enable_keyboard();
  win_framework->setup_trackball();

  Loader *loader = Loader::get_global_ptr();

  for (int i = 0; i < 1000; i++) {
    NodePath mdl = NodePath(loader->load_sync("models/char/engineer"));
    NodePath char_np = mdl.find("**/+CharacterNode");
    CharacterNode *char_node = DCAST(CharacterNode, char_np.node());
    char_list.push_back(char_node);
    Character *character = char_node->get_character();
    character->loop(22, true);
    mdl.reparent_to(win_framework->get_render());
  }

  AsyncTaskManager *tmgr = AsyncTaskManager::get_global_ptr();
  tmgr->add(new GenericAsyncTask("animate", animate_characters, nullptr));

  PStatClient::connect();

  framework.main_loop();

  return 0;
}
