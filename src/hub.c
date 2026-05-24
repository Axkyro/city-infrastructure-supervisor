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
int END_PROGRAM = 0;
int main(int argc, char *argv[]) {

    HubOperation op = parse_command(argc, argv);

    switch (op) {
    case StartMonitor: {
        // we first fork the current process
        int fd[2];
        pipe(fd);

        pid_t ret = fork();
        if (ret == 0) {
            close(fd[0]);
            dup2(fd[1], STDOUT_FILENO);
            close(fd[1]);
            execl("./monitor_reports", "./monitor_reports", NULL);
        } else {
            close(fd[1]);
            char buffer_messages[MAX_MONITOR_MESSAGE_SIZE_BUFFER];
            while (!END_PROGRAM) {
                read(fd[0], buffer_messages, sizeof(buffer_messages) - 1);
                buffer_messages[MAX_MONITOR_MESSAGE_SIZE_BUFFER - 1] = '\0';

                if (strstr(buffer_messages, "run")) {
                    printf("There is already a running monitor!");
                    END_PROGRAM = 1;
                }
                printf("%s", buffer_messages);
            }
        }

        break;
    }

    default:
        fprintf(stderr, "Not yet implemented!\n");

        return 0;
    }
}
