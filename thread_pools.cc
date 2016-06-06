#include "thread_pools.h"

#include <boost/bind.hpp>

namespace thread_pools {
AsioThreadPool::AsioThreadPool() : m_io_service(), m_threads(), m_work(m_io_service)
  , cm_size(3)  {
  for (int i = 0; i < cm_size; ++i)
    m_threads.create_thread(boost::bind(&boost::asio::io_service::run, &m_io_service));
}

AsioThreadPool::~AsioThreadPool() {
  m_io_service.stop();
  m_threads.join_all();
}

boost::asio::io_service& AsioThreadPool::get() {
  return m_io_service;
}

}
