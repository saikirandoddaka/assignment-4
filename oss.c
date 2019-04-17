
// #define DEBUG

#define _GNU_SOURCE

#include <stdbool.h>
#include <stdio.h>
#include <string.h>

#include <unistd.h>
#include <signal.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/types.h>

#include "utils.h"
#include "data.h"
#include "pcb.h"
#include "log.h"

static bool running;
static time next_process_spawn;

static time total_idle_time;

void shutdown()
{
    running = false;
    kill_all_users();
    flog("Shutting down");
}

void signal_handler(int sig) {
    switch(sig) {
    case SIGINT:
        printf("Caught SIGINT");
        break;
    case SIGTERM:
        printf("Caught SIGTERM");
        break;
    case SIGALRM:
        printf("Caught SIGALRM");
        break;
    default:
        return;
    }
    printf(". Terminating\n");
    shutdown();
}

void setup_signals()
{
    sighandler_t ret;
    ret = signal(SIGINT, signal_handler);
    CHECK_RET(ret, SIG_ERR, "signal");

    ret = signal(SIGTERM, signal_handler);
    CHECK_RET(ret, SIG_ERR, "signal");

    ret = signal(SIGALRM, signal_handler);
    CHECK_RET(ret, SIG_ERR, "signal");

#ifndef DEBUG
    alarm(MAX_REAL_TIME);
#endif
}

static int shmid;

void setup_shmem()
{
    shmid = shmget (KEY_SHM, sizeof(struct shm), IPC_CREAT | 0666);
    CHECK_RET(shmid, -1, "shmget");
    // shm defined in data.h
    shm = shmat(shmid, NULL, 0);
    memset(shm, 0, sizeof(shm));
    for (int i = 0; i < MAX_USERS; i++)
        get_pcb(i)->state = terminated;
    CHECK_RET(shm, (void*)-1, "shmat");
}

void destroy_shmem()
{
    shmctl(shmid, IPC_RMID, NULL);
}

bool can_spawn_process()
{
    if (count_processes() >= MAX_USERS)
        return false;
    if (!time_past(shm->clock, next_process_spawn))
        return false;
    if (get_free_pid() == -1)
        return false;
    return true;
}

void exec_user(uint pid)
{
    char spid[4];
    itoa(spid, pid);
    execl("./user", "./user", spid, (char*)NULL);
    perror("execl: ");
    exit(1);
}

void spawn_process()
{
    uint pid = get_free_pid();
    pid_t host_pid = fork();
    CHECK_RET(host_pid, -1, "fork");

    if (host_pid == 0)
        exec_user(pid);
    pcb *p = init_pcb(pid, host_pid);
    flog("Generating process with PID %d and putting it in queue %d", pid, p->priority);

    time to_add = process_spawn_delta;
    time_mul(&to_add, (rand() % 101) * 0.01);
    time_sum(&next_process_spawn, to_add);

    if (!can_spawn_more()) {
        flog("Reached 100 processes. Terminating");
        running = false;
    }
}

void idle()
{
    time idle_until = next_process_unblock();
    if (running && time_past(idle_until, next_process_spawn))
        idle_until = next_process_spawn;
    flog("No processes to run");
    time_sum(&total_idle_time, idle_until);
    time_sub(&total_idle_time, shm->clock);
    shm->clock = idle_until;
}

void run_oss()
{
    running = true;
    while (running || count_processes() > 0) {
        if (running && can_spawn_process())
            spawn_process();

        if (can_schedule())
            schedule_process();
        else
            idle();

        unblock_processes();
        usleep(1);
    }
}

void write_stats()
{
    pcb_write_stats();
    flogf("Total idle time: %d:%d", total_idle_time.sec, total_idle_time.usec);
}

int main()
{
    setup_signals();
    setup_shmem();
    atexit(destroy_shmem);
    log_open("log.txt");
    srand(getpid());

    run_oss();
    write_stats();
    log_close();
}
