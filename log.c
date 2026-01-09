#include "log.h"
#include "command_handler.h"



char log_file_path[LOG_FILE_PATH_SIZE];

void update_log_path(const char *base_path) {
    snprintf(log_file_path, sizeof(log_file_path), "%s/log.txt", base_path);
}

void log_command(char *input) {
    FILE *file;
    char *log_commands[MAX_LOG_COMMANDS];
    int log_count = 0;
    char line[256];

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