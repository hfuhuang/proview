/* 
 * Proview   $Id: rt_statusmon.cpp,v 1.1 2007-05-11 15:04:14 claes Exp $
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

/* rt_statusmon.cpp Status Monitor */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "pwr.h"

static void usage()
{
  printf("\nUsage: rt_statusmon [-f windowmanager]");
}

int main( int argc, char *argv[])
{
  int i;
  int found = 0;
  char wmg[80];
  pwr_tFileName file;
  
  if ( argc > 1) {

    for ( i = 1; i < argc; i++) {
      if ( strcmp( argv[i], "-f") == 0) {
	if ( i+1 >= argc) {
	  usage();
	  exit(0);
	}
	found = 1;
	strcpy( wmg, argv[i+1]);
	i++;
      }
    }
  }
  if ( !found)
    strcpy( wmg, "gtk");

  strcpy( file, argv[0]);
  strcat( file, "_");
  strcat( file, wmg);

  execvp( file, argv);
}
