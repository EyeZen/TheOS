#include <SCHED.h>
#include <PROC.h>
#include <print.h>
#include <Logging.h>

uint8_t current_processes_pidx[NUM_PRIORITY_LEVELS];

void kernel_idle() {
    while(1) asm("hlt");
}

void init_scheduler() {
    for(int i=0; i < NUM_PRIORITY_LEVELS; i++) {
        current_processes_pidx[i] = 0;
    }
    init_proc();
    // logf("Init-Proc: \n");
    // log_scheduling_queues();
}

struct cpu_status_t* schedule(struct cpu_status_t* context) {
    // logf("Scheduling:\n");
    // log_scheduling_queues();
    // save context of pre-emted process    
    processes_list[current_process_priority][current_process_idx]->context = context;
    processes_list[current_process_priority][current_process_idx]->status = READY;
    current_processes_pidx[current_process_priority] = current_process_idx+1;

    bool next_process_found = false;

    for(current_process_priority = 0; 
        current_process_priority < NUM_PRIORITY_LEVELS && next_process_found == false; 
        current_process_priority++) 
    {
        // logfa("Priority_%d:\n", current_process_priority);
        for(current_process_idx = current_processes_pidx[current_process_priority]; 
            next_process_found == false && current_process_idx < MAX_PROCESSES; 
            current_process_idx++) 
        {
            // current_process_idx = (current_process_idx + 1) % MAX_PROCESSES;
            if (processes_list[current_process_priority][current_process_idx] != NULL && processes_list[current_process_priority][current_process_idx]->status == DEAD)
            {
                proc_free(current_process_idx, current_process_priority);
                processes_list[current_process_priority][current_process_idx] = NULL;
            } 
            else if(processes_list[current_process_priority][current_process_idx] != NULL && processes_list[current_process_priority][current_process_idx]->status == READY)
            {
                next_process_found = true;
            }

            // logfa("process_%d: ", current_process_idx);
            // if(next_process_found) {logfa("Runnable\n");}
            // else {logfa("Not Runnable\n");}
        }
        if(current_process_idx == MAX_PROCESSES) {
            // logfa("SchedulingCounterReset\n");
            current_processes_pidx[current_process_priority] = 0;
        }
        // while (processes_list[current_process_priority][current_process_idx] == NULL || processes_list[current_process_priority][current_process_idx]->status != READY);
    }

    // swtich context
    processes_list[current_process_priority][current_process_idx]->status = RUNNING;
    return processes_list[current_process_priority][current_process_idx]->context;
}
