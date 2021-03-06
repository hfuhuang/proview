/* 
 * Proview   $Id: co_cnv.h,v 1.2 2007-06-01 13:36:41 claes Exp $
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
#ifndef co_cnv_h
#define co_cnv_h

#if defined __cplusplus
extern "C" {
#endif

char *cnv_utf8_to_iso8859( char *utf8, size_t utf8_size);
char *cnv_iso8859_to_utf8( char *iso, size_t iso_size);

#if defined __cplusplus
}
#endif
#endif
