#ifndef AXKYRO_TYPES_H
#define AXKYRO_TYPES_H


typedef enum {
    Manager,
    Inspector,
    Missing // no role
} Role;

typedef enum {
    Add,
    List,
    View,
    RemoveReport,
    UpdateThreshold,
    Filter,
} Operation;





#endif
