#include "parser.h"
#include "types.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

Operation str_to_op(const char *str) {
    if(!str)
        return Invalid;
    if (strcmp("add", str) == 0)
        return Add;
    else if (strcmp("list", str) == 0)
        return List;
    else if (strcmp("view", str) == 0)
        return View;
    else if (strcmp("remove_report", str) == 0)
        return RemoveReport;
    else if (strcmp("update_threshold", str) == 0)
        return UpdateThreshold;
    else if (strcmp("filter", str) == 0)
        return Filter;
    else 
        return Invalid;


}
const char *op_to_str(Operation op) {
    switch(op) {
        case Add:
            return "--add";
        case List:
            return "--list";
        case View:
            return "--view";
        case RemoveReport:
            return "--remove_report";
        case UpdateThreshold:
            return "--update_threshold";
        case Filter:
            return "--filter";
        case Invalid:
            return "Invalid";
    }
    return "UNKNOWN ENUM VALUE";
}
Role str_to_role(const char *str) {
    
    if(!str) return Missing;

    if(strcmp("manager", str) == 0)
        return Manager;
    
    if(strcmp("inspector", str) == 0)
        return Inspector;

    return Missing;
}
const char *role_to_str(Role role) {
    
    switch(role) {
        case Manager:
            return "Manager";
        case Inspector:
            return "Inspector";
        case Missing:
            return "Role Missing";
    }
    return "Unknown Role Enum";
}

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
                    cmd->extra.new_threshold = strtol(argv[i+2], NULL, 10);
                    if (cmd->extra.new_threshold == 0 && argv[i+2][0] != '0') {
                        fprintf(stderr, "Please use a number for report_id!\n");
                        return -1;
                    }
                    i+=3;
                    break;
                case Filter:
                    if (i+2 >= argc) {
                        fprintf(stderr, "--filter [arg1] [arg2] <--missing!\n");
                        return -1;
                    }
                    strncpy(cmd->district_id, argv[i+1], MAX_DISTRICT_ID_LEN - 1);
                    cmd->district_id[MAX_DISTRICT_ID_LEN - 1] = '\0';
                    strncpy(cmd->extra.filter_condition, argv[i+2], MAX_FILTER_LEN - 1);
                    cmd->extra.filter_condition[MAX_FILTER_LEN - 1] = '\0';
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
