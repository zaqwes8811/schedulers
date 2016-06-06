#ifndef VIEW_UI_ACTOR_H_
#define VIEW_UI_ACTOR_H_

//#include "core/concepts.h"
//#include "common/error_handling.h"

//#include <concurent_queues.h>

#include <iostream>
#include <memory>
#include <boost/thread.hpp>
#include <boost/thread/future.hpp>

namespace actors {
class UiObject;

/**
  \attention Dark place

  \attention Only in head and shared_ptr

  \fixme How check thread id in callable member?

  \bug On off application ASan SIGSEGV detect. Think trouble in off thread
  \fixme check by TSan.

  template <typename T>  // can't
  http://stackoverflow.com/questions/17853212/using-shared-from-this-in-templated-classes
*/
class UIActor// : public std::enable_shared_from_this<UIActor>
{
public:
//  typedef boost::function1<void, void> Message;

  // FIXME: trouble is not non-arg ctor
  explicit UIActor();

  ~UIActor();

  void post()//Message m )
  {
    try {
//      auto r = mq.try_push( m );
//      if (!r)
//        throw infrastructure_error(FROM_HERE);
    } catch (...) {
//      throw infrastructure_error(FROM_HERE);
    }
  }

//  boost::future<int> RunUI(concepts::db_manager_concept_t db);

private:
  UIActor( const UIActor& );           // no copying
  void operator=( const UIActor& );    // no copying

  bool m_done;                         // le flag
//  fix_extern_concurent::concurent_bounded_try_queue<Message> mq;
//  std::unique_ptr<std::thread> thd;          // le thread

  void Run();

//  std::unique_ptr<UiObject> uiPtr;
};
}

#endif
