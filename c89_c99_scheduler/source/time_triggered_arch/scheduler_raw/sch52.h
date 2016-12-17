#ifndef RAW_SCH51_H_
#define RAW_SCH51_H_

#include "time_triggered_arch/scheduler_raw/main.h"

void SCH_Init_T2(void);
void SCH_Start(void);
void SCH_Dispatch_Tasks(void);

#endif  // RAW_SCH51_H_
