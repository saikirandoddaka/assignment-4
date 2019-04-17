#ifndef PCB_H
#define PCB_H

#include <sys/types.h>

#include "data.h"

bool can_spawn_more();
uint get_free_pid();
uint count_processes();
pcb *get_pcb(uint pid);

pcb *init_pcb(uint pid, pid_t host_pid);
void clear_pcb(pcb *p);
void kill_all_users();

bool can_schedule();
void schedule_process();
time next_process_unblock();
void unblock_processes();

void pcb_write_stats();

#endif // PCB_H
