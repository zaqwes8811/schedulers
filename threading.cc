#include "core/config.h"

#include "schedulers/threading.h"

#include <boost/lexical_cast.hpp>

using namespace std;
using namespace boost;

//======================================================

std::map< int, weak_ptr<executor> > executors::id_to_exec;
boost::recursive_mutex executors::mtx;

std::string current_thread_id()
{
	return boost::lexical_cast<std::string>(this_thread::get_id());
}

void executors::terminate_all()
{
	using namespace std;
	using namespace boost;
	lock_guard<recursive_mutex> _(mtx);
	for( map<int, weak_ptr<executor> >::iterator it = id_to_exec.begin();
			it != id_to_exec.end(); ++it ){
		shared_ptr<executor> e = it->second.lock();
		if( e ){
			e->terminate();
		}
	}

	id_to_exec.clear();
}

void executors::post_task( int id, base::Closure job )
{
	using namespace boost;
	using namespace std;
	lock_guard<recursive_mutex> _(mtx);
	map< int, weak_ptr<executor> >::iterator p = id_to_exec.find( id );
	if( p == id_to_exec.end() ){
		throw runtime_error(string());
	}

	shared_ptr<executor> e = p->second.lock();
	if( e ){
		e->post( job );
	}
	else{
		throw runtime_error(string());
	}
}


//==========================================================
asio_executor::asio_executor()
{
	th.reset(new boost::thread(boost::bind(asio_executor::real_run, &io)));
}
asio_executor::~asio_executor()
{
	using namespace std;
	th->join();
}

void asio_executor::post( base::Closure t )
{
	using namespace std;
	io.post( t );
}

void asio_executor::terminate()
{
	io.stop();
}
