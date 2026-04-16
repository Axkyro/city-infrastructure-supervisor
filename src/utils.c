#include <stdio.h>
#include "utils.h"
#include "types.h"
#include "parser.h"

void command_dump(Command *cmd) {
    printf("User: [%s] | Role: [%s]\n", cmd->user, role_to_str(cmd->role));
    printf("Op: [%s] | District: [%s]\n", op_to_str(cmd->operation), cmd->district_id);

}
