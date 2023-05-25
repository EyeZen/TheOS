#include <syscalls.h>
#include <Console.h>
#include <utils.h>
#include <PROC.h>

// #include <Logging.h>

syscall_t syscalls_list[NUM_SYSCALLS];
char syscalls_name[NUM_SYSCALLS][255];
size_t next_syscall_vector=0;

void syscalls_init() {
    // add_syscall("echo", (syscall_t)echo);
    // add_syscall("ps", (syscall_t)process_list);
}

void add_syscall(char* name, syscall_t syscall) {
    size_t syscall_vector = next_syscall_vector++;
    strcpy(name, syscalls_name[syscall_vector]);
    syscalls_list[syscall_vector] = syscall;
}

syscall_t get_syscall(char* name) {
    // size_t len = strlen(name);
    for(size_t i=0; i < next_syscall_vector; i++) {
        int result = strcmp(name, syscalls_name[i]);
        if(result == 0) {
            return syscalls_list[i];
        }
    }
    return NULL;
}

/////////////////////////
//      syscalls
/////////////////////////

// void echo(void* args) {
//     char* str = (char*)args;
//     console_writeline(str);
//     console_write('\n');
// }

void process_list() {
    console_writeline("PID NAME  PRIORITY STATUS\n");
    for(uint8_t p = 0; p < NUM_PRIORITY_LEVELS; p++) {
        for(size_t i=0; i < MAX_PROCESSES; i++) {
            process_t* process = processes_list[p][i];
            if(process != NULL) {
                console_write("  %d %s      %d    %s\n", process->pid, process->name, process->priority_level, get_process_status(process));
            }
        }
    }
}