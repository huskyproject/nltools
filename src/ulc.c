#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "fidoconfig.h"
#include "ulc.h"
#include "nllog.h"
#include "dir.h"


char *findNodelist(s_fidoconfig *config, int i)
{
    char *nl = malloc(strlen(config->nodelistDir) +
                      strlen(config->nodelists[i].nodelistName) + 5);
    struct dirent *dp;
    DIR *hdir;
    size_t l, l2;
    int found;

    if (nl == NULL)
    {
        logentry(LOG_ERROR, "out of memory");
        return NULL;
    }
    
    strcpy(nl, config->nodelistDir);

    hdir = opendir(config->nodelistDir);

    if (hdir == NULL)
    {
        logentry(LOG_ERROR, "cannot read directory %s", config->nodelistDir);
        free(nl);
        return NULL;
    }

    l = strlen(config->nodelists[i].nodelistName);

    while ((dp = readdir(hdir)) != NULL)
    {
        l2 = strlen(dp->d_name);
        if (l2 == l + 4 &&
            !strncasecmp(config->nodelists[i].nodelistName,
                         dp->d_name, l) &&
            dp->d_name[l] == '.' &&
            isdigit(dp->d_name[l+1]) &&
            isdigit(dp->d_name[l+2]) &&
            isdigit(dp->d_name[l+3]))
        {
            strcat(nl, dp->d_name);
            found = 1;
            break;
        }
    }

    closedir(hdir);

    if (found)
        return nl;

    free(nl);
    return NULL;
}

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
                if (!ul_compile(fin, fout, config->nodelists[i].format,
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
}

    
        

    

int main(void)
{
    s_fidoconfig *config = readConfig();
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
