#ifndef ALIAS_H
#define ALIAS_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "utils.h"

#define MAX_COMMAND_LENGTH 100
#define MAX_ALIASES 100
#define MAX_ALIAS_LENGTH 100

typedef struct {
    char alias[MAX_ALIAS_LENGTH];
    char command[MAX_COMMAND_LENGTH];
} Alias;

extern Alias aliases[MAX_ALIASES];
extern int alias_count;

void load_aliases(const char *filename);
char* expand_alias(const char *input);

#endif // ALIAS_H