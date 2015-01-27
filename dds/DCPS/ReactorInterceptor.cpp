/*
 * $Id$
 *
 *
 * Distributed under the OpenDDS License.
 * See: http://www.opendds.org/license.html
 */

#include "ReactorInterceptor.h"

namespace OpenDDS {
namespace DCPS {

ReactorInterceptor::ReactorInterceptor(ACE_Reactor* reactor,
                                       ACE_thread_t owner)
  : owner_(owner)
  , condition_(mutex_)
{
  this->reactor(reactor);
}

ReactorInterceptor::~ReactorInterceptor()
{
  ACE_GUARD(ACE_Thread_Mutex, guard, this->mutex_);

  // Cancel all pending notifications and dump the command queue.
  this->reactor()->purge_pending_notifications(this);
  while (!command_queue_.empty ()) {
    delete command_queue_.front ();
    command_queue_.pop ();
  }
}

void ReactorInterceptor::wait()
{
  if (owner_ == ACE_Thread::self()) {
    handle_exception(ACE_INVALID_HANDLE);
  } else {
    mutex_.acquire();
    while (!command_queue_.empty()) {
      condition_.wait();
    }
    mutex_.release();
  }
}

int ReactorInterceptor::handle_exception(ACE_HANDLE /*fd*/)
{
  ACE_GUARD_RETURN(ACE_Thread_Mutex, guard, this->mutex_, 0);

  while (!command_queue_.empty()) {
    Command* command = command_queue_.front();
    command_queue_.pop();
    command->execute();
    delete command;
  }

  condition_.signal();

  return 0;
}

}

}