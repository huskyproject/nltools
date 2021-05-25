# nltools
[![Build Status](https://travis-ci.org/huskyproject/nltools.svg?branch=master)](https://travis-ci.org/huskyproject/nltools)
[![Build status](https://ci.appveyor.com/api/projects/status/w31hn6m0xwqbh8c1/branch/master?svg=true)](https://ci.appveyor.com/project/dukelsky/nltools/branch/master)
[![Codacy Badge](https://api.codacy.com/project/badge/Grade/c1970dc5395945839afbae7c79b66a3a)](https://www.codacy.com/app/huskyproject/nltools?utm_source=github.com&amp;utm_medium=referral&amp;utm_content=huskyproject/nltools&amp;utm_campaign=Badge_Grade)

Husky Nodelist Utilities Documentation
======================================

Written 2000 by Tobias Ernst and released to the Public Domain.
Updated by Husky Team.

## I) Abstract

This archive contains a set of nodelist management utilities, in source code
and, if you got a full distribution, also with binaries for some common
operating systems.  The following functionality is provided:

- Compiling raw nodelists into a FIDOUSER.LST file, that can be used for
  nodelist lookup by sysop name, as supported by Msged and others.

- Keeping your raw nodelists up to date, i.E. logic is provided to find the
  nodediff files that apply to your current nodelist files, and a tool to 
  apply nodediffs to nodelists.

This package does NOT presently include something like a V7 nodelist
compiler.  Therefore, this package is enough if you are running an IP node,
but you need additional software if you are running Binkley or some other
software that requires an V7 nodelist index.

The tools can be compiled on Unix, as well as a lot of other platforms (OS/2,
Win32, DOS/DJGPP), provided you have a working fidoconfig library and
installation for that platform.

A major advantage of these tools on Unix is that they have no case
sensitivity problems. I.e. nodelist files and nodelist updates are found no
matter if they are in lower, capital, or MiXeD case.

## II) About Fidoconfig

These tools are written as an addition to the Husky suite of Fidonet
software.  The tools do not read a separate configuration file, but rather
they read a common, global "Fidoconfig" configuration file.  This does not
mean that you need to install the whole Husky suite - you can just use the
programs of this package stand alone if you like, because the precompiled
binaries have the fidoconfig linked in statically.

But if you want to compile the tools, you do need the 
[fidoconfig](https://github.com/huskyproject/fidoconf) library. 
If you do not know what Fidoconfig is, visit 
[Husky project](https://github.com/huskyproject) or its 
[homepage](https://huskyproject.github.io/).
This release of the nltools is designed to be built against fidoconfig. 
You will also require the [huskylib](https://github.com/huskyproject/huskylib)
and [smapi](https://github.com/huskyproject/smapi) libraries because of some 
header files only found therein.

If you only want to use the precompiled binaries, you must set the
environment variable FIDOCONFIG to point to a fidoconfig-style configuration
file, e.g. on OS/2, put SET FIDOCONFIG=e:\bbs\etc\fidoconf.cfg in your
CONFIG.SYS file.  On DOS, put the same command into autoexec.bat

The keywords that you can use in this configuration file will be explained
below, in section V, or of course in the fidoconfig manual.

## III) General Considerations

These tools expect the Nodelist and Nodediff files to be in the FTS-5000 (and
obsoleted FTS-0005) defined format, that means:  text lines that are finished
with a CRLF sequence, and an EOF character after the last CRLF.  This is
normally true on any DOSish computer, and also on a Unix system if you just
extract the distribution archives of Nodelist and Nodediff without giving some
sort of auto conversion option.  You might loose the CRs (^Ms) at the end when
working with a text editor on these files, though.  If the files do not have
the ^M at the line end, the CRC checks will fail.

## IV) About the tools

### a) nlcrc

   You normally do not need to call nlcrc manually, unless you are curious.

   This simple tool checks the CRC checksum in a given Nodelist (not
   Nodediff!) file. Simply invoke it with the filename as argument:

      nlcrc NODELIST.260

   If there is not any output, the check succeeded and the return code will
   be zero.  If the file does not contain a CRC checksum, a message will be
   printed to stderr and the return code will be 4.  If a file I/O error or
   similar happens, a message will be printed to stderr and the return code
   will be 8.  If the file has a checksum, but the check fails, a message
   will be printed to stderr and the return code will be 16.


### b) nldiff

   You normally do not need to call nldiff manually. nldiff is designed to be
   called automatically by nlupdate.

   This tool applies a Nodediff file to a given Nodelist.  It has no
   intelligence as to determining which of multiple Nodediff files is the
   correct one (you have to use other tools for this), but it expects the
   Nodelist filename and the Nodediff filename as explicit arguments with the
   correct day file name extension, as for example in:

      nldiff NODELIST.260 NODEDIFF.267

   This will crate a new file NODELIST.267.

   If you want the old file (NODELIST.260 in this case) to be deleted if the
   process succeeds (this means that no I/O errors occured and that the new
   nodelist file passes a CRC check), you can give the -n parameter:

      nldiff -n NODELIST.260 NODEDIFF.267

   If you also want the nodediff file (NODEDIFF.260) to be deleted, you can
   specify the -d parameter:

      nldiff -d -n NODELIST.260 NODEDIFF.267

### c) ulc

   ulc is the Husky Fido Userlist Compiler. ulc reads all nodelists that are
   configured in Fidoconfig (via the "nodelist" keyword) and creates the
   FIDOUSER.LST file (the name has to be configured with the "fidouserlist"
   keyword). ulc does not take any command line options; it uses fidoconfig
   to determine where to find the nodelist files. A log file named
   "nltools.log" is placed in the fidoconfig log file directory.

   The FIDOUSER.LST file format is defined as follows: The file consists of
   text record of fixed length (65 characters including the terminal \r\n
   sequence). The name of the sysop is left-aligned in the line in reverse
   order (e.g. "Tobias_Ernst" would become "Ernst, Tobias"). Aligned to the
   right of the record is the node number of the user. The records are sorted
   alphabetically, so that a program can use a binary search algorithm to
   find the corresponding node number for a given user name very fast.

   The FIDOUSER.LST file format is supported by many mail readers, e.g. Timed
   and Msged. For Msged, FIDOUSER.LST is currently the best method to
   implement a node lookup at all, because Msged's V7 routines are flawed.

### d) nlupdate

   Nlupdate manages your nodelists and keeps them up to date. For each
   nodelist that you have configured with a "Nodelist" statement (see
   below), it will search the latest nodelist in the "NodelistDir", then
   caluclate the day number of the difference file that is needed to 
   update this nodelist (the algorithm is Y2K safe and knows that 2000 is a
   leap year), then searches the difference file, unpacks it if necessary,
   and applies it to the nodelist using nldiff. nlupdate can also find full
   replacement files if you configure this, and just unpack them and copy
   them over the old nodelist. This is useful for othernets that do not have
   nodediff files. nlupdate can also process daily nodelists full updates if
   you configure this.  - With only a few keywords, nlupdate manages the whole
   process of updating your nodelist files for you.

After you have set the configuration file up properly (see below), you just
need to put the two commands

   nlupdate
   ulc
   ;call your V7 index generator here if necessary

into your weekly maintainance script and all the nodelist tasks are done for
you.

## V) CONFIGURATION SYNTAX

The following text describes the configuration statements in the fidoconfig
file that control the behaviour of ulc and nlupdate.  Just put the
appropriate configuration statements into a text file and point the
FIDOCONFIG environment variable to this file, or add the statements to your
existing fidoconfig file if you have one.
```
   LogFileDir
   ----------
   Syntax:   logFileDir <path>
   Example:  logFileDir /var/spool/fido/log

   Where the logfile goes. This is a *path* name, the filename inside this
   path is hardcoded to be "nltools.log".

   MsgBaseDir
   ---------
   Syntax:  MsgBaseDir <patch>
   Example: MsgBaseDir /var/spool/fido/msgbase

   The nodelist tools do not use this, but you need to have it in the config
   file. Just point it to any existing path. (Other Husky tools would place
   files that store the message base in this directory).

   NodelistDir
   -----------
   Syntax:   nodelistDir <path>
   Example:  nodelistDir /var/spool/fido/nodelist

   This command specifies the path where the actual nodelists are or should
   be written to.  This path contains the raw nodelist. Also, compiled
   nodelists like the FIDOUSER.LST will be stored here.
   This statement cannot be repeated.

   Unpack
   ------
   Syntax:  Unpack "<unpacker command>" <id pos> <id bytes>
```   
   This configures the unpackers to use. It is crucial that you get these
   lines right. For details, please consider the Fidoconfig manual. Below, I
   simply give examples that work. I assume that you use the Freeware unrar
   (available for OS/2, DOS, Windows, and in Source for Unix), the Freeware
   Infozip unzip program (dito), and ARC 5.21. This last packer is required
   for Fidonet node diffs especially; you can get it by f'requesting
   ARC521_2.ZIP at 2:2476/418. This file contains a family mode executable 
   (DOS + OS/2, also runs on Windows 95 and NT), as well as source code that 
   can be compiled on Unix without major problems.

   On Unix/Linux, use the following (adapt the path if necessary):
```
   Unpack "/usr/local/bin/unzip -joLqq $a -d $p" 0 504b0304 
   Unpack "/usr/local/bin/unrar e -o+ -y -c- -p- $a $p/ >/dev/null" 0 52617221
   Unpack "/usr/local/bin/arc eno $a $p'/*.*'" 0 1a
```
   On DOS, OS/2 and Windows, use the following:
```
   Unpack "unzip -joLqq $a -d $p" 0 504b0304 
   Unpack "unrar e -o+ -y -c- -p- $a $p\ >/dev/null" 0 52617221
   Unpack "arc eno $a $p\*.*" 0 1a
```
```
   FidoUserList
   -------------
   Syntax:   fidoUserList <filename>
   Example:  fidoUserList fidouser.lst
```
   If this keyword is present, the nodelist compiler (ulc) is instructed to
   build a user list file with the given filename in the nodelist directory
   (see nodelistdir).  This is a simple text file with fixed line length that
   contains user names (nodes, points) and their corresponding node or
   pointnumbers.  The file is sorted alphabetically by user name (case
   insensitive), so that it can be bsearched to implement a quick node numer
   lookup functinality.  The fido user list file format is understood by
   Msged, for example.
```
   NodeList
   --------
   Syntax:  Nodelist <name>
   Example: Nodelist points24
```
   This statement starts a new nodelist definition.  All the following
   nodelist-related stamtements change the configuration of this nodelist
   until a new nodeelist statement is found.

   The name that you specify must match the base name (without extension and
   without pathname) of the raw, unpacked, nodelist file.  The husky tools
   ulc and nlupdate match the file name case-insensitively, but other tools
   may need the exact spelling.  The raw nodelist file is expected to reside
   in the nodelist directory (see nodelistdir)-
```
   DiffUpdate
   ----------
   Syntax:  DiffUpdate <path_and_basename>
   Example: DiffUpdate /var/spool/filebase/nodediff/nodediff
```
   Here you can specify the base filename of nodelist difference files
   (nodediffs) that are used to keep the corresponding nodelist up to date.
   The argument to the DiffUpdate is the full file name with path of a
   difference file, without the file extension.  For example, if you have a
   file area at /var/spool/filebase/24000, where your ticker places the
   updates for the German Pointlist, and those update files are called
   points24.a26, points24.a33, and so on, you would use
```
       DiffUpdate /var/spool/filebase/24000/points24
```
   The Diffupdate keyword is used by nlupdate, for example.  The nodelist
   updater will unpack the difference file (if it is archived, of course,
   unpacked diffs are also supported), apply the diff to the corresponding
   nodelist, and delete the temporary unpacked diff again.
```
   FullUpdate
   ----------
   Syntax:  FullUpdate <path_and_basename>
   Example: FullUpdate /var/spool/filebase/nodelist/nodelist
```
   This statement works like DiffUpdate.  The difference is that here you
   don't specify the location of a nodelist difference file, but the
   locations where complete nodelist files/archives can be found.  Some
   othernets do not (regularly) distribute a nodediff file, but just hatch a
   new nodelist every few weeks.  In this case, you need the FullUpdate
   statement.
```
   Dailynodelist
   ----------
   Syntax:  Dailynodelist <On/Off>
   Example: Dailynodelist On
```
   This statement tells nlupdate to process a daily nodelist. You have
   to set "FullUpdate" and "Dailynodelist" statements and not the "DiffUpdate"
   statement. With "Dailynodelist" set to "On", nlupdate does a full update of
   the nodelist on a daily basis, instead of checking if the new nodelist is 
   7 days newer.
```
   Defaultzone
   -----------
   Syntax:  DefaultZone <zone>
   Example: DefaultZone 2
```
   Some nodelist files do not start with a Zone entry.  This is the case for
   the German Points24 list, for example, but could also happen for othernets
   that only have one zone.  In this case, you can use the DefaultZone
   keyword to specify the default zone number for all nodes listed in this
   nodelist.
```
   Nodelistformat
   --------------
   Syntax:  Nodelistformat <format>
   Example: NodelistFormat Standard
   Example: NodelistFormat Points24
   Example: NodelistFormat Points4D
```
   Here you can specify the format of the unpacked nodelist.  The default is
   "standard":  this is the normal Fidonet nodelist format.  You can also
   specify "points24", which is needed for the nodelist compiler to recognise
   a point list in the German points24 format as such, so that it can see the
   proper 4D point numbers instead of the fakenet numbers. Or you can specify
   "points4d", which means a 4D point list with "Boss" entries.


## VI) SAMPLE CONFIG

The following lines show a sample configuration file.  If you are only
interested in the nodelist tools, you can just copy those commands into a
text file, modify them according to your needs (OS/2, Win and DOS users may
of course use backslashes instead of forward slashes, and drive letters), and
point the FIDOCONFIG variable to this file.  If, on the other hand, you
already have a fidoconfig installtion, you can copy these commands into your
existing fidoconfig file.
```
   NodelistDir /var/spool/fido/nodelist
   FidoUserList fidouser.lst

   Nodelist nodelist
   DiffUpdate /var/spool/fido/filebase/nodediff/nodediff
   NodelistFormat Standard

   Nodelist points24
   DiffUpdate /var/spool/fido/filebase/24000/pr24diff
   DefaultZone 2
   NodelistFormat Points24
```
## VII) COMPILING

You can compile nltools as part of the Husky project using the top level
Makefile and a proper huskymak.cfg. This works particularly well and easy on
Linux and other Unixes. Instructions on how to do this can be found in the
"huskybse" package, in the file INSTALL.

The following instructions are for users that cannot use the huskymak.cfg
way, but must use legacy makefiles.

- Download and compile SMAPI and FIDOCONFIG.
- Extract nltools on the same level as fidoconfig, e.g. you could have:
    ~/fido/smapi
    ~/fido/fidoconfig
    ~/fido/nltools
- Change to the src subdirectory.
- Select the proper makefile and rename it to "makefile". Edit it. Set the 
  INSTDIR and LIBDIR variables according to your needs.
- Type "make"
- If you get an unresolved symbol error for "fexist", upgrade your
  fidoconfig source code to the latest git level.
- If it worked, type "make install" (Unix only)
- If you like, type "make clean".


## VIII) LICENCSE

These tools are donated to the Public Domain, which means that you can do with
with the SOURCE CODE whatever you want, but also that the author does not
take any responsibilites whatsoever.

In order to produce executables of the tools, you need the fidoconfig library
(and I needed it as well).  The fidoconfig library is not Public Domain, but
it is covered by the GNU LIBRARY GENERAL PUBLIC LICENSE.  A copy of this
license can be found in the "legal" subdirectory.  The binary executables of
the nodelist tools therefore are also covered by the LGPL.

In order to comply with the LGPL, I hereby invite you to download the source
code of both the nodelist tools and fidoconfig from
https://github.com/huskyproject.

## IX)  CONTACT

Questions are appropriate in the LINUX.FIDO.GER, FIDOSOFT.HUSKY, and
FIDO_UTIL Fidonet echo conferences.

For more information see the [Husky project](https://github.com/huskyproject)
and its [homepage](https://huskyproject.github.io/).
