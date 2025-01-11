#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <signal.h>

#define ROUND_ROBIN_QUEUES_AMOUNT 4
#define CORE_COUNT 2
#define QUANTUM 5


typedef enum {
    READY,
    RUNNING,
} State;

typedef struct process {
    int id;
    int priority;
    int start_moment;

    int pid;
    State state;
    int time;
    char *command;
    struct process *next;
} Process;

/*typedef struct runtime_process {*/
/*    Process *process_in_process_table;*/
/*    Process *process_in_round_robin_queue;*/
/*} RuntimeProcess;*/

typedef struct core {
    int quantum;
    Process *process;
} Core;

void processes_orchestrator(int time_moment, Process **processes_list, Process **processes_table, Process ***round_robin_queues);
void add_process_to_round_robin_queues(Process *process, Process **round_robin_queues);
void add_process_to_round_robin_queue(Process *process, Process *round_robin_queue);

void print_processes_table(Process *processes_table);
void print_round_roubin_queues(int amount_of_queues, Process **round_robin_queues);
void print_round_roubin_queue(Process *round_robin_queue);
void print_cpu(int core_count, Core *cpu);

void clock_tick(int quantum, Core *cpu, int core_count, Process **processes_table, int round_robin_queues_amount, Process ***round_robin_queues);
Process* schedule_process(Process *processes_table, int round_robin_queues_amount, Process **round_robin_queues);

Process* create_process(int id, char *command, int start_moment, int priority);
Process* copy_process(Process *process);
void cleanup(Process *processes_table, int round_robin_queues_amount, Process **round_robin_queues, int core_count, Core *cpu);
void free_process(Process *process);

typedef enum {
    TRACE = 0,
    DEBUG,
    INFO,
    WARN,
    ERROR,
    FATAL,
    NONE,
} LogLevel;

LogLevel getLogLevel();
void logTrace(char *msg, ...);
void logDebug(char *msg, ...);
void logInfo(char *msg, ...);
void logWarn(char *msg, ...);
void logError(char *msg, ...);
void logFatal(char *msg, ...);

int main(int argc, char **argv) {
    logInfo("--- starting scheduler...\n");

    logInfo("--- initializing control structures...\n");
    Process *processes_list = NULL;
    Process *processes_table = NULL;
    Process **round_robin_queues = malloc(ROUND_ROBIN_QUEUES_AMOUNT * sizeof(Process*));

    Core *cpu = malloc(CORE_COUNT * sizeof(Core));

    Process *process1 = create_process(1, "teste20", 0, 2);
    Process *process2 = create_process(2, "teste10", 0, 0);
    Process *process3 = create_process(3, "teste30", 20, 0);
    Process *process4 = create_process(4, "teste10", 15, 1);

    process2->next = process1;
    process3->next = process2;
    process4->next = process3;
    processes_list = process4;


    for (int i = 0; i < 25; i++) {
        processes_orchestrator(i, &processes_list, &processes_table, &round_robin_queues);

        print_processes_table(processes_table);
        fprintf(stdout, "\n");
        print_round_roubin_queues(ROUND_ROBIN_QUEUES_AMOUNT, round_robin_queues);
        fprintf(stdout, "\n");

        // maybe sleep here

        clock_tick(QUANTUM, cpu, CORE_COUNT, &processes_table, ROUND_ROBIN_QUEUES_AMOUNT, &round_robin_queues);

        print_cpu(CORE_COUNT, cpu);
        fprintf(stdout, "\n");
    }


    logInfo("--- deinitializing control structures...\n");
    cleanup(processes_table, ROUND_ROBIN_QUEUES_AMOUNT, round_robin_queues, CORE_COUNT, cpu);

    logInfo("--- finishing scheduler...\n");
}

void processes_orchestrator(int time_moment, Process **processes_list, Process **processes_table, Process ***round_robin_queues) {
    Process *lhs = *processes_list;
    Process *process = *processes_list;
    Process *rhs = NULL;

    logInfo("--- spawning processes for the time moment %d...\n", time_moment);
    while (process != NULL) {
        if (process != NULL) {
            logDebug("process = %d\n", process->id);
        } else {
            logDebug("process = NULL\n");
        }

        rhs = process->next;
        if (rhs != NULL) {
            logDebug("rhs = %d\n", rhs->id);
        } else {
            logDebug("rhs = NULL\n");
        }

        if (process->start_moment == time_moment) {
            Process *round_robin_process = copy_process(process);

            logInfo("    adding process %d in the processes table...\n", process->id);
            process->next = *processes_table;
            if (lhs != process) {
                lhs->next = rhs;
            } else {
                lhs = rhs;
            }
            *processes_table = process;
            if (lhs != NULL) {
                logDebug("lhs = %d\n", lhs->id);
            } else {
                logDebug("lhs = NULL\n");
            }

            if (process == *processes_list) {
                *processes_list = lhs;
            }

            add_process_to_round_robin_queues(round_robin_process, *round_robin_queues);
        } else {
            lhs = process;
        }

        process = rhs;
    }
}

void add_process_to_round_robin_queues(Process *process, Process **round_robin_queues) {
    process->next = NULL;
    process->state = READY;

    if (round_robin_queues[process->priority] == NULL) {
        round_robin_queues[process->priority] = process;
        return;
    }

    add_process_to_round_robin_queue(process, round_robin_queues[process->priority]);
}

void add_process_to_round_robin_queue(Process *process, Process *round_robin_queue) {
    if (round_robin_queue->next == NULL) {
        round_robin_queue->next = process;
        return;
    }

    add_process_to_round_robin_queue(process, round_robin_queue->next);
}

void print_processes_table(Process *processes_table) {
    if (processes_table == NULL) {
        fprintf(stdout, "--- PROCESSES TABLE ---\n");
        fprintf(stdout, "ID\tPID\tS\tTIME\tCOMMAND\tSTART\tPRIORITY\n");
        return;
    }

    print_processes_table(processes_table->next);

    Process *p = processes_table;
    char *state = p->state == READY ? "READY" : "RUNNING";
    fprintf(stdout, "%d\t%d\t%s\t%d\t%s\t%d\t%d\n", p->id, p->pid, state, p->time, p->command, p->start_moment, p->priority);
}

void print_round_roubin_queues(int amount_of_queues, Process **round_robin_queues) {
    fprintf(stdout, "--- ROUND ROBIN QUEUES ---\n");
    for (int i = 0; i < amount_of_queues; i++) {
        fprintf(stdout, "RR %d: ", i);
        print_round_roubin_queue(round_robin_queues[i]);
    }
}

void print_round_roubin_queue(Process *round_robin_queue) {
    if (round_robin_queue == NULL) {
        fprintf(stdout, "NULL\n");
        return;
    }

    char *state = round_robin_queue->state == READY ? "READY" : "RUNNING";
    fprintf(stdout, "%d %s -> ", round_robin_queue->id, state);

    print_round_roubin_queue(round_robin_queue->next);
}

void print_cpu(int core_count, Core *cpu) {
    fprintf(stdout, "--- CPU STATE ---\n");
    for (int i = 0; i < core_count; i++) {
        fprintf(stdout, "Core %d: ", i);
        if (cpu[i].process != NULL) {
            fprintf(stdout, "running %d process, quantum remaining is %d seconds...\n", cpu[i].process->id, cpu[i].quantum);
        } else {
            fprintf(stdout, "doing nothing...\n");
        }
    }
}

void clock_tick(int quantum, Core *cpu, int core_count, Process **processes_table, int round_robin_queues_amount, Process ***round_robin_queues) {
    logInfo("--- clock tick...\n");
    for (int i = 0; i < core_count; i++) {
        if (cpu[i].process != NULL) {
            cpu[i].quantum -= 1;
            if (cpu[i].quantum <= 0) {
                // kill(process->pid, SIGSTOP);
                cpu[i].process->state = READY;
                Process *round_robin_process = copy_process(cpu[i].process);
                add_process_to_round_robin_queues(round_robin_process, *round_robin_queues);

                // call scheduler
                cpu[i].process = schedule_process(*processes_table, round_robin_queues_amount, *round_robin_queues);
                if (cpu[i].process != NULL) {
                    cpu[i].process->state = RUNNING;
                    cpu[i].quantum = quantum;
                }

                // kill(cpu[i].process->pid, SIGCONT);
            }
        } else {
            cpu[i].process = schedule_process(*processes_table, round_robin_queues_amount, *round_robin_queues);
            if (cpu[i].process != NULL) {
                cpu[i].process->state = RUNNING;
                cpu[i].quantum = quantum;
            }
        }
    }
}

Process* schedule_process(Process *processes_table, int round_robin_queues_amount, Process **round_robin_queues) {
    logInfo("--- selecting a process to run...\n");
    Process *process = NULL;
    Process *process_in_round_robin_queue = NULL;

    int i = 0; 
    do {
        if (round_robin_queues[i] != NULL) {
            process_in_round_robin_queue = round_robin_queues[i];
            round_robin_queues[i] = round_robin_queues[i]->next;

            logDebug("    got %d process from %d round robin queue...\n", process_in_round_robin_queue->id, process_in_round_robin_queue->priority);

            process = processes_table;
            while (process->id != process_in_round_robin_queue->id) {
                process = process->next;
            }
        }

        i++;
    } while (process == NULL && i < round_robin_queues_amount);

    return process;
}

Process* create_process(int id, char *command, int start_moment, int priority) {
    logInfo("--- creating the process of id %d for the %s binary, this process will be added in the %d round robin queue and will start at %d\n", id, command, priority, start_moment);

    // TODO: check malloc result before assignments
    Process *process = malloc(sizeof(Process));
    process->id = id;
    process->priority = priority;
    process->start_moment = start_moment;

    process->pid = -1;
    process->state = READY;
    process->time = 0;
    process->command = command;
    process->next = NULL;

    logDebug("\tprocess->id = %d\n", process->id);
    logDebug("\tprocess->priority = %d\n", process->priority);
    logDebug("\tprocess->start_moment = %d\n", process->start_moment);

    logDebug("\tprocess->pid = %d\n", process->pid);
    logDebug("\tprocess->state = %d\n", process->state);
    logDebug("\tprocess->binary = %s\n", process->command);
    logDebug("\tprocess->next = %d\n\n", process->next);

    return process;
}

Process* copy_process(Process *process) {
    logInfo("--- creating a copy of process of id %d\n", process->id);

    Process *copy = malloc(sizeof(Process));
    copy->id = process->id;
    copy->priority = process->priority;
    copy->start_moment = process->start_moment;

    copy->pid = process->pid;
    copy->state = process->state;
    copy->time = process->time;
    copy->command = process->command;
    copy->next = NULL;

    logDebug("\tprocess->id = %d\n", copy->id);
    logDebug("\tprocess->priority = %d\n", copy->priority);
    logDebug("\tprocess->start_moment = %d\n", copy->start_moment);

    logDebug("\tprocess->pid = %d\n", copy->pid);
    logDebug("\tprocess->state = %d\n", copy->state);
    logDebug("\tprocess->binary = %s\n", copy->command);
    logDebug("\tprocess->next = %d\n\n", copy->next);

    return copy;
}

void cleanup(Process *processes_table, int round_robin_queues_amount, Process **round_robin_queues, int core_count, Core *cpu) {
    Process *process = processes_table;
    Process *next = NULL;

    logInfo("--- freeing processes table...\n");
    logDebug("--- processes_table == NULL => %d...\n", processes_table == NULL);
    while (process != NULL) {
        next = process->next;
        free_process(process);
        process = next;
    }

    for (int i = 0; i < round_robin_queues_amount; i++) {
        process = round_robin_queues[i];
        next = NULL;

        logInfo("--- freeing %d round robin queue...\n", i);
        while (process != NULL) {
            next = process->next;
            free_process(process);
            process = next;
        }
    }
    logInfo("--- freeing round robin queues...\n");
    free(round_robin_queues);

    logInfo("--- freeing cpu cores...\n");
    /*for (int i = 0; i < core_count; i++) {*/
    /*    free(cpu[i].process);*/
    /*}*/
    free(cpu);
}

void free_process(Process *process) {
    logInfo("    freeing process %d...\n", process->id);

    /*free(process->binary);*/
    free(process);
}

int start_process(Process *process) {
    if (process->pid == -1) {
        logInfo("starting the process %d for the first time\n", process->id);
    }

    if (process->state == RUNNING) {
        logDebug("process of %d pid is already running, nothing to do...\n;", process->pid);
        return 0;
    }

    return 0;
}

LogLevel getLogLevel() {
    char *logLevel = getenv("LOG_LEVEL");
    if (logLevel != NULL) {
        if (strcmp(logLevel, "TRACE") == 0 || strcmp(logLevel, "0") == 0) {
            return TRACE;
        }
        if (strcmp(logLevel, "DEBUG") == 0 || strcmp(logLevel, "1") == 0) {
            return DEBUG;
        }
        if (strcmp(logLevel, "INFO") == 0 || strcmp(logLevel, "2") == 0) {
            return INFO;
        }
        if (strcmp(logLevel, "WARN") == 0 || strcmp(logLevel, "3") == 0) {
            return WARN;
        }
        if (strcmp(logLevel, "ERROR") == 0 || strcmp(logLevel, "4") == 0) {
            return ERROR;
        }
        if (strcmp(logLevel, "FATAL") == 0 || strcmp(logLevel, "5") == 0) {
            return FATAL;
        }
        if (strcmp(logLevel, "NONE") == 0 || strcmp(logLevel, "6") == 0) {
            return NONE;
        }
    }
    return NONE;
}

void logTrace(char *msg, ...) {
    va_list argp;
    va_start(argp, msg);

    if (getLogLevel() <= TRACE) {
        fprintf(stderr, "TRACE: ");
        vfprintf(stderr, msg, argp);
    }

    va_end(argp);
}

void logDebug(char *msg, ...) {
    va_list argp;
    va_start(argp, msg);

    if (getLogLevel() <= DEBUG) {
        fprintf(stderr, "DEBUG: ");
        vfprintf(stderr, msg, argp);
    }

    va_end(argp);
}

void logInfo(char *msg, ...) {
    va_list argp;
    va_start(argp, msg);

    if (getLogLevel() <= INFO) {
        fprintf(stderr, "INFO: ");
        vfprintf(stderr, msg, argp);
    }

    va_end(argp);
}

void logWarn(char *msg, ...) {
    va_list argp;
    va_start(argp, msg);

    if (getLogLevel() <= WARN) {
        fprintf(stderr, "WARN: ");
        vfprintf(stderr, msg, argp);
    }

    va_end(argp);
}

void logError(char *msg, ...) {
    va_list argp;
    va_start(argp, msg);

    if (getLogLevel() <= ERROR) {
        fprintf(stderr, "ERROR: ");
        vfprintf(stderr, msg, argp);
    }

    va_end(argp);
}

void logFatal(char *msg, ...) {
    va_list argp;
    va_start(argp, msg);

    if (getLogLevel() <= FATAL) {
        fprintf(stderr, "FATAL: ");
        vfprintf(stderr, msg, argp);
    }

    va_end(argp);
}
