/* rt_mh_blockdb.c -- Runtime environment - MH Block database

   PROVIEW/R
   Copyright (C) 1996-98 by Mandator AB.

   <Description>.  */

#ifdef OS_ELN
# include ctype
# include stdio
# include stdlib
# include errno
# include string
#else
# include <stdio.h>
# include <stdlib.h>
# include <ctype.h>
# include <errno.h>
# include <string.h>
#endif

#ifdef OS_VMS
# include <starlet.h>
#endif

#include "pwr_class.h"
#include "co_time.h"
#include "rt_mh_msg.h"
#include "rt_errh.h"
#include "rt_mh_blockdb.h"

/* Local macros */

#define Log_Error(a, b) errh_Error("%s\n%m", b, a)
#define Log(b) errh_Info(b)
#define Log_Error_Exit(a, b) {Log_Error(a, b); exit(a);}
#define Log_Error_Return(a, b) {Log_Error(a, b); return (a);}
#define If_Error_Log(a, b) if ((a & 1) != 1) Log_Error(a, b)
#define If_Error_Log_Return(a, b) if ((a & 1) != 1) Log_Error_Return(a, b)
#define If_Error_Log_Exit(a, b) if ((a & 1) != 1) Log_Error_Exit(a, b)

#define mh_cBlockFileVersion 1
#define mh_cPageAlign 511L
#define mh_cPageSize 512L


mh_sBlockDb *
mh_BlockDbOpen (
  char *FileName,
  pwr_tUInt32 *size
)
{
  register mh_sBlockDb *dp;
  register mh_sBlockDbHead *hp;
  char page[mh_cPageSize];
  char msg[512];

  dp = (mh_sBlockDb *) calloc(1, sizeof(mh_sBlockDb));
  if (dp == NULL)
    return(dp);
  hp = &dp->Head;

  /* try to open old blocking file */
#if defined OS_VMS || defined OS_ELN
  dp->File = fopen(FileName, "r+b", "shr=get");
#else
  dp->File = fopen(FileName, "r+");
#endif

  if (dp->File != NULL) {
    sprintf(msg, "BlockDbOpen: old file: %s", FileName);
    Log(msg);
    if (fread(hp, sizeof(*hp), 1, dp->File) != 1) {
#if defined OS_VMS || defined OS_ELN
      sprintf(msg, "BlockDbOpen: read header, %s", strerror(errno, vaxc$errno));
#else
      sprintf(msg, "BlockDbOpen: read header, %s", strerror(errno));
#endif
      Log(msg);
    } else {
      clock_gettime(CLOCK_REALTIME, &hp->OpenTime);
      if (size != NULL) *size = hp->SegSize;
      if (hp->Version == mh_cBlockFileVersion)
	return(dp);
      Log("BlockDbOpen: file version mismatch");
    }
    fclose(dp->File);
  }

  /* try to create new blocking file */
#if defined OS_VMS || defined OS_ELN
  dp->File = fopen(FileName, "w+b", "shr=get");
#else
  dp->File = fopen(FileName, "w+");
#endif

  if (dp->File == NULL) {
#if defined OS_VMS || defined OS_ELN
    sprintf(msg, "BlockDbOpen: %s", strerror(errno, vaxc$errno));
#else
    sprintf(msg, "BlockDbOpen: %s", strerror(errno));
#endif
    Log(msg);
  } else {
    sprintf(msg, "BlockDbOpen: new file, %s", FileName);
    Log(msg);

    memset(hp, 0, sizeof(*hp));
    clock_gettime(CLOCK_REALTIME, &hp->CreationTime);
    clock_gettime(CLOCK_REALTIME, &hp->OpenTime);
    hp->Version = mh_cBlockFileVersion;

    /* Write a file header */

    hp->SectPos = sizeof(page);
    if (fseek(dp->File, 0, SEEK_SET) == 0) {
      memset(page, 0, sizeof(page));
      memcpy(page, hp, sizeof(*hp));
      if (size != NULL) *size = hp->SegSize;
      if (fwrite(page, sizeof(page), 1, dp->File) == 1)
	return(dp);
    }

#if defined OS_VMS || defined OS_ELN
    sprintf(msg, "BlockDbOpen: write header, %s", strerror(errno, vaxc$errno));
#else
    sprintf(msg, "BlockDbOpen: write header, %s", strerror(errno));
#endif
    Log(msg);
    fclose(dp->File);
  }
  free(dp);
  return(NULL);
}


mh_sBlockDb *
mh_BlockDbClose (
  mh_sBlockDb *dp
)
{

  if (dp != NULL) {
    if (dp->File != NULL)
      fclose(dp->File);
    free(dp);
  }
  return NULL;
}

mh_sBlockDb *
mh_BlockDbGet (
  mh_sBlockDb *dp,
  pwr_tUInt32 *size,
  char *buffer
)
{
  register mh_sBlockDbHead *hp = &dp->Head;
  char msg[512];

  clock_gettime(CLOCK_REALTIME, &hp->GetTime);
  hp->GetCount++;

  if (fseek(dp->File, hp->SectPos, SEEK_SET) != 0) goto error;
  if (fread(buffer, hp->SegSize, 1, dp->File) != 1) goto error;

  *size = hp->SegSize;
  return(dp);

error:
#if defined OS_VMS || defined OS_ELN
  sprintf(msg, "BlockDbGet: %s", strerror(errno, vaxc$errno));
#else
  sprintf(msg, "BlockDbGet: %s", strerror(errno));
#endif
  Log(msg);
  return mh_BlockDbClose(dp);
}

mh_sBlockDb *
mh_BlockDbPut ( 
  mh_sBlockDb *dp,
  pwr_tUInt32 size,
  char *buffer
)
{
  register mh_sBlockDbHead *hp = &dp->Head;
  char msg[512];
  char *errc;
  mh_sBlockDbHead Head = dp->Head;
  char page[mh_cPageSize];
  long FileSize;
  long DiffSize;

  memset(page, 0, sizeof(page));

  if (hp->FreeSize < size) {
    Head.SectPos = hp->SectPos + hp->SectSize;
    Head.FreeSize = Head.SectPos - sizeof(page);
  } else {
    Head.SectPos = sizeof(page);
    Head.FreeSize = 0;
  }

  Head.SegSize = size;
  Head.SectSize = (size + mh_cPageAlign) & ~mh_cPageAlign; /* align with page size */
  Head.SectSize = MAX(Head.SectSize, mh_cPageSize); /* Minimum one page */
  DiffSize = Head.SectSize - Head.SegSize;

  /* statistics */
  clock_gettime(CLOCK_REALTIME, &Head.PutTime);
  Head.PutCount++;
  if (Head.FreeSize > Head.MaxFreeSize)
    Head.MaxFreeSize = Head.FreeSize;
  if (Head.SegSize > Head.MaxSegSize)
    Head.MaxSegSize = Head.SegSize;
  if (Head.SectSize > Head.MaxSectSize)
    Head.MaxSectSize = Head.SectSize;
  FileSize = Head.FreeSize + Head.SectSize + sizeof(page);
  if (FileSize > Head.FileSize)
    Head.FileSize = FileSize;

  if (fseek(dp->File, Head.SectPos, SEEK_SET) != 0) goto error;
  if (size > 0)
    if (fwrite(buffer, size, 1, dp->File) != 1) goto error;
  if (DiffSize > 0) {
    if (fwrite(page, DiffSize, 1, dp->File) != 1) goto error;
  }
  if (fflush(dp->File) != 0) goto error;
  if (fseek(dp->File, 0, SEEK_SET) != 0) goto error;
  memcpy(page, &Head, sizeof(Head));
  if (fwrite(page, sizeof(page), 1, dp->File) != 1) goto error;
  if (fflush(dp->File) != 0) goto error;

  *hp = Head;
  return(dp);

error:
#if defined OS_VMS || defined OS_ELN
  errc = strerror(errno, vaxc$errno);
#else
  errc = strerror(errno);
#endif
  sprintf(msg, "BlockDbPut: %s", errc);
  Log(msg);
  return mh_BlockDbClose(dp);
}
