// (c) : http://stackoverflow.com/questions/15278343/c11-thread-safe-queue

#ifndef SAFE_QUEUE
#define SAFE_QUEUE

#include <boost/noncopyable.hpp>

#include <queue>
#include <mutex>
#include <condition_variable>

namespace concurent {

/**
  \brief

  \attention
  If producer speed(task duration) bigger speed consumer queue be grow!
  This case if consumer work much faster then producers put work to queue.

  \fixme need up boundary

*/
template <class T>
class message_queue : public boost::noncopyable
{
public:
	// FIXME: need done() method
	//   See Sean Parent:Better Code:Concur.

  message_queue(void)
    : q()
    , m()
    , c()
  {}

  void enqueue(T t)
  {
    std::lock_guard<std::mutex> lock(m);
    q.push(t);
    c.notify_one();
  }

  T dequeue(void)
  {
    std::unique_lock<std::mutex> lock(m);
    while(q.empty())
    {
      c.wait(lock);
    }
    T val = q.front();
    q.pop();
    return val;
  }

private:
  std::queue<T> q;
  mutable std::mutex m;
  std::condition_variable c;
};
}
#endif
