#include <stdio.h>
#include "types.h"
#include "parser.h"
#include "utils.h"

int main(int argc, char **argv) {
    printf("Sizeof: %lu\n", sizeof(Report));
    Command cmd;
    printf("Result: %d\n", parse_arguments(argc, argv, &cmd));
    command_dump(&cmd);
    
    return 0;
}
