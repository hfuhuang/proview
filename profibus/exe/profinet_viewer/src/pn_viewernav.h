/** 
 * Proview   $Id$
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

#ifndef pn_viewernav_h
#define pn_viewernav_h

/* pn_viewernav.h -- Profinet viewer */

#include <vector>

#ifndef pwr_h
# include "pwr.h"
#endif

#ifndef pwr_class_h
# include "pwr_class.h"
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

class PnDevice {
 public:
  PnDevice() {}
  unsigned char ipaddress[4];
  unsigned char macaddress[6];
  char devname[80];
  int vendorid;
  int deviceid;
};

class ItemDevice;

typedef enum {
  viewitem_eItemType_Device,
  viewitem_eItemType_DeviceAttr
} viewitem_eItemType;

class PnViewerNavBrow {
  public:
    PnViewerNavBrow( BrowCtx *brow_ctx, void *lvnav) : ctx(brow_ctx), viewernav(lvnav) {};
    ~PnViewerNavBrow();

    BrowCtx		*ctx;
    void		*viewernav;
    brow_tNodeClass 	nc_device;
    brow_tNodeClass 	nc_attr;
    flow_sAnnotPixmap 	*pixmap_map;
    flow_sAnnotPixmap 	*pixmap_openmap;
    flow_sAnnotPixmap 	*pixmap_attr;
    flow_sAnnotPixmap 	*pixmap_edit;

    void free_pixmaps();
    void allocate_pixmaps();
    void create_nodeclasses();
    void brow_setup();
};

class PnViewerNav {
  public:
    PnViewerNav( void *v_parent_ctx);
    virtual ~PnViewerNav();

    void 		*parent_ctx;
    PnViewerNavBrow	*brow;
    void 		(*change_value_cb)( void *);
    void 		(*message_cb)( void *, int, const char *);

    virtual void set_input_focus() {}

    void message( int severity, const char *msg);
    void zoom( double zoom_factor);
    void unzoom();
    void set( vector<PnDevice>& dev_vect);
    int set_attr_value( char *value_str);
    int check_attr_value();
    int get_selected_device( ItemDevice **device);
    void vendorid_to_str( unsigned int vendorid, char *vendorid_str, int size);
    void get_device_info( unsigned int vendorid, unsigned int deviceid,
			  char *info, int info_size, char *family, int family_size);

    static int init_brow_cb( FlowCtx *fctx, void *client_data);
    static int brow_cb( FlowCtx *ctx, flow_tEvent event);
};

class ItemDevice {
 public:
  ItemDevice( PnViewerNav *viewernav, const char *item_name,
	      unsigned char *item_ipaddress, unsigned char *item_macaddress, char *item_devname,
	      int vendorid, int deviceid,
	      brow_tNode dest, flow_eDest dest_code);
  viewitem_eItemType	type;
  PnViewerNav  	*viewernav;
  brow_tNode   	node;
  unsigned char	ipaddress[4];
  unsigned char macaddress[8];
  char		ipaddress_str[20];
  char		macaddress_str[20];
  char		devname[80];
  int	       	vendorid;
  int	       	deviceid;
  char		vendorid_str[80];
  char		deviceid_str[20];
  char		infotext[200];
  char		family[80];
  
  int open_children( PnViewerNav *viewernav);
  void close( PnViewerNav *viewernav);
  virtual ~ItemDevice() {}
};

class ItemDeviceAttr {
 public:
  ItemDeviceAttr( PnViewerNav *viewernav, const char *item_name, pwr_eType item_attr_type,
		  void *item_p, brow_tNode dest, flow_eDest dest_code);
    viewitem_eItemType	type;
    char		name[80];
    pwr_eType		attr_type;
    PnViewerNav		*viewernav;
    brow_tNode		node;
    void 		*p;

    virtual ~ItemDeviceAttr() {}
};

#endif



