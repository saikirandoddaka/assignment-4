
#include <math.h>

#include "config.h"
#include "utils.h"

uint min(uint a, uint b)
{
    if (a > b)
        return b;
    return a;
}

uint max(uint a, uint b)
{
    if (a < b)
        return b;
    return a;
}

uint clamp(uint val, uint min, uint max)
{
    if (val < min)
        return min;
    if (val > max)
        return max;
    return val;
}

bool chance(uint value)
{
    return rand() % 100 < value;
}

void itoa(char *buf, long value)
{
    sprintf(buf, "%d", value);
}

void time_mul(time *t, double k)
{
    double new_sec = t->sec * k;
    double new_usec = t->usec * k;
    t->sec = (uint)new_sec;
    t->usec = (uint)(new_usec + (new_sec - floor(new_sec)) * 1000000);
}

static void time_fix(time *t)
{
    t->sec += t->usec / 1000000;
    t->usec %= 1000000;
}

void time_add(time *t, uint k)
{
    t->usec += k;
    time_fix(t);
}

bool time_past(time later, time earlier)
{
    if (later.sec > earlier.sec)
        return true;
    if (later.sec < earlier.sec)
        return false;
    return later.usec >= earlier.usec;
}

void time_sum(time *t, time k)
{
    t->sec += k.sec;
    t->usec += k.usec;
    time_fix(t);
}

void time_sub(time *t, time k)
{
    if (t->usec < k.usec) {
        t->sec--;
        t->usec += 1000000;
    }
    t->sec -= k.sec;
    t->usec -= k.usec;
}
