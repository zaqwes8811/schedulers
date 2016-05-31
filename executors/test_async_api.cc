#define BOOST_THREAD_PROVIDES_FUTURE

#include "actors_and_workers/actors_cc98.h"
#include "actors_and_workers/actors_cc11.h"

#include <gtest/gtest.h>
#include <boost/thread/future.hpp>

#include <memory>
#include <thread>
#include <functional>
#include <string>
#include <future>

/**
@Use case: Looks like good real use case
void CallerMethod() {
  // …

  // launch work asynchronously (in any
  // fashion; for yuks let's use a thread pool)
  // note that the types of "result" and
  // "outTheOther" are now futures.
  result = pool.run( ()=>{
    DoSomething( this, that, outTheOther ) } );

  // These could also take a long time
  // but now run concurrently with DoSomething
  OtherWork();
  MoreOtherWork();

  // … now use result.wait() (might block) and outOther…
}

@Use case:
// .then()
// "Up to this point we have skirted around the matter of waiting for Futures.
You may never need to wait for a Future, because your code is event-driven and all
follow-up action happens in a then-block."

@Use case:
//   Trouble - interliver - operations MUST not have shared state!
//   Else data races!

@Use case:
private generator with incaps. state
*/

class Backgrounder {
public:
  typedef int Data;
  typedef int PrivateData;

  void Save( std::string filename ) { a.Send( [=] {
    //…
  } ); }

  void Print( Data& data ) { a.Send( [=, &data] {
    //…
  } ); }

private:
  PrivateData somePrivateStateAcrossCalls;
  cc11::Actior a;
};

TEST(Sutter, ActiveObj) {
  Backgrounder b;
  b.Save("hello");
}

// result
// http://www.drdobbs.com/cpp/prefer-using-futures-or-callbacks-to-com/226700179?pgno=1
class Backgrounder_ret {
public:
  typedef int Data;
  typedef int PrivateData;

  std::future<bool> Save( std::string filename ) {
    using std::future;
    using std::promise;
    auto p = std::make_shared<promise<bool> > ();
    future<bool> ret = p->get_future();
    a.Send( [=] { p->set_value( true ); } );
    return ret;
  }

  void Print( Data& data ) { a.Send( [=, &data] {
    //…
  } ); }

private:
  PrivateData somePrivateStateAcrossCalls;
  cc11::Actior a;
};

class BackgrounderBoost {
public:
  typedef int Data;
  typedef int PrivateData;

  boost::future<bool> Save( std::string filename ) {
    using boost::future;
    using boost::promise;
    auto p = std::make_shared<promise<bool> > ();
    future<bool> ret = p->get_future();
    a.Send( [=] { p->set_value( true ); } );
    return ret;
  }

  void Print( Data& data ) { a.Send( [=, &data] {
    //…
  } ); }

  void Save(
      std::string filename,
      std::function<void(
        //bool
        )> returnCallback
    ) {
    a.Send( [=] {
      // … do the saving work …
      returnCallback( );//true );//didItSucceed() ? true : false );
    } ); }

private:
  PrivateData somePrivateStateAcrossCalls;
  cc11::Actior a;
};

TEST(Sutter, PullReturn) {
  Backgrounder_ret b;
  auto r = b.Save("h");

  // "pull" ops!!
  r.get();
}

class MyGUI {
public:
  // …

  // When the user clicks [Save]
  void OnSaveClick() {
    // …
    // … turn on saving icon, etc. …
    // …
    std::shared_future<bool> result;
    result = backgrounder.Save( "filename" );
    // queue up a continuation to notify ourselves of the
    // result once it's available

    // FIXME: Is it bad?
    std::async( [=] {
        //SendSaveComplete
            OnSaveComplete
            ( result.get() ); } );
  }


  void OnSaveComplete( bool returnedValue ) {
    // … turn off saving icon, etc. …
  }
private:
  Backgrounder_ret backgrounder;
};

class MyGUI_2b {
public:
  // …

  // When the user clicks [Save]
  void OnSaveClick() {
    // …
    // … turn on saving icon, etc. …
    // …
    boost::shared_future<bool> result;
    result = backgrounder.Save( "filename" );
    // queue up a continuation to notify ourselves of the
    // result once it's available

    // FIXME: Is it bad?
    // no then() in std and boost
    //result.( [=] {
    //    //SendSaveComplete
    //        OnSaveComplete
    //       ( result.get() ); } );
  }

  void OnSaveClick_cb() {
    //boost
    //std::shared_future<bool> result;  // FIXME: Is it needed here?
    backgrounder.Save( std::string("filename"),
           ([=]
      {
        //OnSaveComplete( result.get() );
      })
    );
  }

  void OnSaveComplete( bool returnedValue ) {
    // … turn off saving icon, etc. …
  }
private:
  BackgrounderBoost backgrounder;
};


// FIXME: partial result

// Build async API
// cases
//






