# city-infrastructure-supervisor

## University Project - Current scope:
A C program in the UNIX environment implementing a city infrastructure issue reporting and monitoring system.
City inspectors file structured reports about problems found across urban districts (damaged roads, broken lighting, flooding, etc.).
The system stores and organises these reports on disk, enforces access rules based on Unix file permissions and user roles,
monitors districts through a background process coordinated via signals, and routes data between components using pipes and I/O redirection with dup2.

## Functionality:
Work In Progress

## To compile use:
**make**
The resulting executable will be found in ./bin/
There is ambiguity in the spec on whether when doing an operation as inspector it should be logged or not.
inside the log_operation command in /src/commands.c there is a snippet that can be uncommented if that is the case
