/* 
 * Proview   $Id: rt_bck_dump.c,v 1.2 2005-09-01 14:57:48 claes Exp $
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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "pwr.h"
#include "pwr_class.h"
#include "pwr_baseclasses.h"
#include "co_time.h"
#include "rt_gdh.h"
#include "rt_bckdef.h"

int main (int argc, char **argv)
{
  char filename [512];
  FILE *f;
  BCK_FILEHEAD_STRUCT fh;
  BCK_CYCLEHEAD_STRUCT ch;
  BCK_DATAHEAD_STRUCT dh;
  char timstr [24];
  int c, d;
  unsigned char *datap, *p;
  int i;

/*
 * Open file
 */

  if (argc == 1) {
    printf ("Name of backup file: ");
    scanf ("%s", (char *)&filename);
  } else
    strcpy(filename, argv[1]);

#if defined OS_VMS || defined OS_ELN
  f = fopen (filename, "rb", "shr=upd");
#else
  f = fopen (filename, "rb");
#endif
  if (f == NULL) {
    perror ("fopen");
    return 1;
  }

/*
 * Read header and print it
 */

  fseek (f, 0, 0);
  fread (&fh, sizeof fh, 1, f);
  printf ("Layout version:       %d\n", fh.version);
  if (fh.version != BCK_FILE_VERSION) {
    printf ("This program is built with header version %d\n", BCK_FILE_VERSION);
    return 1;
  }

  time_AtoAscii(&fh.creationtime, time_eFormat_DateAndTime, timstr, sizeof(timstr));

  printf ("Created:              %s\n", timstr);

  for (c=0; c<2; c++) {
    printf ("%s cycle:\n", c == 0 ? "Fast" : "Slow");
    time_AtoAscii(&fh.updatetime[c], time_eFormat_DateAndTime, timstr, sizeof(timstr));

    printf ("  Updated:            %s\n", timstr);
    printf ("  Curdata pointer:    %x\n", fh.curdata [c]);
    printf ("  Cursize:            %x\n", fh.cursize [c]);
  }



/*
 * Dump cycle header and data headers for each cycle
 */

  printf ("\n");
  for (c=0; c<2; c++) {
    if ((fh.curdata [c] == 0) || (fh.cursize [c] == 0)) continue;

    fseek (f, fh.curdata [c], 0);
    fread (&ch, sizeof ch, 1, f);

    printf ("\n%s cycleheader:\n", c == 0 ? "Fast" : "Slow");
    time_AtoAscii(&ch.objtime, time_eFormat_DateAndTime, timstr, sizeof(timstr));

    printf ("  Objtime:            %s\n", timstr);
    printf ("  Length:             %x\n", ch.length);
    printf ("  Cycle:              %x\n", ch.cycle);
    printf ("  Segments:           %x\n", ch.segments);

    for (d=0; d<ch.segments; d++) {
      printf ("\n  Data header %d:\n", d+1);
      fread (&dh, sizeof dh, 1, f);
      printf ("    Valid:            %x\n", dh.valid);
      printf ("    Dynamic:          %x\n", dh.dynamic);
      printf ("    Class:            %x\n", dh.class);
      printf ("    Attrref.Vid:      %x\n", dh.attrref.Objid.vid);
      printf ("    Attrref.Oix:      %x\n", dh.attrref.Objid.oix);
      printf ("    Attrref.Offset:   %x\n", dh.attrref.Offset);
      printf ("    Attrref.Body:     %x\n", dh.attrref.Body);
      printf ("    Attrref.Size:     %x\n", dh.attrref.Size);
      printf ("    Attrref.Indirect: %x\n", dh.attrref.Flags.b.Indirect);
      printf ("    Dataname:         %s\n", dh.dataname);
#ifdef	NODATA
      fseek (f, dh.attrref.Size, 1);
#else
      datap = malloc (dh.attrref.Size);
      fread (datap, dh.attrref.Size, 1, f);
      p = datap;
      for (i=0; i<dh.attrref.Size; i++, p++) {
	if ((i % 16) == 0) printf ("\n	");
	printf ("%02x ", *p);
      }
      free (datap);
      printf ("\n");   
#endif
    }
    printf ("\n");   
  }
  fclose (f);

  return 0;
}

