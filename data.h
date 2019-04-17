#ifndef DATA_H
#define DATA_H

#include <stdbool.h>
#include <stddef.h>

#include "config.h"

typedef enum proc_state_enum {active, blocked, terminated} proc_state;
typedef enum msg_event_enum {none, preempt, block, terminate} msg_event;
typedef enum msg_direction_enum {any, to_oss, to_user} msg_direction;

typedef struct time_struct {
    uint sec;
    uint usec;
} time;

typedef struct message_struct {
    // Message type is required to be long
    union {
        msg_direction direction;
        long _msgtyp;
    };
    time timeslice;
    msg_event event;
    time additional;
} message;

extern const size_t message_size;
extern const time process_spawn_delta;

typedef struct pcb_struct {
    uint pid;
    proc_state state;
    int msg_id;
    uint priority;
    time started_at;
    time blocked_until;
    time last_timeslice_at;
} pcb;

struct shm {
    time clock;
    pcb pcbs[MAX_USERS];
} *shm;

#endif // DATA_H
