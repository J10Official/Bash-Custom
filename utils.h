#ifndef UTILS_H
#define UTILS_H

#include <string.h>
#include <ctype.h>
#include <dirent.h>

// Custom comparison function for sorting directory entries in lexicographic order
int compare(const struct dirent **a, const struct dirent **b);

// Trim whitespace from the input string (leading and trailing)
char *trim_whitespace(char *str);

// Trim leading and trailing quotations
char *trim_quotes(char *str);

#endif // UTILS_H