#include "time_triggered_arch/config.h"  // in every *.c or *.cc file
#include "time_triggered_arch/scheduler/cooperativeScheduler.h"
#include "time_triggered_arch/tasks/onChain.h"

//static
void schDispatch_void() {
  onSlot();
  //lockSlot();
  //ulockSlot();
  //offSlot();
  //otherSlot();
}

void schRunLoop_void() {
  while(1) {
    schDispatch_void();
    break;  // TODO: remove it
  }
}

static
void schStop_void() {
  // TODO: some action
}
