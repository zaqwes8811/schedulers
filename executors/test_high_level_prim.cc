// Motivation:
//  Sean Parent - "No raw sync", Sutter, Intel TBB (планеровщик и task based parallelism)
//  Tasks better.
//

// Thread -> improve latency
// Task -> improve throughtput - not I/O, not ...

//#include <adobe/algorithm/for_each.hpp>  // какой-то странный
//#include <adobe/algorithm/generate.hpp>
#include <gtest/gtest.h>

#include <ostream>
#include <vector>
#include <algorithm>
#include <stdexcept>
#include <future>
#include <thread>

#include <cassert>

// TODO:
// Tut.
//   http://www.justsoftwaresolutions.co.uk/threading/multithreading-in-c++0x-part-8-futures-and-promises.html
 
/// Advanced?
// Barrier
//

// TODO: если что-то вроде планеровщика в future and etc.
// http://stackoverflow.com/questions/14351352/will-asynclaunchasync-in-c11-make-thread-pools-obsolete-for-avoiding-expen

// Async Tasks in C++11: Not Quite There Yet
//   !!http://bartoszmilewski.com/2011/10/10/async-tasks-in-c11-not-quite-there-yet/
TEST(TBP, WrongInCpp11) {
  using namespace std;
  cout << "Main id:" << this_thread::get_id() << endl;
  
}

// Bartosz Milewski
// http://www.youtube.com/watch?v=80ifzK3b8QQ&list=PL1835A90FC78FF8BE
void thFun(int i, int& j) 
{
   std::cout << "Hi " << i << j << std::endl; 
}

void test(std::vector<std::thread>& workers) {
  using namespace std;
  for (int i = 0; i < 10; ++i) {
    // i - shared!!!
    // races
    
    // i !! by value
    auto action = [i]() {
      cout << "Hi from th: " << i << "\n";
    };
    
    // DANGER: it's main stack thread!
    int j = i;
    
    auto th = thread(thFun, i, ref(j));  // вообще проблема - висячие ссылки - not correct, but compiled
    assert(th.joinable());  // может быть detouched?
    workers.push_back(move(th));//th);
    
    // Can join here - тогда значение перед. по ссылке будет валидным
    
  }
}

TEST(TBP, BanchOfThreads) {
  using namespace std;
  vector<thread> workers;

  // 1. ...
  /*auto builder = [action]() {
    return thread(action);
  };
  */
  
  //workers.assign(10, thread(action)));  // thread non-copyble
  //adobe::generate_n(workers, 10, builder);  // для созданных и в адобовской либе _n нет
  
  test(workers);
  // join all
  //adobe::
  for_each(begin(workers), end(workers), mem_fun_ref(&thread::join));
  
  // 2, Move sem., passing to thread
  
  // 3. shared data
  // Pro:
  //  speed
  //
  // Cons:
  //   hard
  //
  // еще можно передавать по значению или делать move
  
  // 4. return from thread - input thread channel - promises - O - future
  
  // 5. Tasks
}

namespace li {
struct List {
  struct Node {
    int x_;
    Node* next_;
    Node(int y) : x_(y), next_(nullptr) {}
  };  
  Node* head_;
  List() : head_(nullptr) {}

  void insert(int x) {
    auto node = new Node(x);
    // looks exception safe - it's ptrs
    {
    node->next_ = head_;
    head_ = node;
    }
  }

  int count() const {
    int n = 0;
    auto cur = head_;
    while (cur != nullptr) {
      ++n;
      cur = cur->next_;
    }
    return n;
  }
};

void f(List& l) {
  for (int i = 0; i < 100; ++i)
    l.insert(i);
}

TEST(CC11, ConcurList) {
  using namespace std;
  vector<thread> workers;
  
  // Scope
  {
    List l;  // legal in scope
    for (int i = 0; i < 10; ++i) {
      auto th = thread(f, ref(l));  // вообще проблема - висячие ссылки - not correct, but compiled
      workers.push_back(move(th));//th);
    }
    for_each(begin(workers), end(workers), mem_fun_ref(&thread::join));
    int t = l.count();
    //assert(t == 1000);
  }
} 
}

namespace th_communic {
using namespace std;
void thFun(promise<string>&& prms) {   // можно и по ссылке, тогда promise not invalidate
  // Exceptions 
  // good функция потока сделать в try - catch
  try {
    throw runtime_error("From future");
    prms.set_value("Hello"); // DANGER: Move
  } catch (...) {
    prms.set_exception(std::current_exception());
  }
}

string func() { 
  throw runtime_error("From future");
  return "hello";
}

TEST(ThCpp11, Comm) {
  promise<string> prms;  // make shared state
  // 
  future<string> ftr = prms.get_future();  // можно и в одном потоке использовать - что-то связанное с исключениями
  
  
  // future getted befor thread
  thread th(&thFun, move(prms));
  
  try {
    // DANGER: если исключени не перехватить, то странно завершитсья
    string str = ftr.get(); // invalidate shared state
  } catch (...) {}
  
  th.join();  // else progr. be terminated
}

TEST(ThCpp11, CommShorter) {
  auto ftr = std::async(&func); 
  // DANGER: если исключени не перехватить, то все нормально
  // rethrow 
  EXPECT_THROW(ftr.get(), runtime_error); // invalidate shared state
  
  // если ничего не возвращеть, то join как бы вызоветсья? main no ended
}

// TODO:
// acync and thread pools
// http://stackoverflow.com/questions/15666443/which-stdasync-implementations-use-thread-pools

/// DANGER: TROUBLE
TEST(ThCpp11, TaskBP) {
  // async - может и не создавать цвет - по умолчанию система сама решает
  // Task - missed by standart
}

TEST(ThCpp11, MapReduce) {
  
}

TEST(ThCpp11, Sync) {
  
}

/// Message passing - procuder/consumer
// wait -> notify. DANGER: no state - if noti. -> wait - no waik
// DANGER: Spurious wakeups !!! Add additional state
/* 
// http://www.youtube.com/watch?v=309Y-QlIvew&index=9&list=PL1835A90FC78FF8BE
Producer
v = true;
cond.notify_one();

Consumer:
while(!v) {
  cond.wait(?);
}

Lock protocol

{
  lock_guard(mtx);
  v = true;
}
cond.notify_one();

// DANGER: Atomically unlock and enter wait state!!
unique_lock lck(mtx);
while (!v) {
  cond.wait(lck);  // похоже по этому передается лок
  ..
  v = false;
}

OR

un..
cond.wait(lck, [&v](){ return v; });
v = false;
*/
}
