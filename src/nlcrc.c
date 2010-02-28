/* $Id$
   Written 1999 by Tobias Ernst and released do the Public Domain.
   This file is part of NLTOOLS, the nodelist processor of the Husky fidonet
   software project.

   Tool to check the CRC value in a Fidonet Nodelist
   References: FTS-0005, FTS-5000
*/

#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>

#include <huskylib/huskylib.h>

#include "crc16.h"
#include "version.h"

enum
{ SCANCOLON, SCANDIGIT, CRCVAL, AFTERCRCVAL, FINISH };
enum
{ ok = 0, unknown_option = 1, illegal_parameter = 2, nodelist_without_crc = 4, error_nodelist =
    8, error_CRC = 16 } exit_codes;

#define PROGRAM "nlcrc"
char *versionStr = "";

unsigned short analyze_first_line( FILE * f )
{
  char c = 0;
  int state = SCANCOLON;
  unsigned short crc = 0;

  while( state != FINISH )
  {
    fread( &c, 1, 1, f );
    if( c == '\n' )
    {
      if( state < CRCVAL )
      {
        fprintf( stderr, "Warning: File does not contain checksum.\n" );
        exit( nodelist_without_crc );
      }
      else
      {
        state = FINISH;
      }
    }
    else if( isdigit( c ) )
    {
      switch ( state )
      {
      case SCANDIGIT:
        crc = c - '0';
        state = CRCVAL;
        break;
      case CRCVAL:
        crc = crc * 10U + ( c - '0' );
        break;
      default:;
      }
    }
    else if( c == ':' )
    {
      switch ( state )
      {
      case SCANCOLON:
        state = SCANDIGIT;
        break;
      default:;
      }
    }
    else
    {
      switch ( state )
      {
      case CRCVAL:
        state = AFTERCRCVAL;
        break;
      default:;
      }
    }
  }
  return crc;
}

#define BUFSZ 32768

unsigned short analyze_rest( FILE * f )
{
  static unsigned char buffer[BUFSZ + 1];
  size_t l;
  unsigned short crc;
  unsigned char *p = NULL;

  crc16_init( &crc );

  errno = 0;
  while( ( ( l = fread( buffer, 1, BUFSZ, f ) ) > 0 ) && p == NULL )
  {
    buffer[l] = '\0';
    p = ( unsigned char * ) strchr( ( char * ) buffer, 0x1A );

    if( p != NULL )
    {
      l = p - buffer;
    }

    crc16_process( &crc, buffer, l );
  }

  crc16_finalize( &crc );

  return crc;
}

void usage(  )
{
  printf( "\nUsage: " PROGRAM " <FILENAME>\n\n"
          "\tIf nothing is printed ans exit code is zero, the CRC was OK. If the CRC is not\n"
          "\tOK, an error message and a exit code 8 is given.\n" );
}

int main( int argc, char **argv )
{
  FILE *f;
  unsigned short should, is;
  int flag_quiet;
  char *nlname = NULL;

  versionStr = GenVersionStr( PROGRAM, VER_MAJOR, VER_MINOR, VER_PATCH, VER_BRANCH, cvs_date );

  {
    int i;
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
            return ok;
          case 'v':
            printversion(  );
            return ok;
          case 'q':
            flag_quiet = 1;
            break;
          default:
            printf( "Illegal option '%c'!\n\n", argv[i][j] );
            usage(  );
            return unknown_option;
          }
      }
      else if( !nlname )
        nlname = argv[i];
      else
      {
        printf( "Illegal call: supports only one nodelist file name!\n"
                "Detected filename: \"%s\"\n" "Extra parameter is \"%s\"\nExit.", nlname, argv[i] );
        return illegal_parameter;
      }
    }
    if( !nlname )
      printf( "Illegal call: nodelist filename is needed!\n" );
    return illegal_parameter;
  }

  if( !flag_quiet )
    printversion(  );

  if( ( f = fopen( nlname, "rb" ) ) == NULL )
  {
    fprintf( stderr, "Cannot open %s.\n", argv[1] );
    return error_nodelist;
  }

  should = analyze_first_line( f );
  is = analyze_rest( f );

  fclose( f );

  if( should != is )
  {
    fprintf( stderr, "Nodelist %s fails CRC check (%u != %u)\n",
             argv[1], ( unsigned ) should, ( unsigned ) is );
    return error_CRC;
  }

  return ok;
}
