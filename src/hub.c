#include "types.h"
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <unistd.h>

#define ARG_READ_FAIL 1

typedef enum {
    StartMonitor,
    CalculateDistrictScores,
} HubOperation;

HubOperation parse_command(int argc, char **argv);
void read_pipe_input(int fd);

int END_PROGRAM = 0;

int main(int argc, char *argv[]) {
    pid_t pids[argc];
    int child_count = 0;

    HubOperation op = parse_command(argc, argv);
    int fd[2];
    pipe(fd);
    switch (op) {
    case StartMonitor: {
        pid_t ret = fork();
        if (ret == 0) {

            close(fd[0]);
            dup2(fd[1], STDOUT_FILENO);
            close(fd[1]);
            execl("./monitor_reports", "./monitor_reports", NULL);
            _exit(1);
        } else {
            pids[child_count++] = ret;
        }
        break;
    }
    case CalculateDistrictScores: {
        for (size_t i = 2; i < argc; i++) {
            pid_t child_scorer = fork();

            if (child_scorer == 0) {
                close(fd[0]);
                dup2(fd[1], STDOUT_FILENO);
                close(fd[1]);
                execl("./scorer", "./scorer", argv[i], NULL);
                _exit(1);
            }
            pids[child_count++] = child_scorer;
        }
        break;
    }

    default:
        fprintf(stderr, "Not such option, try ./city_hub --start_monitor!\n");
    }
    close(fd[1]);
    read_pipe_input(fd[0]);
    for (int i = 0; i < child_count; i++)
        waitpid(pids[i], NULL, 0);
    return 0;
}

HubOperation parse_command(int argc, char **argv) {
    if (argc < 2) {
        fprintf(stderr, "Try hub --start_monitor");
        exit(ARG_READ_FAIL);
    }
    if (strcmp(argv[1], "--start_monitor") == 0) {
        return StartMonitor;
    } else if (strcmp(argv[1], "--calculate_scores") == 0) {
        return CalculateDistrictScores;
    } else {
        fprintf(stderr, "Unknown command: %s\n", argv[1]);
        exit(ARG_READ_FAIL);
    }
}

void read_pipe_input(int fd) {
    char buffer_messages[MAX_MONITOR_MESSAGE_SIZE_BUFFER];

    while (!END_PROGRAM) {
        ssize_t n = read(fd, buffer_messages, sizeof(buffer_messages) - 1);
        if (n < 0)
            break;
        buffer_messages[n] = '\0';

        if (strstr(buffer_messages, "end")) {
            END_PROGRAM = 1;
        }
        if (n == 0) {
            break;
        }
        printf("%s", buffer_messages);
    }
}
