#include <SCHED.h>
#include <PROC.h>
#include <print.h>
#include <Logging.h>

void kernel_idle() {
    while(1) asm("hlt");
}

void init_scheduler() {
    init_proc();
}

struct cpu_status_t* schedule(struct cpu_status_t* context) {
    // save context of pre-emted process    
    processes_list[current_process_idx]->context = context;
    processes_list[current_process_idx]->status = READY;

    do {
        current_process_idx = (current_process_idx + 1) % MAX_PROCESSES;
        if (processes_list[current_process_idx] != NULL && processes_list[current_process_idx]->status == DEAD)
        {
            proc_free(current_process_idx);
            processes_list[current_process_idx] = NULL;
        }
    }
    while (processes_list[current_process_idx] == NULL || processes_list[current_process_idx]->status != READY);

    // swtich context
    processes_list[current_process_idx]->status = RUNNING;
    return processes_list[current_process_idx]->context;
}

// void scheduler_yield() {

// }