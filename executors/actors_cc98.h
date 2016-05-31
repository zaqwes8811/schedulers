// (c) from Sutter

#ifndef S_ACTORS_H_
#define S_ACTORS_H_

#define BOOST_THREAD_PROVIDES_FUTURE  // FIXME: bad

#include "safe_queue_cc11.h"

#include <boost/noncopyable.hpp>
#include <boost/weak_ptr.hpp>
#include <boost/thread.hpp>
#include <boost/asio.hpp>
#include <boost/bind.hpp>
#include <boost/scoped_ptr.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/thread/future.hpp>

#include <string>

namespace executors {

// FIXME: unknow scheduling algorithm
class AsioThreadPool : public boost::noncopyable {
public:
  typedef boost::function0<void> Func;

  AsioThreadPool()
    : m_io_service()
    , m_threads()
    , m_work(m_io_service)
  {
      m_threads.create_thread(boost::bind(&boost::asio::io_service::run, &m_io_service));
  }

  explicit AsioThreadPool(int countWorkers)
    : m_io_service()
    , m_threads()
    , m_work(m_io_service)
  {
    for (int i = 0; i < countWorkers; ++i)
      m_threads.create_thread(boost::bind(&boost::asio::io_service::run, &m_io_service));
  }

  ~AsioThreadPool() {
    m_io_service.stop();
    m_threads.join_all();
  }

public:
  void add(Func f) {
    m_io_service.post(f);
  }

private:
  boost::asio::io_service m_io_service;
  boost::thread_group m_threads;
  boost::asio::io_service::work m_work;
};
}

namespace cc98 {
// Example 1: Active helper, the general OO way
//
class Actor {
public:

  class Message {        // base of all message types
  public:
    virtual ~Message() { }
    virtual void Execute() { }
  };

private:
  // (suppress copying if in C++)

  // private data
  // unique_ptr not assing op
  boost::shared_ptr<Message> done;               // le sentinel
  concurent::message_queue< boost::shared_ptr<Message> > mq;    // le queue
  boost::scoped_ptr<boost::thread> thd;                // le thread
private:
  // The dispatch loop: pump messages until done
  void Run() {
    boost::shared_ptr<Message> msg;
    while( (msg = mq.dequeue()) != done ) {
      msg->Execute();
    }
  }

public:
  // Start everything up, using Run as the thread mainline
  Actor() :
      done( new Message )
    , thd(new boost::thread( boost::bind(&Actor::Run, this)))
    { }

  // Shut down: send sentinel and wait for queue to drain
  ~Actor() {
    Send( done );
    thd->join();
  }

  // Enqueue a message
  void Send( boost::shared_ptr<Message> m ) {
    mq.enqueue( m );
  }
};
}

// From Sutter:
//   http://www.drdobbs.com/parallel/prefer-using-active-objects-instead-of-n/225700095
//   http://www.drdobbs.com/cpp/prefer-using-futures-or-callbacks-to-com/226700179
//   http://www.drdobbs.com/architecture-and-design/know-when-to-use-an-active-object-instea/227500074?pgno=3
// http://www.chromium.org/developers/design-documents/threading
class SingleWorker
{
public:
  // typedefs
  typedef boost::function0<void> Callable;

  // http://stackoverflow.com/questions/19192122/template-declaration-of-typedef-typename-footbar-bar
  //typedef
  //template <typename T>
  //boost::shared_ptr<boost::packaged_task<T> > Task;

  SingleWorker() : m_pool(1) { }

  void post(Callable task) {
    m_pool.add(task);
  }

  static std::string getCurrentThreadId() {
    return boost::lexical_cast<std::string>(boost::this_thread::get_id());
  }

  std::string getId() {
    boost::packaged_task<std::string> t(&getCurrentThreadId);
    boost::future<std::string> f = t.get_future();

    SingleWorker::Callable pkg
        = boost::bind(&boost::packaged_task<std::string>::operator(), boost::ref(t));
    post(pkg);

    return f.get();
  }

private:
  executors::AsioThreadPool m_pool;
};

#endif
