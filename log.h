#ifndef LOG_H
#define LOG_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Constants
#define MAX_LOG_COMMANDS 15
#define LOG_FILE_PATH_SIZE 512



// Function prototypes
void update_log_path(const char *base_path);
void log_command(char *input);
void display_log();
void purge_log();
void execute_log_command(int index, char *prev_dir);

#endif // LOG_H