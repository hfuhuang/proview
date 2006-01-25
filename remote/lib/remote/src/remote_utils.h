/* 
 * Proview   $Id: remote_utils.h,v 1.1 2006-01-12 06:39:33 claes Exp $
 * Copyright (C) 2005 SSAB Oxel�sund AB.
 *
 * This program is free software; you can redistribute it and/or 
 * modify it under the terms of the GNU General Public License as 
 * published by the Free Software Foundation, either version 2 of 
 * the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful 
 * but WITHOUT ANY WARRANTY; without even the implied warranty of 
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the 
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License 
 * along with the program, if not, write to the Free Software 
 * Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 */

#if defined __cplusplus
extern "C" {
#endif

/* Functions for serial lines */

int RemUtils_InitSerialDev(char *device, int speed, int databits, int stopbits, int parity);

/* Functions for radix 50 */

char RemUtils_ConvertR50ToAscii(int i);
int RemUtils_R50ToAscii(unsigned short R50[], char asc[]);
int RemUtils_AsciiToR50(char asc[], short R50[]);

#if defined __cplusplus
}
#endif
