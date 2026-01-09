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



