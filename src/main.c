#include <stdio.h>
#include <stdlib.h>
#include "types.h"
#include "parser.h"
#include "utils.h"
#include "commands.h"

#define PROGRAM_FAIL_ARGS 1
#define PROGRAM_FAIL_OP 2

int main(int argc, char **argv) {

    
    Command cmd;
    if(parse_arguments(argc, argv, &cmd) != 0) {
        fprintf(stderr, "Bad Arguments!\n");
        exit(PROGRAM_FAIL_ARGS);
    }
    // command_dump(&cmd);
    switch (cmd.operation) {
        case Add: 
            if(add(&cmd) != 0) {
                printf("Error --add\n");
                exit(PROGRAM_FAIL_OP);
            }
            break;
        case List:
            if(list(&cmd) != 0) {
                printf("Error --list\n");
                exit(PROGRAM_FAIL_OP);
            }
            break;
        case View:
            if(view(&cmd) != 0) {
                printf("Error --view\n");
                exit(PROGRAM_FAIL_OP);
            }
            break;
        default:
            printf("Not yet implemented!");
    }
    return 0;
}
