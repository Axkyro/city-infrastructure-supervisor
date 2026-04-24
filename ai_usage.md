I uploaded to the AI tools my header files commands.h, types.h and utils.h
as well as access to my hand written parser.c function
from there on the AI gave me the following functions:

Additional Code & Functions: 
int parse_condition(const char *input, char *field, char *op, char *value);
int match_condition(Report *r, const char *field, const char *op, const char *value);
void free_command(Command *cmd);
int compare_int(int a, const char *op, int b); // these are helper functions for the ai
int compare_str(const char *a, const char *op, const char *b);

as well as a modified Filter case inside parser.c


My Report struct uses char **filter_conditions to store multiple conditions that later need to be freed

Used prompt:
Given my command and report structures modify my parser.c Filter case as well as write the functions
int parse_condition(const char *input, char *field, char *op, char *value);
int match_condition(Report *r, const char *field, const char *op, const char *value);
and any additional needed function (possibly to free memory afterwards in main.c)
such that parse_condition splits a field:operator:value string into its three parts,
and match_condition returns 1 if the record satisfies the condition and 0 otherwise.

Tools used: Gemini, ChatGPT
I learned that even with enough information sometimes LLMs such as the ones I used have a hard time integrating flawlessly (that is, on the first try) additional code into the codebase, and they need tinkering before being able to use their output.
