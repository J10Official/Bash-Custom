#ifndef DIRECTORY_H
#define DIRECTORY_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <pwd.h>
#include <dirent.h>
#include <sys/stat.h>
#include <limits.h>
#include <grp.h>
#include <time.h>

// Color constants
#define COLOR_RESET "\033[0m"
#define COLOR_GREEN "\033[32m"
#define COLOR_WHITE "\033[37m"
#define COLOR_BLUE "\033[34m"
#define PATH_MAX 4096

extern char home_path[PATH_MAX];
extern char translated_path[PATH_MAX];
// Function prototypes
void update_home_path(const char *path);
void translate_to_absolute_path(char *path);
void translate_to_relative_path(char *path);
void hop(char *arg, char *prev_dir);
void reveal(int show_all, int long_format, char *reveal_path, char *prev_dir);

#endif // DIRECTORY_H