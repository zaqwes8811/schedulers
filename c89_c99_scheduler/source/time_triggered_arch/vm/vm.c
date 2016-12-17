// Virtual operation vith virtual devices
//
#include "time_triggered_arch/config.h"
#include "time_triggered_arch/vm/vm.h"
#include "time_triggered_arch/vm/virtualDvrPreampl.h"

static int currentChannel_ = 0;

void vmInitialize_void() {

}

static int _vmGetCurrentChannel() {
  return currentChannel_;
}

void vmResetCurrentChannel() {
  if (0 == currentChannel_) {
    // Detector
  } else if (1 == currentChannel_) {
    // Preampl
    preamplReset_void_Virtual();
  } else {

  }
}

void vmOnChannel_void() {
  if (0 == currentChannel_) {
    // Detector
  } else if (1 == currentChannel_) {
    // Preampl
  } else {

  }
}

static void _vmIncrementChannel() {
  currentChannel_++;
  if (GkMaxCountChannels == currentChannel_) {
    currentChannel_ = 0;
  }
}

static int _vmSomeUtilFunction_int() {
  return 0;
}

void vmSlotProcessI2CPkg() {

}
