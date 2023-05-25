#ifndef _SCHED_H
#define _SCHED_H

#include <PROC.h>

void init_scheduler();
struct cpu_status_t* schedule(struct cpu_status_t* context);

extern uint8_t current_processes_pidx[NUM_PRIORITY_LEVELS];

#endif