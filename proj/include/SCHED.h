#ifndef _SCHED_H
#define _SCHED_H

void init_scheduler();
struct cpu_status_t* schedule(struct cpu_status_t* context);

#endif