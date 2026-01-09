#include "neonate.h"



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

