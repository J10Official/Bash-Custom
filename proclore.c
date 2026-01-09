#include "proclore.h"






void proclore(char *pid_str) {
    char path[PATH_MAX];
    char buffer[1024];
    FILE *file;
    int pid;
    char status;
    int pgrp;
    unsigned long vsize;
    char exe_path[PATH_MAX];
    int tty_nr;

    // If no PID is provided, use the current process's PID
    if (pid_str == NULL) {
        pid = getpid();
    } else {
        pid = atoi(pid_str);
    }

    // Construct the path to the /proc/[pid]/stat file
    snprintf(path, sizeof(path), "/proc/%d/stat", pid);

    // Open the /proc/[pid]/stat file
    file = fopen(path, "r");
    if (file == NULL) {
        perror("Error opening /proc/[pid]/stat file");
        return;
    }

    // Read the process information from the /proc/[pid]/stat file
    fscanf(file, "%d %*s %c %*d %d %*d %*d %*d %*d %*u %*u %*u %*u %*u %*u %*u %*d %*d %*d %*d %*d %*d %*d %lu %*d %*d %*d %d", &pid, &status, &pgrp, &vsize, &tty_nr);
    fclose(file);

    // Construct the path to the /proc/[pid]/exe file
    snprintf(path, sizeof(path), "/proc/%d/exe", pid);

    // Read the executable path
    ssize_t len = readlink(path, exe_path, sizeof(exe_path) - 1);
    if (len != -1) {
        exe_path[len] = '\0';
    } else {
        perror("Error reading /proc/[pid]/exe file");
        return;
    }

    // Check if the process is a background process
    char status_with_bg = status;
    if (tty_nr == 0) {
        status_with_bg = '+';
    }

    // Print the process information
    printf("pid : %d\n", pid);
    printf("process status : %c\n", status_with_bg);
    printf("Process Group : %d\n", pgrp);
    printf("Virtual memory : %lu\n", vsize);
    printf("executable path : %s\n", exe_path);
}

