// (c) from Sutter

#ifndef S_ACTORS_CC11_H_
#define S_ACTORS_CC11_H_

#include "safe_queue_cc11.h"
#include "common/error_handling.h"

#include <thread>
#include <memory>
#include <string>

namespace cc11 {
// Example 2: Active helper, in idiomatic C++(0x)
//
class Actor {
public:
    typedef std::function<void()> Message;
    typedef std::function<void()> Callable;

    Actor() : done(false)
    { thd = std::unique_ptr<std::thread>(new std::thread( [=]{ this->Run(); } ) ); }

    ~Actor() {
      post( [&]{ done = true; } ); ;
      thd->join();
    }

    void post( Message m )
    {
      try {
        mq.enqueue( m );
      } catch (...) {
        throw infrastructure_error(FROM_HERE);
      }
    }

private:

  Actor( const Actor& );           // no copying
  void operator=( const Actor& );    // no copying

  bool done;                         // le flag
  concurent::message_queue<Message> mq;        // le queue
  std::unique_ptr<std::thread> thd;          // le thread

  void Run() {
    while( !done ) {
      try {
        auto msg = mq.dequeue();
        msg();            // execute message
      } catch (fatal_error&) {
        throw;
      } catch (...) {
        // FIXME: don't know what to do
        throw;
      }

    } // note: last message sets done to true
  }
};
}


#endif
