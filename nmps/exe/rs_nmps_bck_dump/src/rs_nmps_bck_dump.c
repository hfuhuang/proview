/*************************************************************************
*
*		SSAB OXEL�SUND
*		==============
*************************************************************************
*
* 	PROGRAM		rs_nmps_bck
*	SYSTEM		SSAB 
*
*       Modifierad
*		960205	Claes Sj�fors	Skapad
*
*
*
*	Funktion:	Backup av NMpsCell objekt och data objekt.
*
*
*
**************************************************************************
**************************************************************************/

/*_Include filer_________________________________________________________*/

#ifdef OS_VMS
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <float.h>
#include <libdef.h>
#include <starlet.h>
#include <lib$routines.h>
#include <descrip.h>
#endif

#ifdef OS_ELN
#include stdio
#include stdlib
#include string
#include math
#include float
#include libdef
#include starlet
#include lib$routines
#include descrip
#endif

#include "pwr.h"
#include "pwr_baseclasses.h"
#include "pwr_nmpsclasses.h"
#include "co_cdh.h"
#include "co_time.h"
#include "rt_gdh.h"
#include "rt_errh.h"
#include "rt_gdh_msg.h"
#include "rt_hash_msg.h"
#include "rs_nmps.h"
#include "rs_nmps_msg.h"
#include "rs_sutl.h"

#define ODD(a)	(((int)(a) & 1) != 0)
#define EVEN(a)	(((int)(a) & 1) == 0)
#define max(Dragon,Eagle) ((Dragon) > (Eagle) ? (Dragon) : (Eagle))
#define min(Dragon,Eagle) ((Dragon) < (Eagle) ? (Dragon) : (Eagle))
#ifndef __ALPHA
#define abs(Dragon) ((Dragon) >= 0 ? (Dragon) : (-(Dragon)))
#endif
#define	LogAndExit( status) \
{\
  errh_CErrLog(NMPS__EXIT, errh_ErrArgMsg(status));\
  exit( status);\
}

#define	LogAndReturn( status1, status2) \
{\
  errh_CErrLog(status1, errh_ErrArgMsg(status2));\
  return status2;\
}

#define	Log( status1, status2) \
{\
  errh_CErrLog(status1, errh_ErrArgMsg(status2));\
}

#define	NMPS_BCK_FILEVERSION	1
#define	NMPSBCK_MAX_RECORDS		500

typedef struct {
	pwr_tTime			creationtime;
	int				version;
	} nmpsbck_t_fileheader;

typedef struct {
	pwr_tTime			time;
	char				type[32];
	} nmpsbck_t_recordheader;

typedef struct {
	char				type[32];
	pwr_tObjid			objid;
	pwr_tClassId			class;
	int				size;
	int				last;
	} nmpsbck_t_cellheader;

typedef struct {
	char				type[32];
	pwr_tObjid			objid;
	char				data_name[120];
	pwr_tClassId			class;
	int				size;
	int				last;
	} nmpsbck_t_dataheader;

#define HEADERTYPE_RECORDSTART	"Headertype record start"
#define HEADERTYPE_RECORDEND	"Headertype record end"
#define HEADERTYPE_CELL		"Headertype cell"
#define HEADERTYPE_CELLEND	"Headertype cell end"
#define HEADERTYPE_DATA		"Headertype data"
#define HEADERTYPE_DATAEND	"Headertype data end"

typedef struct nmpsbck_data_tag{
	pwr_tObjid			objid;
	pwr_tObjid			old_objid;
	pwr_tClassId			class;
	int				size;
	char				data_name[80];
	char				*data_ptr;
	gdh_tDlid			data_subid;
	unsigned char			found;
	unsigned char			new;
	unsigned char			created;
	struct nmpsbck_data_tag		*next_ptr;
	struct nmpsbck_data_tag		*prev_ptr;
	} nmpsbck_t_data_list;

typedef struct {
	pwr_tObjid		objid;
	pwr_tClassId		class;
	pwr_sClass_NMpsMirrorCell *cell;
	gdh_tDlid		subid;
	int			size;
	int			backup_now;
	} nmpsbck_t_cell_list;

typedef struct {
	int				CellObjectCount;
	int				DataObjectCount;
	pwr_tUInt32			LoopCount;
	pwr_sClass_NMpsBackupConfig	*bckconfig;
	gdh_tDlid			bckconfig_dlid;
	nmpsbck_t_cell_list		*cellist;
	int				cell_count;
	nmpsbck_t_data_list		*data_list;
	int				init_done;
	char				*buffer;
	int				buffer_size;
	int				cellist_size;
	FILE				*bckfile1;
	FILE				*bckfile2;
	int				file_num;
	int				increment;
	} *bck_ctx;

static pwr_tUInt32 nmpsbck_fulltimestring(
	pwr_tTime	*time,
	char		*timestr)
{
	return time_AtoAscii( time, time_eFormat_DateAndTime, timestr, 80);
}

static pwr_tUInt32 nmpsbck_print_recordheader( 
			nmpsbck_t_recordheader *recordheader)
{
	char	timestr[80];

	printf("Record header\n");
	printf("	type:  %s\n", recordheader->type);
	nmpsbck_fulltimestring( &recordheader->time, timestr);
	printf("	time:  %s\n", timestr);
	return NMPS__SUCCESS;
}

static pwr_tUInt32 nmpsbck_print_fileheader( 
			nmpsbck_t_fileheader *fileheader)
{
	char	timestr[80];

	printf("File header\n");
	printf("	version:  %d\n", fileheader->version);
	nmpsbck_fulltimestring( &fileheader->creationtime, timestr);
	printf("	time:  %s\n", timestr);
	return NMPS__SUCCESS;
}

static pwr_tUInt32 nmpsbck_print_cellheader( 
			nmpsbck_t_cellheader *cellheader)
{

	printf("Cell header\n");
	printf("	type:  %s\n", cellheader->type);
	printf("	objid: %s\n", cdh_ObjidToString( NULL, 
				cellheader->objid, 0));
	printf("	class: %lx\n", cellheader->class);
	printf("	size:  %d\n", cellheader->size);
	return NMPS__SUCCESS;
}

static pwr_tUInt32 nmpsbck_print_dataheader( 
			nmpsbck_t_dataheader *dataheader)
{
	printf("Data header\n");
	printf("	type:  %s\n", dataheader->type);
	printf("	objid: %s\n", cdh_ObjidToString( NULL, 
				dataheader->objid, 0));
	printf("	class: %lx\n", dataheader->class);
	printf("	name:  %s\n", dataheader->data_name);
	printf("	size:  %d\n", dataheader->size);
	return NMPS__SUCCESS;
}
static int		nmpsbck_timecmp( pwr_tTime 	*time_old,
					pwr_tTime	*time_new)
{
	pwr_tTime	testtime;
	int		sts;

	sts = lib$sub_times( time_new, time_old, &testtime);
	if ( sts == LIB$_NEGTIM)
	  return 0;
	return 1;
}

int	nmpsbck_get_filename(
			char	*inname,
			char	*outname,
			char	*ext)
{
	char	*s;
	char	*s2;
	char	filename[80];

	strcpy( outname, inname);

	/* Look for extention in filename */
	if ( ext != NULL)
	{
	  s = strrchr( inname, ':');
	  if ( s == 0)
	    s = inname;

	  s2 = strrchr( s, '>');
	  if ( s2 == 0)
	  {
	    s2 = strrchr( s, ']');
	    if ( s2 == 0)
	      s2 = s;
	  }

	  s = strrchr( s2, '.');
	  if ( s != 0)
	    *s = 0;
	  strcat( outname, ext);
	}
	return NMPS__SUCCESS;
}

/*************************************************************************
*
* Name:		nmpsbck_check_file
*
* Typ		int
*
* Typ		Parameter	IOGF	Beskrivning
*
* Beskrivning: 
*	Check the consistency of a backup file.
*
**************************************************************************/

static pwr_tStatus nmpsbck_check_file( 	bck_ctx		bckctx,
					FILE		*bckfile,
					int		*record_count,
					unsigned int	*record_start,
					unsigned int	*cellarea_start,
					unsigned int	*dataarea_start,
					pwr_tTime	*first_record_time,
					pwr_tTime	*last_record_time,
					int		time_only)
{
	pwr_tStatus		sts;
	nmpsbck_t_fileheader	fileheader;
	nmpsbck_t_recordheader	recordheader;
	nmpsbck_t_recordheader	recordheaderend;
	nmpsbck_t_cellheader	cellheader;
	nmpsbck_t_dataheader	dataheader;
	pwr_tUInt32		csts;
	pwr_tUInt32		actpos;
	int			cell_read_success;
	int			data_read_success;
	char			filename[80];
	int			k;
	char			buffer[2000];

	fgetname( bckfile, filename);
	printf("******************************************************\n");
	printf("Examining file %s\n\n", filename);

	actpos = ftell( bckfile);
	csts = fseek( bckfile, - actpos , 1);
	if (csts != 0) return NMPS__FILEREAD;

	/* Read file header */
	csts = fread( &fileheader, sizeof(fileheader), 1, bckfile);
	if (csts == 0) return NMPS__FILEREAD;

	nmpsbck_print_fileheader( &fileheader);

	/* Check version */
	if (fileheader.version != NMPS_BCK_FILEVERSION)
	  return NMPS__BCKVERSION;

	*record_count = 0;
	cell_read_success = 0;
	data_read_success = 0;
	for (;;)
	{
	  /* Read record header */
	  /* Store the start of the record */
	  *cellarea_start = ftell( bckfile);
	  csts = fread( &recordheader, sizeof(recordheader), 1, bckfile);
	  if (csts == 0)
	    break;

	  nmpsbck_print_recordheader( &recordheader);

	  if (strcmp( recordheader.type, HEADERTYPE_RECORDSTART))
	    break;
	  if (!nmpsbck_timecmp( &fileheader.creationtime, &recordheader.time))
	    break;

	  if ( *record_count == 0)
	  {
	    memcpy( first_record_time, &recordheader.time, 
			sizeof( recordheader.time));
	    if ( time_only)
	      return NMPS__SUCCESS;
	  }
	  else
	  {
	    if (!nmpsbck_timecmp( first_record_time, &recordheader.time))
	      break;
          }

	  /* Read cell area */
	  cell_read_success = 0;
	  /* Store the start of the cell area */
	  *cellarea_start = ftell( bckfile);
	  for (;;)
	  {
	    csts = fread( &cellheader, sizeof(cellheader), 1, bckfile);
	    if (csts == 0)
	      break;

	    nmpsbck_print_cellheader( &cellheader);

	    if ( !strcmp( cellheader.type, HEADERTYPE_CELLEND))
	    {
	      /* End of cell area */
	      cell_read_success = 1;
	      break;
	    }
	    else if ( strcmp( cellheader.type, HEADERTYPE_CELL))
	      break;

	    csts = fread( buffer, cellheader.size, 1, bckfile);
	    if (csts == 0)
	      break;

	    switch( cellheader.class)
	    {
	      case pwr_cClass_NMpsCell:
	      case pwr_cClass_NMpsStoreCell:
	      {
	        pwr_sClass_NMpsCell 	*cell_ptr;
	        plc_t_DataInfo		*data_block_ptr;

	        cell_ptr = (pwr_sClass_NMpsCell *) buffer;
	        data_block_ptr = (plc_t_DataInfo *) &cell_ptr->Data1P;
	        for ( k = 0; k < cell_ptr->LastIndex; k++)
	        {
	          /* Check if the objid already is in the data_db */
	          printf( "         Data%d: %s  %c %c %c\n", k, 
			cdh_ObjidToString( NULL, 
				data_block_ptr->Data_ObjId, 0),
			data_block_ptr->Data_Front ? 'F':' ',
			data_block_ptr->Data_Back ? 'B':' ',
			data_block_ptr->Data_Select ? 'S':' ');
	          data_block_ptr++;
	        }
	        data_block_ptr = (plc_t_DataInfo *) &cell_ptr->DataLP;
	        printf( "         DataL:  %s  %c %c %c\n", 
			cdh_ObjidToString( NULL, 
				data_block_ptr->Data_ObjId, 0),
			data_block_ptr->Data_Front ? 'F':' ',
			data_block_ptr->Data_Back ? 'B':' ',
			data_block_ptr->Data_Select ? 'S':' ');
	        break;
	      }
	      case pwr_cClass_NMpsMirrorCell:
	      {
	        pwr_sClass_NMpsMirrorCell *cell_ptr;
	        plc_t_DataInfoMirCell	*data_block_ptr;

	        cell_ptr = (pwr_sClass_NMpsMirrorCell *) buffer;
	        data_block_ptr = (plc_t_DataInfoMirCell *) &cell_ptr->Data1P;
	        for ( k = 0; k < cell_ptr->LastIndex; k++)
	        {
	          printf( "         Data%d: %s\n", k, 
			cdh_ObjidToString( NULL, 
				data_block_ptr->Data_ObjId, 0));
	          data_block_ptr++;
	        }
	        break;
	      }
	    }
	  }

	  if ( !cell_read_success)
	    break;

	  /* Read data area */
	  data_read_success = 0;
	  /* Store the start of the data area */
	  *dataarea_start = ftell( bckfile);
	  for (;;)
	  {
	    csts = fread( &dataheader, sizeof(dataheader), 1, bckfile);
	    if (csts == 0)
	      break;

	    nmpsbck_print_dataheader( &dataheader);

	    if ( !strcmp( dataheader.type, HEADERTYPE_DATAEND))
	    {
	      /* End of data area */
	      data_read_success = 1;
	      break;
	    }
	    else if ( strcmp( dataheader.type, HEADERTYPE_DATA))
	      break;

	    csts = fseek( bckfile, dataheader.size, 1);
	    if (csts != 0)
	      break;
	  }

	  if ( !data_read_success)
	    break;

	  /* Read record header */
	  csts = fread( &recordheaderend, sizeof(recordheaderend), 1, bckfile);
	  if (csts == 0)
	    break;

	  nmpsbck_print_recordheader( &recordheaderend);

	  if (strcmp( recordheaderend.type, HEADERTYPE_RECORDEND))
	    break;

	  if (!nmpsbck_timecmp( &recordheader.time, &recordheaderend.time))
	    break;

	  (*record_count)++;
	  record_start++;
	  cellarea_start++;
	  dataarea_start++;
	}

	if ( *record_count == 0)
	{
	  /* No valid record found */
	  return NMPS__BCKFILECORRUPT;
	}

	memcpy( last_record_time, &recordheader.time,
			sizeof( recordheader.time));

	return NMPS__SUCCESS;
}


/*************************************************************************
*
* Name:		nmpsbck_read
*
* Typ		int
*
* Typ		Parameter	IOGF	Beskrivning
*
* Beskrivning: 
*	Read the cell objects from backup file.
*
**************************************************************************/

static pwr_tStatus	nmpsbck_read( bck_ctx	bckctx)
{
	pwr_tStatus		sts;
	nmpsbck_t_fileheader	fileheader;
	nmpsbck_t_recordheader	recordheader;
	nmpsbck_t_recordheader	recordheaderend;
	nmpsbck_t_cellheader	cellheader;
	nmpsbck_t_dataheader	dataheader;
	FILE			*bckfile;
	FILE			*bckfile1;
	FILE			*bckfile2;
	pwr_tUInt32		csts;
	pwr_tUInt32		actpos;
	int			k;
	char			*databuff = 0;
	int			databuff_size = 0;
	nmpsbck_t_data_list	*data_ptr;
	nmpsbck_t_data_list	*next_ptr;
	char			*objectp;
	int			size;
	char			filename[80];
	char			file_ext[8];
	int			file_num = 1;
	int			record_count;
	char			*s;
	pwr_tUInt32		cellarea_start[NMPSBCK_MAX_RECORDS];
	pwr_tUInt32		dataarea_start[NMPSBCK_MAX_RECORDS];
	pwr_tUInt32		record_start[NMPSBCK_MAX_RECORDS];
	int			cell_read_success;
	int			data_read_success;
	int			i;
	pwr_tObjid		objid;
	int			data_count;
	int			created;
	pwr_tTime		first_record_time;
	pwr_tTime		last_record_time;
	pwr_tTime		bckfile1_time;
	pwr_tTime		bckfile2_time;
	pwr_tStatus		bckfile1_sts;
	pwr_tStatus		bckfile2_sts;

	/* Open file 1 */
#if defined(OS_ELN)
	bckfile1 = fopen( bckctx->bckconfig->BackupFile, "r+", "shr=get", "ctx=rec");
#elif defined(OS_VMS)
	bckfile1 = fopen( bckctx->bckconfig->BackupFile, "r+", "shr=get");
#else
	bckfile1 = fopen( bckctx->bckconfig->BackupFile, "r+");
#endif
	if (bckfile1 == NULL)
	  bckfile1_sts = NMPS__FILEREAD;
	bckfile1_sts = nmpsbck_check_file( bckctx, bckfile1, &record_count, record_start,
			cellarea_start, dataarea_start, &bckfile1_time,
			&last_record_time, 0);
	return bckfile1_sts;
}

main(int argc, char *argv[])
{
	bck_ctx		bckctx;
	int		sts;
	pwr_sClass_NMpsBackupConfig	bckconfig;

	if ( argc < 2 )
	{
	  printf("Enter filename\n");
	  exit( NMPS__SUCCESS);
	}

	bckctx = calloc( 1 , sizeof( *bckctx));
	if ( bckctx == 0 ) return NMPS__NOMEMORY;

	bckctx->bckconfig = &bckconfig;
	strcpy( bckconfig.BackupFile, argv[1]);

	sts = nmpsbck_read( bckctx);
	exit( sts);
}
