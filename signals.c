#include "signals.h"
#include "processlist.h"








void handle_ping_command(char *pid_str, char *signal_str) {
    pid_t pid = atoi(pid_str);
    int signal_number = atoi(signal_str) % 32; // Modulo 32 for signal handling

    ProcessInfo* process = find_process_by_pid(pid);
    if (process == NULL) {
        printf("No such process found\n");
        return;
    }

    // Send the signal to the process
    if (kill(pid, signal_number) == 0) {
        printf("Sent signal %d to process with pid %d\n", signal_number, pid);

        // Handle specific signals after they are sent
        if (signal_number == SIGKILL) {
            // SIGKILL forces process termination, cannot be caught
            remove_process(pid);
        } else if (signal_number == SIGTSTP || signal_number == SIGSTOP) {
            // SIGTSTP/SIGSTOP stop the process, update its state
            update_process_state(pid, "Stopped");
        } else if (signal_number == SIGCONT) {
            // SIGCONT continues a stopped process, update its state
            update_process_state(pid, "Running");
        }
    } else {
        // Error handling if the signal could not be sent
        perror("Failed to send signal");
    }
}


// Function to bring a process to the foreground
void handle_fg_command(char *pid_str) {
    pid_t pid = atoi(pid_str);

    ProcessInfo* process = find_process_by_pid(pid);
    if (process == NULL) {
        printf("No such process found\n");
        return;
    }

    if (strcmp(process->state, "Stopped") == 0) {
        kill(pid, SIGCONT);
    }

    update_process_state(pid, "Running");

    // Wait for the process to finish
    int status;
    waitpid(pid, &status, 0);
    remove_process(pid);
}

// Function to run a process in the background
void handle_bg_command(char *pid_str) {
    pid_t pid = atoi(pid_str);

    ProcessInfo* process = find_process_by_pid(pid);
    if (process == NULL) {
        printf("No such process found\n");
        return;
    }

    if (strcmp(process->state, "Stopped") == 0) {
        kill(pid, SIGCONT);
    }
    update_process_state(pid, "Running");
}

// Signal handler for Ctrl-C (SIGINT)
void sigint_handler(int sig) {
    for (int i = 0; i < process_count; i++) {
        if (strcmp(process_list[i].state, "Running") == 0) {
            kill(process_list[i].pid, SIGINT);
            update_process_state(process_list[i].pid, "Stopped");
            break;
        }
    }
}

// Signal handler for Ctrl-D (EOF)
void sigquit_handler(int sig) {
    for (int i = 0; i < process_count; i++) {
        kill(process_list[i].pid, SIGKILL);
    }
    exit(0);
}

// Signal handler for Ctrl-Z (SIGTSTP)
// Signal handler for Ctrl-Z (SIGTSTP)
void sigtstp_handler(int sig) {
    pid_t shell_pid = getpid();  // Get the shell's process ID

    if (process_count > 0) {
        int i = process_count - 1;  // Index of the last element (assumes this is the foreground process)
        if (process_list[i].pid != shell_pid && strcmp(process_list[i].state, "Running") == 0) {
            // Send SIGTSTP to the last process (foreground process)
            kill(process_list[i].pid, SIGTSTP);

            // Update process state in the list to "Stopped"
            update_process_state(process_list[i].pid, "Stopped");
            printf("\nProcess with pid %d stopped\n", process_list[i].pid);

            // Switch terminal control back to the shell
            tcsetpgrp(STDIN_FILENO, shell_pid);
        }
    }
}


