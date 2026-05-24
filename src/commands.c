#include "commands.h"
#include "parser.h"
#include "types.h"
#include "utils.h"
#include <fcntl.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <time.h>
#include <unistd.h>

int directory_exists(const char *path, struct stat *info) {

    if (stat(path, info) == -1)
        return 0;

    return S_ISDIR(info->st_mode);
}

int file_exists(const char *path, struct stat *info) {

    if (stat(path, info) == -1)
        return 0;

    return S_ISREG(info->st_mode);
}

int symlink_exists(const char *path, struct stat *info) {

    if (lstat(path, info) == -1)
        return 0;

    return S_ISLNK(info->st_mode);
}

int create_file(const char *path, int permissions) {

    int fd = open(path, O_CREAT | O_WRONLY,
                  0); // 0 because we are asked to use chmod

    if (fd == -1) {
        perror(path);
        return -1;
    }

    close(fd);
    if (enforce_permissions(path, permissions) == -1) // <-- uses chmod
        return -1;
    return 0;
}

int create_directory(const char *path, int permissions) {

    if (mkdir(path, permissions) == -1) {
        perror(path);
        return -1;
    }
    if (enforce_permissions(path, permissions) == -1)
        return -1;
    return 0;
}
void remove_directory_recursive_force(const char *path) {
    execlp("rm", "rm", "-rf", path, NULL);
    perror("execlp");
    _exit(0); // in case execlp somehow fails
}

int create_symlink(const char *path, const char *link) {

    if (symlink(path, link) == -1) {
        perror(path);
        return -1;
    }

    return 0;
}

int remove_symlink(const char *link) {

    if (unlink(link) == -1) {
        perror(link);
        return -1;
    }

    return 0;
}

int file_empty(struct stat *info) { return info->st_size == 0; }

int extract_permissions(struct stat *info) {
    return info->st_mode & 0x1FF; // this way we retrieve the last 9 bits
}

int enforce_permissions(const char *path, int permissions) {

    if (chmod(path, permissions) == -1) {
        perror("chmod");
        return -1;
    }

    return 0;
}

int check_obj_sanity(struct stat *info, int permissions) {
    return extract_permissions(info) == permissions;
}

int check_district_sanity(const char *district_id) {
    struct stat info;
    char path[MAX_PATH_LEN];
    build_dir_path(district_id, path);

    if (!directory_exists(path, &info)) {
        fprintf(stderr, "Cannot find district: [%s]!\n", path);
        return -1;
    } else if (!check_obj_sanity(&info, DISTRICT_PERMS)) {
        fprintf(stderr, "Wrong district permissions: [%s]!\n", path);
        return -1;
    }

    // verifying district.cfg :
    build_file_path(district_id, "district.cfg", path);

    if (!file_exists(path, &info)) {
        fprintf(stderr, "Cannot find file: [%s]!\n", path);
        return -1;
    } else if (!check_obj_sanity(&info, DISTRICT_CFG_PERMS)) {
        fprintf(stderr, "Wrong file permissions: [%s]!\n", path);
        return -1;
    }

    // verifying logged_district :
    build_file_path(district_id, "logged_district", path);

    if (!file_exists(path, &info)) {
        fprintf(stderr, "Cannot find file: [%s]!\n", path);
        return -1;
    } else if (!check_obj_sanity(&info, LOGGED_DISTRICT_PERMS)) {
        fprintf(stderr, "Wrong file permissions: [%s]!\n", path);
        return -1;
    }

    // verifying reports.dat :
    build_file_path(district_id, "reports.dat", path);

    if (!file_exists(path, &info)) {
        fprintf(stderr, "Cannot find file: [%s]!\n", path);
        // checking if a dangling symlink exists
        // snprintf(file_path, MAX_PATH_LEN, "active_reports-%s", path);
        build_symlink_path(district_id, path);
        if (symlink_exists(path, &info)) {
            remove_symlink(path);
            fprintf(stderr, "Warning, dangling symlink removed!\n");
        }
        return -1;
    } else if (!check_obj_sanity(&info, REPORTS_DAT_PERMS)) {
        fprintf(stderr, "Wrong file permissions: [%s]!\n", path);
        return -1;
    }

    build_symlink_path(district_id, path);
    if (!symlink_exists(path, &info)) {
        fprintf(stderr, "Cannot find file: [%s]!\n", path);
        return -1;
    }
    return 0;
}

int check_read_perm(int permissions, Role role) {
    // rwxrwxrwx
    switch (role) {
    case Manager:
        if ((permissions >> 8) & 0x1)
            return 1;
        break;
    case Inspector:
        if ((permissions >> 5) & 0x1)
            return 1;
        break;
    case Missing:
        return 0;
    }
    return 0;
}

int check_write_perm(int permissions, Role role) {
    // rwxrwxrwx
    switch (role) {
    case Manager:
        if ((permissions >> 7) & 0x1)
            return 1;
        break;
    case Inspector:
        if ((permissions >> 4) & 0x1)
            return 1;
        break;
    case Missing:
        return 0;
    }
    return 0;
}

void log_operation_message(Command *cmd, time_t timestamp,
                           const char *message) {

    char log_path[MAX_PATH_LEN];
    build_file_path(cmd->district_id, "logged_district", log_path);

    struct stat info;
    if (!file_exists(log_path, &info))
        return;

    char log_message[MAX_LOG_LEN];
    snprintf(log_message, MAX_LOG_LEN, "%ld\t%s\n", timestamp, message);

    int fd = open(log_path, O_APPEND | O_WRONLY);

    if (fd == -1) {
        perror(log_path);
        return;
    }

    if (write(fd, log_message, strlen(log_message)) == -1)
        perror("Error writing to logged_district");

    if (close(fd) == -1) {
        perror(log_path);
    }
}

void log_operation(Command *cmd, time_t timestamp) {

    char log_path[MAX_PATH_LEN];
    build_file_path(cmd->district_id, "logged_district", log_path);

    struct stat info;
    if (!file_exists(log_path, &info))
        return;

    char log_message[MAX_LOG_LEN];
    snprintf(log_message, MAX_LOG_LEN, "%ld\t%s\t%s\t%s\n", timestamp,
             cmd->user, role_to_str(cmd->role), op_to_str(cmd->operation));

    int fd = open(log_path, O_APPEND | O_WRONLY);

    if (fd == -1) {
        perror(log_path);
        return;
    }

    if (write(fd, log_message, strlen(log_message)) == -1)
        perror("Error writing to logged_district");

    if (close(fd) == -1) {
        perror(log_path);
    }
}

int create_district(Command *cmd) {
    char path[MAX_PATH_LEN];
    build_dir_path(cmd->district_id, path);
    if (create_directory(path, DISTRICT_PERMS) == -1)
        return -1;

    build_file_path(cmd->district_id, "reports.dat", path);

    if (create_file(path, REPORTS_DAT_PERMS) == -1)
        return -1;

    char link_path[MAX_PATH_LEN];
    build_symlink_path(cmd->district_id, link_path);

    if (create_symlink(path, link_path) == -1) {
        return -1;
    }

    build_file_path(cmd->district_id, "logged_district", path);

    if (create_file(path, LOGGED_DISTRICT_PERMS) == -1)
        return -1;

    build_file_path(cmd->district_id, "district.cfg", path);

    if (create_file(path, DISTRICT_CFG_PERMS) == -1)
        return -1;

    // setting the district.cfg escalation threshold
    int fd = open(path, O_WRONLY | O_TRUNC);
    if (fd == -1) {
        perror(path);
        return -1;
    }

    char min_escalation[MAX_ESCALATION_DIGITS_LEN];
    snprintf(min_escalation, MAX_ESCALATION_DIGITS_LEN, "%hhu",
             MIN_ESCALATION_LEVEL);

    if (write(fd, min_escalation, strlen(min_escalation)) == -1) {
        perror("Error writing to district.cfg");
        return -1;
    }

    if (close(fd) == -1) {
        perror(path);
        return -1;
    }
    return 0;
}

int input_report(Report *report, Command *cmd) {

    memset(report, 0x00, sizeof(Report)); // making sure we have a blank report

    // this is safe, cmd->user is guaranteed to be '\0' terminated and fit
    strcpy(report->inspector, cmd->user);

    report->timestamp = time(NULL); // getting the time
    float lng;
    float lat;
    // reading coordinates
    printf("X: ");
    scanf("%f", &lng);
    printf("Y: ");
    scanf("%f", &lat);

    report->coordinates.lng = lng;
    report->coordinates.lat = lat;

    // reading category
    printf("Category (road/lighting/flooding/other): ");
    scanf("%19s", report->issue_category);
    report->issue_category[MAX_CATEGORY_LEN - 1] = '\0';

    // reading severity
    printf("Severity level (1/2/3): ");
    scanf("%hhu", &report->severity_level);

    // checking severity is within bounds
    if (report->severity_level == 0 || report->severity_level > 3) {
        fprintf(stderr, "Cannot have severity_level: [%hhu]!\n",
                report->severity_level);
        return -1;
    }
    // reading description
    printf("Description: ");

    // skipping whitespaces
    int c;
    while ((c = getchar()) != '\n' && c != EOF)
        ;

    fgets(report->description, MAX_DESCRIPTION_LEN, stdin);
    size_t description_len = strlen(report->description);

    if (description_len > 0 && report->description[description_len - 1] == '\n')
        report->description[description_len - 1] = '\0';

    return 0;
}

int add(Command *cmd) {

    struct stat info;
    char path[MAX_PATH_LEN];
    // we temporarily use report_path
    build_dir_path(cmd->district_id, path);
    // verifying district folder integrity
    if (!directory_exists(path, &info)) {
        if (create_district(cmd) == -1)
            return -1;
    } else if (check_district_sanity(cmd->district_id) == -1) {
        // sanitize_district(cmd->district_id);
        return -1;
    }

    // verifying report file integirty
    build_file_path(cmd->district_id, "reports.dat", path);

    // guaranteed to exist because of check_district_sanity
    if (file_exists(path, &info) == 0) { // we retrieve info
        fprintf(stderr, "Cannot find: [%s]!\n", path);
    }
    if (!check_write_perm(extract_permissions(&info), cmd->role)) {
        fprintf(stderr, "Cannot write to reports.dat as: [%s]!\n",
                role_to_str(cmd->role));
        return -1;
    }

    Report new_report;
    if (input_report(&new_report, cmd) == -1)
        return -1;

    new_report.report_id = INITIAL_REPORT_ID;

    int fd = open(path, O_APPEND | O_RDWR);

    if (fd == -1) {
        perror(path);
        return -1;
    }

    if (!file_empty(&info)) { // we have to read what the last report_id was
        lseek(fd, -(off_t)sizeof(Report) + (off_t)sizeof(time_t), SEEK_END);
        if (read(fd, &new_report.report_id, sizeof(uint32_t)) == -1) {
            perror("read");
            close(fd);
            return -1;
        }
        new_report.report_id += 1;
    }

    if (write(fd, &new_report, sizeof(Report)) == -1) {
        perror("write");
        close(fd);
        return -1;
    }
    close(fd);
    log_operation(cmd, new_report.timestamp);

    // retrieving monitor pid
    char monitor_pid_path[MAX_PATH_LEN] = "districts/.monitor_pid";
    fd = open(monitor_pid_path, O_RDONLY);
    char monitor_pid_text[MAX_PID_DIGITS_LEN] = "";
    pid_t monitor_pid;

    if (fd == -1) {
        fprintf(stderr, "Could not find pid file: [%s]!\n", monitor_pid_path);
        log_operation_message(cmd, new_report.timestamp,
                              "failed to inform monitor");
        return 0;
    }

    if (read(fd, &monitor_pid_text, sizeof(monitor_pid_text)) == -1) {
        fprintf(stderr, "Could not read pid from: [%s]!\n", monitor_pid_path);
        log_operation_message(cmd, new_report.timestamp,
                              "failed to inform monitor");
        close(fd);
        return 0;
    }
    // trying to transform text to pid

    monitor_pid = atoi(monitor_pid_text);
    if (monitor_pid == 0) {
        fprintf(stderr, "Could not read pid from: [%s]!\n", monitor_pid_path);
        log_operation_message(cmd, new_report.timestamp,
                              "failed to inform monitor");
        close(fd);
        return 0;
    }
    // sending the signal
    if (kill(monitor_pid, SIGUSR1) == -1) {
        fprintf(stderr, "Failed to send signal to pid: [%d]!\n", monitor_pid);
        log_operation_message(cmd, new_report.timestamp,
                              "failed to inform monitor");
        close(fd);
        return 0;
    }

    close(fd);
    log_operation_message(cmd, new_report.timestamp,
                          "informed monitor of new report");
    return 0;
}

int list(Command *cmd) {
    struct stat info;

    if (check_district_sanity(cmd->district_id) == -1) {
        return -1;
    }

    char report_path[MAX_PATH_LEN];
    build_file_path(cmd->district_id, "reports.dat", report_path);

    // guaranteed to extract reports.dat info because of check_district_sanity
    if (file_exists(report_path, &info) == 0) { // we retrieve info
        fprintf(stderr, "Cannot find: [%s]!\n", report_path);
    }
    if (!check_read_perm(extract_permissions(&info), cmd->role)) {
        fprintf(stderr, "Cannot read reports.dat as: [%s]!\n",
                role_to_str(cmd->role));
        return -1;
    }

    printf("File Permissions: ");
    bits_to_symbol(extract_permissions(&info));
    printf(" | Last modification time: %ld | File size: %ld\n", info.st_mtime,
           info.st_size);

    int fd = open(report_path, O_RDONLY);
    if (fd == -1) {
        perror(report_path);
        return -1;
    }
    size_t reports_count = info.st_size / sizeof(Report);
    Report report;
    for (size_t i = 0; i < reports_count; i++) {
        if (read(fd, &report, sizeof(Report)) == -1) {
            perror("read");
            close(fd);
            return -1;
        }
        report_dump(&report);
    }
    if (close(fd) == -1) {
        perror(report_path);
        return -1;
    }
    log_operation(cmd, time(NULL));
    return 0;
}

int view(Command *cmd) {
    struct stat info;

    if (check_district_sanity(cmd->district_id) == -1) {
        return -1;
    }

    char report_path[MAX_PATH_LEN];
    build_file_path(cmd->district_id, "reports.dat", report_path);

    // guaranteed to extract reports.dat info because of check_district_sanity

    if (file_exists(report_path, &info) == 0) {
        fprintf(stderr, "Cannot find: [%s]!\n", report_path);
    }
    if (!check_read_perm(extract_permissions(&info), cmd->role)) {
        fprintf(stderr, "Cannot read reports.dat as: [%s]!\n",
                role_to_str(cmd->role));
        return -1;
    }

    int fd = open(report_path, O_RDONLY);
    if (fd == -1) {
        perror(report_path);
        return -1;
    }
    size_t reports_count = info.st_size / sizeof(Report);
    Report report;
    for (size_t i = 0; i < reports_count; i++) {
        if (read(fd, &report, sizeof(Report)) == -1) {
            perror("read");
            close(fd);
            return -1;
        }
        if (report.report_id == cmd->extra.report_id) {
            close(fd);
            log_operation(cmd, time(NULL));
            report_dump(&report);
            return 0;
        }
    }
    if (close(fd) == -1) {
        perror(report_path);
        return -1;
    }
    log_operation(cmd, time(NULL));
    printf("Couldn't find report with report_id: [%d]!\n",
           cmd->extra.report_id);
    return 0;
}

int update_threshold(Command *cmd) {
    if (check_district_sanity(cmd->district_id) == -1) {
        return -1;
    }

    char path[MAX_PATH_LEN];
    build_file_path(cmd->district_id, "district.cfg", path);

    struct stat info;
    if (stat(path, &info) == -1) {
        perror("stat");
        return -1;
    }

    if (!check_write_perm(extract_permissions(&info), cmd->role)) {
        fprintf(stderr, "Cannot write to district.cfg as: [%s]!\n",
                role_to_str(cmd->role));
        return -1;
    }

    int fd = open(path, O_WRONLY | O_TRUNC);
    if (fd == -1) {
        perror(path);
        return -1;
    }

    char new_threshold[MAX_ESCALATION_DIGITS_LEN];
    snprintf(new_threshold, MAX_ESCALATION_DIGITS_LEN, "%hhu",
             cmd->extra.new_threshold);

    if (write(fd, new_threshold, strlen(new_threshold)) == -1) {
        perror("Error writing to district.cfg");
        close(fd);
        return -1;
    }

    if (close(fd) == -1) {
        perror(path);
        return -1;
    }
    log_operation(cmd, time(NULL));
    return 0;
}

int remove_report(Command *cmd) {
    struct stat info;

    if (check_district_sanity(cmd->district_id) == -1) {
        return -1;
    }
    if (cmd->role != Manager) {
        fprintf(stderr, "Cannot remove report as: [%s]!\n",
                role_to_str(cmd->role));
        return -1;
    }

    char report_path[MAX_PATH_LEN];
    build_file_path(cmd->district_id, "reports.dat", report_path);

    // guaranteed to extract reports.dat info because of check_district_sanity
    if (file_exists(report_path, &info) == 0) {
        fprintf(stderr, "Cannot find: [%s]!\n", report_path);
    }

    if (!check_write_perm(extract_permissions(&info), cmd->role)) {
        fprintf(stderr, "Cannot write reports.dat as: [%s]!\n",
                role_to_str(cmd->role));
        return -1;
    }

    int fd = open(report_path, O_RDWR);
    if (fd == -1) {
        perror(report_path);
        return -1;
    }
    lseek(fd, 0, SEEK_SET);
    size_t reports_count = info.st_size / sizeof(Report);
    Report report;
    for (size_t i = 0; i < reports_count; i++) {
        if (read(fd, &report, sizeof(Report)) == -1) {
            perror("read");
            close(fd);
            return -1;
        }
        if (report.report_id == cmd->extra.report_id) {

            for (size_t k = i + 1; k < reports_count; k++) {

                lseek(fd, k * sizeof(Report), SEEK_SET);
                if (read(fd, &report, sizeof(Report)) == -1) {
                    perror("read");
                    close(fd);
                    return -1;
                }

                lseek(fd, (k - 1) * sizeof(Report), SEEK_SET);
                if (write(fd, &report, sizeof(Report)) == -1) {
                    perror("write");
                    close(fd);
                    return -1;
                }
            }

            if (ftruncate(fd, (reports_count - 1) * sizeof(Report)) == -1) {
                perror("ftruncate");
                close(fd);
                return -1;
            }
            close(fd);
            log_operation(cmd, time(NULL));
            return 0;
        }
    }
    if (close(fd) == -1) {
        perror(report_path);
        return -1;
    }
    log_operation(cmd, time(NULL));
    printf("Couldn't find report with report_id: [%d]!\n",
           cmd->extra.report_id);
    return 0;
}
int remove_district(Command *cmd) {
    struct stat info;

    char path[MAX_PATH_LEN];
    build_dir_path(cmd->district_id, path);
    if (!directory_exists(path, &info)) {
        fprintf(stderr, "Warning: district [%s] doesn't exist!\n",
                cmd->district_id);
        return 0;
    }
    // two ways, either check that it has write permission on the directory
    if (!check_write_perm(extract_permissions(&info), cmd->role)) {
        fprintf(stderr, "Cannot remove district as: [%s]!\n",
                role_to_str(cmd->role));
        return -1;
    }
    // or in our case we can do a cheap trick and do a match on the role
    // enum
    /*
    if (cmd->role != Manager) {
        fprintf(stderr, "Cannot remove district as: [%s]!\n",
                role_to_str(cmd->role));
        return -1;
    } */

    pid_t process = fork();
    if (process == 0) // we are inside the child
        remove_directory_recursive_force(path);
    else                           // inside the parrent
        waitpid(process, NULL, 0); // here 0 means wait for child process

    build_symlink_path(cmd->district_id, path);
    if (symlink_exists(path, &info))
        if (remove_symlink(path) == -1)
            return -1;

    return 0;
}

int filter(Command *cmd) {
    struct stat info;

    if (check_district_sanity(cmd->district_id) == -1) {
        return -1;
    }

    char report_path[MAX_PATH_LEN];
    build_file_path(cmd->district_id, "reports.dat", report_path);

    if (file_exists(report_path, &info) == 0) {
        fprintf(stderr, "Cannot find: [%s]!\n", report_path);
    }

    if (!check_read_perm(extract_permissions(&info), cmd->role)) {
        fprintf(stderr, "Cannot read reports.dat as: [%s]!\n",
                role_to_str(cmd->role));
        return -1;
    }

    int fd = open(report_path, O_RDONLY);
    if (fd == -1) {
        perror(report_path);
        return -1;
    }
    size_t reports_count = info.st_size / sizeof(Report);
    Report report;
    for (size_t i = 0; i < reports_count; i++) {
        if (read(fd, &report, sizeof(Report)) == -1) {
            perror("read");
            close(fd);
            return -1;
        }

        int match = 1;
        for (size_t k = 0; cmd->extra.filter_conditions[k] != NULL; k++) {
            char field[MAX_FILTER_FIELD_LEN], op[MAX_FILTER_OP_LEN],
                value[MAX_FILTER_VALUE_LEN];
            parse_condition(cmd->extra.filter_conditions[k], field, op, value);

            if (match_condition(&report, field, op, value) == 0) {
                match = 0;
                break;
            }
        }
        if (match)
            report_dump(&report);
    }
    if (close(fd) == -1) {
        perror(report_path);
        return -1;
    }
    log_operation(cmd, time(NULL));
    return 0;
}
