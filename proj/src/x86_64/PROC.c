#include <PROC.h>
#include <utils.h>
#include <KHeap.h>
#include <Logging.h>
#include <Console.h>

#include <syscalls.h>
#include <utils.h>
#include <fbuff.h>

process_t* processes_list[MAX_PROCESSES];
size_t next_free_pid = 0;
process_t* current_executing_process;
size_t current_process_idx = 0;

void init_proc() {
    for(size_t i=0; i < MAX_PROCESSES; i++) {
        processes_list[i] = NULL;
    }
    // add idle process;
    create_process("IDLE", idle, 0);
}

process_t* create_process(char* name, void(*_proc_entry)(void*), void* arg) {
    process_t* process;
    size_t i;
    for (i = 0; i < MAX_PROCESSES; i++) {
        if (processes_list[i] != NULL)
            continue;
        // process = &processes_list[i];
        break;
    }
    if(i == MAX_PROCESSES) {
        logf("Process List Full!\n");
        return NULL;
    }

    process = kmalloc(sizeof(process_t));
    if(process == NULL) {
        logf("NULL PROCESS\n");
        return NULL;
    }
    memset((void*)process, 0, sizeof(process_t));

    uintptr_t stack = (uintptr_t)kmalloc(PROC_DEFAULT_STACK_SIZE);
    if(stack == (uintptr_t)NULL) {
        logf("NULL STACK\n");
        return NULL;
    }
    memset((void*)stack, 0, PROC_DEFAULT_STACK_SIZE);


    strncpy(process->name, name, NAME_MAX_LEN);
    process->pid = next_free_pid++;
    process->status = READY;

    process->context = (struct cpu_status_t*)kmalloc(sizeof(struct cpu_status_t));
    if(memset((void*)(process->context), 0, sizeof(struct cpu_status_t)) != 0) {
        logf("Create Process: error- process context creation failed!\n");
        return NULL;
    }

    process->context->vector_number = 0x101; // 257
    process->context->error_code = 0x0;
    process->context->iret_ss = 0x10; // KERNEL SS
    process->context->iret_rsp = (uint64_t)(stack + PROC_DEFAULT_STACK_SIZE);
    process->context->iret_flags = 0x202;
    process->context->iret_cs = 0x8; // KERNEL CS
    process->context->iret_rip = (uint64_t)process_execution_wrapper;
    process->context->rdi = (uint64_t)_proc_entry;
    process->context->rsi = (uint64_t)arg;
    process->context->rbp = 0;

    processes_list[i] = process;

    return process;
}

void proc_free(size_t idx) {
    process_t* process = processes_list[idx];
    // free stack
    kfree((void*)(process->context->iret_rsp));
    // free context
    kfree((void*)(process->context));
    // free process structure
    kfree((void*)process);
}

void proc_suicide_trap() {
    current_executing_process = processes_list[current_process_idx];
    current_executing_process->status = DEAD;
    logf("(proc_suicide_trap) Suicide function called on process: %d name: %s - Status: %s", current_executing_process->pid, current_executing_process->name, get_process_status(current_executing_process));
    while(1);
}

void process_execution_wrapper( void (*_proc_entry)(void *), void* arg) {
    _proc_entry(arg);
    proc_suicide_trap();
    return;
}

void idle() {
    // char temp_buff[255];
	// size_t len = console_readline(temp_buff, 255, true);
	// console_write('\n');
	// console_writeline(temp_buff);

    // const size_t buff_limit = (NAME_MAX_LEN * PROC_MAX_ARGS_COUNT + 1);
    char buffer[255];
    size_t len, buff_itr;

    char cmdname[NAME_MAX_LEN];
    size_t cmd_len, cmd_itr;

    char argv[PROC_MAX_ARGS_COUNT][NAME_MAX_LEN];
    size_t argc, argv_itr;
    // size_t argv_lens[PROC_MAX_ARGS_COUNT];
    bool escaped, enclosed;

    // clear_console();
    print_set_color(RED, BLACK);
    console_writeline(
        "TTTTT  H   H  EEEEE    OOO    SSS  \n"  
        "  T    H   H  E       O   O  S     \n"
        "  T    HHHHH  EEEE    O   O   SSS  \n"
        "  T    H   H  E       O   O      S \n"
        "  T    H   H  EEEEE    OOO    SSS  \n"
    );
    
    while(1) {
        len = buff_itr = cmd_len = cmd_itr = 0;
        argc = argv_itr = 0;

        print_set_color(WHITE, GREEN);
        console_writeline("\nloneuser:$> ");
        print_set_color(WHITE, BLACK);

        len = console_readline(buffer, 255, true);

        // left trim
        while(buffer[buff_itr] == ' ' && buff_itr < len) buff_itr++;

        // parse command name
        while(buffer[buff_itr] != ' ' && buffer[buff_itr] != '\n' && buff_itr < len)
            cmdname[cmd_itr++] = buffer[buff_itr++];
        cmdname[cmd_itr] = 0;
        cmd_len = cmd_itr;

        // parse command arguments, if any
        for(; argc < PROC_MAX_ARGS_COUNT; argc++) {
            // argument-parse exit condition
            if(buff_itr >= 255 || buffer[buff_itr] == '\n')
                break;

            // skip spaces
            while(buffer[buff_itr] == ' ' && buff_itr < 255) buff_itr++;

            argv_itr = 0; escaped = enclosed = false;
            // parse argument[argc]
            while((buffer[buff_itr] != ' ' || enclosed == true) && buffer[buff_itr] != '\n' && buff_itr < 255) {
                logf("\nscanning: \"%c\"\n", buffer[buff_itr]);
                if(buffer[buff_itr] == '\\') { // next character that follows is escaped, ignore the escape symbol
                    escaped = true;
                    buff_itr++;
                    // continue;
                } 
                else if(buffer[buff_itr] == '\"' || buffer[buff_itr] == '\'') {
                    if(escaped == true) {
                        argv[argc][argv_itr++] = buffer[buff_itr++]; // it was marked as escaped, treat as normal
                        logf("%c", buffer[buff_itr-1]);
                        escaped = false;
                        // continue;
                    } else if(enclosed == true) {
                        enclosed = false;
                        buff_itr++;
                        // continue;
                    } else {
                        enclosed = true;
                        buff_itr++;
                        // continue;
                    }
                } 
                else {
                    argv[argc][argv_itr++] = buffer[buff_itr++]; 
                    logf("%c", buffer[buff_itr-1])
                }
            }
            argv[argc][argv_itr] = 0;;
            // argv_lens[argc] = argv_itr;
        }


        if(strcmp("echo", cmdname) == 0) {
            console_writeline(argv[0]);
            console_write('\n');
        } 
        else if(strcmp("clear", cmdname) == 0) {
            clear_console();
            continue;
        } 
        else if(strcmp("exit", cmdname) == 0) {
            return;
        }
        else {
            // create command as a separate process
            syscall_t command = get_syscall(cmdname);
            if(command == NULL) {
                console_writeline(cmdname);
                console_writeline(": Command Not Found\n");
            } else {
                create_process(cmdname, command, (void*)argv);
            }
        }
    }
    
    while(1);
}

char *get_process_status(process_t *process) {
    switch(process->status) {
        // case NEW:
        //     return "NEW";
        // case INIT:
        //     return "INIT";
        case READY:
            return "READY";
        case RUNNING:
            return "RUNNING";
        // case SLEEP:
        //     return "SLEEP";
        // case WAIT:
        //     return "WAIT";
        case DEAD:
            return "DEAD";
        default:
            return "ERROR";
    }
}