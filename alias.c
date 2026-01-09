#include "alias.h"

Alias aliases[MAX_ALIASES];
int alias_count;

void load_aliases(const char *filename) {
    FILE *file = fopen(filename, "r");
    if (file == NULL) {
        perror("Error opening .myshrc file");
        return;
    }

    char line[256];
    while (fgets(line, sizeof(line), file)) {
        char *alias = strtok(line, "=");
        char *command = strtok(NULL, "\n");
        if (alias && command) {
            trim_whitespace(alias);
            trim_whitespace(command);
            strncpy(aliases[alias_count].alias, alias, MAX_ALIAS_LENGTH);
            strncpy(aliases[alias_count].command, command, MAX_COMMAND_LENGTH);
            alias_count++;
        }
    }

    fclose(file);
}

char* expand_alias(const char *input) {
    //print input and alias[0].alias
    // printf("input: %s\n", input);
    // printf("alias[0].alias: %s\n", aliases[0].alias);
    //print input and alias[0].command
    // printf("input: %s\n", input);
    // printf("alias[0].command: %s\n", aliases[0].command);
    for (int i = 0; i < alias_count; i++) {
        if (strcmp(input, aliases[i].alias) == 0) {
            return strdup(aliases[i].command);
        }
    }
    //no alias found
    // printf("No alias found\n");
    return NULL;
}
