/* 
 * Proview   $Id: rt_bck_load.c,v 1.8 2008-04-25 11:27:17 claes Exp $
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
#include "co_cdh.h"


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
  BCK_CYCLEHEAD_STRUCT	ch_old;
  bck_t_cycleheader	ch;
  BCK_DATAHEAD_STRUCT	dh_old;
  bck_t_writeheader     dh;
  pwr_tUInt32		c;
  pwr_tUInt32		d;
  char			*strp;
  char			*datap;
  char                  *namep;
  char			fname[200];
  pwr_tAName            objectname;

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
    if (fh.version < BCK_FILE_VERSION - 1) {
      errh_Info("BACKUP Loading old file version : %d", fh.version);
      /* Read the cycle data.  */

      for (c=0; c<2; c++) {
        fseek(f, fh.curdata [c], 0);
	fread(&ch_old, sizeof ch_old, 1, f);

        /* Work thru the data segments */

        for (d=0; d<ch_old.segments; d++) {
	  csts = fread(&dh_old, sizeof dh_old, 1, f);
	  if (csts != 0) {
            datap = malloc(dh_old.attrref.Size);
	    csts = fread(datap, dh_old.attrref.Size, 1, f);
	  }
	  if (csts == 0) {
	    SET_ERRNO_STS;
	    break;
          }

	  if (dh_old.valid) {

	    /* Find object */

	    if (dh_old.dynamic) {
              strp = strchr(dh_old.dataname, '.');	/* always is a full object! */
              if (strp != NULL) *strp = '\0';	/* Just make sure... */

	      sts = gdh_CreateObject(dh_old.dataname, dh_old.class, dh_old.attrref.Size,
		&objid, dh_old.attrref.Objid, 0, pwr_cNObjid);

              if (strp != NULL) *strp = '.';

	      if (ODD (sts))
		sts = gdh_SetObjectInfo(dh_old.dataname, datap, dh_old.attrref.Size);
	    } /* Dynamic object */
	    else {
	      sts = gdh_SetObjectInfoAttrref(&dh_old.attrref, datap, dh_old.attrref.Size);
	    }
	  } /* valid segment */

          free(datap);

          if (EVEN (sts)) {
            errh_Error("BACKUP error reloading %s, reason:\n%m", dh_old.dataname, sts);
            sts = 1;
          }

        } /* For all data segments */
        if (EVEN (sts)) break;	/* Fatal! Get out! */
      } /* For each cycle */
    } else if (fh.version == BCK_FILE_VERSION - 1) {

      /* Read the cycle data.  */

      for (c=0; c<2; c++) {
        fseek(f, fh.curdata [c], 0);
	fread(&ch_old, sizeof ch_old, 1, f);

        /* Work thru the data segments */

        for (d=0; d<ch_old.segments; d++) {
	  csts = fread(&dh, sizeof dh, 1, f);
	  if (csts != 0) {
	    if (dh.namesize > 0) {
	      namep = malloc(dh.namesize + 1);
	      csts = fread(namep, dh.namesize + 1, 1, f);
	    } else 
	      namep = NULL;
            datap = malloc(dh.size);
	    csts = fread(datap, dh.size, 1, f);
	  }
	  if (csts == 0) {
	    SET_ERRNO_STS;
	    break;
          }
	  
	  if (dh.valid) {

	    /* Find object */

	    if (dh.dynamic) {
              strp = strchr(namep, '.');	/* always is a full object! */
              if (strp != NULL) *strp = '\0';	/* Just make sure... */

	      sts = gdh_CreateObject(namep, dh.class, dh.size,
		&objid, dh.objid, 0, pwr_cNObjid);

              if (strp != NULL) *strp = '.';

	      if (ODD (sts))
		sts = gdh_SetObjectInfo(namep, datap, dh.size);

              strncpy(objectname, namep, pwr_cSizAName);
	    } /* Dynamic object */
	    else {
	      sts = gdh_ObjidToName (dh.objid, objectname, sizeof(objectname),
	                             cdh_mNName);
	      if (ODD(sts)) {
	        strcat(objectname, namep);
	        sts = gdh_SetObjectInfo(objectname, datap, dh.size);
	      }
	    }
	  } /* valid segment */

          if (EVEN (sts)) {
            errh_Error("BACKUP error reloading %s, reason:\n%m", objectname, sts);
            sts = 1;
          }

          free(datap);
	  free(namep);


        } /* For all data segments */
        if (EVEN (sts)) break;	/* Fatal! Get out! */
      } /* For each cycle */
    } else if (fh.version == BCK_FILE_VERSION) {

      /* Read the cycle data.  */

      for (c=0; c<2; c++) {
        fseek(f, fh.curdata [c], 0);
	fread(&ch, sizeof ch, 1, f);

        /* Work thru the data segments */

        for (d=0; d<ch.segments; d++) {
	  csts = fread(&dh, sizeof dh, 1, f);
	  if (csts != 0) {
	    if (dh.namesize > 0) {
	      namep = malloc(dh.namesize + 1);
	      csts = fread(namep, dh.namesize + 1, 1, f);
	    } else 
	      namep = NULL;
            datap = malloc(dh.size);
	    csts = fread(datap, dh.size, 1, f);
	  }
	  if (csts == 0) {
	    SET_ERRNO_STS;
	    break;
          }
	  
	  if (dh.valid) {

	    /* Find object */

	    if (dh.dynamic) {
              strp = strchr(namep, '.');	/* always is a full object! */
              if (strp != NULL) *strp = '\0';	/* Just make sure... */

	      sts = gdh_CreateObject(namep, dh.class, dh.size,
		&objid, dh.objid, 0, pwr_cNObjid);

              if (strp != NULL) *strp = '.';

	      if (ODD (sts))
		sts = gdh_SetObjectInfo(namep, datap, dh.size);

              strncpy(objectname, namep, pwr_cSizAName);
	    } /* Dynamic object */
	    else {
	      sts = gdh_ObjidToName (dh.objid, objectname, sizeof(objectname),
	                             cdh_mNName);
	      if (ODD(sts)) {
	        strcat(objectname, namep);
	        sts = gdh_SetObjectInfo(objectname, datap, dh.size);
	      }
	    }
	  } /* valid segment */

          if (EVEN (sts)) {
            errh_Error("BACKUP error reloading %s, reason:\n%m", objectname, sts);
            sts = 1;
          }

          free(datap);
	  free(namep);


        } /* For all data segments */
        if (EVEN (sts)) break;	/* Fatal! Get out! */
      } /* For each cycle */
    } /* File version format ok */
    else {
      errh_Error("BACKUP Cannot load backup file with version %d", fh.version);
    }
  } /* Successful header read */

  fclose (f);

  return sts;
}

