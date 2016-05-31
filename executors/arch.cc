
#include "arch.h"

std::shared_ptr<SingleWorker> Threads::s_dbWorker(new SingleWorker);


void Threads::post(Ids id, SingleWorker::Callable fun) {
  auto p = get().lock();
  if (!p)
    throw std::runtime_error(FROM_HERE);
  return p->post(fun);
}
