#ifndef NLLOG_H
#define NLLOG_H

#define LOGNAME "nltools.log"
#ifdef MSDOS
#include <fidoconf/fidoconf.h>
#else
#include <fidoconf/fidoconf.h>
#endif

void loginit(s_fidoconfig *config);
void logdeinit(void);

#define LOG_ERROR   '!'
#define LOG_WARNING '?'
#define LOG_MSG     '+'


int logentry(char level, char *format, ...);

#endif
