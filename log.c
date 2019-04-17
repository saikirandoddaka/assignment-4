
#include <stdlib.h>
#include <stdio.h>
#include <stdarg.h>

#include "config.h"
#include "data.h"

static FILE *log_file;
uint log_lines;

void log_open(char *filename)
{
    if (log_file != NULL) {
        printf("Log file already opened!");
        exit(1);
    }
    log_file = fopen(filename, "w");
    log_lines = 0;
}

void log_close()
{
    fclose(log_file);
    log_file = NULL;
}

void vlogf(char *format, va_list args)
{
    if (log_file == NULL) {
        printf("Writing to log before opening a file\n");
        exit(1);
    }

    fprintf(log_file, "[%3d:%6d] OSS: ", shm->clock.sec, shm->clock.usec);
    vfprintf(log_file, format, args);
    fprintf(log_file, "\n");
    fflush(log_file);
    log_lines++;
}

void flog(char *format, ...) {
    va_list args;

    if (log_lines > MAX_LOG_LINES)
        return;

    va_start(args, format);
    vlogf(format, args);
    va_end(args);
}

void flogf(char *format, ...) {
    va_list args;

    va_start(args, format);
    vlogf(format, args);
    va_end(args);
}
