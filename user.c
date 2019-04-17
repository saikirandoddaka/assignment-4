
#define _GNU_SOURCE

#include <stdbool.h>
#include <stdio.h>

#include <unistd.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <sys/shm.h>

#include "pcb.h"
#include "utils.h"
#include "data.h"

static bool running;

static pcb *p;

void setup(uint pid)
{
    srand(getpid());

    int shmid = shmget (KEY_SHM, sizeof(struct shm), 0666);
    shm = shmat(shmid, NULL, 0);

    p = get_pcb(pid);
}

void handle_message(message *msg)
{
    double used_fraction = 0.01 * (rand() % 99 + 1);
    msg->direction = to_oss;
    if (chance(5)) {
        msg->event = terminate;
        time_mul(&msg->timeslice, used_fraction);
        running = false;
        return;
    }
    if (chance(10)) {
        msg->event = block;
        time_mul(&msg->timeslice, used_fraction);
        msg->additional.sec = rand() % 6;
        msg->additional.usec = rand() % 1001;
        return;
    }
    if (chance(33)) {
        msg->event = preempt;
        time_mul(&msg->timeslice, used_fraction);
        return;
    }
    msg->event = none;
    return;
}

void run()
{
    message msg;
    running = true;
    while (running) {
        msgrcv(p->msg_id, &msg, message_size, to_user, 0);
        handle_message(&msg);
        msgsnd(p->msg_id, &msg, message_size, 0);
        usleep(1);
    }
}

void shutdown()
{
    shmdt(shm);
}

int main(int argc, char *argv[])
{
    int pid = atoi(argv[1]);

    setup(pid);
    run();
    shutdown();
}
