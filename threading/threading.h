#ifndef AAW_ARCH_H_
#define AAW_ARCH_H_

#include <QtCore/QEvent>
#include <QtWidgets/QApplication>

#include <map>
#include <string>

//=====================================================

namespace base
{
typedef std::function<void()> Closure;
}

class executor
{
public:
	virtual ~executor() {}

	virtual void post( base::Closure job ) = 0;

	static std::string current_thread_id();
};

//====================================================

class executors
{
public:
	static void post_task( int id, base::Closure fun );
  	static void add( int id, executor* e );
private:
  	executors();
    static std::map< int, executor* > id_to_exec_;
};

//====================================================

class qt_executor : public QApplication, public executor
{
public:
	qt_executor( int argc, char **argv );

	//executor
	void post(base::Closure job);
private:
	void customEvent(QEvent* e);
};

#define DCHECKTHREAD( id ) \
	assert( executor::current_thread_id() == executors::byId( id ));

#endif
