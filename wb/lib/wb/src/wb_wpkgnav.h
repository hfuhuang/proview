/* 
 * Proview   $Id: wb_wpkgnav.h,v 1.5 2007-01-04 07:29:04 claes Exp $
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

#ifndef wb_wpkgnav_h
#define wb_wpkgnav_h

#ifndef pwr_h
# include "pwr.h"
#endif

#ifndef wb_ldh_h
# include "wb_ldh.h"
#endif

#ifndef flow_h
#include "flow.h"
#endif

#ifndef flow_browapi_h
#include "flow_browapi.h"
#endif

#ifndef wb_wnav_brow_h
#include "wb_wnav_brow.h"
#endif

#define wpkgnav_cVersion	"X3.3b"
#define WPKGNAV_BROW_MAX	25

typedef enum {
  wpkg_mDisplayMode__ 			= 0,
  wpkg_mDisplayMode_FileOrderTime 	= 1 << 0,
  wpkg_mDisplayMode_FileDiff		= 1 << 1,
  wpkg_mDisplayMode_FilePath		= 1 << 2
} wpkg_mDisplayMode;

class WItemPkg;
class WItemPkgPackage;

class WPkgNav {
  public:
    WPkgNav(
	void 		*wa_parent_ctx,
	char 		*wa_name,
	wb_eUtility	wa_utility,
	pwr_tStatus 	*status);
    virtual ~WPkgNav();

    void 		*parent_ctx;
    char 		name[80];
    WNavBrow		*brow;
    void 		(*message_cb)( void *, char, char *);
    void 		(*set_clock_cursor_cb)( void *);
    void 		(*reset_cursor_cb)( void *);
    void 		(*change_value_cb)( void *);
    wb_eUtility		utility;
    int			displayed;
    int			display_mode;

    virtual void set_inputfocus() {}

    void message( char sev, char *text);
    int root_objects();
    void redraw();
    void enable_events();
    void set_display_mode( int mode) { display_mode = mode;}
    int get_select( WItemPkg ***items, int *item_cnt);
    void refresh_node( WItemPkg *item);
    WItemPkg *get_parent( WItemPkg *item);
    void delete_package( WItemPkg *item);
    void delete_package( WItemPkgPackage *item);
    void get_zoom( double *zoom_factor);
    void zoom( double zoom_factor);
    void unzoom();

    static int brow_cb( FlowCtx *ctx, flow_tEvent event);
    static int init_brow_cb( FlowCtx *fctx, void *client_data);

};

class WItemPkg {
  public:
    WItemPkg() : node(0) {};
    virtual int open_children( WNavBrow *brow, double x, double y, int display_mode) 
      { return 1;}
    virtual int close( WNavBrow *brow, double x, double y);

    brow_tNode		node;
    char	 	name[120];

    virtual ~WItemPkg() {}
};

class WItemPkgNode : public WItemPkg {
  public:
    WItemPkgNode( WNavBrow *brow, char *item_name, char *item_nodename, int item_bus,
		  pwr_mOpSys item_opsys, brow_tNode dest, flow_eDest dest_code);
    int	 open_children( WNavBrow *brow, double x, double y, int display_mode);
    char nodename[32];
    int bus;
    pwr_mOpSys opsys;
};

class WItemPkgPackage : public WItemPkg {
  public:
    WItemPkgPackage( WNavBrow *brow, char *item_name, char *item_packagename, pwr_tTime item_time,
		  brow_tNode dest, flow_eDest dest_code);
    int	 open_children( WNavBrow *brow, double x, double y, int display_mode);
    char packagename[120];
    pwr_tTime time;
};

class WItemPkgFile : public WItemPkg {
  public:
    WItemPkgFile( WNavBrow *brow, char *item_name, char *item_filename, pwr_tTime item_time,
		  int dmode, brow_tNode dest, flow_eDest dest_code);
    char filename[120];
    pwr_tTime time;
};

#endif




