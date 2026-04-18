#include <stdio.h>
#include "types.h"
#include "parser.h"
#include "utils.h"
#include "commands.h"
int main(int argc, char **argv) {
    Command cmd;
    printf("Result: %d\n", parse_arguments(argc, argv, &cmd));
    command_dump(&cmd);
    printf("----\n") ;
    printf("Result district: %d\n", add_district(&cmd));
    
    return 0;
}
