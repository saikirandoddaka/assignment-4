
#include <errno.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <sys/wait.h>

#include "config.h"
#include "utils.h"
#include "data.h"
#include "pcb.h"
#include "queue.h"
#include "log.h"

static pid_t host_pids[MAX_USERS];
static queue queues[4];
static uint total_processes;

static time total_turnaround_time;
static time total_wait_time;
static time total_sleep_time;

bool can_spawn_more()
{
    return total_processes < MAX_TOTAL_USERS;
}

uint get_free_pid()
{
    for (int i = 0; i < MAX_USERS; i++) {
        if (host_pids[i] == 0)
            return i;
    }
    return -1;
}

uint count_processes()
{
    uint count = 0;
    for (int i = 0; i < MAX_USERS; i++) {
        if (host_pids[i] != 0)
            count++;
    }
    return count;
}

pcb *get_pcb(uint pid)
{
    return &shm->pcbs[pid];
}

pcb *init_pcb(uint pid, pid_t host_pid)
{
    pcb *p = get_pcb(pid);
    p->pid = pid;
    p->state = active;
    p->msg_id = msgget(KEY_MSG + pid, IPC_CREAT | 0666);
    CHECK_RET(p->msg_id, -1, "msgget");
    p->priority = chance(20);
    p->started_at = shm->clock;
    p->blocked_until.sec = 0;
    p->blocked_until.usec = 0;

    host_pids[pid] = host_pid;
    push(&queues[p->priority], pid);
    total_processes++;
    return p;
}

void clear_pcb(pcb *p)
{
    p->state = terminated;
    host_pids[p->pid] = 0;
    msgctl(p->msg_id, IPC_RMID, NULL);
}

void kill_all_users()
{
    for (int i = 0; i < MAX_USERS; i++) {
        pcb *p = get_pcb(i);
        if (p->state != terminated) {
            kill(host_pids[p->pid], SIGKILL);
            clear_pcb(p);
        }
    }
}

bool can_schedule()
{
    for (int i = 0; i < MAX_USERS; i++)
        if (get_pcb(i)->state == active)
            return true;
    return false;
}

uint get_timeslice(uint priority)
{
    return QUANTUM >> priority;
}

void schedule_process()
{
    pcb *p;
    message msg;
    msg._msgtyp = 0;
    for (int i = 0; i < 4; i++) {
        if (!empty(&queues[i])) {
            p = get_pcb(pop(&queues[i]));
            break;
        }
    }

    msg.direction = to_user;
    msg.timeslice.sec = 0;
    msg.timeslice.usec = get_timeslice(p->priority);
    time_add(&shm->clock, rand() % 99901 + 100);

    flog("Dispatching process with PID %d from queue %d", p->pid, p->priority);
    int ret1 = msgsnd(p->msg_id, &msg, message_size, 0);
    CHECK_RET(ret1, -1, "msgsnd");
    ssize_t ret2;
    do {
        ret2 = msgrcv(p->msg_id, &msg, message_size, to_oss, 0);
    } while (errno == EINTR);
    CHECK_RET(ret2, -1, "msgrcv");

    time_sum(&total_turnaround_time, msg.timeslice);

    time_sum(&total_wait_time, shm->clock);
    time_sub(&total_wait_time, p->last_timeslice_at);
    time_sub(&total_wait_time, msg.timeslice);
    p->last_timeslice_at = shm->clock;

    flog("Receiving that process %d ran for %d nanoseconds", p->pid, msg.timeslice.usec);

    switch(msg.event) {
    case preempt:
        flog("Not using its entire time quantum");
    case none:
        if (p->priority > 0 && p->priority < 3)
            p->priority++;
        push(&queues[p->priority], p->pid);
        flog("Putting process with PID %d into queue %d", p->pid, p->priority);
        break;
    case block:
        p->blocked_until = shm->clock;
        time_sum(&p->blocked_until, msg.additional);
        time_sum(&total_sleep_time, msg.additional);
        p->state = blocked;
        flog("Blocking process with PID %d for %d:%d", p->pid, msg.additional.sec, msg.additional.usec);
        break;
    case terminate:
        flog("Terminating process with PID %d", p->pid);
        waitpid(host_pids[p->pid], NULL, 0);
        clear_pcb(p);
        break;
    }
}

time next_process_unblock()
{
    time next_pcb_time = {999999, 999999};
    for (int i = 0; i < MAX_USERS; i++) {
        pcb *p = get_pcb(i);
        if (p->state == blocked && time_past(next_pcb_time, p->blocked_until)) {
            next_pcb_time = p->blocked_until;
        }
    }
    return next_pcb_time;
}

void unblock_processes()
{
    for (int i = 0; i < MAX_USERS; i++) {
        pcb *p = get_pcb(i);
        if (p->state == blocked)
            if (time_past(shm->clock, p->blocked_until)) {
                p->state = active;
                p->priority = min(p->priority, 1);
                push(&queues[p->priority], p->pid);
                flog("Unblocking process with PID %d and putting it into queue %d", p->pid, p->priority);
                time_add(&shm->clock, 10);
            }
    }
}

void pcb_write_stats()
{
    time_mul(&total_turnaround_time, 1.0 / total_processes);
    time_mul(&total_wait_time, 1.0 / total_processes);
    time_mul(&total_sleep_time, 1.0 / total_processes);
    flogf("Average turnaround time: %d:%d", total_turnaround_time.sec, total_turnaround_time.usec);
    flogf("Average wait time: %d:%d", total_wait_time.sec, total_wait_time.usec);
    flogf("Average sleep time: %d:%d", total_sleep_time.sec, total_sleep_time.usec);
}
