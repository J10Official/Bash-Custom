#include "command_handler.h"
#include "utils.h"
#include "processlist.h"
#include "directory.h"
#include "alias.h"
#include "log.h"
#include "neonate.h"
#include "networking.h"
#include "proclore.h"
#include "seek.h"
#include "signals.h"














#define COLOR_RESET "\033[0m"
#define COLOR_GREEN "\033[32m"
#define COLOR_WHITE "\033[37m"
#define COLOR_BLUE "\033[34m"
#define PATH_MAX 4096
#define MAX_COMMAND_LENGTH 100


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

    //store the current directory in a variable
    char *current_dir = prev_dir;
    update_log_path(current_dir);
    update_home_path(current_dir);

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
        // Translate the current directory to a relative path
        translate_to_relative_path(cwd);

        // Display the prompt
        printf("%s@%s:%s> ", pw->pw_name, hostname, cwd);

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
