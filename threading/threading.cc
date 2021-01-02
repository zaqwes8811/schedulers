#include "threading.h"

#include <boost/lexical_cast.hpp>
#include <QtWidgets/QApplication>

#include <iostream>
#include <cassert>
#include <thread>

using namespace std;

std::string executor::current_thread_id()
{
	return boost::lexical_cast<std::string>(std::this_thread::get_id());
}

//==================================================

std::map< int, executor* > executors::id_to_exec_;

void executors::post_task( int id, base::Closure job )
{
	auto r = id_to_exec_.find( id );
	if( r == id_to_exec_.end() ){
		return;
	}

	if( r->second ){
		r->second->post( job );
	}
}

void executors::add( int id, executor* e )
{
	id_to_exec_[ id ] = e;
}

//==================================================

static const auto callable_type = QEvent::Type(1001);

class job_event : public QEvent
{
public:
	job_event( base::Closure job ) : QEvent( callable_type )
	{
		this->job = job;
	}
	base::Closure job;
};

qt_executor::qt_executor( int argc, char **argv ) :
		QApplication( argc, argv )
{ }

void qt_executor::post(base::Closure job)
{
	QApplication::postEvent( this, new job_event( job ) );
}

void qt_executor::customEvent(QEvent* e)
{
	if( e->type() == callable_type ){
		job_event* jobevent = (job_event*)e;
		jobevent->job();
	}
};

