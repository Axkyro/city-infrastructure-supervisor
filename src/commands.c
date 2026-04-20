#include "commands.h"
#include "parser.h"
#include "types.h"
#include "utils.h"
#include <fcntl.h>
#include <stdio.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
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
    enforce_permissions(path, permissions); // <-- uses chmod

    return 0;
}

int create_directory(const char *path, int permissions) {

    if (mkdir(path, permissions) == -1) {
        perror(path);
        return -1;
    }

    return 0;
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

int check_district_sanity(const char *path) {
    struct stat info;
    if (!directory_exists(path, &info)) {
        fprintf(stderr, "Cannot find district: [%s]!\n", path);
        return -1;
    } else if (!check_obj_sanity(&info, DISTRICT_PERMS)) {
        fprintf(stderr, "Wrong district permissions: [%s]!\n", path);
        return -1;
    }

    char file_path[MAX_PATH_LEN];

    // verifying district.cfg :
    build_path(path, "district.cfg", file_path);

    if (!file_exists(file_path, &info)) {
        fprintf(stderr, "Cannot find file: [%s]!\n", file_path);
        return -1;
    } else if (!check_obj_sanity(&info, DISTRICT_CFG_PERMS)) {
        fprintf(stderr, "Wrong file permissions: [%s]!\n", file_path);
        return -1;
    }

    // verifying logged_district :
    build_path(path, "logged_district", file_path);

    if (!file_exists(file_path, &info)) {
        fprintf(stderr, "Cannot find file: [%s]!\n", file_path);
        return -1;
    } else if (!check_obj_sanity(&info, LOGGED_DISTRICT_PERMS)) {
        fprintf(stderr, "Wrong file permissions: [%s]!\n", file_path);
        return -1;
    }

    // verifying reports.dat :
    build_path(path, "reports.dat", file_path);

    if (!file_exists(file_path, &info)) {
        fprintf(stderr, "Cannot find file: [%s]!\n", file_path);
        return -1;
    } else if (!check_obj_sanity(&info, REPORTS_DAT_PERMS)) {
        fprintf(stderr, "Wrong file permissions: [%s]!\n", file_path);
        return -1;
    }

    snprintf(file_path, MAX_PATH_LEN, "active_reports-%s", path);
    if (!symlink_exists(file_path, &info)) {
        fprintf(stderr, "Cannot find file: [%s]!\n", file_path);
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

//
void log_operation(Command *cmd, time_t timestamp) {

    char log_path[MAX_PATH_LEN];
    build_path(cmd->district_id, "logged_district", log_path);

    struct stat info;
    if (!file_exists(log_path, &info))
        return;

    if (!check_write_perm(cmd->role, extract_permissions(&info))) {
        fprintf(stderr, "Cannot write to log as: [%s]!\n",
                role_to_str(cmd->role));
        return;
    }

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

    if (create_directory(cmd->district_id, DISTRICT_PERMS) == -1)
        return -1;
    char path[MAX_PATH_LEN];
    build_path(cmd->district_id, "reports.dat", path);

    if (create_file(path, REPORTS_DAT_PERMS) == -1)
        return -1;

    char link_path[MAX_PATH_LEN];
    snprintf(link_path, MAX_PATH_LEN, "active_reports-%s", cmd->district_id);

    if (create_symlink(path, link_path) == -1) {
        return -1;
    }

    build_path(cmd->district_id, "logged_district", path);

    if (create_file(path, LOGGED_DISTRICT_PERMS) == -1)
        return -1;

    build_path(cmd->district_id, "district.cfg", path);

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

    // reading coordinates
    printf("X: ");
    scanf("%f", &report->coordinates.lng);
    printf("Y: ");
    scanf("%f", &report->coordinates.lat);

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

    // verifying district folder integrity

    if (!directory_exists(cmd->district_id, &info)) {
        if (create_district(cmd) == -1)
            return -1;
    } else if (check_district_sanity(cmd->district_id) == -1) {
        // sanitize_district(cmd->district_id);
        fprintf(stderr, "Irregular file permissions on district found: [%s]!\n",
                cmd->district_id);

        return -1;
    }

    // verifying report file integirty
    char report_path[MAX_PATH_LEN];
    build_path(cmd->district_id, "reports.dat", report_path);

    // guaranteed to exist because of check_district_sanity
    file_exists(report_path, &info); // we retrieve info

    if (!check_write_perm(extract_permissions(&info), cmd->role)) {
        printf("Cannot write to reports.dat as: [%s]!\n",
               role_to_str(cmd->role));
        return -1;
    }

    Report new_report;
    input_report(&new_report, cmd);

    new_report.report_id = INITIAL_REPORT_ID;

    int fd = open(report_path, O_APPEND | O_RDWR);

    if (fd == -1) {
        perror(report_path);
        return -1;
    }

    if (!file_empty(&info)) { // we have to read what the last report_id was
        lseek(fd, -sizeof(Report) + sizeof(time_t), SEEK_END);
        if (read(fd, &new_report.report_id, sizeof(uint32_t)) == -1) {
            perror("read");
            return -1;
        }
        new_report.report_id += 1;
    }

    if (write(fd, &new_report, sizeof(Report)) == -1) {
        perror("write");
        return -1;
    }
    close(fd);
    log_operation(cmd, new_report.timestamp);
    return 0;
}

/*
int update_threshold(Command *cmd) {
    char path[MAX_PATH_LEN];
    snprintf(path, MAX_PATH_LEN, "%s/district.cfg", cmd->district_id);
    log_op(cmd, time(NULL));
    if(extract_permissions(path) != DISTRICT_CFG_FLAGS) {
        fprintf(stderr, "Modified [%s] permissions, refusing to continue!\n",
path); return -1;
    }
    if(cmd->role != Manager) {
        printf("You cannot change threshold as %s\n", role_to_str(cmd->role));
        return 0;
    }
    int fd = open(path, O_WRONLY);
    if(fd == -1) {
        fprintf(stderr, "Couldn't open: [%s]\n", path);
        return 0;
    }
    char to_write[MAX_SEVERITY_DIGITS_LEN];
    snprintf(to_write, MAX_SEVERITY_DIGITS_LEN, "%hhu\n",
cmd->extra.new_threshold); if(write(fd, to_write, MAX_SEVERITY_DIGITS_LEN) !=
MAX_SEVERITY_DIGITS_LEN){ fprintf(stderr, "Couldn't set new_threshold!\n");
        return -1;
    }
    return 0;

}

int add(Command *cmd) {
    if(!exists_district(cmd->district_id))
        if(create_district(cmd) != 0)
            return -1;

    Report new_report;
    new_report.report_id = INITIAL_REPORT_ID;
    if(input_report(&new_report, cmd) != 0) {
        return -1;
    }

    char reports_path[MAX_PATH_LEN];
    snprintf(reports_path, MAX_PATH_LEN, "%s/reports.dat", cmd->district_id);

    int fd = open(reports_path, O_APPEND | O_RDWR | O_CREAT, REPORTS_DAT_FLAGS);

    if(fd == -1) {
        fprintf(stderr, "Error on: %s\n", reports_path);
        return -1;
    }
    if(!is_empty_file(reports_path)) {
        // reading the last report value
        lseek(fd,-REPORT_SIZE + sizeof(new_report.timestamp), SEEK_END);
        if(read(fd, &new_report.report_id, sizeof(new_report.report_id)) == -1)
{ fprintf(stderr, "Couldn't read last record_id!\n"); return -1;
        }
        new_report.report_id += 1;
    }

    if(write(fd, &new_report, (size_t)sizeof(new_report))!= sizeof(Report)) {
        fprintf(stderr, "Couldn't write report!\n");
        report_dump(&new_report);
    }
    close(fd);
    log_op(cmd, new_report.timestamp);
    return 0;
}
int view(Command *cmd) {
if(!exists_district(cmd->district_id)) {
        fprintf(stderr, "District: [%s] doesn't exist!\n", cmd->district_id);
        return -1;
    }

    log_op(cmd, time(NULL));
    char path[MAX_PATH_LEN];
    snprintf(path, MAX_PATH_LEN, "%s/reports.dat", cmd->district_id);
    int fd = open(path, O_RDONLY, REPORTS_DAT_FLAGS);
    if(fd==-1) {
        fprintf(stderr, "Couldn't open: %s\n", path);
        return -1;
    }

    struct stat info;
    if(stat(path, &info) == -1) {
        fprintf(stderr, "Failed stat on %s\n", path);
        return -1;
    }

    if (is_empty_file(path)){
        printf("No reports!\n");
        return 0;
    }
    Report report;
    for(int i = 0; i < info.st_size / REPORT_SIZE; i++)
    {
        if(read(fd, &report, REPORT_SIZE ) != REPORT_SIZE) {
            fprintf(stderr, "Failed reading! Corrupted reports.dat in [%s]!\n",
                    path);
            return -1;
        }
        if(report.report_id == cmd->extra.report_id) {
            printf("Found Report!\n");
            report_dump(&report);
            return 0;
        }

    }
    printf("Failed to find report id: [%d]\n", cmd->extra.report_id);
    return -1;

}
int list(Command *cmd) {
    if(!exists_district(cmd->district_id)) {
        fprintf(stderr, "District: [%s] doesn't exist!\n", cmd->district_id);
        return -1;
    }

    log_op(cmd, time(NULL));
    char path[MAX_PATH_LEN];
    snprintf(path, MAX_PATH_LEN, "%s/reports.dat", cmd->district_id);
    int fd = open(path, O_RDONLY, REPORTS_DAT_FLAGS);
    if(fd==-1) {
        fprintf(stderr, "Couldn't open: %s\n", path);
        return -1;
    }

    struct stat info;
    if(stat(path, &info) == -1) {
        fprintf(stderr, "Failed stat on %s\n", path);
        return -1;
    }
    int permissions = extract_permissions(path);
    printf("File: %s | ", path); bits_to_symbol(permissions);
    printf("\nLast modification: %ld | size: %ld\n", info.st_mtime,
info.st_size); printf("Reports:\n"); if (is_empty_file(path)){ printf("No
reports!"); return 0;
    }
    Report report;
    for(int i = 0; i < info.st_size / REPORT_SIZE; i++)
    {
        if(read(fd, &report, REPORT_SIZE ) != REPORT_SIZE) {
            fprintf(stderr, "Failed reading! Corrupted reports.dat in [%s]!\n",
                    path);
            return -1;
        }
        printf("==============================================\n");
        report_dump(&report);
    }
    return 0;
}
*/
