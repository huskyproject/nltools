#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fidoconf/fidoconf.h>
#include "ulc.h"
#include "nllog.h"
#if !(defined(_MSC_VER) && (_MSC_VER >= 1200))
#include <fidoconf/dirlayer.h>
#endif
#include "nlstring.h"
#include "nlfind.h"

int process(s_fidoconfig *config)
{
    int i, rv=0;
    char *fidouserlist, *nodelist;
    FILE *fin, *fout;

    if (config->fidoUserList == NULL)
    {
        logentry(LOG_ERROR, "no fido user list configured in fidoconfig.\n");
        return 8;
    }

    if (config->nodelistDir == NULL)
    {
        logentry(LOG_ERROR,
                 "no nodelist directory configured in fidoconfig.\n");
        return 8;
    }


    if (config->nodelistCount < 1 )
    {
        logentry(LOG_ERROR,
                 "no nodelist configured in fidoconfig.\n");
        return 8;
    }

    fidouserlist = malloc(strlen(config->nodelistDir) +
                          strlen(config->fidoUserList) + 1);
    if (fidouserlist == NULL)
    {
        logentry(LOG_ERROR, "out of memory.\n");
        return 8;
    }

    strcpy(fidouserlist, config->nodelistDir);
    strcat(fidouserlist, config->fidoUserList);


    fout = fopen(fidouserlist, "w+b");
    if (fout == NULL)
    {
        logentry(LOG_ERROR, "cannot open %s.\n", fidouserlist);
        free(fidouserlist);
        return 8;
    }

    logentry(LOG_MSG, "building %s", fidouserlist);

    for (i = 0; i < config->nodelistCount; i++)
    {
        nodelist = findNodelist(config, i);

        if (nodelist == NULL)
        {
            logentry(LOG_WARNING, "no instance of nodelist %s found.\n",
                     config->nodelists[i].nodelistName);
            if (rv < 4) rv = 4;
        }
        else
        {
            logentry(LOG_MSG, "using %s", nodelist);

            fin = fopen(nodelist, "rb");
            if (fin == NULL)
            {
                logentry(LOG_ERROR, "error opening %s.\n", nodelist);
                if (rv < 8) rv = 8;
            }
            else
            {
                if (!ul_compile(fin, fout,
                                config->nodelists[i].format == fts5000 ?
                                F_NODELIST: F_POINTS24,
                                config->nodelists[i].defaultZone))
                {
                    logentry(LOG_ERROR, "error during compile");
                    if (rv < 8) rv = 8;
                }
            }
        }
        free(nodelist);
    }

    logentry(LOG_MSG, "sorting");
    if (!ul_sort(fout))
    {
        logentry(LOG_ERROR, "error while sorting");
        if (rv < 8) rv = 8;
    }
    logentry(LOG_MSG, "done");
    

    fclose(fin);
    fclose(fout);
    return rv;
}

int main(void)
{
    s_fidoconfig *config = readConfig(NULL);
    int rv;

    if (config != NULL)
    {
        loginit(config);
        logentry(LOG_MSG, "ulc - userlist compiler rev. %s", REV);

        rv=process(config);

        logdeinit();
        disposeConfig(config);
        return rv;
        
    }
    else
    {
        fprintf (stderr, "Fatal: Cannot open fidoconfig.\n");
        return 8;
    }
}
