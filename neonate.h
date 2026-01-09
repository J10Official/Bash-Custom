#ifndef NEONATE_H
#define NEONATE_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <dirent.h>
#include <sys/stat.h>
#include <limits.h>
#include <termios.h>
#include <fcntl.h>

// Constants
#define PATH_MAX 4096

// Function prototypes
int get_most_recent_pid();
int kbhit(void);
void neonate(int time_arg);

#endif // NEONATE_H