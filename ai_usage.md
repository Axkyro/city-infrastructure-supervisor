Used prompt:
This is my commands.h:
#ifndef AXKYRO_COMMANDS_H
#define AXKYRO_COMMANDS_H

#include "types.h"
#include <sys/stat.h>

int directory_exists(const char *path, struct stat *info);
int file_exists(const char *path, struct stat *info);
int symlink_exists(const char *path, struct stat *info);

int create_file(const char *path, int permissions);
int create_directory(const char *path, int permissions);

int create_symlink(const char *path, const char *link);
int remove_symlink(const char *link);

int file_empty(struct stat *info);

int extract_permissions(struct stat *info);
int enforce_permissions(const char *path, int permissions);

int check_obj_sanity(struct stat *info, int permissions);
int check_district_sanity(const char *path);

// int sanitize_district(const char *path); optional

int check_read_perm(int permissions, Role role);
int check_write_perm(int permissions, Role role);

void log_operation(Command *cmd, time_t timestamp);

int create_district(Command *cmd);

int input_report(Report *report, Command *cmd);


int add(Command *cmd);
int list(Command *cmd);
int view(Command *cmd);
int remove_report(Command *cmd);
int update_threshold(Command *cmd);
int filter(Command *cmd);

#endif

These are my types.h:
#ifndef AXKYRO_TYPES_H
#define AXKYRO_TYPES_H

#define MAX_USERNAME_LEN        50
#define MAX_DISTRICT_ID_LEN     20
#define MAX_FILTER_LEN          30
#define MAX_CATEGORY_LEN        20
#define MAX_DESCRIPTION_LEN     200
#define MAX_PATH_LEN            70
#define MAX_LOG_LEN             120

#define DISTRICT_PERMS          0750
#define LOGGED_DISTRICT_PERMS   0644
#define REPORTS_DAT_PERMS       0664
#define DISTRICT_CFG_PERMS      0640

#define MIN_ESCALATION_LEVEL 2
#define MAX_ESCALATION_DIGITS_LEN 5 // must always be >= 2

#define INITIAL_REPORT_ID 0

#include <stdint.h>
#include <time.h>

typedef enum {
    Manager,
    Inspector,
    Missing 
} Role;

typedef enum {
    Add,
    List,
    View,
    RemoveReport,
    UpdateThreshold,
    Filter,
    Invalid,
} Operation;

typedef struct {
    char user[MAX_USERNAME_LEN];
    Role role;
    Operation operation;
    
    char district_id[MAX_DISTRICT_ID_LEN];

    union { // used for additional possible fields  
        uint32_t report_id;
        uint8_t new_threshold;
        char **filter_conditions;
    } extra;
} Command;

typedef struct {
    time_t timestamp; 
    uint32_t report_id;
    struct {
        float lat;
        float lng;
    } coordinates;
    char inspector[MAX_USERNAME_LEN];
    char issue_category[MAX_CATEGORY_LEN];
    char description[MAX_DESCRIPTION_LEN];
    uint8_t severity_level;
} __attribute__((packed)) Report;

#endif
This is my manually written parser function:
int parse_arguments(int argc, char **argv, Command *cmd) {
    if (!cmd) return -1; // received NULL cmd
    cmd->operation = Invalid;
    cmd->role = Missing;
    cmd->district_id[0] = '\0';
    size_t i = 1;  // arg counter
    if(argc == 1) { fprintf(stderr, "No flags!\n"); return -1; }
    while ( i < argc ) {
        // checking for a given user
        if (strcmp("--user", argv[i]) == 0) {
            if ( i + 1 == argc) {
                fprintf(stderr, "--user [arg] <-- missing!\n");
                return -1;
            }
            strncpy(cmd->user, argv[i+1], MAX_USERNAME_LEN - 1);
            cmd->user[MAX_USERNAME_LEN - 1] = '\0';
            i += 2;
        } else if (strcmp("--role", argv[i]) == 0) {
            if (i + 1 == argc) {
                fprintf(stderr, "--role [arg] <-- missing!\n");
                return -1;
            }
            cmd->role = str_to_role(argv[i+1]);
            if(cmd->role == Missing) {
                fprintf(stderr, "Unknown role: %s\n", argv[i+1]);
                fprintf(stderr, "Try --role manager\n");
                return -1;
            }
            i += 2;
        } else if ( strlen(argv[i]) > 2 && argv[i][0] == '-' && argv[i][1] == '-') {
            cmd->operation = str_to_op(argv[i] + 2);
            switch(cmd->operation) {
                case Invalid:
                    fprintf(stderr, "Unknown operation: [%s]\n", argv[i]+2);
                    return -1;
                case Add:
                    if (i+1 == argc) { 
                        fprintf(stderr, "--add [arg] <-- missing!\n");
                        return -1;
                    }
                    strncpy(cmd->district_id, argv[i+1], MAX_DISTRICT_ID_LEN - 1);
                    cmd->district_id[MAX_DISTRICT_ID_LEN - 1] = '\0';
                    i+=2;
                    break;
                case List:
                    if (i+1 == argc) { 
                        fprintf(stderr, "--list [arg] <-- missing!\n");
                        return -1;
                    }
                    strncpy(cmd->district_id, argv[i+1], MAX_DISTRICT_ID_LEN - 1);
                    cmd->district_id[MAX_DISTRICT_ID_LEN - 1] = '\0';
                    i+=2;
                    break;
                case View:
                    if (i+2 >= argc) {
                        fprintf(stderr, "--view [arg1] [arg2] <--missing!\n");
                        return -1;
                    }
                    strncpy(cmd->district_id, argv[i+1], MAX_DISTRICT_ID_LEN - 1);
                    cmd->district_id[MAX_DISTRICT_ID_LEN - 1] = '\0';
                    cmd->extra.report_id = strtol(argv[i+2], NULL, 10);
                    // not completely safe!
                    if (cmd->extra.report_id == 0 && argv[i+2][0] != '0'){
                        fprintf(stderr, "Please use a number for report_id!\n");
                        return -1;
                    }
                    i+=3;
                    break;
                case RemoveReport:
                    if (i+2 >= argc) {
                        fprintf(stderr, "--remove_report [arg1] [arg2] <--missing!\n");
                        return -1;
                    }
                    strncpy(cmd->district_id, argv[i+1], MAX_DISTRICT_ID_LEN - 1);
                    cmd->district_id[MAX_DISTRICT_ID_LEN - 1] = '\0';
                    cmd->extra.report_id = strtol(argv[i+2], NULL, 10);
                    if (cmd->extra.report_id == 0 && argv[i+2][0] != '0'){
                        fprintf(stderr, "Please use a number for report_id!\n");
                        return -1;
                    }
                    i+=3;
                    break;
                case UpdateThreshold:
                    if (i+2 >= argc) {
                        fprintf(stderr, "--change_threshold [arg1] [arg2] <--missing!\n");
                        return -1;
                    }
                    strncpy(cmd->district_id, argv[i+1], MAX_DISTRICT_ID_LEN - 1);
                    cmd->district_id[MAX_DISTRICT_ID_LEN - 1] = '\0';
                    cmd->extra.new_threshold = (uint8_t)strtol(argv[i+2], NULL, 10);
                    if (cmd->extra.new_threshold == 0 && argv[i+2][0] != '0') {
                        fprintf(stderr, "Please use a number for report_id!\n");
                        return -1;
                    }
                    i+=3;
                    break;
                case Filter:/*
                    if (i+2 >= argc) {
                        fprintf(stderr, "--filter [arg1] [arg2] <--missing!\n");
                        return -1;
                    }
                    strncpy(cmd->district_id, argv[i+1], MAX_DISTRICT_ID_LEN - 1);
                    cmd->district_id[MAX_DISTRICT_ID_LEN - 1] = '\0';
                    strncpy(cmd->extra.filter_condition, argv[i+2], MAX_FILTER_LEN - 1);
                    cmd->extra.filter_condition[MAX_FILTER_LEN - 1] = '\0'; */
                    i+=3;
                    break;
            }
        } else {
            fprintf(stderr, "Unkown flag: [%s]\n", argv[i]);
            return -1;
        } 
    } 
    return 0;
}

Help me write the 

The filter Command and AI-Assisted Condition Matching
The filter command accepts one (or optionally more) condition(s). If more conditions are supported, they are given as distinct arguments separated by spaces. The command prints all reports that satisfy all of them (conditions are implicitly joined by AND). A condition is a single string of the form:
field:operator:value
Supported fields: severity, category, inspector, timestamp. Supported operators: ==, !=, <, <=, >, >=.
What you are expected to do with AI assistance:
Use an AI assistant to help you implement two functions:
Describe your record structure and ask the AI to generate a function
        int parse_condition(const char *input, char *field, char *op, char *value);
      which splits a field:operator:value string into its three parts.
Describe the fields and their types and ask the AI to generate a function
        int match_condition(Report *r, const char *field, const char *op, const char *value);
      which returns 1 if the record satisfies the condition and 0 otherwise.
Review both functions line by line. You must be able to explain them fully at your presentation.
Write the filter logic yourself: open reports.dat, read records one by one with read(), parse each command-line condition with parse_condition(), test each record against every condition with match_condition(), and print records where all conditions return 1.
Describe in an ai_usage.md file the tool used, the prompts you gave, what was generated, what you changed and why, and what you learned. This file is a required deliverable and will be discussed at your presentation.

you can use the extra.conditions field in command to store multiple conditions, and i can later free them in main
