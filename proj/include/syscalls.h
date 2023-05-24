#ifndef _SYSCALLS_H
#define _SYSCALLS_H

#define NUM_SYSCALLS 16

typedef void(*syscall_t)(void*);

void syscalls_init();
void add_syscall(char* name, syscall_t syscall);
syscall_t get_syscall(char* name);

// syscalls
// void echo(void* args);

#endif