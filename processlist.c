# include "processlist.h"






ProcessInfo process_list[100];
int process_count = 0;

void add_process(const char *command, pid_t pid, const char *state) {
    if (process_count < 100) {
        strncpy(process_list[process_count].command, command, sizeof(process_list[process_count].command) - 1);
        process_list[process_count].pid = pid;
        strncpy(process_list[process_count].state, state, sizeof(process_list[process_count].state) - 1);
        process_count++;
    }
}

void update_process_state(pid_t pid, const char *state) {
    for (int i = 0; i < process_count; i++) {
        if (process_list[i].pid == pid) {
            strncpy(process_list[i].state, state, sizeof(process_list[i].state) - 1);
            break;
        }
    }
}

int compare_processes(const void *a, const void *b) {
    return strcasecmp(((ProcessInfo *)a)->command, ((ProcessInfo *)b)->command);
}

void print_activities() {
    qsort(process_list, process_count, sizeof(ProcessInfo), compare_processes);
    for (int i = 0; i < process_count; i++) {
        printf("%d : %s - %s\n", process_list[i].pid, process_list[i].command, process_list[i].state);
    }
}

void remove_process(pid_t pid) {
    for (int i = 0; i < process_count; i++) {
        if (process_list[i].pid == pid) {
            for (int j = i; j < process_count - 1; j++) {
                process_list[j] = process_list[j + 1];
            }
            process_count--;
            break;
        }
    }
}

// Function to find a process by pid in the process list
ProcessInfo* find_process_by_pid(pid_t pid) {
    for (int i = 0; i < process_count; i++) {
        if (process_list[i].pid == pid) {
            return &process_list[i];
        }
    }
    return NULL;
}