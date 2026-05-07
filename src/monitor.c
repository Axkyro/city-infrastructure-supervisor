#include "types.h"
#include <fcntl.h>
#include <signal.h>
#include <stdio.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <unistd.h>

#define OPEN_FAIL 1
#define WRITE_FAIL 2
#define DELETE_FAIL 3

void sig_handler(int signo);

volatile sig_atomic_t KILL_PROGRAM = 0;

int main() {
    // setup
    char path[MAX_PATH_LEN] = "districts/.monitor_pid";
    int fd = open(path, O_WRONLY | O_TRUNC | O_CREAT, 0644);
    if (fd == -1) {
        perror(path);
        return OPEN_FAIL;
    }
    pid_t monitor_pid = getpid();
    char process[MAX_PID_DIGITS_LEN];
    snprintf(process, MAX_PID_DIGITS_LEN, "%d", monitor_pid);

    if (write(fd, process, strlen(process)) != strlen(process)) {
        fprintf(stderr, "Failed write to %s!\n", path);
        return WRITE_FAIL;
    }

    if (close(fd) == -1) {
        perror(path);
        return WRITE_FAIL;
    }

    // main part
    struct sigaction action;
    action.sa_handler = sig_handler;
    action.sa_flags = 0;
    sigemptyset(&action.sa_mask);

    if (sigaction(SIGINT, &action, NULL) == -1) {
        perror("Error SIGINT");
        return 0;
    }

    if (sigaction(SIGUSR1, &action, NULL) == -1) {
        perror("Error SIGUSR1");
        return 0;
    }

    while (!KILL_PROGRAM)
        sleep(1);

    // cleanup

    if (unlink(path) == -1) {
        fprintf(stderr, "Failed to delete: [%s]!\n", path);
        return DELETE_FAIL;
    }

    /*
    process = fork();

    if (process == 0) {
        execlp("rm", "rm", path, NULL);
        perror(path);
        return DELETE_FAIL;
    } else {
        waitpid(process, NULL, 0);
        return 0;
    }
    */
    return 0;
}

void sig_handler(int signo) {
    if (signo == SIGUSR1) {
        printf("A new report has been added!\n");
    } else if (signo == SIGINT) {
        printf("SIGINT received. Ending monitor!\n");
        KILL_PROGRAM = 1;
    }
}
