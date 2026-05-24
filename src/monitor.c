#include "types.h"
#include <fcntl.h>
#include <linux/limits.h>
#include <signal.h>
#include <stdio.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <unistd.h>

#define OPEN_FAIL 1
#define WRITE_FAIL 2
#define DELETE_FAIL 3
#define ALREADY_RUNNING 4

void sig_handler(int signo); // signal handler for SIGINT and SIGUSR1

volatile sig_atomic_t KILL_PROGRAM = 0;

int main() {
    // setup
    char path[MAX_PATH_LEN] = "districts/.monitor_pid";
    int fd = open(path, O_RDONLY, 0644);

    if (fd != -1) {
        char monitor_pid_text[MAX_PID_DIGITS_LEN] = "";

        // strange fail case
        if (read(fd, &monitor_pid_text, sizeof(monitor_pid_text)) == -1) {
            return ALREADY_RUNNING;
        }

        printf("end: Monitor already running with pid: %s!\n",
               monitor_pid_text);
        fflush(stdout);

        close(fd);
        return ALREADY_RUNNING;
    }

    fd = open(path, O_WRONLY | O_TRUNC | O_CREAT, 0644);
    if (fd == -1) {
        perror(path);
        return OPEN_FAIL;
    }

    fd = open(path, O_WRONLY | O_TRUNC | O_CREAT, 0644);

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
        printf("new: A new report has been added!\n");
        fflush(stdout);
    } else if (signo == SIGINT) {
        printf("end: SIGINT received. Ending monitor!\n");
        fflush(stdout);
        KILL_PROGRAM = 1;
    }
}
