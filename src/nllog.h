#ifndef NLLOG_H
#define NLLOG_H

#define LOGNAME "nltools.log"
#include "fidoconfig.h"

void loginit(s_fidoconfig *config);
void logdeinit(void);

#define LOG_ERROR   '!'
#define LOG_WARNING '?'
#define LOG_MSG     '+'


int logentry(char level, char *format, ...);

#endif
