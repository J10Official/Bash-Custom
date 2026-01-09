#include "utils.h"









// Custom comparison function for sorting directory entries in lexicographic order
int compare(const struct dirent **a, const struct dirent **b) {
    return strcasecmp((*a)->d_name, (*b)->d_name);
}

// trim whitespace from the input string (leading and trailing)

char *trim_whitespace(char *str) {
    char *end;

    // Trim leading space
    while (isspace((unsigned char)*str)) str++;

    if (*str == 0) {  // All spaces?
        return str;
    }

    // Trim trailing space
    end = str + strlen(str) - 1;
    while (end > str && isspace((unsigned char)*end)) end--;

    // Write new null terminator character
    end[1] = '\0';

    return str;
}

//trim leading and trailng quotations
char *trim_quotes(char *str) {
    if (str[0] == '\"' || str[0] == '\'') {
        str++;
    }
    if (str[strlen(str) - 1] == '\"' || str[strlen(str) - 1] == '\'') {
        str[strlen(str) - 1] = '\0';
    }
    return str;
}


