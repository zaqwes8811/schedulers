#ifndef AAW_ARCH_H_
#define AAW_ARCH_H_

#include "actors_cc98.h"

#include <boost/shared_ptr.hpp>

#include <algorithm>
#include <map>
#include <string>

#ifndef FROM_HERE
#define FROM_HERE ""
#endif

// FIXME: incapsulate thread id's
// Static and global - lifetime troubles
class Threads {
public:
  enum Ids
  { DB };

  static void post(Ids id, SingleWorker::Callable fun);

private:
  static std::string decodeId(Ids id) {
    std::map<int, std::string> m;
    m[DB] = dbId();

    if (m.find(id) == m.end())
      throw std::runtime_error(FROM_HERE);

    return m[id];
  }

  static std::string dbId() {
    auto p = get().lock();
    if (!p)
      throw std::runtime_error(FROM_HERE);
    return p->getId();
  }

  Threads();

  static std::shared_ptr<SingleWorker> s_dbWorker;  // make weak access

  static std::weak_ptr<SingleWorker> get() {
    return s_dbWorker;
  }
};

#endif
