#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>

#include <fidoconf/fidoconf.h>
#include <fidoconf/log.h>

#include "ulc.h"
#if !(defined(_MSC_VER) && (_MSC_VER >= 1200))
#include <fidoconf/dirlayer.h>
#endif
#include "nlstring.h"
#include "nlfind.h"
#include "version.h"

int process(s_fidoconfig *config)
{
    int i, rv=0;
    char *fidouserlist, *nodelist;
    FILE *fin, *fout;

    if (config->fidoUserList == NULL)
    {
        w_log(LL_CRIT, "No fido user list configured in fidoconfig.");
        return 8;
    }

    if (config->nodelistDir == NULL)
    {
        w_log(LL_CRIT,
                 "No nodelist directory configured in fidoconfig.");
        return 8;
    }


    if (config->nodelistCount < 1 )
    {
        w_log(LL_CRIT,
                 "No nodelist configured in fidoconfig.");
        return 8;
    }

    fidouserlist = malloc(strlen(config->nodelistDir) +
                          strlen(config->fidoUserList) + 1);
    if (fidouserlist == NULL)
    {
        w_log(LL_CRIT, "Out of memory.");
        return 8;
    }

    strcpy(fidouserlist, config->nodelistDir);
    strcat(fidouserlist, config->fidoUserList);


    fout = fopen(fidouserlist, "w+b");
    if (fout == NULL)
    {
        w_log(LL_CRIT, "Cannot open %s: %s", fidouserlist, strerror(errno));
        free(fidouserlist);
        return 8;
    }

    w_log(LL_INFO, "Building %s", fidouserlist);

    for (i = 0; i < config->nodelistCount; i++)
    {
        nodelist = findNodelist(config, i);

        if (nodelist == NULL)
        {
            w_log(LL_ALERT, "No instance of nodelist %s found.",
                     config->nodelists[i].nodelistName);
            if (rv < 4) rv = 4;
        }
        else
        {
            w_log(LL_INFO, "Using %s", nodelist);

            fin = fopen(nodelist, "rb");
            if (fin == NULL)
            {
                w_log(LL_ERROR, "Error opening %s: %s", nodelist, strerror(errno));
                if (rv < 8) rv = 8;
            }
            else
            {
                int format;

                switch (config->nodelists[i].format)
                {
                case points4d:
                    format = F_POINTS4D;
                    break;
                case points24:
                    format = F_POINTS24;
                    break;
                case fts5000:
                default:
                    format = F_NODELIST;
                }

                if (!ul_compile(fin, fout, format,
                                config->nodelists[i].defaultZone))
                {
                    w_log(LL_ERROR, "Error during compile");
                    if (rv < 8) rv = 8;
                }
            }
        }
        free(nodelist);
    }

    w_log(LL_INFO, "Sorting");
    if (!ul_sort(fout))
    {
        w_log(LL_ERROR, "Error while sorting");
        if (rv < 8) rv = 8;
    }

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
        openLog(LOGNAME, "ulc " REV, config);

        w_log(LL_START, "ulc - userlist compiler rev. %s", REV);

        rv=process(config);

        w_log( LL_STOP, "Done" );
        closeLog();
        disposeConfig(config);
        return rv;

    }
    else
    {
        fprintf (stderr, "Fatal: Cannot open fidoconfig.\n");
        return 8;
    }
}
