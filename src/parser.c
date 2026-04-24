#include "parser.h"
#include "types.h"
#include "utils.h"
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

Operation str_to_op(const char *str) {
    if (!str)
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
    switch (op) {
    case Add:
        return "add";
    case List:
        return "list";
    case View:
        return "view";
    case RemoveReport:
        return "remove_report";
    case UpdateThreshold:
        return "update_threshold";
    case Filter:
        return "filter";
    case Invalid:
        return "Invalid";
    }
    return "UNKNOWN ENUM VALUE";
}

Role str_to_role(const char *str) {

    if (!str)
        return Missing;

    if (strcmp("manager", str) == 0)
        return Manager;

    if (strcmp("inspector", str) == 0)
        return Inspector;

    return Missing;
}

const char *role_to_str(Role role) {

    switch (role) {
    case Manager:
        return "manager";
    case Inspector:
        return "inspector";
    case Missing:
        return "missing-role";
    }
    return "Unknown Role Enum";
}

int parse_arguments(int argc, char **argv, Command *cmd) {
    if (!cmd)
        return -1; // received NULL cmd
    cmd->operation = Invalid;
    cmd->role = Missing;
    cmd->district_id[0] = '\0';
    cmd->extra.filter_conditions = NULL;
    cmd->user[0] = '\0';
    size_t i = 1; // arg counter
    if (argc == 1) {
        fprintf(stderr, "No flags!\n");
        return -1;
    }
    while (i < argc) {
        // checking for a given user
        if (strcmp("--user", argv[i]) == 0) {
            if (i + 1 == argc) {
                fprintf(stderr, "--user [arg] <-- missing!\n");
                free_command(cmd);
                return -1;
            }
            strncpy(cmd->user, argv[i + 1], MAX_USERNAME_LEN - 1);
            cmd->user[MAX_USERNAME_LEN - 1] = '\0';
            i += 2;
        } else if (strcmp("--role", argv[i]) == 0) {
            if (i + 1 == argc) {
                fprintf(stderr, "--role [arg] <-- missing!\n");
                free_command(cmd);
                return -1;
            }
            cmd->role = str_to_role(argv[i + 1]);
            if (cmd->role == Missing) {
                fprintf(stderr, "Unknown role: %s\n", argv[i + 1]);
                fprintf(stderr, "Try --role manager\n");
                free_command(cmd);
                return -1;
            }
            i += 2;
        } else if (strlen(argv[i]) > 2 && argv[i][0] == '-' &&
                   argv[i][1] == '-') {
            cmd->operation = str_to_op(argv[i] + 2);
            switch (cmd->operation) {
            case Invalid:
                fprintf(stderr, "Unknown operation: [%s]\n", argv[i] + 2);
                free_command(cmd);
                return -1;
            case Add:
                if (i + 1 == argc) {
                    fprintf(stderr, "--add [arg] <-- missing!\n");
                    free_command(cmd);
                    return -1;
                }
                strncpy(cmd->district_id, argv[i + 1], MAX_DISTRICT_ID_LEN - 1);
                cmd->district_id[MAX_DISTRICT_ID_LEN - 1] = '\0';
                i += 2;
                break;
            case List:
                if (i + 1 == argc) {
                    fprintf(stderr, "--list [arg] <-- missing!\n");
                    free_command(cmd);
                    return -1;
                }
                strncpy(cmd->district_id, argv[i + 1], MAX_DISTRICT_ID_LEN - 1);
                cmd->district_id[MAX_DISTRICT_ID_LEN - 1] = '\0';
                i += 2;
                break;
            case View:
                if (i + 2 >= argc) {
                    fprintf(stderr, "--view [arg1] [arg2] <--missing!\n");
                    free_command(cmd);
                    return -1;
                }
                strncpy(cmd->district_id, argv[i + 1], MAX_DISTRICT_ID_LEN - 1);
                cmd->district_id[MAX_DISTRICT_ID_LEN - 1] = '\0';
                cmd->extra.report_id = strtol(argv[i + 2], NULL, 10);
                // not completely safe!
                if (cmd->extra.report_id == 0 && argv[i + 2][0] != '0') {
                    fprintf(stderr, "Please use a number for report_id!\n");
                    free_command(cmd);
                    return -1;
                }
                i += 3;
                break;
            case RemoveReport:
                if (i + 2 >= argc) {
                    fprintf(stderr,
                            "--remove_report [arg1] [arg2] <--missing!\n");
                    free_command(cmd);
                    return -1;
                }
                strncpy(cmd->district_id, argv[i + 1], MAX_DISTRICT_ID_LEN - 1);
                cmd->district_id[MAX_DISTRICT_ID_LEN - 1] = '\0';
                cmd->extra.report_id = strtol(argv[i + 2], NULL, 10);
                if (cmd->extra.report_id == 0 && argv[i + 2][0] != '0') {
                    fprintf(stderr, "Please use a number for report_id!\n");
                    free_command(cmd);
                    return -1;
                }
                i += 3;
                break;
            case UpdateThreshold:
                if (i + 2 >= argc) {
                    fprintf(stderr,
                            "--change_threshold [arg1] [arg2] <--missing!\n");
                    free_command(cmd);
                    return -1;
                }
                strncpy(cmd->district_id, argv[i + 1], MAX_DISTRICT_ID_LEN - 1);
                cmd->district_id[MAX_DISTRICT_ID_LEN - 1] = '\0';
                cmd->extra.new_threshold =
                    (uint8_t)strtol(argv[i + 2], NULL, 10);
                if (cmd->extra.new_threshold == 0 && argv[i + 2][0] != '0') {
                    fprintf(stderr,
                            "Please use a number for new threshold value!\n");
                    free_command(cmd);
                    return -1;
                }
                i += 3;
                break;

            // THE CASE BELOW IS AI GENERATED
            case Filter:
                if (i + 1 >= argc) {
                    fprintf(stderr, "--filter [district_id] [conditions...] "
                                    "<-- missing!\n");
                    return -1;
                }

                // 1. Get the district ID
                strncpy(cmd->district_id, argv[i + 1], MAX_DISTRICT_ID_LEN - 1);
                cmd->district_id[MAX_DISTRICT_ID_LEN - 1] = '\0';

                // 2. Count how many conditions there are
                int cond_count = 0;
                int j = i + 2;
                // Keep counting until we hit the end of args, or another flag
                // (starts with --)
                while (j < argc && strncmp(argv[j], "--", 2) != 0) {
                    cond_count++;
                    j++;
                }

                if (cond_count == 0) {
                    fprintf(stderr, "Missing conditions for --filter!\n");
                    return -1;
                }

                // 3. Allocate the array (+1 for the NULL terminator)
                cmd->extra.filter_conditions =
                    malloc((cond_count + 1) * sizeof(char *));
                if (!cmd->extra.filter_conditions) {
                    perror("malloc");
                    return -1;
                }

                // 4. Duplicate the strings into our new array
                for (int k = 0; k < cond_count; k++) {
                    cmd->extra.filter_conditions[k] = strdup(argv[i + 2 + k]);
                }
                // Cap it with NULL so you can iterate over it safely later
                cmd->extra.filter_conditions[cond_count] = NULL;

                // Advance our main loop counter past the district AND all the
                // conditions
                i += 2 + cond_count;
                break;
            }
        } else {
            fprintf(stderr, "Unkown flag: [%s]\n", argv[i]);
            free_command(cmd);
            return -1;
        }
    }
    return 0;
}
