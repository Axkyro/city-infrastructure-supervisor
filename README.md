# city-infrastructure-supervisor

## University Project - Current scope:

A C program in the UNIX environment implementing a city infrastructure issue reporting and monitoring system.
City inspectors file structured reports about problems found across urban districts (damaged roads, broken lighting, flooding, etc.).
The system stores and organises these reports on disk, enforces access rules based on Unix file permissions and user roles,
monitors districts through a background process coordinated via signals, and routes data between components using pipes and I/O redirection with dup2.

Full Spec provided by the teacher: 
https://docs.google.com/document/d/1BoCPPOKTwwGXswjsYwY94Jp2G7nTr0SR-4fQndMcv8Y/edit?usp=sharing

## Functionality:

WIP
Current examples:

./city_manager --role <manager|inspector> --user <username> <operation> <district_id> [extra_args]

./city_manager --role inspector --user kyro --add downtown
./city_manager --role inspector --user kyro --list downtown # lists all reports
./city_manager --role inspector --user kyro --view downtown 0 # to see specific report

other operations:
--update_threshold <district_id> <new_threshold_value> (only for manager)
--remove_district <district_id> (only for manager)
--remove_report <district_id> <report_id> (only for manager)


## To compile use:
**make**
The resulting executable will be found in ./bin/
Districts will be created inside ./districts after running the program once
