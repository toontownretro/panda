// Filename: test_threaddata.cxx
// Created by:  cary (16Sep98)
// 
////////////////////////////////////////////////////////////////////

#include <pandabase.h>
#include "ipc_thread.h"

// test a derived class from thread with thread specific data, as well as the
// joining mechanism.

class thread_with_data : public thread {
   private:
      int _my_thread_id_plus_two;
      void* run_undetached(void* ptr) {
	 int arg = *(int*)ptr;
	 delete (int*)ptr;
	 cerr << "Thread: run invoked with arg " << arg << endl;
	 cerr << "Thread: my id is " << get_id() << endl;
	 cerr << "Thread: my private data (id plus 2) is "
	      << _my_thread_id_plus_two << endl;
	 int* rv = new int(_my_thread_id_plus_two + 1);
	 cerr << "Thread: returning " << *rv << endl;
	 return (void*)rv;
      }
      ~thread_with_data(void) {}
      static void* make_arg(const int i) { return (void*)new int(i); }
   public:
      thread_with_data(void) : thread(make_arg(5)) {
	 _my_thread_id_plus_two = get_id() + 2;
	 start_undetached();
      }
};

int main(int, char**)
{
   thread_with_data *t = new thread_with_data;
   cerr << "main: joining" << endl;
   int* rv;
   t->join((void**)&rv);
   cerr << "main: joined - got return value " << *rv << endl;
   delete rv;
   return 0;
}
