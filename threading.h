#ifndef THREADING_H
#define THREADING_H

#include <boost/thread/future.hpp>
#include <boost/thread/recursive_mutex.hpp>
#include <boost/asio.hpp>
#include <boost/function/function0.hpp>
#include <boost/thread/lock_guard.hpp>
#include <boost/weak_ptr.hpp>
#include <boost/scoped_ptr.hpp>

#include <iostream>
#include <cassert>

//===========================================================

namespace base
{
typedef boost::function0<void> Closure;

// fixme: сделать разделение на success/failure
// но проблема все равно остается
//
// Этот поток с http сервеном все портит
struct Future
{

private:
	//Promise p;  // видимое для success/failure
	//boost::promise<>
};
}

class executor
{
public:
	virtual ~executor() {}
	virtual void post( base::Closure t ) = 0;
	virtual void terminate() = 0;
};

//===========================================================

class work_executor : public executor
{
public:
	work_executor(boost::asio::io_service* io)
	{
		this->io = io;
	}

	void post( base::Closure t )
	{
		if( io ){
			io->post( t );
		}
	}

	void terminate()
	{
		if( io ){
			io->stop();
		}
	}
private:
	boost::asio::io_service* io;
};

//================================================

class executors
{
public:
	// fixme: Очень проблеманя функция - она блокирует поток, не ясно как в некоторых других
	//   случаях получить ответ из другого потока
	template <typename T, typename Func>
	static T post( int id, Func func )
	{
		using namespace boost;
		shared_ptr<packaged_task<T> > t =
				shared_ptr<packaged_task<T> >( new packaged_task<T>( func ) );

		shared_ptr<future<T> > f( new future<T>( t->get_future() ) );

		post_task( id, bind(&packaged_task<T>::operator(), t) );
		return f->get();
	}

	static void post_task( int id, base::Closure job );

	static void add( int id, boost::weak_ptr<executor> e )
	{
		using namespace boost;
		lock_guard<recursive_mutex> _(mtx_);
		id_to_exec_[ id ] = e;
	}

	static void terminate_all();
private:
	executors();
	static std::map< int, boost::weak_ptr<executor> > id_to_exec_;
	static boost::recursive_mutex mtx_;
};

//=======================================================

// Есть проблемы при завершении процесса
class asio_executor : public executor
{

	static void real_run(boost::asio::io_service* io)
	{
		using namespace boost::asio;
		io_service::work w(*io);
		io->run();
	}

public:
	asio_executor();
	~asio_executor();
	void post( base::Closure t );
	void terminate();

private:
	boost::asio::io_service io_;
	boost::scoped_ptr<boost::thread> thread_;
};

//===================================================

// позволяет выполнять оставшиеся в очереди задачи при завершении
class spec_executor : executor
{

};

//===================================================

std::string current_thread_id();

#endif
