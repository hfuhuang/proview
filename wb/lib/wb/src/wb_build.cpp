/* 
 * Proview   $Id: wb_build.cpp,v 1.9 2008-10-15 06:04:55 claes Exp $
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


#include "pwr.h"
#include "pwr_baseclasses.h"
#include "cow_msgwindow.h"
#include "co_dcli.h"
#include "co_time.h"
#include "co_syi.h"
#include "rt_load.h"
#include "wb_foe_msg.h"
#include "wb_pwrb_msg.h"
#include "wb_utl_api.h"
#include "wb_build.h"
#include "wb_name.h"
#include "wb_lfu.h"
#include "wb_merep.h"
#include "wb_log.h"

#include "glow.h"
#include "glow_growctx.h"
#include "glow_growapi.h"

#include "ge_graph.h"
#include "ge.h"

void wb_build::classlist( pwr_tCid cid)
{
  pwr_tStatus 		sumsts;

  // Build all objects of specified class
  sumsts = PWRB__NOBUILT;
  for ( wb_object o = m_session.object( cid); o.oddSts(); o = o.next()) {

    // Call build method for object
    switch ( cid) {
    case pwr_cClass_plc:
      plcpgm( o.oid());
      break;
    case pwr_cClass_XttGraph:
      xttgraph( o.oid());
      break;
    case pwr_cClass_WebHandler:
      webhandler( o.oid());
      break;
    case pwr_cClass_WebGraph:
      webgraph( o.oid());
      break;
    case pwr_cClass_Application:
      application( o.oid());
      break;
    case pwr_cClass_PlcProcess:
      application( o.oid());
      break;
    default: 
      m_sts = PWRB__NOBUILT;
    }
    if ( evenSts())
      sumsts = m_sts;
    else if ( sumsts == PWRB__NOBUILT && m_sts != PWRB__NOBUILT && m_sts != PWRB__INLIBHIER)
      sumsts = m_sts;
  }
  m_sts = sumsts;
}

void wb_build::node( char *nodename, void *volumelist, int volumecnt)
{
  lfu_t_volumelist *vlist = (lfu_t_volumelist *)volumelist;
  pwr_tTime vtime;
  pwr_tTime btime;
  pwr_tFileName fname;
  pwr_tObjName vname;
  int bussid = -1;
  int rebuild = 1;
  pwr_tStatus status;
  char currentnode[80];
  char node[80];
  pwr_tStatus sumsts;

  printf( "Build node %s\n", nodename);

  wb_log::push();

  if ( !opt.manual) {
    // Check if there is any new dbsfile
    rebuild = 0;
    for ( int i = 0; i < volumecnt; i++) {
      if ( cdh_NoCaseStrcmp( nodename, vlist[i].p1) == 0) {
	if ( bussid == -1) {
	  char systemname[80], systemgroup[80], pname[80];
	  pwr_tVid *vl;
	  pwr_tString40 *vnl;
	  int vcnt;
	
	  // Get time for current bootfile
	  status = sscanf( vlist[i].p3, "%d", &bussid);
	  if ( status != 1) {
	    // Bussid error
	  }
	
	  sprintf( fname, load_cNameBoot, 
		   load_cDirectory, vlist[i].p2, bussid);
	  cdh_ToLower( fname, fname);
	  dcli_translate_filename( fname, fname);
	  status = lfu_ReadBootFile( fname, &btime, systemname, systemgroup, &vl, &vnl, 
				     &vcnt, pname);
	  if ( EVEN(status)) {
	    rebuild = 1;
	  }	 
	  strcpy( node, vlist[i].p2);
	}
      
	if ( vlist[i].volume_id == m_session.vid()) {
	  // Build current volume
	  volume();
	}
      
	cdh_ToLower( vname, vlist[i].volume_name);
	if ( vlist[i].volume_id >= cdh_cUserVolMin &&
	     vlist[i].volume_id <= cdh_cUserVolMax) {
	  sprintf( fname, "$pwrp_load/%s.dbs", vname);
	  dcli_translate_filename( fname, fname);
	  m_sts = dcli_file_time( fname, &vtime);
	  if ( evenSts()) {
	    // Dbs file is missing
	    char msg[200];
	    sprintf( msg, "Loadfile for volume %s not created", vname);
	    MsgWindow::message('E', msg, msgw_ePop_Yes);
	    return;
	  }
	  if ( vtime.tv_sec > btime.tv_sec)
	    rebuild = 1;
	}
      }
    }
  }
  wb_log::pull();

  if ( opt.force || opt.manual || rebuild) {
    m_sts = lfu_create_bootfile( nodename, (lfu_t_volumelist *)volumelist, volumecnt,
				 opt.debug);
    if ( ODD(m_sts))
      wb_log::log( wlog_eCategory_NodeBuild, nodename, 0);      
  }
  else
    m_sts = PWRB__NOBUILT;


  sumsts = m_sts;

  syi_NodeName( &m_sts, currentnode, sizeof(currentnode));

  if ( cdh_NoCaseStrcmp( node, currentnode) == 0) {
    pwr_tFileName src_fname, dest_fname;
    pwr_tCmd	cmd;
    pwr_tTime	dest_time, src_time;


    // Copy xtt_help.dat from $pwrp_cnf to $pwrp_exe
    sprintf( src_fname, "$pwrp_cnf/%s/xtt_help.dat", node);
    dcli_translate_filename( src_fname, src_fname);
    m_sts = dcli_file_time( src_fname, &src_time);
    if ( evenSts()) {
      strcpy( src_fname, "$pwrp_cnf/xtt_help.dat");
      dcli_translate_filename( src_fname, src_fname);
      m_sts = dcli_file_time( src_fname, &src_time);
      if ( evenSts()) {
	char msg[200];
	sprintf( msg, "File is missing $pwrp_cnf/xtt_help.dat");
	MsgWindow::message('E', msg, msgw_ePop_Yes);
      }
    }

    if ( oddSts()) {
      strcpy( dest_fname, "$pwrp_exe/xtt_help.dat");
      dcli_translate_filename( dest_fname, dest_fname);
      m_sts = dcli_file_time( dest_fname, &dest_time);
      if ( opt.force || evenSts() || src_time.tv_sec > dest_time.tv_sec) {
	sprintf( cmd, "cp %s %s", src_fname, dest_fname);
	system( cmd);
	sprintf( cmd, "Build:    Copy %s -> $pwrp_exe", src_fname);
	MsgWindow::message( 'I', cmd, msgw_ePop_No);
	m_sts = PWRB__SUCCESS;
      }
      else
	m_sts = PWRB__NOBUILT;
    }  
    if ( sumsts == PWRB__NOBUILT && m_sts != PWRB__NOBUILT)
      sumsts = m_sts;

    // Copy pwrp_alias.dat from $pwrp_cnf to $pwrp_load
    sprintf( src_fname, "$pwrp_cnf/%s/pwrp_alias.dat", node);
    dcli_translate_filename( src_fname, src_fname);
    m_sts = dcli_file_time( src_fname, &src_time);
    if ( evenSts()) {
      strcpy( src_fname, "$pwrp_cnf/pwrp_alias.dat");
      dcli_translate_filename( src_fname, src_fname);
      m_sts = dcli_file_time( src_fname, &src_time);
    }
    
    if ( oddSts()) {
      strcpy( dest_fname, "$pwrp_load/pwrp_alias.dat");
      dcli_translate_filename( dest_fname, dest_fname);
      m_sts = dcli_file_time( dest_fname, &dest_time);
      if ( opt.force || evenSts() || src_time.tv_sec > dest_time.tv_sec) {
	sprintf( cmd, "cp %s %s", src_fname, dest_fname);
	system( cmd);
	sprintf( cmd, "Build:    Copy %s -> $pwrp_load", src_fname);
	MsgWindow::message( 'I', cmd, msgw_ePop_No);
	m_sts = PWRB__SUCCESS;
      }
      else
	m_sts = PWRB__NOBUILT;
    }
    else
      m_sts = PWRB__NOBUILT;

    if ( sumsts == PWRB__NOBUILT && m_sts != PWRB__NOBUILT)
      sumsts = m_sts;

    // Copy ld_appl_...txt from $pwrp_cnf to $pwrp_load
    sprintf( src_fname, load_cNameAppl, "$pwrp_cnf", node, bussid);
    dcli_translate_filename( src_fname, src_fname);
    m_sts = dcli_file_time( src_fname, &src_time);
    if ( evenSts()) {
      char dir[80];
      strcpy( dir, "$pwrp_cnf/");
      sprintf( src_fname, load_cNameAppl, dir, node, bussid);
      dcli_translate_filename( src_fname, src_fname);
      m_sts = dcli_file_time( src_fname, &src_time);
    }

    if ( oddSts()) {
      sprintf( dest_fname, load_cNameAppl, "$pwrp_load/", node, bussid);
      dcli_translate_filename( dest_fname, dest_fname);
      m_sts = dcli_file_time( dest_fname, &dest_time);
      if ( opt.force || evenSts() || src_time.tv_sec > dest_time.tv_sec) {
	sprintf( cmd, "cp %s %s", src_fname, dest_fname);
	system( cmd);
	sprintf( cmd, "Build:    %s -> $pwrp_load", src_fname);
	MsgWindow::message( 'I', cmd, msgw_ePop_No);
	m_sts = PWRB__SUCCESS;
      }
      else
	m_sts = PWRB__NOBUILT;
    }
    else
      m_sts = PWRB__NOBUILT;

    if ( sumsts == PWRB__NOBUILT && m_sts != PWRB__NOBUILT)
      sumsts = m_sts;
  }

  if ( sumsts != PWRB__NOBUILT) {
    char msg[200];

    sprintf( msg, "Build:    Node     %s", nodename);
    MsgWindow::message('I', msg, msgw_ePop_No);
  }

  m_sts = sumsts;
}

void wb_build::volume()
{
  switch ( m_session.cid()) {
  case pwr_eClass_RootVolume:
  case pwr_eClass_SubVolume:
  case pwr_eClass_SharedVolume:
    rootvolume(0);
    break;
  case pwr_eClass_ClassVolume:
  case pwr_eClass_DetachedClassVolume:
    classvolume(0);
    break;
  default:
    ;
  }
}

void wb_build::rootvolume( pwr_tVid vid)
{
  pwr_tStatus 		sumsts, plcsts;
  pwr_tOid		oid;
  pwr_tTime		modtime;
  pwr_tObjName		vname;
  pwr_tFileName		fname;
  pwr_tTime		dbs_time, rtt_time;
  pwr_tCmd		cmd;
  char			msg[80];

  wb_log::push();

  if ( !opt.manual) {
    // Build all plcpgm
    classlist( pwr_cClass_plc);
    if ( evenSts()) return;
    plcsts = sumsts = m_sts;

    // Build all XttGraph
    classlist( pwr_cClass_XttGraph);
    if ( evenSts()) return;
    if ( sumsts == PWRB__NOBUILT && m_sts != PWRB__NOBUILT)
      sumsts = m_sts;

    classlist( pwr_cClass_WebHandler);
    if ( evenSts()) return;
    if ( sumsts == PWRB__NOBUILT && m_sts != PWRB__NOBUILT)
      sumsts = m_sts;

    // Build all WebGraph
    classlist( pwr_cClass_WebGraph);
    if ( evenSts()) return;
    if ( sumsts == PWRB__NOBUILT && m_sts != PWRB__NOBUILT)
      sumsts = m_sts;

    classlist( pwr_cClass_Application);
    if ( evenSts()) return;
    if ( sumsts == PWRB__NOBUILT && m_sts != PWRB__NOBUILT)
      sumsts = m_sts;

    classlist( pwr_cClass_PlcProcess);
    if ( evenSts()) return;
    if ( sumsts == PWRB__NOBUILT && m_sts != PWRB__NOBUILT)
      sumsts = m_sts;
  }
  wb_log::pull();

  // Create loadfiles
  oid.oix = 0;
  oid.vid = m_session.vid();
  wb_attribute a = m_session.attribute( oid, "SysBody", "Modified");
  if ( !a) {
    m_sts = a.sts();
    return;
  }

  a.value( &modtime);
  if ( !a) {
    m_sts = a.sts();
    return;
  }

  cdh_ToLower( vname, m_session.name());
  sprintf( fname, "$pwrp_load/%s.dbs", vname);
  dcli_translate_filename( fname, fname);
  m_sts = dcli_file_time( fname, &dbs_time);

  // Get time for classvolumes
  wb_merep *merep = ((wb_vrep *)m_session)->merep();
  pwr_tTime mtime = pwr_cNTime;
  pwr_tTime t;
  pwr_tStatus sts;
  for ( wb_mvrep *mvrep = merep->volume( &sts);
	ODD(sts);
	mvrep = merep->nextVolume( &sts, mvrep->vid())) {
    mvrep->time( &t);
    if ( time_Acomp( &t, &mtime) == 1)
      mtime = t;
  }

  if ( opt.force || opt.manual || evenSts() || time_Acomp( &modtime, &dbs_time) == 1 ||
       time_Acomp( &mtime, &dbs_time) == 1 || plcsts != PWRB__NOBUILT) {
    m_sts = lfu_create_loadfile( (ldh_tSession *) &m_session);
    if ( evenSts()) return;
    m_sts = ldh_CreateLoadFile( (ldh_tSession *) &m_session);
    if ( evenSts()) return;

    sprintf( msg, "Build:    Volume   Loadfiles created volume %s", m_session.name());
    MsgWindow::message('I', msg, msgw_ePop_No);

    wb_log::log( &m_session, wlog_eCategory_VolumeBuild, m_session.vid());

    sumsts = PWRB__SUCCESS;
  }
  else
    m_sts = sumsts;

  cdh_uVolumeId	uvid;
  uvid.pwr = m_session.vid();
  sprintf( fname, "$pwrp_load/" load_cNameRttCrr,
	   uvid.v.vid_3, uvid.v.vid_2, uvid.v.vid_1, uvid.v.vid_0);
  dcli_translate_filename( fname, fname);
  m_sts = dcli_file_time( fname, &rtt_time);
  if ( opt.crossref && ( evenSts() || time_Acomp( &modtime, &rtt_time) == 1)) {
    strcpy( cmd, "create crossreferencefiles");
    m_wnav->command( cmd);
    if ( ODD(sumsts))
      sumsts = PWRB__SUCCESS;
	 
    sprintf( msg, "Build:    Volume   Crossreference file generated volume %s", m_session.name());
    MsgWindow::message('I', msg, msgw_ePop_No);
  }


  m_sts = sumsts;
}

void wb_build::classvolume( pwr_tVid vid)
{
  pwr_tCmd cmd;
  pwr_tFileName fname;
  pwr_tObjName name;
  pwr_tTime wbl_time, dbs_time, h_time;
  pwr_tStatus fsts;

  if ( vid == 0) {
    // Build current volume
    cdh_ToLower( name, m_session.name());
  }
  else {
    wb_env env = m_session.env();
    wb_volume v = env.volume(vid);
    if ( !v) {
      m_sts = v.sts();
      return;
    }
    strcpy( name, v.name());
  }

  // Get time for wb_load file
  sprintf( fname, "$pwrp_db/%s.wb_load", name);
  dcli_translate_filename( fname, fname);
  m_sts = dcli_file_time( fname, &wbl_time);
  if ( evenSts())
    return;

  // Get time for dbs file
  sprintf( fname, "$pwrp_load/%s.dbs", name);
  dcli_translate_filename( fname, fname);
  fsts = dcli_file_time( fname, &dbs_time);

  // Get time for classvolumes
  wb_merep *merep = ((wb_erep *)m_session.env())->merep();
  pwr_tTime mtime = pwr_cNTime;
  pwr_tTime t;
  pwr_tStatus sts;
  for ( wb_mvrep *mvrep = merep->volume( &sts);
	ODD(sts);
	mvrep = merep->nextVolume( &sts, mvrep->vid())) {
    if ( m_session.vid() == mvrep->vid())
      continue;
    // Check only system class and manufact class volumes
    if ( mvrep->vid() > cdh_cSystemClassVolMax &&
	 (mvrep->vid() < cdh_cManufactClassVolMin ||
	  mvrep->vid() > cdh_cManufactClassVolMax))
      continue;

    mvrep->time( &t);
    if ( time_Acomp( &t, &mtime) == 1)
      mtime = t;
  }


  // Create new loadfile
  if ( opt.force || EVEN(fsts) || wbl_time.tv_sec > dbs_time.tv_sec ||
       mtime.tv_sec > dbs_time.tv_sec) {
    sprintf( cmd, "create snapshot/file=\"$pwrp_db/%s.wb_load\"", name);
    m_sts = m_wnav->command( cmd);
  }
  else
    m_sts = PWRB__NOBUILT;

  // Get time for struct file
  sprintf( fname, "$pwrp_inc/pwr_%sclasses.h", name);
  dcli_translate_filename( fname, fname);
  fsts = dcli_file_time( fname, &h_time);

  // Create new struct file
  if ( opt.force || EVEN(fsts) || wbl_time.tv_sec > h_time.tv_sec) {
    sprintf( cmd, "create struct/file=\"$pwrp_db/%s.wb_load\"", name);
    m_sts = m_wnav->command( cmd);
  }

  if ( m_sts != PWRB__NOBUILT) {
    char msg[80];

    sprintf( msg, "Build:    Volume   %s", name);
    MsgWindow::message('I', msg, msgw_ePop_No);
  }
}

void wb_build::planthier( pwr_tOid oid)
{
  pwr_tStatus 		sumsts;

  m_hierarchy = oid;

  // Build all plcpgm
  classlist( pwr_cClass_plc);
  if ( evenSts()) return;
  sumsts = m_sts;

  // Build all XttGraph
  classlist( pwr_cClass_XttGraph);
  if ( evenSts()) return;
  if ( sumsts == PWRB__NOBUILT && m_sts != PWRB__NOBUILT)
    sumsts = m_sts;

  m_sts = sumsts;
}

void wb_build::nodehier( pwr_tOid oid)
{
  pwr_tStatus sumsts;
  m_hierarchy = oid;

  // Build all XttGraph
  classlist( pwr_cClass_XttGraph);
  if ( evenSts()) return;
  sumsts = m_sts;

  classlist( pwr_cClass_WebHandler);
  if ( evenSts()) return;
  if ( sumsts == PWRB__NOBUILT && m_sts != PWRB__NOBUILT)
    sumsts = m_sts;

  classlist( pwr_cClass_WebGraph);
  if ( evenSts()) return;
  if ( sumsts == PWRB__NOBUILT && m_sts != PWRB__NOBUILT)
    sumsts = m_sts;

  classlist( pwr_cClass_Application);
  if ( evenSts()) return;
  if ( sumsts == PWRB__NOBUILT && m_sts != PWRB__NOBUILT)
    sumsts = m_sts;

  classlist( pwr_cClass_PlcProcess);
  if ( evenSts()) return;
  if ( sumsts == PWRB__NOBUILT && m_sts != PWRB__NOBUILT)
    sumsts = m_sts;

  m_sts = sumsts;
}

void wb_build::plcpgm( pwr_tOid oid)
{
  int check_hierarchy = cdh_ObjidIsNotNull( m_hierarchy);
  int hierarchy_found = 0;

  wb_object o = m_session.object(oid);
  if ( !o) { 
    m_sts = o.sts(); 
    return; 
  }

  // Check that no ancestor is a LibHier
  for ( wb_object p = o.parent(); p.oddSts(); p = p.parent()) {
    if ( p.cid() == pwr_eClass_LibHier) {
      m_sts =  PWRB__INLIBHIER;
      return;
    }
    if ( check_hierarchy && cdh_ObjidIsEqual( m_hierarchy, p.oid()))
      hierarchy_found = 1;
  }

  if ( check_hierarchy && !hierarchy_found) {
    m_sts = PWRB__NOBUILT;
    return;
  }

  m_sts = utl_compile( (ldh_tSession *)&m_session, ldh_SessionToWB( (ldh_tSession *)&m_session), 
		     o.longName().name(cdh_mName_volumeStrict), 0, 0, 0, 
		      !opt.force, opt.debug, 0, 0); 
  if ( oddSts() && m_sts != GSX__NOMODIF) {
    char msg[200];

    sprintf( msg, "Build:    PlcPgm    %s", o.longName().name(cdh_mName_path | cdh_mName_object));
    MsgWindow::message('I', msg, msgw_ePop_No, oid);
  }
  else if ( m_sts == GSX__NOMODIF) {
    m_sts = PWRB__NOBUILT;
  }
}

void wb_build::xttgraph( pwr_tOid oid)
{
  pwr_tFileName src_fname, dest_fname;
  pwr_tCmd	cmd;
  pwr_tString80	action;
  pwr_tString80	name;
  pwr_tTime	dest_time, src_time;
  int 		check_hierarchy = cdh_ObjidIsNotNull( m_hierarchy);
  int 		hierarchy_found = 0;
  int 		is_frame, is_applet;
  char 		java_name[80];
  pwr_tStatus  	fsts;
  int		jexport;
  char		*s;

  wb_object o = m_session.object(oid);
  if ( !o) {
    m_sts = o.sts();
    return;
  }

  // Check that no ancestor is a LibHier
  for ( wb_object p = o.parent(); p.oddSts(); p = p.parent()) {
    if ( p.cid() == pwr_eClass_LibHier) {
      m_sts = PWRB__INLIBHIER;
      return;
    }
    if ( check_hierarchy && cdh_ObjidIsEqual( m_hierarchy, p.oid()))
      hierarchy_found = 1;
  }

  if ( check_hierarchy && !hierarchy_found) {
    m_sts = PWRB__NOBUILT;
    return;
  }

  wb_attribute a = m_session.attribute( oid, "RtBody", "Action");
  if ( !a) {
    m_sts = a.sts();
    return;
  }

  a.value( &action);
  if ( !a) {
    m_sts = a.sts();
    return;
  }

  if ( strstr( action, ".pwg")) {
    strcpy( src_fname, "$pwrp_pop/");
    strcat( src_fname, action);
    dcli_translate_filename( src_fname, src_fname);
    m_sts = dcli_file_time( src_fname, &src_time);
    if ( evenSts()) {
      m_sts = PWRB__NOBUILT;
      return;
    }

    strcpy( dest_fname, "$pwrp_exe/");
    strcat( dest_fname, action);
    dcli_translate_filename( dest_fname, dest_fname);
    m_sts = dcli_file_time( dest_fname, &dest_time);
    if ( opt.force || evenSts() || src_time.tv_sec > dest_time.tv_sec) {
      sprintf( cmd, "cp %s %s", src_fname, dest_fname);
      system( cmd);
      sprintf( cmd, "Build:    XttGraph copy $pwrp_pop/%s -> $pwrp_exe", action);
      MsgWindow::message( 'I', cmd, msgw_ePop_No, oid);

      strcpy( name, action);
      if (( s = strrchr( name, '.')))
	*s = 0;
      wb_log::log( wlog_eCategory_GeBuild, name, 0);
      m_sts = PWRB__SUCCESS;
    }
    else
      m_sts = PWRB__NOBUILT;

    jexport = 0;
    fsts = grow_IsJava( src_fname, &is_frame, &is_applet, java_name);
    if ( EVEN(fsts)) {
      m_sts = fsts;
      return;
    }
    if ( (is_frame || is_applet)  && 
	 strcmp( java_name, "") == 0) {
      // Java name is not yet set, use the default java name
      strcpy( java_name, action);
      if ( (s = strchr( java_name, '.')) != 0)
	*s = 0;
      java_name[0] = _toupper( java_name[0]);
    }
    if ( is_frame) {
      // Check exported java frame
      sprintf( dest_fname, "$pwrp_pop/%s.java", java_name);
      dcli_translate_filename( dest_fname, dest_fname);
      fsts = dcli_file_time( dest_fname, &dest_time);
      if ( opt.force || EVEN(fsts) || time_Acomp( &src_time, &dest_time) == 1)
	jexport = 1;
    }
    if ( is_applet) {
      // Check exported java applet
      sprintf( dest_fname, "$pwrp_pop/%s_A.java", java_name);
      dcli_translate_filename( dest_fname, dest_fname);
      fsts = dcli_file_time( dest_fname, &dest_time);
      if ( opt.force || EVEN(fsts) || time_Acomp( &src_time, &dest_time) == 1)
	jexport = 1;
    }
    if ( jexport) {
      if ( !m_wnav) {
	sprintf( cmd, "Build:    XttGraph  Unable to export java in this environment %s", action);
	MsgWindow::message('W', cmd, msgw_ePop_No, oid);
      }
      else {
	Ge *gectx = m_wnav->ge_new( action);
	strcpy( cmd, "export java");
	gectx->command( cmd);
	delete gectx;

	sprintf( cmd, "Build:    XttGraph  Export java %s", action);
	MsgWindow::message('I', cmd, msgw_ePop_No, oid);

	m_sts = PWRB__SUCCESS;
      }
    }
  }
}

void wb_build::webgraph( pwr_tOid oid)
{
  pwr_tFileName dest_fname;
  pwr_tCmd	cmd;
  pwr_tString80	java_name;
  pwr_tString80	name;
  pwr_tTime	dest_time, src_time;
  int 		check_hierarchy = cdh_ObjidIsNotNull( m_hierarchy);
  int 		hierarchy_found = 0;
  int 		is_frame, is_applet;
  char		jname[80];
  pwr_tStatus  	fsts;
  int		jexport;
  int 		found;
  pwr_tFileName found_file, file_spec;
  pwr_tFileName graph_name, dir;
  char		dev[80], type[80];
  int		version;

  wb_object o = m_session.object(oid);
  if ( !o) {
    m_sts = o.sts();
    return;
  }

  // Check that no ancestor is a LibHier
  for ( wb_object p = o.parent(); p.oddSts(); p = p.parent()) {
    if ( p.cid() == pwr_eClass_LibHier) {
      m_sts = PWRB__INLIBHIER;
      return;
    }
    if ( check_hierarchy && cdh_ObjidIsEqual( m_hierarchy, p.oid()))
      hierarchy_found = 1;
  }

  if ( check_hierarchy && !hierarchy_found) {
    m_sts = PWRB__NOBUILT;
    return;
  }

  wb_attribute a = m_session.attribute( oid, "RtBody", "Name");
  if ( !a) {
    m_sts = a.sts();
    return;
  }

  a.value( java_name);
  if ( !a) {
    m_sts = a.sts();
    return;
  }

  cdh_ToLower( java_name, java_name);
  java_name[0] = toupper(java_name[0]);

  // Get the .pwg file for this javaname
  sprintf( name, "$pwrp_pop/%s.pwg", cdh_Low(java_name));

  dcli_translate_filename( name, name);
  m_sts = dcli_file_time( name, &src_time);
  if ( evenSts()) {
    // Search in all pwg files
    found = 0;
    strcpy( file_spec, "$pwrp_pop/*.pwg");
    for ( fsts = dcli_search_file( file_spec, found_file, DCLI_DIR_SEARCH_INIT);
	  ODD(fsts);
	  fsts = dcli_search_file( file_spec, found_file, DCLI_DIR_SEARCH_NEXT)) {

      fsts = grow_IsJava( found_file, &is_frame, &is_applet, jname);
      if ( EVEN(fsts)) continue;

      if ( is_frame && strcmp( jname, java_name) == 0) {
	dcli_parse_filename( found_file, dev, dir, graph_name, type, &version);
	strcpy( name, found_file);
	found = 1;
	break;
      }      
    }
    dcli_search_file( file_spec, found_file, DCLI_DIR_SEARCH_END);

    if ( !found) {
      char msg[200];
      sprintf( msg, "Graph for %s not found", java_name);
      MsgWindow::message('E', msg, msgw_ePop_Yes, oid);
      return;
    }
  }

  m_sts = dcli_file_time( name, &src_time);
  if ( evenSts()) return;

  // Check exported java frame
  jexport = 0;
  sprintf( dest_fname, "$pwrp_pop/%s.java", java_name);
  dcli_translate_filename( dest_fname, dest_fname);
  fsts = dcli_file_time( dest_fname, &dest_time);
  if ( opt.force || EVEN(fsts) || time_Acomp( &src_time, &dest_time) == 1)
    jexport = 1;


  if ( jexport) {
    if ( !m_wnav) {
      sprintf( cmd, "Build:    WebGraph  Unable to export java in this environment %s", java_name);
      MsgWindow::message('W', cmd, msgw_ePop_No, oid);
    }
    else {
      Ge *gectx = m_wnav->ge_new( graph_name);
      strcpy( cmd, "export java");
      gectx->command( cmd);
      delete gectx;

      sprintf( cmd, "Build:    WebGraph  Export java %s", java_name);
      MsgWindow::message('I', cmd, msgw_ePop_No, oid);
      
      m_sts = PWRB__SUCCESS;
    }
  }
}

void wb_build::webhandler( pwr_tOid oid)
{
  pwr_tTime	modtime;
  pwr_tString80 file_name, name;
  pwr_tFileName fname;
  pwr_tTime 	ftime;
  pwr_tTime	xtthelp_time, html_time;
  char 		*s;
  pwr_tStatus   fsts;

  wb_object o = m_session.object(oid);
  if ( !o) {
    m_sts = o.sts();
    return;
  }

  modtime = o.modTime();

  wb_attribute a = m_session.attribute( oid, "RtBody", "FileName");
  if ( !a) {
    m_sts = a.sts();
    return;
  }
  a.value( &file_name);
  if ( !a) {
    m_sts = a.sts();
    return;
  }
  // Parse the name of the start page
  if ( (s = strrchr( file_name, '/')) ||
       (s = strrchr( file_name, '<')) ||
       (s = strrchr( file_name, ':')))
    strcpy( name, s+1);
  else
    strcpy( name, file_name);

  if ( (s = strrchr( name, '.')))
    *s = 0;

  sprintf( fname, "$pwrp_web/%s_opwin_menu.html", name);
  dcli_translate_filename( fname, fname);
  fsts = dcli_file_time( fname, &ftime);

  m_sts = PWRB__NOBUILT;
  if ( opt.force || EVEN(fsts) || time_Acomp( &modtime, &ftime) == 1) {
    // modtime > ftime
    m_sts = Graph::generate_web( (ldh_tSession *)&m_session);
    if ( evenSts()) return;

    char msg[200];
    sprintf( msg, "Build:    WebHandler Webpage generated %s", fname);
    MsgWindow::message( 'I', msg, msgw_ePop_No, oid);
  }

  // Check if xtthelp should be converted to html
  dcli_translate_filename( fname, "$pwrp_exe/xtt_help.dat");
  fsts = dcli_file_time( fname, &xtthelp_time);
  if ( EVEN(fsts)) return;
  
  dcli_translate_filename( fname, "$pwrp_web/xtt_help_index.html");
  fsts = dcli_file_time( fname, &html_time);
  if ( opt.force || EVEN(fsts) || time_Acomp( &xtthelp_time, &html_time) == 1) {
    system( "co_convert -d $pwrp_web -t $pwrp_exe/xtt_help.dat");

    char msg[200];
    sprintf( msg, "Build:    WebHandler xtt_help.dat converted to html");
    MsgWindow::message( 'I', msg, msgw_ePop_No, oid);
    m_sts = PWRB__SUCCESS;
  }
}

void wb_build::application( pwr_tOid oid)
{
  pwr_tString80 buildcmd;
  pwr_tCmd	cmd;
  int 		check_hierarchy = cdh_ObjidIsNotNull( m_hierarchy);
  int 		hierarchy_found = 0;
  int		sts;

  wb_object o = m_session.object(oid);
  if ( !o) {
    m_sts = o.sts();
    return;
  }

  // Check that no ancestor is a LibHier
  for ( wb_object p = o.parent(); p.oddSts(); p = p.parent()) {
    if ( p.cid() == pwr_eClass_LibHier) {
      m_sts =  PWRB__INLIBHIER;
      return;
    }
    if ( check_hierarchy && cdh_ObjidIsEqual( m_hierarchy, p.oid()))
      hierarchy_found = 1;
  }

  if ( check_hierarchy && !hierarchy_found) {
    m_sts = PWRB__NOBUILT;
    return;
  }

  wb_attribute a = m_session.attribute( oid, "DevBody", "BuildCmd");
  if ( !a) {
    m_sts = a.sts();
    return;
  }
  a.value( &buildcmd);
  if ( !a) {
    m_sts = a.sts();
    return;
  }

  if ( strcmp( buildcmd, "") == 0) {
    m_sts = PWRB__NOBUILT;
    return;
  }

  // Exectute the build command
  dcli_translate_filename( cmd, buildcmd);
  sts = system( cmd);
  if ( sts != 0) {
    char msg[300];

    sprintf( msg, "Build Application error %s", o.longName().name(cdh_mName_path | cdh_mName_object));
    MsgWindow::message( 'E', msg, msgw_ePop_Yes, oid);
    m_sts = PWRB__SUCCESS;
  }
  else {
    m_sts = PWRB__NOBUILT;
  }
}





