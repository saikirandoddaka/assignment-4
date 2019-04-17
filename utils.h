#ifndef UTILS_H
#define UTILS_H

#include <stdlib.h>
#include <stdio.h>

#include "config.h"
#include "data.h"

#define CHECK_RET(VAR, ERROR_RET, MSG) {if (VAR == ERROR_RET) { perror(MSG); exit(1); }}

uint min(uint a, uint b);
uint max(uint a, uint b);
uint clamp(uint val, uint min, uint max);
bool chance(uint value);
void itoa(char *buf, long value);

void time_mul(time *t, double k);
void time_add(time *t, uint k);
bool time_past(time later, time earlier);
void time_sum(time *t, time k);
void time_sub(time *t, time k);

#endif // UTILS_H
