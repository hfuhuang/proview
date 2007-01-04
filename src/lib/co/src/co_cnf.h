/** 
 * Proview   $Id: co_cnf.h,v 1.3 2007-01-04 07:51:42 claes Exp $
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
 **/

#ifndef co_cnf_h
#define co_cnf_h

/* co_cnf.h
   Configuration file. */

#ifdef __cplusplus
extern "C" {
#endif


char *cnf_get_value( char *name, char *value);

#ifdef __cplusplus
}
#endif
#endif
