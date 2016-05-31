
#include "actors_and_workers/actors_cc98.h"
#include "actors_and_workers/actors_cc11.h"

#include <folly/futures/Future.h>
#include <gtest/gtest.h>
#include <folly/Executor.h>
#include <boost/thread/thread.hpp>
#include <folly/futures/ManualExecutor.h>
#include <folly/futures/InlineExecutor.h>

#include <iostream>

using namespace folly;
using namespace std;

class MyExecutor : public folly::Executor
{
public:
  virtual void add(Func f) {
    a.Send(f);
  }

private:
  cc11::Actior a;
};

void foo(int x) {
  // do something with x
  cout << "foo(" << x << ")" << endl;
}

MyExecutor ge;

TEST(FollyTest, My) {
  //auto e = MyExecutor();
  MyExecutor e;

  cout << "making Promise" << endl;
  shared_ptr<Promise<int> > p = make_shared<Promise<int> >();
  Future<int> f = p->getFuture();

  auto work = [](Try<int>&& t) {
    foo(t.value());
  };

  // continue
  // work, but trouble. put in queue but destruct
  f.via(&ge).then(work);

  cout << "Future chain made" << endl;

  // launch
  e.add([p] {
    cout << "fulfilling Promise" << endl;
    p->setValue(42);
    cout << "Promise fulfilled" << endl;
  });

  //while(true);
  //boost::thread::(boost::chrono::seconds(2));
}

namespace mc {
// Async api:
//   Error handling
class MemcacheClient {
 public:
  struct GetReply {
    enum class Result {
      FOUND,
      NOT_FOUND,
      SERVER_ERROR,
    };

    Result result;
    // The value when result is FOUND,
    // The error message when result is SERVER_ERROR or CLIENT_ERROR
    // undefined otherwise
    std::string value;
  };

  GetReply get(std::string key) { return GetReply(); }

  // typical - way to "callback hell"
  int get(std::string key, std::function<void(GetReply)> callback);

  // used future
  // "This is slightly more useful than a synchronous API, but it's not yet ideal"
  Future<GetReply> async_get(std::string key);
};
}
















