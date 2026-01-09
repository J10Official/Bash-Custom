#ifndef SIGNALS_H
#define SIGNALS_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <termios.h>

#include "processlist.h"

// Function prototypes
void handle_ping_command(char *pid_str, char *signal_str);
void handle_fg_command(char *pid_str);
void handle_bg_command(char *pid_str);
void sigint_handler(int sig);
void sigquit_handler(int sig);
void sigtstp_handler(int sig);

#endif // SIGNALS_H