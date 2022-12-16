/* $Id$
   This file is part of NLTOOLS, the nodelist processor of the Husky fidonet
   software project.
*/

#ifndef _VERSION_H
#define _VERSION_H

#include "cvsdate.h"

/* basic version number */
#define nltools_VER_MAJOR  1
#define nltools_VER_MINOR  9
#define nltools_VER_PATCH  0
#define nltools_VER_BRANCH BRANCH_CURRENT

extern char      *versionStr;

#define printversion(  ) { printf( "%s\n\n", versionStr ); }

#endif
