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
    return "TO BE IMPLEMENTED";
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
    return "Work In Progress";
}

int parse_arguments(int argc, char **argv, Command *cmd) {
    if (!cmd) return -1; // received NULL cmd
    if(argc == 1) { fprintf(stderr, "No flags!\n"); return -1; }
    cmd->operation = Invalid;
    cmd->role = Missing;
    size_t i = 1;  // arg counter

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
                default:
                    i++;
            }
        } else {
            fprintf(stderr, "Unkown flag: [%s]\n", argv[i]);
            return -1;
        } 
    } 
    return 0;
}
