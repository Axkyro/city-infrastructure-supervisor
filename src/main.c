#include <stdio.h>
#include "types.h"
#include "parser.h"

int main(int argc, char **argv) {
    Command cmd;
    printf("Result: %d\n", parse_arguments(argc, argv, &cmd));

    return 0;
}
