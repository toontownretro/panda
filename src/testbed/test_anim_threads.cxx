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
pvector<PT(Job)> animate_jobs;

class AnimateJob : public Job {
public:
  AnimateJob(int start, int end) :
    _start(start),
    _end(end)
  {
  }

  virtual void execute() override {
    for (int i = _start; i < _end; ++i) {
      char_list[i]->update();
    }
  }

private:
  int _start, _end;
};

AsyncTask::DoneStatus
animate_characters(GenericAsyncTask *task, void *data) {
  JobSystem *sys = JobSystem::get_global_ptr();
  for (Job *job : animate_jobs) {
    sys->schedule(job);
  }
  return AsyncTask::DS_cont;
}

int
main(int argc, char *argv[]) {
  JobSystem *sys = JobSystem::get_global_ptr();
  sys->initialize();

  PStatClient::connect();

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

  int num_groups = Thread::get_num_supported_threads() - 1;
  int chars_per_group = (int)char_list.size() / num_groups;
  int remainder = (int)char_list.size() % num_groups;
  for (int i = 0; i < num_groups; ++i) {
    int start = i * chars_per_group;
    int end = (i + 1) * chars_per_group;
    if (i == num_groups - 1) {
      end += remainder;
    }
    PT(AnimateJob) job = new AnimateJob(start, end);
    animate_jobs.push_back(job);
  }

  AsyncTaskManager *tmgr = AsyncTaskManager::get_global_ptr();
  tmgr->add(new GenericAsyncTask("animate", animate_characters, nullptr));

  framework.main_loop();

  return 0;
}
