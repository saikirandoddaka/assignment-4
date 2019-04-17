#ifndef LOG_H
#define LOG_H

void log_open(char *filename);
void log_close();
// Print formatted message to logs
void flog(char *format, ...);
// Print important formatted message to logs
void flogf(char *format, ...);

#endif // LOG_H
