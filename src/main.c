#include "commands.h"
#include "parser.h"
#include "types.h"
#include "utils.h"
#include <stdio.h>
#include <string.h>

#define PROGRAM_FAIL_ARGS 1
#define PROGRAM_FAIL_CMD 3
#define PROGRAM_FAIL_OP 2

int main(int argc, char **argv) {
    Command cmd;
    int status = 0;

    if (parse_arguments(argc, argv, &cmd) != 0) {
        fprintf(stderr, "Bad Arguments!\n");
        return PROGRAM_FAIL_ARGS;
    }

    if (cmd.operation == Invalid || cmd.role == Missing ||
        strlen(cmd.user) == 0) {
        fprintf(stderr, "Missing operation, role, or user!\n");
        status = PROGRAM_FAIL_CMD;
    } else {
        switch (cmd.operation) {
        case Add:
            status = add(&cmd);
            break;
        case List:
            status = list(&cmd);
            break;
        case View:
            status = view(&cmd);
            break;
        case RemoveReport:
            status = remove_report(&cmd);
            break;
        case Filter:
            status = filter(&cmd);
            break;
        case UpdateThreshold:
            status = update_threshold(&cmd);
            break;
        default:
            printf("Not yet implemented!\n");
            status = -1;
        }

        if (status != 0) {
            fprintf(stderr, "Error during --%s\n", op_to_str(cmd.operation));
            status = PROGRAM_FAIL_OP;
        }
    }

    free_command(&cmd);

    return status;
}
