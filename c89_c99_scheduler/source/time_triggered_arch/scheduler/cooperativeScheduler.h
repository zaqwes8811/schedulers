#ifndef CO_OPERATIVE_SCH_H_
#define CO_OPERATIVE_SCH_H_

// INFO: source idea "Patterns for time-triggered embedded systems"

// FIXME: how make task delay expended without add same new
//   Task is planned but we prolongate it
// FIXME: Tasks is unique! Maybe put not pointer but struct
//  fptrs is unique

void schRunLoop_void();
void schDispatch_void();

// About:
//   Non deleted ever!
void schAddPeriodicTask();

// About:
//   Note
void schAddSelfDeletedTask();

#endif  // CO_OPERATIVE_SCH_H_
