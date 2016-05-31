#ifndef AP_DS_H_
#define AP_DS_H_

#include <boost/noncopyable.hpp>
#include <boost/thread/mutex.hpp>
#include <boost/thread.hpp>

#include <list>

namespace fix_extern_space {
// Существующие интерфейсы
// Boost - lookfree ds
// TBB - best but...
// С++11 - потокобезопасные контейнеры - таких там нет
//   http://stackoverflow.com/questions/15278343/c11-thread-safe-queue

// Intel TBB
template <typename T>
class concurent_queue_ : public boost::noncopyable {
public:
  concurent_queue_() {}

  void push(const T& source);
  bool try_pop(T& destination);

  bool empty() const;  // may not be precist cos pending
};

template <typename T>
class concurent_bounded_queue : public boost::noncopyable  {
public:
  concurent_bounded_queue() {}

  void push(const T& source);
  bool try_pop(T& destination);

  bool empty() const;
};

//
// TODO: очередь от Шена
// http://channel9.msdn.com/Events/GoingNative/2013/Cpp-Seasoning
//
// В вопросах кто-то не лестно отозвался о реализации, но что он сказал?
template <typename T>
class concurent_queue
{
  boost::mutex mutex_;
  std::list<T> q_;
public:
  void enqueue(T x) {
    // allocation here
    std::list<T> tmp;
    tmp.push_back(x);//move(x));

    // threading
    {
      boost::lock_guard<boost::mutex> lock(mutex_);
      // вроде бы константное время
      q_.splice(end(q_), tmp);

      // для вектора может неожиданно потреб. реаллокация
    }
  }

  // ...
};

/** Pro
   http://stackoverflow.com/questions/20488854/implementation-of-a-bounded-blocking-queue-using-c

 https://www.threadingbuildingblocks.org/docs/help/reference/containers_overview/concurrent_bounded_queue_cls.htm
 Assume all includes are there

  http://www.boost.org/doc/libs/1_35_0/doc/html/thread/synchronization.html#thread.synchronization.condvar_ref.condition_variable_any
*/
struct BoundedBlockingQueueTerminateException
    : virtual std::exception,
      virtual boost::exception { };

// http://stackoverflow.com/questions/5018783/c-pthread-blocking-queue-deadlock-i-think
/**
  \param T - assign operator, copy ctor

*/
template<typename T>
class concurent_try_queue : public boost::noncopyable {
public:
  // types
  typedef T value_type;
  //typedef A allocator_type;
  typedef T& reference;
  typedef const T& const_reference;

  explicit concurent_try_queue(int size)
      : cm_size(size), m_nblocked_pop(0), m_nblocked_push(0)
      , m_with_blocked(false)
      , m_stopped(false)
      , m_current_size(0)
  {
    if (cm_size < 1){
        // BOOST_THROW_EXCEPTION
    }
  }

  ~concurent_try_queue() {
    //stop(true);
  }

  bool empty() {
    boost::mutex::scoped_lock lock(m_mtx);
    return m_q.empty();
  }

  std::size_t size() {
    boost::mutex::scoped_lock lock(m_mtx);
    return m_current_size;
    //return m_q.size();  // O(n) for list
  }

  bool try_push(const T& x) {
    std::list<T> tmp;
    tmp.push_back(x);

    {
      boost::mutex::scoped_lock lock(m_mtx);
      //if (m_q.size() == cm_size)//size)
      //if (size() == cm_size)//size)  // self deadlock
      if (m_current_size == cm_size)//size)
          return false;

      //m_q.push(x);  // bad
      //m_q.splice(boost::end(m_q), tmp);
      m_q.splice(m_q.end(), tmp);
      m_current_size++;
    }
    //if (m_with_blocked) m_pop_cv.notify_one();
    return true;
  }

  bool try_pop(T& popped) {
    {
      boost::mutex::scoped_lock lock(m_mtx);
      if (m_q.empty())
          return false;

      popped = m_q.front();
      m_q.pop_front();  // pop()
      m_current_size--;
    }
    //if (m_with_blocked) m_push_cv.notify_one();
    return true;
  }

private:
  void stop(bool wait) {
    boost::mutex::scoped_lock lock(m_mtx);
    m_stopped = true;
    m_pop_cv.notify_all();
    m_push_cv.notify_all();

    if (wait) {
      while (m_nblocked_pop)
        m_pop_cv.wait(lock);
      while (m_nblocked_push)
        m_push_cv.wait(lock);
    }
  }

  /**
    \brief blocked call
  */
  void wait_and_pop(T& popped) {
    boost::mutex::scoped_lock lock(m_mtx);

    ++m_nblocked_pop;

    while (!m_stopped && m_q.empty())
      m_pop_cv.wait(lock);

    --m_nblocked_pop;

    if (m_stopped) {
      m_pop_cv.notify_all();
      BOOST_THROW_EXCEPTION(BoundedBlockingQueueTerminateException());
    }

    popped = m_q.front();
    m_q.pop_front();//pop();

    m_push_cv.notify_one();  // strange
  }

  void wait_and_push(const T& item) {
    {
      boost::mutex::scoped_lock lock(m_mtx);

      ++m_nblocked_push;
      while (!m_stopped && m_q.size() == size())
        m_push_cv.wait(lock);
      --m_nblocked_push;

      if (m_stopped) {
        m_push_cv.notify_all();
        BOOST_THROW_EXCEPTION(BoundedBlockingQueueTerminateException());
      }

      m_q.push(item);
    }
    m_pop_cv.notify_one();
  }

private:
  int m_current_size;  // for std::list
  std::
  //queue
  list
  <T> m_q;  // FIXME: to list
  boost::mutex m_mtx;
  boost::condition_variable_any m_pop_cv; // q.empty() condition
  boost::condition_variable_any m_push_cv; // q.size() == size condition
  int m_nblocked_pop;
  int m_nblocked_push;
  bool m_stopped;
  const int cm_size;
  const bool m_with_blocked;
};
}  // space
#endif
