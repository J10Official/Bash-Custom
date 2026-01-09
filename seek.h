#ifndef SEEK_H
#define SEEK_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <pwd.h>
#include <dirent.h>
#include <sys/stat.h>
#include <errno.h>

// Color constants
#define COLOR_RESET "\033[0m"
#define COLOR_GREEN "\033[32m"
#define COLOR_WHITE "\033[37m"
#define COLOR_BLUE "\033[34m"
#define PATH_MAX 4096

// Function prototypes
void seek(int only_directories, int only_files, int execute_flag, char *search_term, char *search_dir, char *prev_dir, int *found_match);

#endif // SEEK_H