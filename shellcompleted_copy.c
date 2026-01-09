#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pwd.h>
#include <limits.h>
#include <string.h>
#include <dirent.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <time.h>
#include <grp.h>
#include <errno.h>
#include <ctype.h>
#include <sys/wait.h>
#include <signal.h>
#include <fcntl.h>
#include <termios.h>
#include <arpa/inet.h>
#include <netdb.h>



#define COLOR_RESET "\033[0m"
#define COLOR_GREEN "\033[32m"
#define COLOR_WHITE "\033[37m"
#define COLOR_BLUE "\033[34m"
#define PATH_MAX 4096


#define MAX_ALIASES 100
#define MAX_ALIAS_LENGTH 100
#define MAX_COMMAND_LENGTH 100



void handle_command(char *input, char *prev_dir, int is_background);
void execute_piped_commands(char **commands, int num_commands, char *prev_dir);
void execute_commands(char **foreground, int num_foreground, char **background, int num_background, char *prev_dir);
void process_input(char *input, char *prev_dir);
char *trim_whitespace(char *str);
void log_command(char *command);
void update_process_state(pid_t pid, const char *state);
void add_process(const char *command, pid_t pid, const char *state);
void hop(char *path, char *prev_dir);
void reveal(int show_all, int long_format, char *path, char *prev_dir);
void display_log();
void purge_log();
void execute_log_command(int index, char *prev_dir);
void seek(int only_directories, int only_files, int execute_flag, char *search_term, char *search_dir, char *prev_dir, int *found_match);
void proclore(char *pid);
void print_activities();
int compare_processes(const void *a, const void *b);

typedef struct {
    char command[256];
    pid_t pid;
    char state[16];
} ProcessInfo;

ProcessInfo process_list[100];
int process_count = 0;

void add_process(const char *command, pid_t pid, const char *state) {
    if (process_count < 100) {
        strncpy(process_list[process_count].command, command, sizeof(process_list[process_count].command) - 1);
        process_list[process_count].pid = pid;
        strncpy(process_list[process_count].state, state, sizeof(process_list[process_count].state) - 1);
        process_count++;
    }
}

void update_process_state(pid_t pid, const char *state) {
    for (int i = 0; i < process_count; i++) {
        if (process_list[i].pid == pid) {
            strncpy(process_list[i].state, state, sizeof(process_list[i].state) - 1);
            break;
        }
    }
}

int compare_processes(const void *a, const void *b) {
    return strcasecmp(((ProcessInfo *)a)->command, ((ProcessInfo *)b)->command);
}

void print_activities() {
    qsort(process_list, process_count, sizeof(ProcessInfo), compare_processes);
    for (int i = 0; i < process_count; i++) {
        printf("%d : %s - %s\n", process_list[i].pid, process_list[i].command, process_list[i].state);
    }
}

void remove_process(pid_t pid) {
    for (int i = 0; i < process_count; i++) {
        if (process_list[i].pid == pid) {
            for (int j = i; j < process_count - 1; j++) {
                process_list[j] = process_list[j + 1];
            }
            process_count--;
            break;
        }
    }
}

void process_input(char *input, char *prev_dir);
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


#define MAX_LOG_COMMANDS 15

void log_command(char *input) {
    FILE *file;
    char *log_commands[MAX_LOG_COMMANDS];
    int log_count = 0;
    char line[256];
    char log_file_path[512];

    // Get the home directory path
    const char *home_dir = getenv("HOME");
    if (home_dir == NULL) {
        perror("Error getting home directory");
        return;
    }

    // Construct the absolute path for the log file
    snprintf(log_file_path, sizeof(log_file_path), "%s/Code/Year_2/OSN/MP1/log.txt", home_dir);

    // Open the log file for reading
    file = fopen(log_file_path, "r");
    if (file == NULL) {
        perror("Error opening log file for reading");
        return;
    }

    while (fgets(line, sizeof(line), file) != NULL && log_count < MAX_LOG_COMMANDS) {
        line[strcspn(line, "\n")] = 0; // Remove newline character
        log_commands[log_count++] = strdup(line);
    }
    fclose(file);

    // Check if the new command contains "log"
    if (strstr(input, "log") != NULL) {
        return;
    }

    // Check if the new command is the same as the last logged command
    if (log_count > 0 && strcmp(log_commands[log_count - 1], input) == 0) {
        return;
    }

    // Add the new command to the log
    if (log_count == MAX_LOG_COMMANDS) {
        free(log_commands[0]);
        memmove(log_commands, log_commands + 1, (MAX_LOG_COMMANDS - 1) * sizeof(char *));
        log_count--;
    }
    log_commands[log_count++] = strdup(input);

    // Open the log file for writing
    file = fopen(log_file_path, "w");
    if (file == NULL) {
        perror("Error opening log file for writing");
        return;
    }

    for (int i = 0; i < log_count; i++) {
        fprintf(file, "%s\n", log_commands[i]);
        free(log_commands[i]);
    }
    fclose(file);
}

void display_log() {
    FILE *file;
    char line[256];
    char log_file_path[512];

    // Get the home directory path
    const char *home_dir = getenv("HOME");
    if (home_dir == NULL) {
        perror("Error getting home directory");
        return;
    }

    // Construct the absolute path for the log file
    snprintf(log_file_path, sizeof(log_file_path), "%s/Code/Year_2/OSN/MP1/log.txt", home_dir);

    // Open the log file for reading
    file = fopen(log_file_path, "r");
    if (file == NULL) {
        perror("Error opening log file for reading");
        return;
    }

    // Display the contents of the log file
    int index = 0;
    while (fgets(line, sizeof(line), file) != NULL) {
        printf("%d: %s", index++, line);
    }
    fclose(file);
}

void purge_log() {
    char log_file_path[512];

    // Get the home directory path
    const char *home_dir = getenv("HOME");
    if (home_dir == NULL) {
        perror("Error getting home directory");
        return;
    }

    // Construct the absolute path for the log file
    snprintf(log_file_path, sizeof(log_file_path), "%s/Code/Year_2/OSN/MP1/log.txt", home_dir);

    // Open the log file for writing (truncate to zero length)
    FILE *file = fopen(log_file_path, "w");
    if (file == NULL) {
        perror("Error opening log file for writing");
        return;
    }
    fclose(file);
}

void execute_log_command(int index, char *prev_dir) {
    FILE *file;
    char line[256];
    char log_file_path[512];

    // Get the home directory path
    const char *home_dir = getenv("HOME");
    if (home_dir == NULL) {
        perror("Error getting home directory");
        return;
    }

    // Construct the absolute path for the log file
    snprintf(log_file_path, sizeof(log_file_path), "%s/Code/Year_2/OSN/MP1/log.txt", home_dir);

    // Open the log file for reading
    file = fopen(log_file_path, "r");
    if (file == NULL) {
        perror("Error opening log file for reading");
        return;
    }

    // Read the command at the specified index
    int current_index = 0;
    while (fgets(line, sizeof(line), file) != NULL) {
        if (current_index == index) {
            line[strcspn(line, "\n")] = 0; // Remove newline character
            process_input(line, prev_dir);
            break;
        }
        current_index++;
    }
    fclose(file);
}



void hop(char *arg, char *prev_dir) {
    char current_dir[PATH_MAX];
    if (getcwd(current_dir, sizeof(current_dir)) == NULL) {
        perror("getcwd");
        return;
    }

    if (arg == NULL || strcmp(arg, "~") == 0) {
        // Change to home directory
        struct passwd *pw = getpwuid(getuid());
        if (pw == NULL) {
            perror("getpwuid");
            return;
        }
        if (chdir(pw->pw_dir) != 0) {
            perror("chdir");
        } else {
            strcpy(prev_dir, current_dir);
        }
    } else if (strcmp(arg, "-") == 0) {
        // Change to previous directory
        if (chdir(prev_dir) != 0) {
            perror("chdir");
        } else {
            printf("%s\n", prev_dir);
            strcpy(prev_dir, current_dir);
        }
    } else if (strcmp(arg, ".") == 0) {
        // Stay in the current directory
    } else if (strcmp(arg, "..") == 0) {
        // Change to parent directory
        if (chdir("..") != 0) {
            perror("chdir");
        } else {
            strcpy(prev_dir, current_dir);
        }
    } else if (arg[0] == '/') {
        // Change to specified absolute path
        if (chdir(arg) != 0) {
            perror("chdir");
        } else {
            strcpy(prev_dir, current_dir);
        }
    } else {
        // Change to specified relative directory
        if (chdir(arg) != 0) {
            perror("chdir");
        } else {
            strcpy(prev_dir, current_dir);
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
        reveal_path = pw->pw_dir;  // Expand ~ to the home directory
    } else if (strcmp(reveal_path, "-") == 0) {
        reveal_path = prev_dir;  // Use previous directory for '-'
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


//process input and put the commands in the arrays

// void process_input(char *input, char foreground[1024][1024], char background[1024][1024]) {
//     char *token = strtok(input, ";");
//     int i = 0;
//     while (token != NULL) {
//         strcpy(foreground[i], token);
//         token = strtok(NULL, ";");
//         i++;
//     }
//     foreground[i][0] = '\0';
//     i = 0;
//     token = strtok(input, "&");
//     while (token != NULL) {
//         strcpy(background[i], token);
//         token = strtok(NULL, "&");
//         i++;
//     }
//     background[i][0] = '\0';

//     //print the commands in the arrays
//     printf("Foreground commands:\n");
//     for (int j = 0; foreground[j][0] != '\0'; j++) {
//         printf("%s\n", foreground[j]);
//     }
//     printf("Background commands:\n");
//     for (int j = 0; background[j][0] != '\0'; j++) {
//         printf("%s\n", background[j]);
//     }
// }

void seek(int only_directories, int only_files, int execute_flag, char *search_term, char *search_dir, char *prev_dir, int *found_match) {
    // Check for invalid flag combination
    if (only_directories && only_files) {
        printf("Invalid flags!\n");
        return;
    }

    // Handle special symbols for the directory
    if (strcmp(search_dir, "~") == 0) {
        struct passwd *pw = getpwuid(getuid());
        if (pw == NULL) {
            perror("getpwuid");
            return;
        }
        search_dir = pw->pw_dir;  // Expand ~ to home directory
    } else if (strcmp(search_dir, "-") == 0) {
        search_dir = prev_dir;  // Use previous directory
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


void proclore(char *pid_str) {
    char path[PATH_MAX];
    char buffer[1024];
    FILE *file;
    int pid;
    char status;
    int pgrp;
    unsigned long vsize;
    char exe_path[PATH_MAX];
    int tty_nr;

    // If no PID is provided, use the current process's PID
    if (pid_str == NULL) {
        pid = getpid();
    } else {
        pid = atoi(pid_str);
    }

    // Construct the path to the /proc/[pid]/stat file
    snprintf(path, sizeof(path), "/proc/%d/stat", pid);

    // Open the /proc/[pid]/stat file
    file = fopen(path, "r");
    if (file == NULL) {
        perror("Error opening /proc/[pid]/stat file");
        return;
    }

    // Read the process information from the /proc/[pid]/stat file
    fscanf(file, "%d %*s %c %*d %d %*d %*d %*d %*d %*u %*u %*u %*u %*u %*u %*u %*d %*d %*d %*d %*d %*d %*d %lu %*d %*d %*d %d", &pid, &status, &pgrp, &vsize, &tty_nr);
    fclose(file);

    // Construct the path to the /proc/[pid]/exe file
    snprintf(path, sizeof(path), "/proc/%d/exe", pid);

    // Read the executable path
    ssize_t len = readlink(path, exe_path, sizeof(exe_path) - 1);
    if (len != -1) {
        exe_path[len] = '\0';
    } else {
        perror("Error reading /proc/[pid]/exe file");
        return;
    }

    // Check if the process is a background process
    char status_with_bg = status;
    if (tty_nr == 0) {
        status_with_bg = '+';
    }

    // Print the process information
    printf("pid : %d\n", pid);
    printf("process status : %c\n", status_with_bg);
    printf("Process Group : %d\n", pgrp);
    printf("Virtual memory : %lu\n", vsize);
    printf("executable path : %s\n", exe_path);
}

// Function to find a process by pid in the process list
ProcessInfo* find_process_by_pid(pid_t pid) {
    for (int i = 0; i < process_count; i++) {
        if (process_list[i].pid == pid) {
            return &process_list[i];
        }
    }
    return NULL;
}


// Function to handle the ping command
void handle_ping_command(char *pid_str, char *signal_str) {
    pid_t pid = atoi(pid_str);
    int signal_number = atoi(signal_str) % 32; // Modulo 32 for signal handling

    ProcessInfo* process = find_process_by_pid(pid);
    if (process == NULL) {
        printf("No such process found\n");
        return;
    }

    // Send the signal to the process
    if (kill(pid, signal_number) == 0) {
        printf("Sent signal %d to process with pid %d\n", signal_number, pid);

        // Handle specific signals after they are sent
        if (signal_number == SIGKILL) {
            // SIGKILL forces process termination, cannot be caught
            remove_process(pid);
        } else if (signal_number == SIGTSTP || signal_number == SIGSTOP) {
            // SIGTSTP/SIGSTOP stop the process, update its state
            update_process_state(pid, "Stopped");
        } else if (signal_number == SIGCONT) {
            // SIGCONT continues a stopped process, update its state
            update_process_state(pid, "Running");
        }
    } else {
        // Error handling if the signal could not be sent
        perror("Failed to send signal");
    }
}


// Function to bring a process to the foreground
void handle_fg_command(char *pid_str) {
    pid_t pid = atoi(pid_str);

    ProcessInfo* process = find_process_by_pid(pid);
    if (process == NULL) {
        printf("No such process found\n");
        return;
    }

    if (strcmp(process->state, "Stopped") == 0) {
        kill(pid, SIGCONT);
    }

    update_process_state(pid, "Running");

    // Wait for the process to finish
    int status;
    waitpid(pid, &status, 0);
    remove_process(pid);
}

// Function to run a process in the background
void handle_bg_command(char *pid_str) {
    pid_t pid = atoi(pid_str);

    ProcessInfo* process = find_process_by_pid(pid);
    if (process == NULL) {
        printf("No such process found\n");
        return;
    }

    if (strcmp(process->state, "Stopped") == 0) {
        kill(pid, SIGCONT);
    }
    update_process_state(pid, "Running");
}

// Signal handler for Ctrl-C (SIGINT)
void sigint_handler(int sig) {
    for (int i = 0; i < process_count; i++) {
        if (strcmp(process_list[i].state, "Running") == 0) {
            kill(process_list[i].pid, SIGINT);
            update_process_state(process_list[i].pid, "Stopped");
            break;
        }
    }
}

// Signal handler for Ctrl-D (EOF)
void sigquit_handler(int sig) {
    for (int i = 0; i < process_count; i++) {
        kill(process_list[i].pid, SIGKILL);
    }
    exit(0);
}

// Signal handler for Ctrl-Z (SIGTSTP)
// Signal handler for Ctrl-Z (SIGTSTP)
void sigtstp_handler(int sig) {
    pid_t shell_pid = getpid();  // Get the shell's process ID

    if (process_count > 0) {
        int i = process_count - 1;  // Index of the last element (assumes this is the foreground process)
        if (process_list[i].pid != shell_pid && strcmp(process_list[i].state, "Running") == 0) {
            // Send SIGTSTP to the last process (foreground process)
            kill(process_list[i].pid, SIGTSTP);

            // Update process state in the list to "Stopped"
            update_process_state(process_list[i].pid, "Stopped");
            printf("\nProcess with pid %d stopped\n", process_list[i].pid);

            // Switch terminal control back to the shell
            tcsetpgrp(STDIN_FILENO, shell_pid);
        }
    }
}


int get_most_recent_pid() {
    DIR *dir;
    struct dirent *entry;
    int max_pid = -1;
    struct stat statbuf;
    char path[PATH_MAX];

    if ((dir = opendir("/proc")) == NULL) {
        perror("opendir");
        return -1;
    }

    while ((entry = readdir(dir)) != NULL) {
        int ret = snprintf(path, sizeof(path), "/proc/%s", entry->d_name);
        if (ret < 0 || ret >= sizeof(path)) {
            fprintf(stderr, "Path truncation occurred\n");
            continue;
        }
        if (stat(path, &statbuf) == 0 && S_ISDIR(statbuf.st_mode)) {
            int pid = atoi(entry->d_name);
            if (pid > 0 && pid > max_pid) {
                max_pid = pid;
            }
        }
    }

    closedir(dir);
    return max_pid;
}

int kbhit(void) {
    struct termios oldt, newt;
    int ch;
    int oldf;

    tcgetattr(STDIN_FILENO, &oldt);
    newt = oldt;
    newt.c_lflag &= ~(ICANON | ECHO);
    tcsetattr(STDIN_FILENO, TCSANOW, &newt);
    oldf = fcntl(STDIN_FILENO, F_GETFL, 0);
    fcntl(STDIN_FILENO, F_SETFL, oldf | O_NONBLOCK);

    ch = getchar();

    tcsetattr(STDIN_FILENO, TCSANOW, &oldt);
    fcntl(STDIN_FILENO, F_SETFL, oldf);

    if (ch != EOF) {
        ungetc(ch, stdin);
        return 1;
    }

    return 0;
}

void neonate(int time_arg) {
    while (1) {
        int pid = get_most_recent_pid();
        if (pid != -1) {
            printf("%d\n", pid);
        } else {
            printf("Error retrieving most recent PID\n");
        }

        sleep(time_arg);

        if (kbhit() && getchar() == 'x') {
            break;
        }
    }
}

#define BUFFER_SIZE 4096

#define BUFFER_SIZE 4096

#define BUFFER_SIZE 4096

void fetch_man_page(const char *command_name) {
    int sockfd;
    struct sockaddr_in server_addr;
    struct hostent *server;
    char request[BUFFER_SIZE];
    char response[BUFFER_SIZE];
    int bytes_received;
    int header_end = 0;

    // Create a socket
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0) {
        perror("Error opening socket");
        exit(EXIT_FAILURE);
    }

    // Get the server by name
    server = gethostbyname("man.he.net");
    if (server == NULL) {
        fprintf(stderr, "Error, no such host\n");
        exit(EXIT_FAILURE);
    }

    // Set up the server address structure
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    memcpy(&server_addr.sin_addr.s_addr, server->h_addr_list[0], server->h_length);
    server_addr.sin_port = htons(80);

    // Connect to the server
    if (connect(sockfd, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
        perror("Error connecting");
        exit(EXIT_FAILURE);
    }

    // Format the GET request with the correct URL for fetching man pages
    snprintf(request, sizeof(request),
             "GET /cgi-bin/man/man2html?query=%s HTTP/1.1\r\n"
             "Host: man.he.net\r\n"
             "Connection: close\r\n\r\n",
             command_name);

    // Send the GET request
    if (send(sockfd, request, strlen(request), 0) < 0) {
        perror("Error sending request");
        exit(EXIT_FAILURE);
    }

    // Debug: Print the request
    printf("Request sent:\n%s\n", request);

    // Receive the response
    while ((bytes_received = recv(sockfd, response, sizeof(response) - 1, 0)) > 0) {
        response[bytes_received] = '\0';

        // Debug: Print the raw response
        printf("Raw response received:\n%s\n", response);

        // Skip the HTTP headers
        if (!header_end) {
            char *header_end_ptr = strstr(response, "\r\n\r\n");
            if (header_end_ptr) {
                header_end = 1;
                printf("%s", header_end_ptr + 4);
            }
        } else {
            printf("%s", response);
        }
    }

    if (bytes_received < 0) {
        perror("Error receiving response");
    }

    // Close the socket
    close(sockfd);
}


typedef struct {
    char alias[MAX_ALIAS_LENGTH];
    char command[MAX_COMMAND_LENGTH];
} Alias;

Alias aliases[MAX_ALIASES];
int alias_count = 0;

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

#define MAX_LINE 1024
#define MAX_COMMANDS 100

void handle_command(char *input, char *prev_dir, int is_background) {
    char *expanded_command = expand_alias(input);
    if (expanded_command) {
        input = expanded_command;
        // printf("Expanded command: %s\n", input);
    }

    // Split input by pipes
    char *commands[MAX_COMMANDS];
    int num_commands = 0;
    char *command = strtok(input, "|");

    while (command != NULL) {
        commands[num_commands++] = command;
        command = strtok(NULL, "|");
    }

    // Validate pipe usage
    for (int i = 0; i < num_commands; i++) {
        commands[i] = trim_whitespace(commands[i]);
        if (strlen(commands[i]) == 0) {
            printf("Invalid use of pipe\n");
            return;
        }
    }

    if (num_commands > 1) {
        // Execute piped commands
        execute_piped_commands(commands, num_commands, prev_dir);
        return;
    }

    // Existing handle_command logic for single commands
    command = strtok(input, " ");
    char *arg;
    char *args[100];
    int arg_count = 0;
    char *input_file = NULL;
    char *output_file = NULL;
    int append = 0;

    // Collect all arguments
    args[arg_count++] = command; // First argument is the command itself
    while ((arg = strtok(NULL, " ")) != NULL) {
        if (strcmp(arg, "<") == 0) {
            input_file = strtok(NULL, " ");
        } else if (strcmp(arg, ">") == 0) {
            output_file = strtok(NULL, " ");
            append = 0;
        } else if (strcmp(arg, ">>") == 0) {
            output_file = strtok(NULL, " ");
            append = 1;
        } else {
            args[arg_count++] = arg;
        }
    }
    args[arg_count] = NULL; // Null-terminate the arguments array
    
    //trim leading and trailing quotations form arguments
    for (int i = 0; i < arg_count; i++) {
        args[i] = trim_quotes(args[i]);
    }
    
    if (strcmp(command, "hop") == 0) {
        // If no argument is present, hop to the home directory
        if (arg_count == 1) {
            hop(NULL, prev_dir);
        } else {
            // Execute hop sequentially with all arguments
            for (int i = 1; i < arg_count; i++) {
                hop(args[i], prev_dir);
            }
        }
    } else if (strcmp(command, "reveal") == 0) {
        // Parse all flags and path
        int show_all = 0;
        int long_format = 0;
        char *reveal_path = ".";

        for (int i = 1; i < arg_count; i++) {
            if (strcmp(args[i], "-") == 0) {
                // Special case: "-" is treated as a path
                reveal_path = args[i];
            } else if (args[i][0] == '-') {
                for (int j = 1; args[i][j] != '\0'; j++) {
                    if (args[i][j] == 'a') {
                        show_all = 1;
                    } else if (args[i][j] == 'l') {
                        long_format = 1;
                    }
                }
            } else {
                reveal_path = args[i];
            }
        }

        reveal(show_all, long_format, reveal_path, prev_dir);
    } else if (strcmp(command, "log") == 0) {
        if (arg_count == 1) {
            display_log();
        } else if (strcmp(args[1], "purge") == 0) {
            purge_log();
        } else if (strcmp(args[1], "execute") == 0) {
            if (arg_count > 2) {
                int index = atoi(args[2]);
                execute_log_command(index, prev_dir);
            } else {
                printf("Error: Missing index for log execute command\n");
            }
        } else {
            printf("Unknown log command: %s\n", args[1]);
        }
    } else if (strcmp(command, "seek") == 0) {
        int only_directories = 0;
        int only_files = 0;
        int execute_flag = 0;
        char *search_term = NULL;
        char *search_dir = ".";
        int found_match = 0;

        for (int i = 1; i < arg_count; i++) {
            if (args[i][0] == '-') {
                for (int j = 1; args[i][j] != '\0'; j++) {
                    if (args[i][j] == 'd') {
                        only_directories = 1;
                    } else if (args[i][j] == 'f') {
                        only_files = 1;
                    } else if (args[i][j] == 'e') {
                        execute_flag = 1;
                    }
                }
            } else if (search_term == NULL) {
                search_term = args[i];
            } else {
                search_dir = args[i];
            }
        }

        if (search_term == NULL) {
            printf("Error: Missing search term\n");
        } else {
            seek(only_directories, only_files, execute_flag, search_term, search_dir, prev_dir, &found_match);
            if (!found_match) {
                printf("No matches found\n");
            }
        }
    } else if (strcmp(command, "proclore") == 0) {
        if (arg_count == 1) {
            proclore(NULL); // No argument, use current process
        } else {
            proclore(args[1]); // Use provided PID
        }
    } else if (strcmp(command, "activities") == 0) {
        print_activities();
    } else if (strcmp(command, "ping") == 0) {
        if (arg_count == 3) {
            handle_ping_command(args[1], args[2]);
        } else {
            printf("Usage: ping <pid> <signal_number>\n");
        }
    } else if (strcmp(command, "fg") == 0) {
        if (arg_count == 2) {
            handle_fg_command(args[1]);
        } else {
            printf("Usage: fg <pid>\n");
        }
    } else if (strcmp(command, "bg") == 0) {
        if (arg_count == 2) {
            handle_bg_command(args[1]);
        } else {
            printf("Usage: bg <pid>\n");
        }
    } 
   else if (strcmp(command, "neonate") == 0) {
        if (arg_count == 3 && strcmp(args[1], "-n") == 0) {
            int time_arg = atoi(args[2]);
            neonate(time_arg);
        } else {
            printf("Usage: neonate -n [time_arg]\n");
        }
    }
    else if (strcmp(command, "iMan") == 0) {
        if (arg_count == 2) {
            fetch_man_page(args[1]);
        } else {
            printf("Usage: iMan <command_name>\n");
        }
    } 
     else {
        // Handle system commands
        pid_t pid = fork();
        if (pid == 0) {
            // Child process

            // Handle input redirection
            if (input_file != NULL) {
                int fd = open(input_file, O_RDONLY);
                if (fd < 0) {
                    perror("No such input file found!");
                    exit(1);
                }
                dup2(fd, STDIN_FILENO);
                close(fd);
            }

            // Handle output redirection
            if (output_file != NULL) {
                int fd;
                if (append) {
                    fd = open(output_file, O_WRONLY | O_CREAT | O_APPEND, 0644);
                } else {
                    fd = open(output_file, O_WRONLY | O_CREAT | O_TRUNC, 0644);
                }
                if (fd < 0) {
                    perror("Failed to open output file");
                    exit(1);
                }
                dup2(fd, STDOUT_FILENO);
                close(fd);
            }

            // Redirect input/output for background processes
            if (is_background) {
                int fd = open("/dev/null", O_RDWR);
                if (fd < 0) {
                    perror("Failed to open /dev/null");
                    exit(1);
                }
                dup2(fd, STDIN_FILENO);
                dup2(fd, STDOUT_FILENO);
                dup2(fd, STDERR_FILENO);
                close(fd);
            }

            execvp(command, args);
            // If execvp returns, it must have failed
            printf("ERROR: '%s' is not a valid command\n", command);
            exit(EXIT_FAILURE);
        } else if (pid > 0) {
            // Parent process
            add_process(command, pid, "Running");
            if (is_background) {
                // Background process
                printf("%d\n", pid);
            } else {
                // Foreground process
                time_t start_time = time(NULL);
                int status;
                waitpid(pid, &status, 0);
                time_t end_time = time(NULL);
                int elapsed_time = (int)(end_time - start_time);
                if (elapsed_time > 2) {
                    printf("%s : %ds\n", command, elapsed_time);
                }
                remove_process(pid);
            }
        } else {
            perror("fork");
        }
    }
}

void execute_piped_commands(char **commands, int num_commands, char *prev_dir) {
    int pipefds[2 * (num_commands - 1)];
    pid_t pid;
    int status;

    // Create pipes
    for (int i = 0; i < num_commands - 1; i++) {
        if (pipe(pipefds + i * 2) < 0) {
            perror("pipe");
            exit(EXIT_FAILURE);
        }
    }

    for (int i = 0; i < num_commands; i++) {
        pid = fork();
        if (pid == 0) {
            // Child process

            // Redirect input from previous pipe if not the first command
            if (i > 0) {
                if (dup2(pipefds[(i - 1) * 2], 0) < 0) {
                    perror("dup2");
                    exit(EXIT_FAILURE);
                }
            }

            // Redirect output to next pipe if not the last command
            if (i < num_commands - 1) {
                if (dup2(pipefds[i * 2 + 1], 1) < 0) {
                    perror("dup2");
                    exit(EXIT_FAILURE);
                }
            }

            // Close all pipe file descriptors
            for (int j = 0; j < 2 * (num_commands - 1); j++) {
                close(pipefds[j]);
            }

            // Execute the command
            handle_command(commands[i], prev_dir, 0);
            exit(EXIT_FAILURE);
        } else if (pid < 0) {
            perror("fork");
            exit(EXIT_FAILURE);
        }
    }

    // Close all pipe file descriptors in the parent process
    for (int i = 0; i < 2 * (num_commands - 1); i++) {
        close(pipefds[i]);
    }

    // Wait for all child processes to finish
    for (int i = 0; i < num_commands; i++) {
        wait(&status);
    }
}

void execute_commands(char **foreground, int num_foreground, char **background, int num_background, char *prev_dir) {
    // Execute background commands
    for (int i = 0; i < num_background; i++) {
        handle_command(background[i], prev_dir, 1); // 1 for background
    }

    // Execute foreground commands
    for (int i = 0; i < num_foreground; i++) {
        handle_command(foreground[i], prev_dir, 0); // 0 for foreground
    }
}

void process_input(char *input, char *prev_dir) {
    input = trim_whitespace(input);
    log_command(input);

    char *foreground_commands[MAX_COMMANDS];
    int num_foreground = 0;
    char *background_commands[MAX_COMMANDS];
    int num_background = 0;

    // Tokenize the input by ';'
    char *command;
    command = strtok(input, ";");

    while (command != NULL) {
        command = trim_whitespace(command);
        if (strlen(command) > 0) {
            char *ampersand;
            while ((ampersand = strchr(command, '&')) != NULL) {
                *ampersand = '\0';
                command = trim_whitespace(command); // Trim trailing spaces
                if (strlen(command) > 0) {
                    background_commands[num_background++] = strdup(command);
                }
                command = ampersand + 1; // Move past '&'
            }
            command = trim_whitespace(command);
            if (command != NULL && strlen(command) > 0) {
                foreground_commands[num_foreground++] = strdup(command);
            }
        }
        command = strtok(NULL, ";");
    }
    execute_commands(foreground_commands, num_foreground, background_commands, num_background, prev_dir);
}

int main() {
    char hostname[1024];
    char cwd[1024];
    char input[1024];
    char prev_dir[PATH_MAX];
    struct passwd *pw;

    // Get the username
    pw = getpwuid(getuid());
    if (pw == NULL) {
        perror("getpwuid");
        exit(EXIT_FAILURE);
    }

    // Get the hostname
    if (gethostname(hostname, sizeof(hostname)) != 0) {
        perror("gethostname");
        exit(EXIT_FAILURE);
    }

    // Initialize prev_dir to the current directory
    if (getcwd(prev_dir, sizeof(prev_dir)) == NULL) {
        perror("getcwd");
        exit(EXIT_FAILURE);
    }

    // Set up signal handlers
    signal(SIGINT, sigint_handler);
    signal(SIGQUIT, sigquit_handler);
    signal(SIGTSTP, sigtstp_handler);

    // Load aliases from .myshrc file
    load_aliases(".myshrc");

    while (1) {
        // Get the current working directory
        if (getcwd(cwd, sizeof(cwd)) == NULL) {
            perror("getcwd");
            exit(EXIT_FAILURE);
        }

        // Display the prompt
        printf("%s@%s:%s$ ", pw->pw_name, hostname, cwd);

        // Read the user input
        if (fgets(input, sizeof(input), stdin) == NULL) {
            perror("fgets");
            exit(EXIT_FAILURE);

        }

        // Remove the newline character from the input
        input[strcspn(input, "\n")] = '\0';

        // Process input and put the commands in the arrays
        process_input(input, prev_dir);

        // Check for background process completion
        int status;
        pid_t pid;
        while ((pid = waitpid(-1, &status, WNOHANG)) > 0) {
            ProcessInfo* process_info = find_process_by_pid(pid);
            if (process_info != NULL) {
                if (WIFEXITED(status)) {
                    printf("%s exited normally (%d)\n", process_info->command, pid);
                } else {
                    printf("%s exited abnormally (%d)\n", process_info->command, pid);
                }
                remove_process(pid);
            } else {
                if (WIFEXITED(status)) {
                    printf("Process %d exited normally\n", pid);
                } else {
                    printf("Process %d exited abnormally\n", pid);
                }
                remove_process(pid);
            }
        }
    }

    return 0;
}
