#include "time_triggered_arch/config.h"

#ifdef __cplusplus
extern "C" {
#endif

#include "time_triggered_arch/vm/vm.h"
#include "time_triggered_arch/scheduler/cooperativeScheduler.h"
#include "time_triggered_arch/tasks/onChain.h"
#include "time_triggered_arch/scheduler_raw/sch52.h"

#ifdef __cplusplus
}
#endif

#include <gtest/gtest.h>

TEST(BlockedLoop, Create) {
  vmInitialize_void();

  // Just example
  onSignal();

  // Launch event loop
  schDispatch_void();
}

TEST(SchPair, Base) {
  // Set up the scheduler
  SCH_Init_T2();

  // Prepare for the 'Flash_LED' task
  //LED_Flash_Init();
  // Add the 'Flash LED' task (on for ~1000 ms, off for ~1000 ms)
  // - timings are in ticks (1 ms tick interval)
  // (Max interval / delay is 65535 ticks)
  //SCH_Add_Task(LED_Flash_Update, 0, 1000);

  // Start the scheduler
  SCH_Start();

  while(1) {
    SCH_Dispatch_Tasks();
    break;  // FIXME: remove it
  }

  // FIXME: how replane on spot task

  // Add periodic tasks

  // Other tasks added from periodic tasks
  while(1) {
    //schDispatchPeriodic();

    // Self deleted tasks
    //schDispatchOneSpot();
    break;
  }
}
