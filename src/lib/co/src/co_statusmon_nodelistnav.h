/* 
 * Proview   $Id: co_statusmon_nodelistnav.h,v 1.1 2007-05-16 12:32:26 claes Exp $
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

#ifndef co_statusmon_nodelistnav_h
#define co_statusmon_nodelistnav_h

/* co_statusmon_nodelistnav.h -- Status Monitor. */


// Status is defined as int i xlib...

#include <vector>
#include "statussrv_utl.h"

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

class MsgWindow;

typedef enum {
  nodelistnav_eItemType_Node,
  nodelistnav_eItemType_Attr,
  nodelistnav_eItemType_AttrSts,
  nodelistnav_eItemType_AttrSysSts
} nodelistnav_eItemType;


class NodelistNavBrow {
  public:
    NodelistNavBrow( BrowCtx *brow_ctx, void *evl) : ctx(brow_ctx), nodelistnav(evl) {};
    ~NodelistNavBrow();

    BrowCtx		*ctx;
    void		*nodelistnav;
    brow_tNodeClass 	nc_event;
    brow_tNodeClass 	nc_node;
    brow_tNodeClass 	nc_attr;
    brow_tNodeClass 	nc_sys_sts_attr;
    brow_tNodeClass 	nc_sts_attr;
    flow_sAnnotPixmap 	*pixmap_leaf;
    flow_sAnnotPixmap 	*pixmap_map;
    flow_sAnnotPixmap 	*pixmap_openmap;
    flow_sAnnotPixmap 	*pixmap_attr;

    void free_pixmaps();
    void allocate_pixmaps();
    void create_nodeclasses();
    void brow_setup();
};

class NodeData {
 public:
  NodeData() :
    SystemStatus(0), SystemTime(pwr_cNTime), BootTime(pwr_cNTime), RestartTime(pwr_cNTime),
    Restarts(0)
    { 
      strcpy( Description, "");
      strcpy( SystemStatusStr, "");
    }
  pwr_tStatus		SystemStatus;
  char			SystemStatusStr[120];
  pwr_tString80		Description;
  pwr_tTime		SystemTime;
  pwr_tTime		BootTime;
  pwr_tTime		RestartTime;
  int			Restarts;
  char			Version[20];
};

class ItemNode;
class CoWow;

class NodelistNode {
 public:
  NodelistNode( char *name) :
    item(0), connection_sts(0), init_done(0)
    { 
      strncpy( node_name, name, sizeof(node_name));
    }
  char			node_name[80];
  ItemNode		*item;
  pwr_tStatus		connection_sts;
  int			init_done;  
};

class NodelistNav {
 public:
  NodelistNav( void *ev_parent_ctx, MsgWindow *nodelistnav_msg_window,
	       char *nodelistnav_nodename, int nodelistnav_msgw_pop);
  virtual ~NodelistNav();

  void 			*parent_ctx;
  NodelistNavBrow      	*brow;
  int			nodelist_size;
  int			max_size;
  vector<NodelistNode>  node_list;
  int			trace_started;
  int			scantime;
  int			first_scan;
  CoWow			*wow;
  MsgWindow		*msg_window;
  char			nodename[40];
  static const char 	config_file[40];
  int			msgw_pop;

  virtual void set_input_focus() {}
  virtual void trace_start() {}
  virtual void beep() {}

  void zoom( double zoom_factor);
  void unzoom();
  void set_nodraw();
  void reset_nodraw();
  void read();
  void draw();
  int update_nodes();
  void force_trace_scan();
  void message( pwr_tStatus sts, char *node, int idx, char *text);
  int select_node( int idx);
  void remove_node( char *name);
  int get_selected_node( char *name);
  void save();
  void add_node( char *name);
  void set_msgw_pop( int pop) { msgw_pop = pop;}

  static void attrvalue_to_string( int type_id, void *value_ptr, 
			    char *str, int size, int *len, char *format);    
  static int init_brow_cb( FlowCtx *fctx, void *client_data);
  static int brow_cb( FlowCtx *ctx, flow_tEvent event);
  static int trace_scan_bc( brow_tObject object, void *p);
  static int trace_connect_bc( brow_tObject object, char *name, char *attr, 
			       flow_eTraceType type, void **p);
  static int trace_disconnect_bc( brow_tObject object);
};

class ItemBase {
 public:
  ItemBase( NodelistNav *item_nodelistnav, char *item_name) : 
    nodelistnav(item_nodelistnav)
    {
      strcpy( name, item_name);
    }
  virtual ~ItemBase() {}

  nodelistnav_eItemType	type;
  NodelistNav		*nodelistnav;
  brow_tNode		node;
  char	 		name[120];

  virtual int	      	open_children( NodelistNav *nodelistnav, double x, double y) {return 1;}
  virtual int	       	close( NodelistNav *nodelistnav, double x, double y) {return 1;}
};

class ItemNode : public ItemBase {
 public:
  ItemNode( NodelistNav *item_nodelistnav, char *item_name, 
	    brow_tNode dest, flow_eDest dest_code);
  
  NodeData     	data;
  statussrv_sGetExtStatus xdata;
  int		syssts_open;

  int	       	open_children( NodelistNav *nodelistnav, double x, double y);
  int		close( NodelistNav *nodelistnav, double x, double y);
  int 		update_color( NodelistNav *nodelistnav, pwr_tStatus sts);
};

//! Item for a normal attribute.
class ItemAttr : public ItemBase {
  public:
    ItemAttr( NodelistNav *item_nodelistnav, char *item_name, char *attr,
	int attr_type, int attr_size, void *attr_value_p,
	brow_tNode dest, flow_eDest dest_code);
    void       	*value_p;
    char       	old_value[120];
    int        	first_scan;
    int	       	type_id;
    int	       	size;
};

//! Item for a system status attribute.
class ItemAttrSysSts : public ItemBase {
  public:
    ItemAttrSysSts( NodelistNav *item_nodelistnav, char *item_name, char *attr,
		     int attr_type, int attr_size, void *attr_value_p, void *attr_status_p,
		     ItemNode *attr_parent, brow_tNode dest, flow_eDest dest_code);
    void       	*value_p;
    void       	*status_p;
    char       	old_value[120];
    int        	first_scan;
    int	       	type_id;
    int	       	size;
    ItemNode    *parent;

    virtual int	      	open_children( NodelistNav *nodelistnav, double x, double y);
    virtual int	       	close( NodelistNav *nodelistnav, double x, double y);
};

//! Item for a server status attribute.
class ItemAttrSts : public ItemBase {
  public:
    ItemAttrSts( NodelistNav *item_nodelistnav, char *item_name, char *attr,
		 char *attr_value_p, 
		 pwr_tStatus *attr_status_p, char *attr_name_p, 
		 ItemAttrSysSts *attr_parent, brow_tNode dest, 
		 flow_eDest dest_code);
    char       	*value_p;
    pwr_tStatus *status_p;
    char       	*name_p;
    char       	old_value[120];
    char       	old_name[120];
    int        	first_scan;
    int	       	type_id;
    int	       	size;
    ItemAttrSysSts  *parent;
};

#endif

