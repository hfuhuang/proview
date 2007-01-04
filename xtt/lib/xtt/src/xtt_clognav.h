/* 
 * Proview   $Id: xtt_clognav.h,v 1.4 2007-01-04 08:22:46 claes Exp $
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

#ifndef xtt_clognav_h
#define xtt_clognav_h

/* xtt_clognav.h -- Console message window. */


// Status is defined as int i xlib...

#include <vector>
using namespace std;

#ifndef pwr_h
# include "pwr.h"
#endif

#ifndef rt_errh_h
# include "rt_errh.h"
#endif

#ifndef flow_h
#include "flow.h"
#endif

#ifndef flow_browctx_h
#include "flow_browctx.h"
#endif

#ifndef flow_browapi_h
#include "flow_browapi.h"
#endif

typedef enum {
  clognav_eItemType_Msg,
  clognav_eItemType_Restart
} clognav_eItemType;

class CLogNavFilter {
 public:
  CLogNavFilter() :
    show_success(true), show_info(true), show_warning(true), show_error(true),
    show_fatal(true), show_text(true) 
    { strcpy( str, "");}
  bool show_success;
  bool show_info;
  bool show_warning;
  bool show_error;
  bool show_fatal;
  bool show_text;
  char str[200];
};

class CLogNavBrow {
  public:
    CLogNavBrow( BrowCtx *brow_ctx, void *evl) : ctx(brow_ctx), clognav(evl) {};
    ~CLogNavBrow();

    BrowCtx		*ctx;
    void		*clognav;
    brow_tNodeClass 	nc_event;
    brow_tNodeClass 	nc_msg_info;
    brow_tNodeClass 	nc_msg_warning;
    brow_tNodeClass 	nc_msg_error;
    brow_tNodeClass 	nc_msg_fatal;
    brow_tNodeClass 	nc_restart;
    brow_tNodeClass 	nc_text;
    flow_sAnnotPixmap 	*pixmap_leaf;
    flow_sAnnotPixmap 	*pixmap_map;
    flow_sAnnotPixmap 	*pixmap_openmap;

    void free_pixmaps();
    void allocate_pixmaps();
    void create_nodeclasses();
    void brow_setup();
};

class CLogMsg {
 public:
  CLogMsg( errh_eSeverity msg_severity, char *msg_logger, int msg_pid,
       pwr_tTime msg_time, char *msg_text) :
    severity(msg_severity), pid(msg_pid), time(msg_time)
    { 
      strncpy( logger, msg_logger, sizeof(logger));
      strncpy( text, msg_text, sizeof(text)); 
    }
  errh_eSeverity 	severity;
  char 			logger[40];
  int			pid;
  pwr_tTime		time;
  char			text[200];
};

class CLogFile {
 public:
  CLogFile() 
    {
      strcpy( name, "");
    }
  CLogFile( char *file_name, pwr_tTime file_time) :
    time( file_time)
    {
      strcpy( name, file_name);
    }
  char name[200];
  pwr_tTime time;
};

class CLogNav {
 public:
  CLogNav( void *ev_parent_ctx);
  virtual ~CLogNav();

  void 			*parent_ctx;
  CLogNavBrow		*brow;
  CLogNavFilter		filter;
  int			clog_size;
  int			max_size;
  vector<CLogMsg>    	msg_list;		
  vector<CLogFile>    	file_list;		
  int			current_pos_high;
  int			current_pos_low;

  virtual void set_input_focus() {}

  void zoom( double zoom_factor);
  void unzoom();
  void set_nodraw();
  void reset_nodraw();
  void read( int *idx_list, int idx_cnt);
  void set_filter( bool success, bool info, bool warning, bool error, bool fatal,
		   bool text, char *str);
  void get_filter( bool *success, bool *info, bool *warning, bool *error, bool *fatal,
		   bool *text);
  void draw();
  void get_files();
  int next_file();
  int prev_file();
  int update();

  static int init_brow_cb( FlowCtx *fctx, void *client_data);
  static int brow_cb( FlowCtx *ctx, flow_tEvent event);
};

class ItemMsgBase {
 public:
  ItemMsgBase( CLogNav *item_clognav, char *item_name, brow_tNode dest) :
    clognav(item_clognav), node(dest)
    {
      strcpy( name, item_name);
    }
    clognav_eItemType	type;
    CLogNav		*clognav;
    brow_tNode		node;
    char	 	name[40];
};

class ItemMsgRestart : public ItemMsgBase {
 public:
    ItemMsgRestart( CLogNav *clognav, char *item_name, pwr_tTime item_time,
	brow_tNode dest, flow_eDest dest_code);
    pwr_tTime time;
};

class ItemMsg : public ItemMsgBase {
  public:
    ItemMsg( CLogNav *clognav, char *item_name, errh_eSeverity item_severity, 
	char *item_logger, int item_pid, pwr_tTime item_time, char *item_text,
	brow_tNode dest, flow_eDest dest_code);
    errh_eSeverity	severity;
    char		logger[40];
    int			pid;
    pwr_tTime		time;
    char		text[200];
};

#endif


