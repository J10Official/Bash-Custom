#include "directory.h"
#include "utils.h"





char home_path[PATH_MAX];

char translated_path[PATH_MAX];

void update_home_path(const char *path) {
    strncpy(home_path, path, sizeof(home_path) - 1);
    home_path[sizeof(home_path) - 1] = '\0';
    printf("Home path: %s\n", home_path);
    // //print home path
    // printf("Home path: %s\n", home_path);
}

// void translate_to_absolute_path(char *path) {
//     // If the input path is null or empty, do nothing
//     if (path == NULL || *path == '\0') {
//         return;
//     }

//     // Temporary buffer to hold the final path
//     char temp[PATH_MAX];

//     // Handle the case where path is exactly "~"
//     if (strcmp(path, "~") == 0) {
//         // Replace "~" with the home_path
//         strncpy(path, home_path, PATH_MAX - 1);
//         path[PATH_MAX - 1] = '\0';  // Ensure null termination
//     }
//     // Handle the case where path starts with "~/"
//     else if (strncmp(path, "~/", 2) == 0) {
//         // Format the path by replacing "~/" with the home_path
//         snprintf(temp, sizeof(temp), "%s/%s", home_path, path + 2);

//         // Copy the result back to path and ensure null termination
//         strncpy(path, temp, PATH_MAX - 1);
//         path[PATH_MAX - 1] = '\0';
//     }
//     // If the path doesn't start with "~", it's already an absolute or relative path, so no changes are made
// }

// The function to translate paths
void translate_to_absolute_path(char *path) {
    // If the input path is null, empty, or does not start with "~/", do nothing
    if (path == NULL || *path == '\0' || strncmp(path, "~/", 2) != 0) {
        return;
    }

    // Clear the translated_path buffer
    translated_path[0] = '\0';

    // Copy home_path to translated_path
    strncpy(translated_path, home_path, PATH_MAX - 1);
    translated_path[PATH_MAX - 1] = '\0';  // Ensure null termination

    // Concatenate "/" and the rest of the original path after "~/"
    strncat(translated_path, "/", PATH_MAX - strlen(translated_path) - 1);
    strncat(translated_path, path + 2, PATH_MAX - strlen(translated_path) - 1);
    // printf("Translated path: %s\n", translated_path);
}


void translate_to_relative_path(char *path) {
    if (strncmp(path, home_path, strlen(home_path)) == 0) {
        char temp[PATH_MAX];
        if (snprintf(temp, sizeof(temp), "~/%s", path + strlen(home_path) + 1) >= sizeof(temp)) {
            fprintf(stderr, "Path too long\n");
            return;
        }
        strncpy(path, temp, PATH_MAX - 1);
        path[PATH_MAX - 1] = '\0';
    }
}



void hop(char *arg, char *prev_dir) {
    char current_dir[PATH_MAX];
    if (getcwd(current_dir, sizeof(current_dir)) == NULL) {
        perror("getcwd");
        return;
    }

    if (arg == NULL || strcmp(arg, "~") == 0) {
        // Change to home directory
        if (chdir(home_path) != 0) {
            perror("chdir");
        } else {
            strcpy(prev_dir, current_dir);
            if (getcwd(current_dir, sizeof(current_dir)) != NULL) {
                printf("%s\n", current_dir);
            }
        }
    } else if (strcmp(arg, "-") == 0) {
        // Change to previous directory
        if (chdir(prev_dir) != 0) {
            perror("chdir");
        } else {
            // printf("%s\n", prev_dir);
            strcpy(prev_dir, current_dir);
            if (getcwd(current_dir, sizeof(current_dir)) != NULL) {
                printf("%s\n", current_dir);
            }
        }
    } else if (strcmp(arg, ".") == 0) {
        // Stay in the current directory
    } else if (strcmp(arg, "..") == 0) {
        // Change to parent directory
        if (chdir("..") != 0) {
            perror("chdir");
        } else {
            strcpy(prev_dir, current_dir);
            if (getcwd(current_dir, sizeof(current_dir)) != NULL) {
                printf("%s\n", current_dir);
            }
        }
    } else if (arg[0] == '/') {
        // Change to specified absolute path
        if (chdir(arg) != 0) {
            perror("chdir");
        } else {
            strcpy(prev_dir, current_dir);
            if (getcwd(current_dir, sizeof(current_dir)) != NULL) {
                printf("%s\n", current_dir);
            }
        }
    } else if (strncmp(arg, "~/", 2) == 0) {
        // Translate to absolute path
        translate_to_absolute_path(arg);
        // Change to specified absolute path
        if (chdir(translated_path) != 0) {
            perror("chdir");
        } else {
            strcpy(prev_dir, current_dir);
            if (getcwd(current_dir, sizeof(current_dir)) != NULL) {
                printf("%s\n", current_dir);
            }
        }
    } else {
        // Change to specified relative directory
        if (chdir(arg) != 0) {
            perror("chdir");
        } else {
            strcpy(prev_dir, current_dir);
            if (getcwd(current_dir, sizeof(current_dir)) != NULL) {
                printf("%s\n", current_dir);
            }
        }
    }
}

void reveal(int show_all, int long_format, char *reveal_path, char *prev_dir) {
    // Handle special symbols
    if (strcmp(reveal_path, "~") == 0) {
        struct passwd *pw = getpwuid(getuid());
        if (pw == NULL) {
            perror("getpwuid");
            return;
        }
        reveal_path = home_path;  // Expand ~ to the home directory
    } else if (strcmp(reveal_path, "-") == 0) {
        reveal_path = prev_dir;  // Use previous directory for '-'
    }
    else if (strncmp(reveal_path, "~/", 2) == 0) {
        // Translate to absolute path
        translate_to_absolute_path(reveal_path);
        reveal_path = translated_path;
    }

    // Open the specified directory
    DIR *dir = opendir(reveal_path);
    if (dir == NULL) {
        perror("opendir");
        return;
    }

    // Read directory entries into an array
    struct dirent **namelist;
    int n = scandir(reveal_path, &namelist, NULL, compare);
    if (n < 0) {
        perror("scandir");
        closedir(dir);
        return;
    }

    // Print directory entries
    for (int i = 0; i < n; i++) {
        struct dirent *entry = namelist[i];
        if (!show_all && entry->d_name[0] == '.') {
            free(entry);
            continue;
        }
        char full_path[PATH_MAX];
        snprintf(full_path, sizeof(full_path), "%s/%s", reveal_path, entry->d_name);
        struct stat file_stat;
        if (stat(full_path, &file_stat) != 0) {
            perror("stat");
            free(entry);
            continue;
        }
        if (long_format) {
            struct passwd *pw = getpwuid(file_stat.st_uid);
            struct group *gr = getgrgid(file_stat.st_gid);
            printf("%c%s%s%s %ld %s %s %ld %s ",
                   S_ISDIR(file_stat.st_mode) ? 'd' : '-',
                   (file_stat.st_mode & S_IRUSR) ? "r" : "-",
                   (file_stat.st_mode & S_IWUSR) ? "w" : "-",
                   (file_stat.st_mode & S_IXUSR) ? "x" : "-",
                   file_stat.st_nlink,
                   pw ? pw->pw_name : "unknown",
                   gr ? gr->gr_name : "unknown",
                   file_stat.st_size,
                   ctime(&file_stat.st_mtime));
        }
        if (S_ISDIR(file_stat.st_mode)) {
            printf(COLOR_BLUE "%s" COLOR_RESET "\n", entry->d_name);
        } else if (file_stat.st_mode & S_IXUSR) {
            printf(COLOR_GREEN "%s" COLOR_RESET "\n", entry->d_name);
        } else {
            printf(COLOR_WHITE "%s" COLOR_RESET "\n", entry->d_name);
        }
        free(entry);
    }
    free(namelist);
    closedir(dir);
}
