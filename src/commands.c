#include "commands.h"
#include "utils.h"
#include "types.h"
#include "parser.h"
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <time.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>

#define DIR_FLAGS 0750
#define LOGGED_DISTRICT_FLAGS 0644
#define REPORTS_DAT_FLAGS 0664
#define DISTRICT_CFG_FLAGS 0640
#define MAX_PATH_LEN MAX_DISTRICT_ID_LEN + 20
#define MAX_LOG_LEN 300
#define INITIAL_REPORT_ID 0

void log_op(Command *cmd, time_t timestamp) {
    char path[MAX_PATH_LEN];
    snprintf(path, MAX_PATH_LEN, "%s/logged_district", cmd->district_id);
    int fd = open(path, O_CREAT | O_APPEND | O_WRONLY, LOGGED_DISTRICT_FLAGS);
    if(fd == -1) {
        fprintf(stderr, "Couldn't open: %s\n", path);
        fprintf(stderr, "Failed to log!\n");
        return;
    }
    char log[MAX_LOG_LEN];
    memset(log, 0x00, sizeof(log));
    snprintf(log, MAX_LOG_LEN, "%ld\t%s\t%s\t%s\n", 
            timestamp, cmd->user, role_to_str(cmd->role),
            op_to_str(cmd->operation));
    if(write(fd, log, sizeof(log)) != sizeof(log)) {
        fprintf(stderr, "Didn't write log!\n");
    }
     
}

int input_report(Report *report,Command *cmd) {
    memset(report, 0x00, sizeof(Report)); // making sure we have a blank report
    
    // this is safe, cmd->user is guaranteed to be '\0' terminated and fit
    strcpy(report->inspector, cmd->user);

    report->timestamp = time(NULL); // getting the time
    
    // reading coordinates
    printf("X: "); scanf("%f", &report->coordinates.lng);
    printf("Y: "); scanf("%f", &report->coordinates.lat);

    // reading category
    printf("Category (road/lighting/flooding/other): ");
    scanf("%19s", report->issue_category);
    report->issue_category[MAX_CATEGORY_LEN - 1] = '\0';

    // reading severity 
    printf("Severity level (1/2/3): ");
    scanf("%hhu", &report->severity_level);
    
    // checking severity is within bounds
    if(report->severity_level == 0 || report->severity_level > 3) {
        fprintf(stderr, "Invalid severity level!\n");
        return -1;
    }
    // reading description
    printf("Description: ");
    getchar();
    fgets(report->description, MAX_DESCRIPTION_LEN, stdin);
    report->description[strlen(report->description) - 1] = '\0';

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
        if(read(fd, &new_report.report_id, sizeof(new_report.report_id)) == -1) {
            fprintf(stderr, "Couldn't read last record_id!\n");
            return -1;
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
    printf("\nLast modification: %ld | size: %ld\n", info.st_mtime, info.st_size);
    printf("Reports:\n");
    if (is_empty_file(path)){
        printf("No reports!");
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
        printf("==============================================\n");
        report_dump(&report);
    }
    return 0;
}




int exists_district(const char *path) {
    struct stat info;
    if(stat(path, &info) == -1) {
        if (errno == ENOENT) {
            return 0;
        }
    }
    return 1;
}

int extract_permissions(const char *path) {
    struct stat info;
    if (stat(path, &info) != 0 ) {
        fprintf(stderr, "Couldn't read [%s] permissions!\n", path);
    }

    return 0x1FF & info.st_mode;
}
int is_empty_file(const char *path) {
    struct stat info;
    
    if (stat(path, &info) != 0 ) {
        fprintf(stderr, "Couldn't read [%s] permissions!\n", path);
    }

    return info.st_size == 0;
}


int create_district(Command *cmd) {
    
    if(strlen(cmd->district_id) == 0) {
        fprintf(stderr, "Empty district received!\n"); 
        return -1; 
    }

    int check = mkdir(cmd->district_id, 0); // we could also do mkdir(..., DIR_FLAGS);
    
    if (check == -1) {
        fprintf(stderr, "District [%s] directory already exists!\n", cmd->district_id);
        return -1;
    }

    if (chmod(cmd->district_id, DIR_FLAGS) != 0) {
        fprintf(stderr, "Couldn't change directory permissions!\n");
        return -1;
    }
    int fd;
    char path[MAX_PATH_LEN] = "";
    strcat(path, cmd->district_id);
    strcat(path, "/");
    size_t cat_point = strlen(path);
    
    // creating reports.dat
    strcpy(path + cat_point, "reports.dat");
    fd = open(path, O_CREAT | O_RDONLY, 0);
    
    if(fd == -1) {
        fprintf(stderr, "Couldn't create file: %s\n", path);
        return -1;
    }

    if(chmod(path, REPORTS_DAT_FLAGS) != 0) {
        fprintf(stderr, "Couldn't change permissions on: %s\n", path);
        return -1;
    }
    close(fd);
    
    // creating district.cfg
    strcpy(path + cat_point, "district.cfg");
    fd = open(path, O_CREAT | O_RDONLY, 0);
    
    if(fd == -1) {
        fprintf(stderr, "Couldn't create file: %s\n", path);
        return -1;
    }

    if(chmod(path, DISTRICT_CFG_FLAGS) != 0 ) {
        fprintf(stderr, "Couldn't change permissions on: %s\n", path);
        return -1;
    }
    close(fd);

    // creating logged_district
    strcpy(path + cat_point, "logged_district");
    fd = open(path, O_CREAT | O_RDONLY, 0);

    if(fd == -1) {
        fprintf(stderr, "Couldn't create file: %s\n", path);
        return -1;
    }

    if(chmod(path, LOGGED_DISTRICT_FLAGS) != 0) {
        fprintf(stderr, "Couldn't change permissions on: %s\n", path);
        return -1;
    }

    close(fd);
    
    


    return 0;
}

