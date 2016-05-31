/**
  https://events.yandex.ru/lib/talks/2488/
  How make check(need thread)
  FIXME: how get result

  http://stackoverflow.com/questions/4548395/how-do-retrieve-the-thread-id-from-a-boostthread

  From Chromium:
    http://www.chromium.org/developers/lock-and-condition-variable
    These models are "threads + mutexes + condition-variables", and "threads + message-passing"

  Sean Parent
    http://channel9.msdn.com/events/GoingNative/2013/Cpp-Seasoning

  Trouble:
  - composition and dep. tasks - then()? ->> graph flow (TBB) etc.
  - "Callback hell"

  http://nickhutchinson.me/libdispatch/

  https://github.com/facebook/folly/tree/master/folly/futures
*/

#include "actors_and_workers/actors_cc98.h"
#include "actors_and_workers/arch.h"

#include <gtest/gtest.h>
#include <boost/bind/bind.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/make_shared.hpp>

#include <stdio.h>

namespace objs {
int failed_func() {
  //try {
    //print("  task_int_1()\n");
    //throw 9;
    // http://www.boost.org/doc/libs/1_55_0/libs/exception/doc/frequently_asked_questions.html
    // https://groups.google.com/forum/#!topic/boost-list/E0C_gZDuydk
    BOOST_THROW_EXCEPTION(std::runtime_error(""));
    //boost::throw_exception(std::runtime_error(""));
    //error = boost::exception_ptr();
  //} catch (...) {
    //error = boost::current_exception();
  //}
}


class NonThreadSafeObj {
public:
  void append() {
    m_s += "hello";
  }

private:
  std::string m_s;
};
}  // space


TEST(AsPl, SingleThread)
{
  SingleWorker worker;

  boost::packaged_task<int> task(objs::failed_func);
  boost::future<int> fi = task.get_future();

  SingleWorker::Callable f = boost::bind(&boost::packaged_task<int>::operator(), boost::ref(task));
  worker.post(f);

  EXPECT_THROW(fi.get(), std::runtime_error);
}

TEST(AsPl, SingleThreadShared)
{
  using boost::make_shared;
  using boost::packaged_task;
  using boost::shared_ptr;
  using boost::future;

  SingleWorker worker;
  SingleWorker worker1;

  objs::NonThreadSafeObj obj;

  {
    packaged_task<int> t0(objs::failed_func);
    future<int> f0 = t0.get_future();

    packaged_task<int> t1(objs::failed_func);
    future<int> f1 = t1.get_future();

    SingleWorker::Callable c0
        = boost::bind(&boost::packaged_task<int>::operator(), boost::ref(t0));

    SingleWorker::Callable c1
        = boost::bind(&boost::packaged_task<int>::operator(), boost::ref(t1));

    worker.post(c0);
    worker.post(c1);
    // worker1 - races

    EXPECT_THROW(f0.get(), std::runtime_error);
    EXPECT_THROW(f1.get(), std::runtime_error);

    std::cout << SingleWorker::getCurrentThreadId() << std::endl;
    std::cout << worker.getId() << std::endl;
  }
}

TEST(AsPl, ThMon)
{
  using boost::make_shared;
  using boost::packaged_task;
  using boost::shared_ptr;
  using boost::future;

  {
    packaged_task<int> t0(objs::failed_func);
    future<int> f0 = t0.get_future();

    packaged_task<int> t1(objs::failed_func);
    future<int> f1 = t1.get_future();

    SingleWorker::Callable c0
        = boost::bind(&boost::packaged_task<int>::operator(), boost::ref(t0));

    SingleWorker::Callable c1
        = boost::bind(&boost::packaged_task<int>::operator(), boost::ref(t1));

    Threads::post(Threads::DB, c1);
    Threads::post(Threads::DB, c0);

    EXPECT_THROW(f0.get(), std::runtime_error);
    EXPECT_THROW(f1.get(), std::runtime_error);
  }
}
