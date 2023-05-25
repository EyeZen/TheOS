#ifndef _PROC_H
#define _PROC_H

#include <IDT.h>
#include <stddef.h>
#include <stdbool.h>
#include <stdint.h>

#define NAME_MAX_LEN 255
#define MAX_PROCESSES 64
#define MAX_RESOURCE_IDS 255
#define PROC_DEFAULT_STACK_SIZE 0x10000 // 64KB
#define PROC_MAX_ARGS_COUNT 1

#define NUM_PRIORITY_LEVELS 2
#define MAX_PRIORITY 0
#define MIN_PRIORITY (NUM_PRIORITY_LEVELS - 1)

enum status_t {
    NEW,
    INIT,
    RUNNING,
    READY,
    SLEEP,
    WAIT,
    DEAD
};
typedef enum status_t status_t; 

struct process_t
{
    size_t pid;
    status_t status;
    uint8_t priority_level;
    // void* root_page_table;
    struct cpu_status_t* context;
    char name[NAME_MAX_LEN];
    size_t resources[MAX_RESOURCE_IDS];
};
typedef struct process_t process_t;


extern process_t* processes_list[NUM_PRIORITY_LEVELS][MAX_PROCESSES];
extern size_t current_process_idx;
extern uint8_t current_process_priority;

void init_proc();
process_t* create_process(char* name, void (*function)(void*), void* arg, uint8_t priority_level);
void proc_free(size_t idx, uint8_t priority_level);
void proc_suicide_trap();
void process_execution_wrapper( void (*_proc_entry)(void *), void* arg);
void idle();
char *get_process_status(process_t *process);
void process_list();

void print_ver(bool flag);

#endif