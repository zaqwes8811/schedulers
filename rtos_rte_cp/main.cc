
#include <gtest/gtest.h>

#include <thread>
#include <cstring>

using namespace testing;

//===========================================================================================
//===========================================================================================
//===========================================================================================

pthread_attr_t fifo_sched_attr;
pthread_attr_t orig_sched_attr;
struct sched_param fifo_param;

#define SCHED_POLICY SCHED_FIFO
#define MAX_ITERATIONS (1000000)


void print_scheduler(int schedType) {


  switch (schedType) {
    case SCHED_FIFO:
      printf("Pthread policy is SCHED_FIFO\n");
      break;
    case SCHED_OTHER:
      printf("Pthread policy is SCHED_OTHER\n");
      break;
    case SCHED_RR:
      printf("Pthread policy is SCHED_RR\n");
      break;
    default:
      printf("Pthread policy is UNKNOWN\n");
  }
}

void set_scheduler(void) {
  int max_prio, scope, rc, cpuidx;
  cpu_set_t cpuset;

  printf("INITIAL ");
  int schedType = sched_getscheduler(getpid());
  print_scheduler(schedType);

  pthread_attr_init(&fifo_sched_attr);
  pthread_attr_setinheritsched(&fifo_sched_attr, PTHREAD_EXPLICIT_SCHED);
  pthread_attr_setschedpolicy(&fifo_sched_attr, SCHED_POLICY);
  CPU_ZERO(&cpuset);
  cpuidx = (3);
  CPU_SET(cpuidx, &cpuset);
  pthread_attr_setaffinity_np(&fifo_sched_attr, sizeof(cpu_set_t), &cpuset);

  max_prio = sched_get_priority_max(SCHED_POLICY);
  fifo_param.sched_priority = max_prio;

  if ((rc = sched_setscheduler(getpid(), SCHED_POLICY, &fifo_param)) < 0)
    perror("sched_setscheduler");

  pthread_attr_setschedparam(&fifo_sched_attr, &fifo_param);

  printf("ADJUSTED ");
  print_scheduler(sched_getscheduler(getpid()));
}

void *starterThread(void *threadp) {
  sleep(10);


  return nullptr;
}

void starterThread_cpp() {
//  std::this_thread::sleep_for(std::chrono::seconds(1));

  sched_param sch;
  int policy;
  pthread_getschedparam(pthread_self(), &policy, &sch);
//  std::lock_guard<std::mutex> lk(iomutex);
  std::cout << "Thread " << 0 << " is executing at priority "
            << sch.sched_priority << " policy:" << policy << '\n';
  sleep(1);
}

TEST(Rt, Thread) {
  int rc;
  int i, j;
  cpu_set_t cpuset;

  // TODO for process? for main thread?
  set_scheduler();

  // get affinity set for main thread
  auto mainthread = pthread_self();

  // Check the affinity mask assigned to the thread
  rc = pthread_getaffinity_np(mainthread, sizeof(cpu_set_t), &cpuset);
  if (rc != 0)
    perror("pthread_getaffinity_np");
  else {
    printf("main thread running on CPU=%d, CPUs =", sched_getcpu());

    for (j = 0; j < CPU_SETSIZE; j++)
      if (CPU_ISSET(j, &cpuset))
        printf(" %d", j);

    printf("\n");
  }

  // Realtime threads
  // https://stackoverflow.com/questions/8408258/how-to-create-and-display-threads-attributes
  // https://stackoverflow.com/questions/18884510/portable-way-of-setting-stdthread-priority-in-c11 !!!
  // https://en.cppreference.com/w/cpp/thread/thread/native_handle
  std::thread t(starterThread_cpp);
  sched_param sch;
  int policy;
  pthread_getschedparam(t.native_handle(), &policy, &sch);
  sch.sched_priority = 20;

  if (pthread_setschedparam(t.native_handle(), SCHED_FIFO, &sch)) {
    std::cout << "Failed to setschedparam: " << std::strerror(errno) << '\n';
  }

  auto handle = t.native_handle();
//  int schedType = sched_getscheduler(t.);
//  print_scheduler(schedType);

  t.join();
//  t.
//  t.

//  pthread_t startthread;
//  pthread_create(&startthread,   // pointer to thread descriptor
//                 &fifo_sched_attr,     // use FIFO RT max priority attributes
//                 starterThread, // thread function entry point
//                 (void *) 0 // parameters to pass in
//  );
//
//  pthread_join(startthread, nullptr);
}

//===========================================================================================
//===========================================================================================
//===========================================================================================

int main(int argc, char **argv) {
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}