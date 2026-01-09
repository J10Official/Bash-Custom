#include "seek.h"
#include "directory.h"

void seek(int only_directories, int only_files, int execute_flag, char *search_term, char *search_dir, char *prev_dir, int *found_match) {
    // Check for invalid flag combination
    if (only_directories && only_files) {
        printf("Invalid flags!\n");
        return;
    }

    // Handle special symbols for the directory
    if (strcmp(search_dir, "~") == 0) {
        search_dir = home_path;  // Expand ~ to home directory
    } else if (strcmp(search_dir, "-") == 0) {
        search_dir = prev_dir;  // Use previous directory
    }
     else if (strncmp(search_dir, "~/", 2) == 0) {
        // Translate to absolute path
        translate_to_absolute_path(search_dir);
        search_dir = translated_path;
    }
    // Open the directory
    DIR *dir = opendir(search_dir);
    if (dir == NULL) {
        perror("opendir");
        return;
    }

    struct dirent *entry;
    struct stat file_stat;
    char full_path[PATH_MAX];
    int match_count = 0;
    int directory_match = 0;
    int file_match = 0;
    char single_match[PATH_MAX] = {0};  // To hold the path of a single match for the -e flag

    // Search the directory
    while ((entry = readdir(dir)) != NULL) {
        snprintf(full_path, sizeof(full_path), "%s/%s", search_dir, entry->d_name);
        
        // Skip '.' and '..'
        if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0) {
            continue;
        }

        // Get file status
        if (stat(full_path, &file_stat) != 0) {
            // Print error message only if it's not a permission issue
            if (errno != ENOENT) {
                perror("stat");
            }
            continue;
        }

        // Check if the name matches the search term
        if (strstr(entry->d_name, search_term) != NULL) {
            // Check if it's a directory or file
            int is_directory = S_ISDIR(file_stat.st_mode);
            int is_file = S_ISREG(file_stat.st_mode);

            // Apply flags
            if ((only_directories && !is_directory) || (only_files && !is_file)) {
                continue;  // Skip if it doesn't match the -d or -f flags
            }

            // Print the matching entry
            if (is_directory) {
                printf(COLOR_BLUE "%s/%s" COLOR_RESET "\n", search_dir, entry->d_name);
                directory_match++;
            } else if (is_file) {
                printf(COLOR_GREEN "%s/%s" COLOR_RESET "\n", search_dir, entry->d_name);
                file_match++;
            }

            // Store the match if -e flag is active
            if (execute_flag) {
                strcpy(single_match, full_path);
            }

            match_count++;
            *found_match = 1;  // Set the flag to indicate a match was found
        }

        // If it's a directory, recursively call seek
        if (S_ISDIR(file_stat.st_mode)) {
            seek(only_directories, only_files, execute_flag, search_term, full_path, search_dir, found_match);
        }
    }

    closedir(dir);

    // Handle the -e flag
    if (execute_flag) {
        if (match_count == 1) {
            if (directory_match == 1 && file_match == 0) {
                // Change to the directory if it's a single match and a directory
                if (chdir(single_match) != 0) {
                    perror("Missing permissions for task!");
                } else {
                    printf("Changed directory to %s\n", single_match);
                }
            } else if (file_match == 1 && directory_match == 0) {
                // Print file content if it's a single match and a file
                FILE *file = fopen(single_match, "r");
                if (file == NULL) {
                    perror("Missing permissions for task!");
                } else {
                    char line[256];
                    while (fgets(line, sizeof(line), file) != NULL) {
                        printf("%s", line);
                    }
                    fclose(file);
                }
            }
        }
    }
}

