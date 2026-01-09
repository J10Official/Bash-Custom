#ifndef COMMAND_HANDLER_H
#define COMMAND_HANDLER_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/wait.h>
#include <time.h>

#define MAX_LINE 1024
#define MAX_COMMANDS 100

void handle_command(char *input, char *prev_dir, int is_background);
void execute_piped_commands(char **commands, int num_commands, char *prev_dir);
void execute_commands(char **foreground, int num_foreground, char **background, int num_background, char *prev_dir);
void process_input(char *input, char *prev_dir);

#endif // COMMAND_HANDLER_H