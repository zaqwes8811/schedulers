/**
  \author zaqwes8811 github.com MIT license
*/
//
// Задачи должны быть завершаемые.
//
// http://www.threadingbuildingblocks.org/docs/help/index.htm#tbb_userguide/Task-Based_Programming.htm
// "..algorithms composed from non-blocking tasks."
// I/O or mutex
// http://www.ibm.com/developerworks/library/j-jtp0730/
//
// Alternatives:
//   http://www.ibm.com/developerworks/library/j-jtp0730/ -  thread-per-task
//
// Scheduler:
//  http://www.threadingbuildingblocks.org/docs/help/index.htm#tbb_userguide/Task-Based_Programming.htm
//
// Thinks:
//   ошибки лучше передавать в вызывающую функцию. future и прочие работают, но как скрестить с asio
//
//
// Effective pools:
//  - Sutter
//  - http://www.ibm.com/developerworks/library/j-jtp0730/
//    "Understand your tasks." - может для разных типов задач разные пулы
//
// Pools
//   http://stackoverflow.com/questions/16677287/boostthreadpoolpool-vs-boostthread-group
//   http://stackoverflow.com/questions/19500404/how-to-create-a-thread-pool-using-boost-in-c
//   http://techiesaint.wordpress.com/2012/12/16/implementing-threadpool-pattern-in-c-using-boostthread-pool/
//   http://www.tonicebrian.com/2012/05/23/thread-pool-in-c/
//
// Hand-maded threadpool
//   http://stackoverflow.com/questions/22284557/thread-pool-implementation-using-pthreads
//   http://rextester.com/discussion/XHA86284/ThreadPool
//   http://www.youtube.com/watch?v=FfbZfBk-3rI

// TODO: Try http://blog.think-async.com/2008/10/asynchronous-forkjoin-using-asio.html
//   но это не на пуле, это просто так
//
// About own thread pool
//   http://codereview.stackexchange.com/questions/40536/simple-thread-pool-in-c
//
// http://stackoverflow.com/questions/19500404/how-to-create-a-thread-pool-using-boost-in-c
// Похоже нужен планеровщик. Не очень понял последную часть
//
// http://thisthread.blogspot.co.il/2011/04/multithreading-with-asio.html
//
// http://www.tonicebrian.com/2012/05/23/thread-pool-in-c/
//
// !!http://mostlycoding.blogspot.com.es/2009/05/asio-library-has-been-immensely-helpful.html
//
// Magic http://stackoverflow.com/questions/19572140/how-do-i-utilize-boostpackaged-task-function-parameters-and-boostasioio
//
#ifndef AP_POOL_H_
#define AP_POOL_H_

#include <boost/noncopyable.hpp>
#include <boost/weak_ptr.hpp>
#include <boost/thread/thread.hpp>
//#include <boost/thread/detail/thread_group.hpp>
#include <boost/asio.hpp>

namespace thread_pools {
class AsioThreadPool {
public:
  AsioThreadPool();
  ~AsioThreadPool();

  /**
    \attention I don't know how incapsulate io_service.post()

    \return m_io_service ref

    Thread safity
    http://www.boost.org/doc/libs/1_42_0/doc/html/boost_asio/reference/io_service.html
    http://www.boost.org/doc/libs/1_41_0/doc/html/boost_asio/overview/core/threads.html
    http://stackoverflow.com/questions/6369264/is-boostio-servicepost-thread-safe
    http://stackoverflow.com/questions/10051330/is-it-thread-safe-when-one-thread-add-timer-to-boostasioio-service-and-the-o
  */
  boost::asio::io_service& get();

private:
  boost::asio::io_service m_io_service;
  boost::thread_group m_threads;
  boost::asio::io_service::work m_work;  // protect if now work doc_asio/521
  int cm_size;
};

}

#endif
