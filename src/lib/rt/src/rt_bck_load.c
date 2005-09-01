/* 
 * Proview   $Id: rt_bck_load.c,v 1.3 2005-09-01 14:57:55 claes Exp $
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

/* rt_bck_load.c
   This module contains the code for reading and
   restoring the information in the backup data file.  */

#ifdef OS_ELN
# include $vaxelnc
# include stdlib
# include stdio
# include descrip
# include string
# include errno
#else
# include <stdlib.h>
# include <stdio.h>
# include <string.h>
# include <errno.h>
#endif

#ifdef OS_VMS
# include <descrip.h>
# include <starlet.h>
#endif


#include "pwr.h"
#include "pwr_class.h"
#include "pwr_baseclasses.h"
#include "rt_errh.h"
#include "rt_gdh.h"
#include "rt_gdh_msg.h"
#include "rt_bckdef.h"
#include "rt_bck_load.h"
#include "co_dcli.h"


#if defined OS_VMS || defined OS_ELN
# define SET_ERRNO_STS  sts = vaxc$errno
# define A_MODE 	, "shr=get"
# define FGETNAME	fgetname (f, (char *)&tmpstr)
#else
# define SET_ERRNO_STS  perror("rt_bck_load"); sts = 2
# define A_MODE
# define FGETNAME	backup_confp->BackupFile
#endif

/* Open the backup datafile, read the information and load it into rtdb.
   This should be done before bck_Init is invoked.  */

pwr_tStatus
bck_LoadBackup ()
{
  pwr_tStatus		sts;
  pwr_tInt32		csts;
  pwr_tObjid		objid;
  pwr_sClass_Backup_Conf *backup_confp;	/* Backup_Conf object pointer */
  FILE			*f;
  BCK_FILEHEAD_STRUCT	fh;		/* File header */
  BCK_CYCLEHEAD_STRUCT	ch;
  BCK_DATAHEAD_STRUCT	dh;
  pwr_tUInt32		c;
  pwr_tUInt32		d;
  char			*strp;
  char			*datap;
  char			fname[200];

#if defined OS_VMS || defined OS_ELN
  short			msglen;
  struct dsc$descriptor tmpstrdsc;
  char			tmpstr [256];
#endif

  /* Find the local Backup_Conf object.  */

  sts = gdh_GetClassList(pwr_cClass_Backup_Conf, &objid);
  while (ODD(sts)) {
    sts = gdh_ObjidToPointer(objid, (pwr_tAddress *)&backup_confp);
    if (ODD(sts)) break;
    sts = gdh_GetNextObject(objid, &objid);
  }
  if (EVEN(sts)) return sts;

  /* Open the backup file.  */
  dcli_translate_filename( fname, backup_confp->BackupFile);

  f = fopen( fname, "r+" A_MODE);

  if (f == NULL) {
    SET_ERRNO_STS;
    errh_Error("Load Backup: Failed to open %s, errno = %d", 
                backup_confp->BackupFile, sts); 
    return sts;
  }

  errh_Info("BACKUP loading information from %s", FGETNAME);

  sts = 1;	/* Guess all ok */

  /* Read the header.  */

  csts = fread(&fh, sizeof fh, 1, f);
  if (csts == 0) {
    SET_ERRNO_STS;
  } else {
    if (fh.version != BCK_FILE_VERSION) {
      errh_Error("BACKUP Cannot load backup file with version %d", fh.version);
    } else {

      /* Read the cycle data.  */

      for (c=0; c<2; c++) {
        fseek(f, fh.curdata [c], 0);
	fread(&ch, sizeof ch, 1, f);

        /* Work thru the data segments */

        for (d=0; d<ch.segments; d++) {
	  csts = fread(&dh, sizeof dh, 1, f);
	  if (csts != 0) {
            datap = malloc(dh.attrref.Size);
	    csts = fread(datap, dh.attrref.Size, 1, f);
	  }
	  if (csts == 0) {
	    SET_ERRNO_STS;
	    break;
          }

	  if (dh.valid) {

	    /* Find object */

	    if (dh.dynamic) {
              strp = strchr(dh.dataname, '.');	/* always is a full object! */
              if (strp != NULL) *strp = '\0';	/* Just make sure... */

	      sts = gdh_CreateObject(dh.dataname, dh.class, dh.attrref.Size,
		&objid, dh.attrref.Objid, 0, pwr_cNObjid);

              if (strp != NULL) *strp = '.';
	    } /* Dynamic object */

	    if (ODD (sts)) {
	      sts = gdh_SetObjectInfo(dh.dataname, datap, dh.attrref.Size);
	    }
	  } /* valid segment */

          free(datap);

          if (EVEN (sts)) {
            errh_Error("BACKUP error reloading %s, reason:\n%m", dh.dataname, sts);
            sts = 1;
          }

        } /* For all data segments */
        if (EVEN (sts)) break;	/* Fatal! Get out! */
      } /* For each cycle */
    } /* File version format ok */
  } /* Successful header read */

  fclose (f);

  return sts;
}

