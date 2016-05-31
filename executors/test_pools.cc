// Thinking
// http://rsdn.ru/forum/cpp.applied/5468499.flat.2
// !!! http://stackoverflow.com/questions/19572140/how-do-i-utilize-boostpackaged-task-function-parameters-and-boostasioio
// Для портабельной передач исключений между потоками нужна фичи языка
// http://stackoverflow.com/questions/8876459/boost-equivalent-of-stdasync

#define BOOST_THREAD_PROVIDES_FUTURE

#include "actors_and_workers/actors_cc98.h"

#include <gtest/gtest.h>

// TROUBLE: для верий буста выше некоторой нужно TIME_UTC -> TIME_UTC_ - что-то связано с С11
#include <boost/version.hpp>
#if BOOST_VERSION < 105000
#include <boost/thread/xtime.hpp>
namespace boost {
  enum xtime_compat {
    //TIME_UTC_=TIME_UTC
  };
}
#endif

#include <boost/asio.hpp>
#include <boost/thread.hpp>
#include <boost/thread/thread.hpp>
#include <boost/thread/mutex.hpp>
#include <boost/thread/future.hpp>
#include <boost/throw_exception.hpp>  // sudden
#include <boost/bind.hpp>
#include <boost/exception_ptr.hpp>
#include <boost/make_shared.hpp>

#include <iostream>
#include <sstream>
#include <string>
#include <stdexcept>

#include <cstdio>

using boost::shared_ptr;
using boost::make_shared;
using boost::bind;
using boost::packaged_task;
using boost::future;
using boost::ref;
using boost::cref;
using boost::bind;
using std::cout;
using std::endl;
using std::string;
using std::cout;
using std::ostringstream;

namespace {
boost::mutex m_io_monitor;
boost::mutex mio; // 1

std::string ans("forty two");

int int_no_params()
{
  return 42;
}

int int_with_params(int param)
{
  return param;
}

std::string string_no_params()
{
  return std::string("forty two");
}

std::string funcReturnStringWithParams(std::string & param)
{
  //boost::this_thread::sleep()
  //sleep(3);
  return param;
}

void logMsg(const std::string& reason, int i)
{
  static int k;
  //++k;

  boost::lock_guard<boost::mutex> lock(mio); // 2.
  std::cout << reason << " for " << i << " at " << std::time(0) << std::endl;
}

void jobOne(int secs) // 3.
{
  logMsg("Start jobOne", secs);
  boost::this_thread::sleep(boost::posix_time::millisec(1000 * secs));
  logMsg("End jobOne", secs);
}

void jobTwo(int millisecs, boost::exception_ptr& error)
{
  try {
    logMsg("Start jobTwo", millisecs);
    boost::this_thread::sleep(boost::posix_time::millisec(millisecs));
    logMsg("End jobTwo", millisecs);
    logMsg("Throw()", millisecs);
    BOOST_THROW_EXCEPTION(std::runtime_error(""));
  } catch (...) {
    error = boost::current_exception();
  }
}


void print(string text)
{
  boost::mutex::scoped_lock lock(m_io_monitor);
  cout << text;
}

template<typename T>
string to_string(T const & value)
{
  ostringstream ost;
  ost << value;
  ost.flush();
  return ost.str();
}

void task_1()
{
  print(string("  task_1()\n"));
  //throw 5;
}

void task_2()
{
  print(string("  task_2()\n"));
  throw 5;
}

void task_3()
{
  print(string("  task_3()\n"));
}

void task_with_parameter(int value)
{
  print("  task_with_parameter(" + to_string(value) + ")\n");
}

int failed_func()
{
  //try {
    print("  task_int_1()\n");
    //throw 9;
    // http://www.boost.org/doc/libs/1_55_0/libs/exception/doc/frequently_asked_questions.html
    // https://groups.google.com/forum/#!topic/boost-list/E0C_gZDuydk
    BOOST_THROW_EXCEPTION(std::runtime_error(""));
    //boost::throw_exception(std::runtime_error(""));
    //error = boost::exception_ptr();
    return 1;
  //} catch (...) {
    //error = boost::current_exception();
  //}
}

// http://stackoverflow.com/questions/14422319/boost-threading-conceptualization-questions
// Non compiled
//
// http://www.mr-edd.co.uk/blog/
class Data {

};

int run_sim(Data*) { return 0; }
const unsigned nsim = 50;

void run(int tNumber) 
{ 
  // https://gist.github.com/snaewe/1393807
  boost::asio::io_service svc;  // 2.
  boost::thread_group threads;
 
  boost::exception_ptr error;  // похоже одна на поток
  {
    std::auto_ptr<boost::asio::io_service::work> work(new boost::asio::io_service::work(svc)); //3.
    for (int i = 0; i < tNumber; ++i)   // 4.
      threads.create_thread (boost::bind (&boost::asio::io_service::run, &svc));
 
    svc.post(boost::bind(jobOne, 2)); // 5.
    svc.post(boost::bind(jobOne, 1));
    
    svc.post(boost::bind(jobTwo, 50, boost::ref(error)));
 
    //svc.stop ();          // 6.
  }
  threads.join_all ();      // 7.

  EXPECT_TRUE(error);
  EXPECT_THROW(boost::rethrow_exception(error), std::runtime_error);
}

namespace tests {
class Templ {
public:
  std::pair<int, int> doTheThing(
      const int& in
      ) {
    return std::make_pair(0, 9);
  }
};

TEST(ThPool, AsioVersionCheck) {
  // http://www.boost.org/doc/libs/1_47_0/libs/exception/doc/tutorial_exception_ptr.html
  run(4);  // если нет планировщика, то лучше потоков побольше чем ядер
}

TEST(ThPool, OwnAsioPool) {
  executors::AsioThreadPool p;
  Templ templ;

  int in = 9;

  // http://stackoverflow.com/questions/17286458/use-member-function-in-stdpackaged-task
  packaged_task<std::pair<int, int> >
      t(bind(bind(&Templ::doTheThing, ref(templ), _1), cref(in)));
  future<std::pair<int, int> > f_memb = t.get_future();

  packaged_task<std::string> task(bind(&funcReturnStringWithParams, ans));
  future<string> f = task.get_future();

  p.add(bind(&packaged_task<std::string>::operator(), boost::ref(task)));
  p.add(bind(&packaged_task<std::pair<int, int>  >::operator(), boost::ref(t)));

  //EXPECT_FALSE(f.is_ready());
  while (!f.is_ready()) {
    cout << "wait" << endl;
  }

  string answer = f.get();
  std::pair<int, int> r = f_memb.get();
  std::cout << r.second << " string_with_params: " << answer << std::endl;
}

// http://stackoverflow.com/questions/19572140/how-do-i-utilize-boostpackaged-task-function-parameters-and-boostasioio
// http://stackoverflow.com/questions/4084777/creating-a-thread-pool-using-boost/4085345#4085345
// http://think-async.com/Asio/Recipes
// DANGER: а как с ошибками то быть?
TEST(ThPool, AsioBase) {
  using boost::shared_ptr;
  using boost::make_shared;  // not work
  using boost::bind;
  using std::string;

  boost::asio::io_service io_service;
  boost::thread_group threads;
  boost::asio::io_service::work work(io_service);

  for (int i = 0; i < 3; ++i)
    threads.create_thread(bind(&boost::asio::io_service::run, &io_service));

  typedef boost::packaged_task<std::string> task_t;
  shared_ptr<task_t> example = boost::make_shared<task_t>(bind(&funcReturnStringWithParams, ans));
  boost::future<string> f = example->get_future();
  
  // одну задачу
  io_service.post(boost::bind(&task_t::operator(), example));

  string answer = f.get();
  std::cout << "string_with_params: " << answer << std::endl;

  // stop pull
  io_service.stop();
  threads.join_all();
}
}

}  // space

