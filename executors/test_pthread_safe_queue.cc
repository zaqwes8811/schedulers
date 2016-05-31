/**
// TODO: multiple consumers 
// TODO: нельзя прочитать до истечения некоторого интервала времени после вставки
// TODO: а как разблокировать очередь - завершить поток? wait some time?

*/
 
#include <boost/noncopyable.hpp>
//#include <boost/lockfree/queue.hpp>
#include <gtest/gtest.h>

#include <list>
#include <iostream>

// DANGER: unlock unlocked - undef. beh.
#include <pthread.h>

using std::list;

// Part1
// http://www.ibm.com/developerworks/ru/library/au-multithreaded_structures1/
// No exception safe
namespace concurent {
template <typename T>
class QueuePthread {
public:
  QueuePthread() {
    pthread_mutex_init(&_lock, NULL);
  }
  ~QueuePthread() {
    pthread_mutex_destroy(&_lock);
  }
  
  void push(const T& value) {
    pthread_mutex_lock(&_lock);
    _list.push_back(value);  // лучше врезка нового списка и вне критической секции
    pthread_mutex_unlock(&_lock);
  }
  
  // вообще из сетторов по значению возвращать не стоит
  // но операция должна быть атомарной
  T pop(/*T& r - тоже плохо, т.к. возвращаем хэндл, нет это возвращение ссылки на то чего быть не должно*/) {
    // TODO: а то что не под локом это нормально?
    if (_list.empty()) {
      throw "Elem not founded.";
    }
    
    pthread_mutex_lock(&_lock);
    T _temp = _list.front();
    _list.pop_front();
    pthread_mutex_unlock(&_lock);
    return _temp;
  }
  
  //  Плохо, т.к. возвращаем хэндл - нет - может не так и плохо
  // нет это возвращение ссылки на то чего быть не должно - это из Саттера, но не про то - T& pop();
  // 
  // похоже этот вариант лучший
  void pop(T& r) {
    // TODO: а то что не под локом это нормально?
    if (_list.empty()) {
      throw "Elem not founded.";
    }
    
    pthread_mutex_lock(&_lock);
    T _temp = _list.front();
    r = _temp;
    _list.pop_front();
    pthread_mutex_unlock(&_lock);
  }
  
private:
  // DANGER: !!!
  QueuePthread(const QueuePthread&);
  QueuePthread& operator=(const QueuePthread&);
  
  list<T> _list;
  pthread_mutex_t _lock;
};

// No correct:
// Возможно с двумя списками да
// http://codereview.stackexchange.com/questions/41604/thread-safe-concurrent-fifo-queue-in-c
template <typename T>
class QueuePthreadV2 {
public:
  QueuePthreadV2() {
    pthread_mutex_init(&_rlock, NULL); pthread_mutex_init(&_wlock, NULL);
  }
  ~QueuePthreadV2() {
    pthread_mutex_destroy(&_rlock);  pthread_mutex_destroy(&_wlock);
  }
  
  void push(const T& value) {
    pthread_mutex_lock(&_wlock);
    _list.push_back(value);
    pthread_mutex_unlock(&_wlock);
  }
  
  // вообще из сетторов по значению возвращать не стоит
  // но операция должна быть атомарной
  T pop() {
    // TODO: а то что не под локом это нормально?
    if (_list.empty()) {
      throw "Elem not founded.";
    }
    
    pthread_mutex_lock(&_rlock);
    T _temp = _list.front();
    _list.pop_front();
    pthread_mutex_unlock(&_rlock);
    return _temp;
  }
  
private:
  list<T> _list;
  
  // DANGER: !!!
  QueuePthreadV2(const QueuePthreadV2&);
  QueuePthreadV2& operator=(const QueuePthreadV2&);
  
  // http://hardclub.donntu.edu.ua/projects/qt/qq/qq11-mutex.html
  pthread_mutex_t _rlock, _wlock;  // TODO: а как так? А не должен быть один?
};

template <typename T>
class BlockedQueue
 : public boost::noncopyable  // DANGER: вообще то зависимотсь от буста
 {
public:
  //
  typedef T value_type;
  
  // construct/destroy/copy
  BlockedQueue() {
    pthread_mutex_init(&lock_, NULL);
    pthread_cond_init(&cond_, NULL);
  }
  
  ~BlockedQueue() {
    pthread_mutex_destroy(&lock_);
    pthread_cond_destroy(&cond_);
  }
  
  void push(const T& src) {
    //{
      pthread_mutex_lock(&lock_);
      const bool was_empty = list_.empty();
      list_.push_back(src);  // bad onder lock
      pthread_mutex_unlock(&lock_);
    //}
    
    if (was_empty)
      pthread_cond_broadcast(&cond_);
  }
  
  void try_pop(T& dst) {
    // http://stackoverflow.com/questions/2763714/why-do-pthreads-condition-variable-functions-require-a-mutex
    
    // DANGER: спонтанные просыпания!!
    pthread_mutex_lock(&lock_);
    /// atomic
    while (list_.empty())  // так правильнее - защита от пробуждений
    //if (list_.empty()) 
    {
    /// atomic
      // Q: мьютекс разблокируется?
      // Чтобы дать producer'у разблокироваться
      // Мьютек должен быть захвачен
      pthread_cond_wait(&cond_, &lock_);  
      // "magically locked again"
    }
    
    dst = list_.front();
    list_.pop_front();
    
    // http://stackoverflow.com/questions/1778780/if-you-unlock-an-already-unlocked-mutex-is-the-behavior-undefined
    pthread_mutex_unlock(&lock_); // не опасно ли? 
  }
  
private:
  list<T> list_;
  // http://bytes.com/topic/c/answers/493246-copying-mutexes-cv-pthread_ts
  pthread_mutex_t lock_;
  pthread_cond_t cond_;
};

// V0 - hard now - DANGER: в статье много ошибок
template <typename T>
class BlockedTimedQueue
 : public boost::noncopyable  // DANGER: вообще то зависимотсь от буста
 {
public:
  //
  typedef T value_type;
   
private:
   list<T> list_;
  // http://bytes.com/topic/c/answers/493246-copying-mutexes-cv-pthread_ts
  pthread_mutex_t lock_;
  pthread_cond_t cond_;
};

template <typename T>
class BoundedBlockedQueue
 : public boost::noncopyable  // DANGER: вообще то зависимотсь от буста
 {
public:
  //
  typedef T value_type;
  
  // construct/destroy/copy
  explicit BoundedBlockedQueue(int size) : size_(0), maxSize_(size) {
    pthread_mutex_init(&lock_, NULL);
    pthread_cond_init(&rcond_, NULL);
    pthread_cond_init(&wcond_, NULL);
  }
  
  ~BoundedBlockedQueue() {
    pthread_mutex_destroy(&lock_);
    pthread_cond_destroy(&rcond_);
    pthread_cond_destroy(&wcond_);
  }
  
  void push(const T& src) {
    // Resize vector - non thread safe
    pthread_mutex_lock(&lock_);
    const bool was_empty = list_.empty();
    while (get_size_() == maxSize_) {
      pthread_cond_wait(&wcond_, &lock_);
    }
    
    list_.push_back(src);
    ++size_;
    pthread_mutex_unlock(&lock_);
    if (was_empty)
      pthread_cond_broadcast(&rcond_);
  }
  
  void pop(T& dst) {
    pthread_mutex_lock(&lock_);
    const bool was_full = (get_size_( ) == maxSize_);
    while (list_.empty()) {
      pthread_cond_wait(&rcond_, &lock_);
    }
    dst = list_.front();  // may throw - assign
    
    list_.pop_front();  // no throw
    --size_;
    
    pthread_mutex_unlock(&lock_);
    if (was_full)
      pthread_cond_broadcast(&wcond_);
  }
   
private:
  int get_size_() const { return size_; }
  list<T> list_;  // size() - O(n)
  int size_;  // list size - O(1)
  const int maxSize_;
  
  // не тужен вектор - его длина - O(1) - удаление из начала очень дорогая операция
  //vector<T> array_;  
  
  pthread_mutex_t lock_;
  pthread_cond_t rcond_, wcond_;
};
}

