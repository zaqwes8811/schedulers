#include "threading.h"

#include <QtWidgets/QApplication>
#include <QtCore/QEvent>
#include <boost/asio.hpp>
#include <boost/thread.hpp>
#include <boost/bind.hpp>

#include <thread>
#include <iostream>
#include <functional>
#include <memory>

using namespace std;
using std::string;
using std::cout;

using std::thread;  // тут будет зависеть, хотя

enum eThreadIds { DB, UI };

void job();

class asio_pool : public executor
{

public:
	~asio_pool()
	{
		svc.stop ();
		threads.join_all ();
		delete th;
	}

	void start()
	{
		th = new std::thread( std::bind( &asio_pool::thread_func, &(*this) ) );
	}

	void post( base::Closure job )
	{
		svc.post( job );
	}

private:
	boost::asio::io_service svc;  // 2.
	boost::thread_group threads;

	void thread_func()
	{
		executors::add( DB, this );
		// https://gist.github.com/snaewe/1393807
		int tNumber = 4;
		boost::asio::io_service::work work( svc );
		for (int i = 0; i < tNumber; ++i ){
			// http://think-async.com/Asio/Recipes
			// http://stackoverflow.com/questions/12577035/creating-a-boostasio-worker-thread-with-boostthread-under-linux
			auto f = static_cast< size_t (boost::asio::io_service::*) () >(&boost::asio::io_service::run);
			threads.create_thread ( std::bind( f, &svc ) );
		}
	}

	std::thread* th;
};

//================================================================

class Window
{
public:
	void done( int val )
	{
//		DCHECKTHREAD( UI );
		executors::post_task( DB, job );
		cout << "h" << endl;
	}
};

Window gw;

void job( )
{
//	DCHECKTHREAD( DB );

	this_thread::sleep_for( chrono::seconds( 1 ) );
	int val = 10;
	executors::post_task( UI, bind( &Window::done, gw, val ) );
	cout << "e" << endl;
}

int main(int argc, char **argv)
{
	qt_executor app( argc, argv );
	asio_pool a;
	a.start();

	executors::add( UI, &app );

	executors::post_task( DB, job );

	cout << "before" << endl;

	return app.exec();
}
