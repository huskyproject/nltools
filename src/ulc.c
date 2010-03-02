/* $Id$
   Written 1999 by Tobias Ernst and released do the Public Domain.
   This file is part of NLTOOLS, the nodelist processor of the Husky fidonet
   software project.

   Fidouserlist compiler.
*/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>

#include <fidoconf/fidoconf.h>
#include <huskylib/huskylib.h>
#include <huskylib/log.h>

#include "ulc.h"
#if !(defined(_MSC_VER) && (_MSC_VER >= 1200))
# include <huskylib/dirlayer.h>
#endif
#include "nlstring.h"
#include "nlfind.h"
#include "version.h"

int process( s_fidoconfig * config )
{
  unsigned int i;
  int rv = 0;
  char *fidouserlist, *nodelist;
  FILE *fin, *fout;
  fin = fout = NULL;

  if( config->fidoUserList == NULL )
  {
    w_log( LL_CRIT, "No fido user list configured in fidoconfig." );
    return 8;
  }

  if( config->nodelistDir == NULL )
  {
    w_log( LL_CRIT, "No nodelist directory configured in fidoconfig." );
    return 8;
  }


  if( config->nodelistCount < 1 )
  {
    w_log( LL_CRIT, "No nodelist configured in fidoconfig." );
    return 8;
  }

  fidouserlist = malloc( strlen( config->nodelistDir ) + strlen( config->fidoUserList ) + 1 );
  if( fidouserlist == NULL )
  {
    w_log( LL_CRIT, "Out of memory." );
    return 8;
  }

  strcpy( fidouserlist, config->nodelistDir );
  strcat( fidouserlist, config->fidoUserList );


  fout = fopen( fidouserlist, "w+b" );
  if( fout == NULL )
  {
    w_log( LL_CRIT, "Cannot open %s: %s", fidouserlist, strerror( errno ) );
    free( fidouserlist );
    return 8;
  }

  w_log( LL_INFO, "Building %s", fidouserlist );

  for( i = 0; i < config->nodelistCount; i++ )
  {
    nodelist = findNodelist( config, i );

    if( nodelist == NULL )
    {
      w_log( LL_ALERT, "No instance of nodelist %s found.", config->nodelists[i].nodelistName );
      if( rv < 4 )
        rv = 4;
    }
    else
    {
      w_log( LL_INFO, "Using %s", nodelist );

      fin = fopen( nodelist, "rb" );
      if( fin == NULL )
      {
        w_log( LL_ERROR, "Error opening %s: %s", nodelist, strerror( errno ) );
        if( rv < 8 )
          rv = 8;
      }
      else
      {
        int format;

        switch ( config->nodelists[i].format )
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

        if( !ul_compile( fin, fout, format, config->nodelists[i].defaultZone ) )
        {
          w_log( LL_ERROR, "Error during compile" );
          if( rv < 8 )
            rv = 8;
        }
      }
    }
    free( nodelist );
  }

  w_log( LL_INFO, "Sorting" );
  if( !ul_sort( fout ) )
  {
    w_log( LL_ERROR, "Error while sorting" );
    if( rv < 8 )
      rv = 8;
  }

  if( fin )
    fclose( fin );
  fclose( fout );
  return rv;
}

void usage( void )
{
  printf( "Usage:\n"
          "ulc [-hvq] [-c config]\n"
          "where\n"
          "      -h         print this help and exit\n"
          "      -v         print version information\n"
          "      -q         quiet mode\n" "      -c config  specify fidoconfig file\n" );
}

int main( int argc, char **argv )
{
  s_fidoconfig *config;
  int rv, i, flag_quiet = 0;
  char *versionStr, *configfile = NULL;
  versionStr = GenVersionStr( "ulc", VER_MAJOR, VER_MINOR, VER_PATCH, VER_BRANCH, cvs_date );
  for( i = 1; i < argc; i++ )
  {
    int j, plen;
    if( argv[i][0] == '-' )
    {
      int plen = sstrlen( argv[i] );
      for( j = 1; j < plen; j++ )
        switch ( argv[i][j] )
        {
        case 'h':
          printversion(  );
          usage(  );
          return 0;
        case 'v':
          printversion(  );
          return 0;
        case 'c':
          if( plen > ++j )
          { configfile = argv[i] + j;
            j=plen;
          }
          else if( argc > ++i )
            configfile = argv[i];
          else
          {
            w_log( LL_ERR, "Fatal: parameter after -c is required\n" );
            return 1;
          }
          break;
        case 'q':
          flag_quiet = 1;
        }
    }
  }

  if( !flag_quiet )
    printversion(  );
  config = readConfig( configfile );
  if( config != NULL )
  {
    initLog( config->logFileDir, config->logEchoToScreen, config->loglevels,
             config->screenloglevels );
    openLog( LOGNAME, versionStr );
    w_log( LL_START, "Start" );
    rv = process( config );
    w_log( LL_STOP, "End" );
    closeLog(  );
    disposeConfig( config );
    return rv;
  }
  else
  {
    w_log( LL_ERR, "Cannot open fidoconfig.\n" );
    return 8;
  }
}
