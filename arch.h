#ifndef AAW_ARCH_H_
#define AAW_ARCH_H_

//#include "actors_cc11.h"

#include "actor_ui.h"

#include <algorithm>
#include <map>
#include <string>
//#include <future>

#ifndef FROM_HERE
#define FROM_HERE ""
#endif


/**
  From Sutter:
  http://www.drdobbs.com/parallel/prefer-using-active-objects-instead-of-n/225700095
  http://www.drdobbs.com/cpp/prefer-using-futures-or-callbacks-to-com/226700179
  http://www.drdobbs.com/architecture-and-design/know-when-to-use-an-active-object-instea/227500074?pgno=3
  http://www.chromium.org/developers/design-documents/threading

  http://stackoverflow.com/questions/19192122/template-declaration-of-typedef-typename-footbar-bar
  typedef
  template <typename T>
  boost::shared_ptr<boost::packaged_task<T> > Task;
*/
//class SingleWorker
//{
//public:
//  // typedefs
//  typedef std::function<void()> Callable;

//  SingleWorker() { }

//  static std::string getCurrentThreadId();

//  void post(Callable task) {
//    m_pool.post(task);
//  }

//  std::string GetId();

//private:
//  cc11::Actor m_pool;
//};

///**
//  \fixme incapsulate thread id's
//  Static and global - lifetime troubles

//  \attention Real trouble with checking current thread, not dispatch
//*/
//class Dispatcher {
//public:
//  enum Ids
//  { DB, UI };

//  static void Post(Ids id, SingleWorker::Callable fun);

//  /**
//    \attention Stop on ui event

//    \fixme Bad. Coupled with particular application. Make
//      Promise of Packaged Task
//  */
//  static std::future<int> ActivateUiEventLoop(concepts::db_manager_concept_t db);

//private:
//  Dispatcher();

//  static std::string decodeId(Ids id);

//  static std::string dbId();

//  static std::shared_ptr<SingleWorker> s_dbWorker;  // make weak access
//  static gc::SharedPtr<actors::UIActor> s_ui_actor;

//  static std::weak_ptr<SingleWorker> get_db() {
//    return s_dbWorker;
//  }

//  static std::weak_ptr<actors::UIActor> get_ui() {
//    return s_ui_actor;
//  }
//};

#endif
