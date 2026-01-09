#ifndef PROCESSLIST_H
#define PROCESSLIST_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>

typedef struct {
    char command[256];
    pid_t pid;
    char state[16];
} ProcessInfo;

extern ProcessInfo process_list[100];
extern int process_count;

void add_process(const char *command, pid_t pid, const char *state);
void update_process_state(pid_t pid, const char *state);
int compare_processes(const void *a, const void *b);
void print_activities();
void remove_process(pid_t pid);
ProcessInfo* find_process_by_pid(pid_t pid);

#endif // PROCESSLIST_H