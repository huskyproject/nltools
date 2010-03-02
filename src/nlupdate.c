/* $Id$
   Written 1999 by Tobias Ernst and released do the Public Domain.
   This file is part of NLTOOLS, the nodelist processor of the Husky fidonet
   software project.

   Main program of the NLTOOLS.
*/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <ctype.h>
#include <time.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/stat.h>

#include <huskylib/compiler.h>

#ifdef HAS_IO_H
# include <io.h>
#endif

#ifdef HAS_UNISTD_H
# include <unistd.h>
#endif

#include <huskylib/dirlayer.h>

#include <fidoconf/fidoconf.h>
#include <huskylib/xstr.h>
#include <fidoconf/common.h>
#include <huskylib/log.h>

#ifdef USE_HPTZIP
# include <hptzip/hptzip.h>
#endif

#include "nlstring.h"
#include "nlfind.h"
#include "nldate.h"
#include "julian.h"
#include "ulc.h"
#include "version.h"

/* store the nldiff command name */
static char *differ = NULL;

/*
# if 0
*/
/* This is not needed now because I imported fexist.c from smapi, and the
   routines in fexist.c are a little more intelligent than this one! */
/*
static int nl_fexist(char *filename)
{
   FILE *f = fopen(filename, "rb");
   if (f == NULL)
      return 0;
   fclose(f);
   return 1;
}

#else
#define nl_fexist fexist
#endif
*/

static char *mk_uncompressdir( char *nldir )
{
  char *upath = NULL;
  size_t l;
  FILE *f;
  l = strlen( nldir ) + 12;
  /* construct a temporary directory name below nodelist dir */
  xstrscat( &upath, nldir, "nlupdate.tmp", NULL );
  adaptcase( upath );

  /* try to create it */
  mymkdir( upath );

  w_log( LL_DIR, "Use temp dir: '%s'", upath );

  /* check if we can write to the directory */
  nfree( upath );
  xscatprintf( &upath, "%s%s%c%s", nldir, "nlupdate.tmp", PATH_DELIM, "nlupdate.flg" );
  f = fopen( upath, "w" );
  if( f == NULL )
  {
    fprintf( stderr, "%s\n", upath );
    upath[l] = '\0';
    w_log( LL_ERROR, "Cannot create/access temporary directory (%s): %s", upath,
           strerror( errno ) );
    free( upath );
    return NULL;
  }
  fclose( f );
  remove( upath );
  upath[l + 1] = '\0';

  return upath;
}

int destroy_uncompressdir( char *upath )
{
  size_t l = strlen( upath );
  char *dfile;
  husky_DIR *hdir;

  hdir = husky_opendir( upath );
  if( hdir == NULL )
  {
    w_log( LL_ERROR, "Cannot read temporary directory (%s): %s", upath, strerror( errno ) );
    free( upath );
    return 0;
  }

  while( ( dfile = husky_readdir( hdir ) ) != NULL )
  {
    if( ( strcmp( dfile, "." ) ) && ( strcmp( dfile, ".." ) ) )
    {
      strcpy( upath + l, dfile );
      /* there is room for this because of the
         addition of FILENAME_MAX in mk_uncompressdir */

      if( remove( upath ) )
      {
        w_log( LL_ERROR, "Cannot delete file '%s': %s", upath, strerror( errno ) );
      }
      else
        w_log( LL_DEL, "File '%s' removed", upath );
    }
  }
  husky_closedir( hdir );

  upath[l - 1] = '\0';

  if( rmdir( upath ) )
  {
    w_log( LL_ERROR, "Cannot remove temporary directory '%s': %s", upath, strerror( errno ) );
    free( upath );
    return 0;
  }

  free( upath );
  return 1;
}

static int uncompress( s_fidoconfig * config, char *directory, char *filename, char *tempdir )
{
  char *tmpfilename = NULL;
  char cmd[256];
  FILE *f;
  int j, found, exitcode;
  UINT i;

  xstrscat( &tmpfilename, directory, filename, NULL );

  /* open the packet so we can see what archiver has been used */
  f = fopen( tmpfilename, "rb" );
  if( f == NULL )
  {
    w_log( LL_ERROR, "Cannot access %s: %s", tmpfilename, strerror( errno ) );
    return 0;
  }

  /* find what unpacker to use */
  for( i = 0, found = 0; ( i < config->unpackCount ) && !found; i++ )
  {
    fseek( f, config->unpack[i].offset, config->unpack[i].offset >= 0 ? SEEK_SET : SEEK_END );
    if( ferror( f ) )
    {
      w_log( LL_ERROR, "Seek error on %s: %s", tmpfilename, strerror( errno ) );
      break;
    }
    for( found = 1, j = 0; j < config->unpack[i].codeSize; j++ )
    {
      if( ( getc( f ) & config->unpack[i].mask[j] ) != config->unpack[i].matchCode[j] )
        found = 0;
    }
    if( found )
      break;
  }

  fclose( f );

  if( !found )
  {
    w_log( LL_ERROR, "Did not find an appropriate unpacker for %s", tmpfilename );
    nfree( tmpfilename );
    return 0;
  }

  /* create the unpack command */
  /* unpack_command(cmd,config->unpack[i].call, directory, filename, tempdir); */
  fillCmdStatement( cmd, config->unpack[i].call, tmpfilename, "", tempdir );

  /* execute the unpacker */
  w_log( LL_EXEC, "Executing '%s'", cmd );

  if( fc_stristr( config->unpack[i].call, ZIPINTERNAL ) )
  {
    exitcode = 1;
#ifdef USE_HPTZIP
    exitcode = UnPackWithZlib( tmpfilename, NULL, tempdir );
#endif
  }
  else
  {
    exitcode = cmdcall( cmd );
  }

  if( exitcode != 0 )
  {
    w_log( LL_EXEC, "Unpacker returned exit code '%d'", exitcode );
    nfree( tmpfilename );
    return 0;
  }
  adaptcase_refresh_dir( tempdir );

  nfree( tmpfilename );
  return 1;
}

static char *get_uncompressed_filename( s_fidoconfig * config,
                                        char *directory, char *filename,
                                        char *tempdir, int expday, int *reason )
{
  size_t l = strlen( filename );
  char *rv;

  w_log( LL_FUNC, "get_uncompressed_filename()" );

  if( reason )
  {
    *reason = 0;                /* means: all sorts of errors */
  }

  assert( l >= 4 );             /* a match should never happen w/o ".???" at the end */

  if( isdigit( filename[l - 3] ) && expday == atoi( filename + l - 3 ) )
  {
    /* the file is not compressed and the day number in the file
       name matches the expected day number */

    w_log( LL_DEBUG, "File '%s' is not compressed", filename );

    if( ( rv = malloc( strlen( directory ) + l + 1 ) ) == NULL )
    {
      w_log( LL_ERROR, "Out of memory" );
      w_log( LL_FUNC, "get_uncompressed_filename() failed" );
      return NULL;
    }
    strcpy( rv, directory );
    strcat( rv, filename );
    w_log( LL_FUNC, "get_uncompressed_filename() OK" );
    return rv;
  }
  else if( expday % 100 == atoi( filename + l - 2 ) )
  {
    /* the file is compressed and its name looks like it could match */

    w_log( LL_DEBUG, "File '%s' is compressed", filename );

    if( !uncompress( config, directory, filename, tempdir ) )
    {
      w_log( LL_DEBUG, "Uncompress failed" );
      return NULL;
    }

    if( ( rv = malloc( strlen( tempdir ) + l + 1 ) ) == NULL )
    {
      w_log( LL_ERROR, "Out of memory" );
      w_log( LL_FUNC, "get_uncompressed_filename() failed" );
      return NULL;
    }
    strcpy( rv, tempdir );
    strcat( rv, filename );
    sprintf( rv + strlen( tempdir ) + l - 3, "%03d", expday % 1000 );

    adaptcase( rv );
    w_log( LL_DEBUG, "Expected uncompressed filename after adaptcase(): %s", rv );
    if( access( rv, F_OK | R_OK ) )
    {
      w_log( LL_WARN, "Uncompressed file '%s' does not exist", rv );
      free( rv );
      w_log( LL_FUNC, "get_uncompressed_filename() failed" );
      return NULL;
    }
    w_log( LL_FUNC, "get_uncompressed_filename() OK" );
    return rv;
  }
  else
  {
    /* no match */

    w_log( LL_DEBUG, "File suffix %d does not match exp day %d", atoi( filename + l - 2 ), expday );

    if( reason )
    {
      *reason = 1;              /* means: file extension simply doesn't match */
    }

    w_log( LL_FUNC, "get_uncompressed_filename() failed" );
    return NULL;
  }
}

static char *basedir( const char *stemname )
{
  char *rv;
  size_t l;
  int j;

  l = strlen( stemname );

  if( ( rv = malloc( l + 1 ) ) == NULL )
  {
    return NULL;
  }
  strcpy( rv, stemname );

  for( j = l - 1; j >= 0 && rv[j] != '/' && rv[j] != '\\' && rv[j] != ':'; j-- )
  {
    rv[j] = '\0';
  }

  return rv;
}

static int call_differ( char *nodelist, char *nodediff )
{
  char *cmd = NULL;
  int rv;

  xscatprintf( &cmd, "%s -n %s %s", differ, nodelist, nodediff );
  w_log( LL_EXEC, "Executing diff processor (%s)", cmd );
  rv = system( cmd );
  nfree( cmd );
  if( rv != 0 )
  {
    w_log( LL_ERROR, "Diff processor returned exit code '%d'", rv );
    return 0;
  }
  return 1;
}



/* Purpose of try_full_update:

   fullbase + match might contain a full update for rawnl, with julian
   being the julian day number that we expect to find in fullbase + match.
   try to unpack the update and see if it really contains this day number, and
   if so, use it.

   Return values: 0: no hit, no error
                  1: fatal error
                  2: hit, but non-fatal error
                  3: hit

*/

static int try_full_update( s_fidoconfig * config, char *rawnl, char *fullbase,
                            nlist * fulllist, int j, char *tmpdir, long julian, int nl )
{
  char *ufn;
  char *newfn;
  int ndnr;
  int rv = 0;
  int reason;

  w_log( LL_FUNC, "try_full_update()" );

  if( fulllist->julians[j] == -1 )
  {
    w_log( LL_FUNC, "try_full_update() failed (fulllist->julians[j]==-1)" );
    return 0;                   /* this full update has an unknown date number */
  }

  decode_julian_date( julian, NULL, NULL, NULL, &ndnr );

  ufn = get_uncompressed_filename( config, fullbase, fulllist->matches[j], tmpdir, ndnr, &reason );

  w_log( LL_FILENAME, "Uncomressed filename: %s", ufn );

  if( !reason )
    fulllist->julians[j] = -1;

  if( ufn != NULL )
  {
    if( julian == ( fulllist->julians[j] = parse_nodelist_date( ufn ) ) )
    {
      w_log( LL_INFO, "Found full update: %s", ufn );

      newfn = malloc( strlen( config->nodelistDir ) +
                      strlen( config->nodelists[nl].nodelistName ) + 5 );

      if( newfn == NULL )
      {
        w_log( LL_ERROR, "Out of memory" );
        w_log( LL_FUNC, "try_full_update() failed" );
        return 1;
      }
      else
      {
        strcpy( newfn, config->nodelistDir );
        strcat( newfn, config->nodelists[nl].nodelistName );
        sprintf( newfn + strlen( newfn ), ".%03d", ndnr );

        w_log( LL_FILE, "Copy '%s' to '%s'", ufn, newfn );

        if( copy_file( ufn, newfn, 1 ) )
        {
          w_log( LL_ERROR, "Error copying '%s' to '%s'", ufn, newfn );
          w_log( LL_FUNC, "try_full_update() failed" );
          return 1;
        }
        else
        {
          rv = 3;
          if( rawnl != NULL )
          {
            w_log( LL_FILE, "Replacing '%s' with '%s'", rawnl, newfn );

            if( remove( rawnl ) )
            {
              w_log( LL_ERROR, "Cannot remove '%s'", rawnl );
              rv = 2;           /* error, but still proceed */
            }
          }
          else
          {
            w_log( LL_CREAT, "Creating '%s'", newfn );
          }

        }
        free( newfn );
      }
    }
    else
    {
      w_log( LL_ALERT, "%s does not contain nodelist for day %03d", ufn, ndnr );
    }

    free( ufn );
  }
  w_log( LL_FUNC, "try_full_update() OK" );
  return rv;
}



/* Do_update finds an update for a already existing unpacked nodelist. it first
   parses the day number of the existing nodelist instance, then increases it
   by steps of 7, and looks for matching diff or full updates. If there is no
   existing instance of the nodelst, you must use create_instance instead.

   today is passed as argument, because we only want to call julian_today()
   once. Imagine nlupdate being started at 23:59:59 .... */

static int do_update( s_fidoconfig * config, int nl, char *rawnl, long today, char *tmpdir )
{
  long julian, i;
  int ndnr;
  int checkdiff = 1;
  int j = 0;
  int rv = 1;
  char *diffbase = NULL, *fullbase = NULL;
  nlist *difflist = NULL, *fulllist = NULL;
  int hit;
  char *ufn = NULL, *cfn = NULL;

  w_log( LL_FUNC, "do_update()" );

  if( ( julian = parse_nodelist_date( rawnl ) ) == -1 )
    return 0;

  if( today - julian < 7 )
  {
    w_log( LL_INFO, "%s younger than 7 days, no update necessary", rawnl );
    return 1;
  }

  /* build the list of candidate nodediff files */
  if( config->nodelists[nl].diffUpdateStem != NULL )
  {
    if( ( diffbase = basedir( config->nodelists[nl].diffUpdateStem ) ) != NULL )
    {
      difflist = find_nodelistfiles( diffbase,
                                     config->nodelists[nl].diffUpdateStem + strlen( diffbase ), 1 );
    }
    else
      difflist = NULL;
  }

  /* build the list of candidate full update files */
  if( config->nodelists[nl].fullUpdateStem )
  {
    if( ( fullbase = basedir( config->nodelists[nl].fullUpdateStem ) ) != NULL )
    {
      fulllist = find_nodelistfiles( fullbase,
                                     config->nodelists[nl].fullUpdateStem + strlen( fullbase ), 1 );
    }
    else
      fullbase = NULL;
  }

  if( !fulllist && !difflist )
  {
    if( config->nodelists[nl].fullUpdateStem )
    {
      if( !access( config->nodelists[nl].fullUpdateStem, F_OK ) )
      {
        w_log( LL_INFO, "Full update '%s' not an FTS5000 compatible",
               config->nodelists[nl].fullUpdateStem );
        xstrscat( &ufn, config->nodelistDir, config->nodelists[nl].nodelistName, NULL );

        w_log( LL_FILE, "Copy '%s' to '%s'", config->nodelists[nl].fullUpdateStem, ufn );

        if( copy_file( config->nodelists[nl].fullUpdateStem, ufn, 1 ) )
        {
          w_log( LL_ERROR, "Error copying '%s' to '%s'", config->nodelists[nl].fullUpdateStem,
                 ufn );
          w_log( LL_FUNC, "do_update() failed" );
          nfree( ufn );
          return 0;
        }
        else
        {
          w_log( LL_FUNC, "do_update() OK" );
          nfree( ufn );
          return 1;
        }
      }
      else
      {
        w_log( LL_INFO, "Full update '%s' not found", config->nodelists[nl].fullUpdateStem );
        w_log( LL_FUNC, "do_update() failed" );
        return 0;
      }
    }
  }

  /* search for diffs or full updates */

  for( i = julian + 7; i <= today && rawnl; i += 7 )
  {
    decode_julian_date( i, NULL, NULL, NULL, &ndnr );
    hit = 0;

    if( checkdiff && difflist != NULL ) /* search for diffs */
    {
      for( j = 0; j < difflist->n && !hit; j++ )
      {

        w_log( LL_DEBUG, "Checking %s%s", diffbase, difflist->matches[j] );

        ufn = get_uncompressed_filename( config, diffbase,
                                         difflist->matches[j], tmpdir, ndnr, NULL );

        w_log( LL_DEBUG, "ufn is: %s", ufn );

        if( ufn != NULL )
        {

          /* nodediffs contain the header of the nodelist to which
             they apply as the first line, so we need to compare the
             parsed date with julian, not with i */

          if( julian == parse_nodelist_date( ufn ) )
          {
            w_log( LL_INFO, "Found diff update: %s", ufn );

            if( call_differ( rawnl, ufn ) )
            {
              hit = 1;
              difflist->applied[j] = 1;
            }
          }
          else
          {
            w_log( LL_INFO, "%s is not applicable to %s", rawnl, ufn );
          }
          free( ufn );
        }
      }
    }

    if( !hit )
    {
      if( ( fulllist == NULL ) && config->nodelists[nl].fullUpdateStem )
      {
        /* non-standard full update nodelist file */

#if 0
        if( add_match( fulllist, config->nodelists[nl].fullUpdateStem ) )
        {
          switch ( try_full_update( config, rawnl, fullbase, fulllist, j, tmpdir, i, nl ) )
          {
          case 0:              /* no hit, no error */
            break;
          case 1:
            i = today;          /* fatal error; stop searching! */
            rv = 0;
            break;
          case 2:
            rv = 0;             /* hit, minor error, but don't exit loop! */
            hit = 1;
            break;
          case 3:
            hit = 1;            /* hit, no errors; */
            break;
          }
        }
#endif
      }
      else if( fulllist != NULL )
      {                         /* search for fulls */
        for( j = 0; j < fulllist->n && !hit; j++ )
        {

          w_log( LL_DEBUG, "Checking %s%s", fullbase, fulllist->matches[j] );

          switch ( try_full_update( config, rawnl, fullbase, fulllist, j, tmpdir, i, nl ) )
          {
          case 0:              /* no hit, no error */
            break;
          case 1:
            i = today;          /* fatal error; stop searching! */
            rv = 0;
            break;
          case 2:
            rv = 0;             /* hit, minor error, but don't exit loop! */
            hit = 1;
            break;
          case 3:
            hit = 1;            /* hit, no errors; */
            break;
          }
        }
      }
    }

    if( hit )                   /* analyse the new nodelist */
    {
      if( rawnl != NULL )
        free( rawnl );
      rawnl = findNodelist( config, nl );
      if( rawnl == NULL )
      {
        w_log( LL_ERROR,
               "Instance of nodelist %s vanished during update!?",
               config->nodelists[nl].nodelistName );
      }
      else if( ( i = parse_nodelist_date( rawnl ) ) == -1 )
      {
        free( rawnl );
        rawnl = NULL;
      }
      else if( i <= julian )
      {
        w_log( LL_ERROR, "Nodelist %s still as old as before!?", rawnl );
        rv = 0;
        i = today;              /* exit loop :-( */
      }
      else
      {
        julian = i;
      }
      checkdiff = 1;
    }
    else
    {
      checkdiff = 0;
    }
  }

  if( config->nodelists[nl].delAppliedDiff )
    for( j = 0; j < difflist->n; j++ )
      if( difflist->applied[j] )
      {
        xstrscat( &cfn, diffbase, difflist->matches[j], NULL );
        if( remove( cfn ) )
        {
          w_log( LL_ERROR, "Cannot delete file '%s': %s", cfn, strerror( errno ) );
        }
        else
          w_log( LL_DEL, "File '%s' removed", cfn );
        nfree( cfn );
      }

  if( fullbase != NULL )
    free( fullbase );
  if( diffbase != NULL )
    free( diffbase );
  free_nlist( fulllist );
  fulllist = NULL;

  /* check if nodelist is extraordinarily old, just for information */
  if( today - julian > 14 )
  {
    w_log( LL_ALERT, "%s is more than two weeks old, but still no way to update it", rawnl );
  }

  free( rawnl );
  return rv;
}

/* Create_instance is used instead of do_instance if there is not yet any
   unpacked instance of the nodelist in the nodelist directory. In this case,
   of course, diff updates are useless, and we only search for full
   updates. Create_instance will start at the current day number, and then
   decrease it in steps of 1 until it finds a matching full update.

   today is passed as argument, because we only want to call julian_today()
   once. Imagine nlupdate being started at 23:59:59 .... */

static int create_instance( s_fidoconfig * config, int nl, long today, char *tmpdir )
{
  long i;
  int j;
  int rv = 0;
  char *fullbase = NULL;
  nlist *fulllist = NULL;
  int hit;

  /* build the list of candidate full update files */

  w_log( LL_FUNC, "create_instance()" );

  if( config->nodelists[nl].fullUpdateStem )
  {
    if( ( fullbase = basedir( config->nodelists[nl].fullUpdateStem ) ) != NULL )
    {
      fulllist = find_nodelistfiles( fullbase,
                                     config->nodelists[nl].fullUpdateStem + strlen( fullbase ), 1 );
    }
    else
      fullbase = NULL;
  }
  else
  {
    w_log( LL_ERR, "fullupdate not present" );
    w_log( LL_FUNC, "create_instance() failed" );
    return 0;
  }

  if( fulllist == NULL )
  {                             /* non-standard full update nodelist file */
    if( !access( config->nodelists[nl].fullUpdateStem, F_OK ) )
    {
      w_log( LL_INFO, "Full update '%s' not an FTS5000 compatible",
             config->nodelists[nl].fullUpdateStem );
      nfree( fullbase );
      xstrscat( &fullbase, config->nodelistDir, config->nodelists[nl].nodelistName, NULL );

      w_log( LL_FILE, "Copy '%s' to '%s'", config->nodelists[nl].fullUpdateStem, fullbase );

      if( copy_file( config->nodelists[nl].fullUpdateStem, fullbase, 1 ) )
      {
        w_log( LL_ERROR, "Error copying '%s' to '%s'", config->nodelists[nl].fullUpdateStem,
               fullbase );
        w_log( LL_FUNC, "create_instance() failed" );
        nfree( fullbase );
        return 0;
      }

      nfree( fullbase );
      w_log( LL_FUNC, "create_instance() OK" );
      return 1;
    }
    else
    {
      w_log( LL_INFO, "Full update '%s' not found", config->nodelists[nl].fullUpdateStem );
      w_log( LL_FUNC, "create_instance() failed" );
      return 0;
    }
  }

  /* search for diffs or full updates */
  hit = 0;
  for( i = today; i > today - 10 * 366 && !rv; i-- )
  {
    if( ( fulllist != NULL ) && ( !hit ) )
    {                           /* search for fulls */
      for( j = 0; j < fulllist->n && !hit; j++ )
      {
        switch ( try_full_update( config, NULL, fullbase, fulllist, j, tmpdir, i, nl ) )
        {
        case 0:                /* no hit, no error */
          break;
        case 1:
          i = today - 10 * 366; /* fatal error; stop searching! */
          rv = 0;
          break;
        case 2:
        case 3:
          rv = 1;               /* hit */
          break;
        }
      }
    }
  }

  if( fullbase != NULL )
    free( fullbase );

  w_log( LL_FUNC, "create_instance() rc=%d", rv );
  return rv;
}


static int process( s_fidoconfig * config )
{
  UINT i;
  int rv = 0;
  char *nodelist;
  char *tmpdir;
  long today = julian_today(  );

  if( config->nodelistDir == NULL )
  {
    w_log( LL_CRIT, "No nodelist directory configured in fidoconfig" );
    return 8;
  }

  if( config->nodelistCount < 1 )
  {
    w_log( LL_CRIT, "No nodelist configured in fidoconfig" );
    return 8;
  }

  if( ( tmpdir = mk_uncompressdir( config->nodelistDir ) ) != NULL )
  {
    for( i = 0; i < config->nodelistCount; i++ )
    {
      nodelist = findNodelist( config, i );

      if( nodelist == NULL )
      {
        w_log( LL_ALERT, "No instance of nodelist %s found.", config->nodelists[i].nodelistName );

        /* New: If there is no instance, we can still try
           to find a full update! */

        if( config->nodelists[i].fullUpdateStem != NULL )
        {
          w_log( LL_INFO, "Check fullupdate: %s", config->nodelists[i].fullUpdateStem );
          if( !create_instance( config, i, today, tmpdir ) )
          {
            w_log( LL_ALERT,
                   "no full update for nodelist %s found.", config->nodelists[i].nodelistName );
            if( rv < 4 )
              rv = 4;
          }
          else
          {
            nodelist = findNodelist( config, i );
            if( nodelist == NULL )
            {
              w_log( LL_ERROR, "Unpacked full update, but "
                     "still no instance of nodelist %s !?", config->nodelists[i].nodelistName );
              if( rv < 8 )
                rv = 8;
            }
          }
        }
        else
        {
          w_log( LL_ALERT, "Don't know how to create instance"
                 " of nodelist %s (no full update configured)", config->nodelists[i].nodelistName );
          if( rv < 4 )
            rv = 4;
        }
      }

      if( nodelist != NULL )
      {
        if( config->nodelists[i].diffUpdateStem == NULL &&
            config->nodelists[i].fullUpdateStem == NULL )
        {
          w_log( LL_ALERT, "Don't know how to update "
                 "nodelist %s", config->nodelists[i].nodelistName );
          if( rv < 4 )
            rv = 4;
          free( nodelist );
        }
        else
        {
          w_log( LL_INFO, "Trying to update %s", nodelist );

          if( !do_update( config, i, nodelist, today, tmpdir ) )
          {
            if( rv < 8 )
              rv = 8;
          }
        }
      }
    }

    destroy_uncompressdir( tmpdir );
  }

  w_log( LL_FUNC, "%s::process() done", __FILE__ );
  return rv;
}

void usage()
{
  printf(
      "USAGE:\n"
      "\tnlupdate [-h] [-v] [-q] [-c config]\n"
      "where:\n"
      "\t-h\t - print this help and exit;\n"
      "\t-v\t - print version information and exit;\n"
      "\t-q\t - quiet mode (supress normal output, print errors only).\n"
      "\t-c config - read configuration from alternate fidoconfig file.\n"
        );
}

int main( int argc, char **argv )
{
  s_fidoconfig *config;
  int rv;
  int l = 0, i, flag_quiet = 0;
  char *versionStr, *configfile = NULL;

  versionStr = GenVersionStr( "nlupdate", VER_MAJOR, VER_MINOR, VER_PATCH, VER_BRANCH, cvs_date );

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
          usage(  );
          return 0;
        case 'v':
          printversion(  );
          return 0;
        case 'c':
          if( plen > ++j )
          { configfile = argv[i] + j;
            j=plen;
          }else if( argc > ++i )
            configfile = argv[i];
          else
          {
            w_log( LL_ERR, "Fatal: parameter after \"-c\" is required\n" );
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

  /* construct the name of the nldiff command */
  if( argc )
  {
    l = strlen( argv[0] );
    if( l )
    {
      for( l--; l >= 0 && argv[0][l] != '/' && argv[0][l] != '\\' && argv[0][l] != ':'; l-- );

      l++;
    }
  }
  differ = malloc( l + 7 );
  if( l )
    memcpy( differ, argv[0], l );
  strcpy( differ + l, "nldiff" );

  /* run the main program */
  if( config != NULL )
  {
    {
      char errloglevels[3] = { LL_ERR, LL_CRIT, '\0' };
      initLog( config->logFileDir, config->logEchoToScreen, config->loglevels,
             flag_quiet? errloglevels : config->screenloglevels );
    }
    openLog( LOGNAME, versionStr );
    w_log( LL_START, "Start" );

    rv = process( config );

    w_log( LL_STOP, "End" );
    closeLog(  );
    disposeConfig( config );
    free( differ );
    return rv;

  }
  else
  {
    w_log( LL_ERR, "Fatal: Cannot open fidoconfig.\n" );
    free( differ );
    return 8;
  }
}
