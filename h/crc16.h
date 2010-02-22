/* $Id$
   Written 1999 by Tobias Ernst and released do the Public Domain.
   This file is part of NLTOOLS, the nodelist processor of the Husky fidonet
   software project.

*/
#ifndef __CRC16_H
#define __CRC16_H

void crc16_init(unsigned short *crcptr);
void crc16_process(unsigned short *crcptr, const unsigned char *buffer, size_t length);
void crc16_finalize(unsigned short *crcptr);


#endif
