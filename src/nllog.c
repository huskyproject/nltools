
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <stdio.h>
#include <time.h>

#include "nllog.h"

static FILE *flog = NULL;

void loginit(s_fidoconfig *config)
{
    char *dir;
    char *logfile = NULL;
    
    if (config->logFileDir != NULL)
    {
        dir = config->logFileDir;
    }
    else
    {
        dir = "";
    }

    logfile = malloc(strlen(dir) + strlen(LOGNAME) + 1);

    if (logfile != NULL)
    {
        strcpy(logfile, dir);
        strcat(logfile, LOGNAME);
        flog = fopen(logfile, "a");
        if (flog == NULL)
        {
            fprintf (stderr, "Warning: cannot open logfile %s\n",
                     logfile);
        }
        free(logfile);
    }
}

void logdeinit(void)
{
    if (flog != NULL)
    {
        fprintf (flog, "\n");
        fclose(flog);
        flog = NULL;
    }
}


int logentry(char level, char *format, ...)
{
    va_list args;
    int i;
    char timestr[30];
    time_t t;
    FILE *f;


    time(&t);

    strftime(timestr, 29, "%d %b %H:%M:%S", localtime(&t));

    for (i = 0; i <= 1; i++)
    {
        if (!i)
            if (level == LOG_WARNING || level == LOG_ERROR)
                f = stderr;
            else
                f = stdout;
        else
            f = flog;

        if (!f) continue;

        
        fprintf (f, "%c %s NLTOOL ", level, timestr);
        va_start(args, format);
        vfprintf (f, format, args);
        va_end(args);
        fprintf (f, "\n");
    }
}        


