/* xtt_xnav_brow.cpp --

   PROVIEW/R  
   Copyright (C) 1996 by Comator Process AB.

   <Description>.  */

#include "flow_std.h"

#include <stdio.h>
#include <stdlib.h>

extern "C" {
#include "pwr_privilege.h"
#include "rt_gdh.h"
#include "rt_gdb.h"
#include "rt_gdh_msg.h"
#include "co_cdh.h"
#include "co_dcli.h"
#include "co_time.h"
#include "co_api.h"
#include "co_msg.h"
#include "pwr_baseclasses.h"
#include "rt_xnav_msg.h"
}

#include <Xm/Xm.h>
#include <Xm/XmP.h>
#include <Xm/Text.h>
#include <Mrm/MrmPublic.h>
#include <X11/Intrinsic.h>
#include <X11/Xlib.h>
#include <X11/Xutil.h>

#include "flow.h"
#include "flow_browctx.h"
#include "flow_browapi.h"
#include "flow_browwidget.h"

#include "xtt_xnav_brow.h"

#include "xnav_bitmap_leaf8.h"
#include "xnav_bitmap_leaf10.h"
#include "xnav_bitmap_leaf12.h"
#include "xnav_bitmap_leaf14.h"
#include "xnav_bitmap_leaf16.h"
#include "xnav_bitmap_leaf18.h"
#include "xnav_bitmap_leaf20.h"
#include "xnav_bitmap_leaf24.h"
#include "xnav_bitmap_map8.h"
#include "xnav_bitmap_map10.h"
#include "xnav_bitmap_map12.h"
#include "xnav_bitmap_map14.h"
#include "xnav_bitmap_map16.h"
#include "xnav_bitmap_map18.h"
#include "xnav_bitmap_map20.h"
#include "xnav_bitmap_map24.h"
#include "xnav_bitmap_openmap8.h"
#include "xnav_bitmap_openmap10.h"
#include "xnav_bitmap_openmap12.h"
#include "xnav_bitmap_openmap14.h"
#include "xnav_bitmap_openmap16.h"
#include "xnav_bitmap_openmap18.h"
#include "xnav_bitmap_openmap20.h"
#include "xnav_bitmap_openmap24.h"
#include "xnav_bitmap_ref8.h"
#include "xnav_bitmap_ref10.h"
#include "xnav_bitmap_ref12.h"
#include "xnav_bitmap_ref14.h"
#include "xnav_bitmap_ref16.h"
#include "xnav_bitmap_ref18.h"
#include "xnav_bitmap_ref20.h"
#include "xnav_bitmap_ref24.h"
#include "xnav_bitmap_crossref8.h"
#include "xnav_bitmap_crossref10.h"
#include "xnav_bitmap_crossref12.h"
#include "xnav_bitmap_crossref14.h"
#include "xnav_bitmap_crossref16.h"
#include "xnav_bitmap_crossref18.h"
#include "xnav_bitmap_crossref20.h"
#include "xnav_bitmap_crossref24.h"
#include "xnav_bitmap_openattr12.h"
#include "xnav_bitmap_attr8.h"
#include "xnav_bitmap_attr10.h"
#include "xnav_bitmap_attr12.h"
#include "xnav_bitmap_attr14.h"
#include "xnav_bitmap_attr16.h"
#include "xnav_bitmap_attr18.h"
#include "xnav_bitmap_attr20.h"
#include "xnav_bitmap_attr24.h"
#include "xnav_bitmap_attrarra8.h"
#include "xnav_bitmap_attrarra10.h"
#include "xnav_bitmap_attrarra12.h"
#include "xnav_bitmap_attrarra14.h"
#include "xnav_bitmap_attrarra16.h"
#include "xnav_bitmap_attrarra18.h"
#include "xnav_bitmap_attrarra20.h"
#include "xnav_bitmap_attrarra24.h"
#include "xnav_bitmap_attrarel8.h"
#include "xnav_bitmap_attrarel10.h"
#include "xnav_bitmap_attrarel12.h"
#include "xnav_bitmap_attrarel14.h"
#include "xnav_bitmap_attrarel16.h"
#include "xnav_bitmap_attrarel18.h"
#include "xnav_bitmap_attrarel20.h"
#include "xnav_bitmap_attrarel24.h"
#include "xnav_bitmap_action8.h"
#include "xnav_bitmap_action10.h"
#include "xnav_bitmap_action12.h"
#include "xnav_bitmap_action14.h"
#include "xnav_bitmap_action16.h"
#include "xnav_bitmap_action18.h"
#include "xnav_bitmap_action20.h"
#include "xnav_bitmap_action24.h"
#include "xnav_bitmap_crrread12.h"
#include "xnav_bitmap_crrwrite12.h"
#include "xnav_bitmap_script12.h"
#include "xnav_bitmap_list12.h"
#include "xnav_bitmap_graph12.h"
#include "xnav_bitmap_curve12.h"
#include "xnav_bitmap_file12.h"
#include "xnav_bitmap_symbol12.h"
#include "xnav_bitmap_morehelp12.h"
#include "xnav_bitmap_closehelp12.h"
#include "xnav_bitmap_object8.h"
#include "xnav_bitmap_object10.h"
#include "xnav_bitmap_object12.h"
#include "xnav_bitmap_object14.h"
#include "xnav_bitmap_object16.h"
#include "xnav_bitmap_object18.h"
#include "xnav_bitmap_object20.h"
#include "xnav_bitmap_object24.h"
#include "xnav_bitmap_attrenum12.h"
#include "xnav_bitmap_attrmask12.h"
#include "xnav_bitmap_block2_8.h"
#include "xnav_bitmap_block2_10.h"
#include "xnav_bitmap_block2_12.h"
#include "xnav_bitmap_block2_14.h"
#include "xnav_bitmap_block2_16.h"
#include "xnav_bitmap_block2_18.h"
#include "xnav_bitmap_block2_20.h"
#include "xnav_bitmap_block2_24.h"
#include "xnav_bitmap_blockr_8.h"
#include "xnav_bitmap_blockr_10.h"
#include "xnav_bitmap_blockr_12.h"
#include "xnav_bitmap_blockr_14.h"
#include "xnav_bitmap_blockr_16.h"
#include "xnav_bitmap_blockr_18.h"
#include "xnav_bitmap_blockr_20.h"
#include "xnav_bitmap_blockr_24.h"
#include "xnav_bitmap_blockd_8.h"
#include "xnav_bitmap_blockd_10.h"
#include "xnav_bitmap_blockd_12.h"
#include "xnav_bitmap_blockd_14.h"
#include "xnav_bitmap_blockd_16.h"
#include "xnav_bitmap_blockd_18.h"
#include "xnav_bitmap_blockd_20.h"
#include "xnav_bitmap_blockd_24.h"
#include "xnav_bitmap_alarm2_8.h"
#include "xnav_bitmap_alarm2_10.h"
#include "xnav_bitmap_alarm2_12.h"
#include "xnav_bitmap_alarm2_14.h"
#include "xnav_bitmap_alarm2_16.h"
#include "xnav_bitmap_alarm2_18.h"
#include "xnav_bitmap_alarm2_20.h"
#include "xnav_bitmap_alarm2_24.h"
#include "xnav_bitmap_arrowdown8.h"
#include "xnav_bitmap_arrowdown10.h"
#include "xnav_bitmap_arrowdown12.h"
#include "xnav_bitmap_arrowdown14.h"
#include "xnav_bitmap_arrowdown16.h"
#include "xnav_bitmap_arrowdown18.h"
#include "xnav_bitmap_arrowdown20.h"
#include "xnav_bitmap_arrowdown24.h"
#include "xnav_bitmap_arrowright8.h"
#include "xnav_bitmap_arrowright10.h"
#include "xnav_bitmap_arrowright12.h"
#include "xnav_bitmap_arrowright14.h"
#include "xnav_bitmap_arrowright16.h"
#include "xnav_bitmap_arrowright18.h"
#include "xnav_bitmap_arrowright20.h"
#include "xnav_bitmap_arrowright24.h"


#if 0
XNavBrow::~XNavBrow()
{
  if ( this != ((XNav *)xnav)->brow_stack[0] && this != ((XNav *)xnav)->brow)
  {
    brow_DeleteSecondaryCtx( ctx);
    free_pixmaps();
  }
}
#endif

//
//  Free pixmaps
//
void XNavBrow::free_pixmaps()
{
  brow_FreeAnnotPixmap( ctx, pixmap_leaf);
  brow_FreeAnnotPixmap( ctx, pixmap_map);
  brow_FreeAnnotPixmap( ctx, pixmap_openmap);
  brow_FreeAnnotPixmap( ctx, pixmap_ref);
  brow_FreeAnnotPixmap( ctx, pixmap_crossref);
  brow_FreeAnnotPixmap( ctx, pixmap_openattr);
  brow_FreeAnnotPixmap( ctx, pixmap_attr);
  brow_FreeAnnotPixmap( ctx, pixmap_attrarray);
  brow_FreeAnnotPixmap( ctx, pixmap_attrarrayelem);
  brow_FreeAnnotPixmap( ctx, pixmap_crrread);
  brow_FreeAnnotPixmap( ctx, pixmap_crrwrite);
  brow_FreeAnnotPixmap( ctx, pixmap_script);
  brow_FreeAnnotPixmap( ctx, pixmap_list);
  brow_FreeAnnotPixmap( ctx, pixmap_file);
  brow_FreeAnnotPixmap( ctx, pixmap_graph);
  brow_FreeAnnotPixmap( ctx, pixmap_curve);
  brow_FreeAnnotPixmap( ctx, pixmap_action);
  brow_FreeAnnotPixmap( ctx, pixmap_symbol);
  brow_FreeAnnotPixmap( ctx, pixmap_morehelp);
  brow_FreeAnnotPixmap( ctx, pixmap_closehelp);
  brow_FreeAnnotPixmap( ctx, pixmap_object);
  brow_FreeAnnotPixmap( ctx, pixmap_attrenum);
  brow_FreeAnnotPixmap( ctx, pixmap_attrmask);
  brow_FreeAnnotPixmap( ctx, pixmap_block2);
  brow_FreeAnnotPixmap( ctx, pixmap_blockr);
  brow_FreeAnnotPixmap( ctx, pixmap_blockd);
  brow_FreeAnnotPixmap( ctx, pixmap_alarm2);
  brow_FreeAnnotPixmap( ctx, pixmap_arrowdown);
  brow_FreeAnnotPixmap( ctx, pixmap_arrowright);
}

//
//  Create pixmaps for leaf, closed map and open map
//
void XNavBrow::allocate_pixmaps()
{
	flow_sPixmapData pixmap_data;
	int i;
	
          i = 0;
	  pixmap_data[i].width =xnav_bitmap_leaf8_width;
	  pixmap_data[i].height =xnav_bitmap_leaf8_height;
	  pixmap_data[i++].bits = (char *)xnav_bitmap_leaf8_bits;
	  pixmap_data[i].width =xnav_bitmap_leaf10_width;
	  pixmap_data[i].height =xnav_bitmap_leaf10_height;
	  pixmap_data[i++].bits = (char *)xnav_bitmap_leaf10_bits;
	  pixmap_data[i].width =xnav_bitmap_leaf12_width;
	  pixmap_data[i].height =xnav_bitmap_leaf12_height;
	  pixmap_data[i++].bits = (char *)xnav_bitmap_leaf12_bits;
	  pixmap_data[i].width =xnav_bitmap_leaf14_width;
	  pixmap_data[i].height =xnav_bitmap_leaf14_height;
	  pixmap_data[i++].bits = (char *)xnav_bitmap_leaf14_bits;
	  pixmap_data[i].width =xnav_bitmap_leaf16_width;
	  pixmap_data[i].height =xnav_bitmap_leaf16_height;
	  pixmap_data[i++].bits = (char *)xnav_bitmap_leaf16_bits;
	  pixmap_data[i].width =xnav_bitmap_leaf18_width;
	  pixmap_data[i].height =xnav_bitmap_leaf18_height;
	  pixmap_data[i++].bits = (char *)xnav_bitmap_leaf18_bits;
	  pixmap_data[i].width =xnav_bitmap_leaf20_width;
	  pixmap_data[i].height =xnav_bitmap_leaf20_height;
	  pixmap_data[i++].bits = (char *)xnav_bitmap_leaf20_bits;
	  pixmap_data[i].width =xnav_bitmap_leaf20_width;
	  pixmap_data[i].height =xnav_bitmap_leaf20_height;
	  pixmap_data[i++].bits = (char *)xnav_bitmap_leaf20_bits;
	  pixmap_data[i].width =xnav_bitmap_leaf24_width;
	  pixmap_data[i].height =xnav_bitmap_leaf24_height;
	  pixmap_data[i++].bits = (char *)xnav_bitmap_leaf24_bits;

	  brow_AllocAnnotPixmap( ctx, &pixmap_data, &pixmap_leaf);

          i = 0;
	  pixmap_data[i].width =xnav_bitmap_map8_width;
	  pixmap_data[i].height =xnav_bitmap_map8_height;
	  pixmap_data[i++].bits = (char *)xnav_bitmap_map8_bits;
	  pixmap_data[i].width =xnav_bitmap_map10_width;
	  pixmap_data[i].height =xnav_bitmap_map10_height;
	  pixmap_data[i++].bits = (char *)xnav_bitmap_map10_bits;
	  pixmap_data[i].width =xnav_bitmap_map12_width;
	  pixmap_data[i].height =xnav_bitmap_map12_height;
	  pixmap_data[i++].bits = (char *)xnav_bitmap_map12_bits;
	  pixmap_data[i].width =xnav_bitmap_map14_width;
	  pixmap_data[i].height =xnav_bitmap_map14_height;
	  pixmap_data[i++].bits = (char *)xnav_bitmap_map14_bits;
	  pixmap_data[i].width =xnav_bitmap_map16_width;
	  pixmap_data[i].height =xnav_bitmap_map16_height;
	  pixmap_data[i++].bits = (char *)xnav_bitmap_map16_bits;
	  pixmap_data[i].width =xnav_bitmap_map18_width;
	  pixmap_data[i].height =xnav_bitmap_map18_height;
	  pixmap_data[i++].bits = (char *)xnav_bitmap_map18_bits;
	  pixmap_data[i].width =xnav_bitmap_map20_width;
	  pixmap_data[i].height =xnav_bitmap_map20_height;
	  pixmap_data[i++].bits = (char *)xnav_bitmap_map20_bits;
	  pixmap_data[i].width =xnav_bitmap_map20_width;
	  pixmap_data[i].height =xnav_bitmap_map20_height;
	  pixmap_data[i++].bits = (char *)xnav_bitmap_map20_bits;
	  pixmap_data[i].width =xnav_bitmap_map24_width;
	  pixmap_data[i].height =xnav_bitmap_map24_height;
	  pixmap_data[i++].bits = (char *)xnav_bitmap_map24_bits;

	  brow_AllocAnnotPixmap( ctx, &pixmap_data, &pixmap_map);

          i = 0;
	  pixmap_data[i].width =xnav_bitmap_openmap8_width;
	  pixmap_data[i].height =xnav_bitmap_openmap8_height;
	  pixmap_data[i++].bits = (char *)xnav_bitmap_openmap8_bits;
	  pixmap_data[i].width =xnav_bitmap_openmap10_width;
	  pixmap_data[i].height =xnav_bitmap_openmap10_height;
	  pixmap_data[i++].bits = (char *)xnav_bitmap_openmap10_bits;
	  pixmap_data[i].width =xnav_bitmap_openmap12_width;
	  pixmap_data[i].height =xnav_bitmap_openmap12_height;
	  pixmap_data[i++].bits = (char *)xnav_bitmap_openmap12_bits;
	  pixmap_data[i].width =xnav_bitmap_openmap14_width;
	  pixmap_data[i].height =xnav_bitmap_openmap14_height;
	  pixmap_data[i++].bits = (char *)xnav_bitmap_openmap14_bits;
	  pixmap_data[i].width =xnav_bitmap_openmap16_width;
	  pixmap_data[i].height =xnav_bitmap_openmap16_height;
	  pixmap_data[i++].bits = (char *)xnav_bitmap_openmap16_bits;
	  pixmap_data[i].width =xnav_bitmap_openmap18_width;
	  pixmap_data[i].height =xnav_bitmap_openmap18_height;
	  pixmap_data[i++].bits = (char *)xnav_bitmap_openmap18_bits;
	  pixmap_data[i].width =xnav_bitmap_openmap20_width;
	  pixmap_data[i].height =xnav_bitmap_openmap20_height;
	  pixmap_data[i++].bits = (char *)xnav_bitmap_openmap20_bits;
	  pixmap_data[i].width =xnav_bitmap_openmap20_width;
	  pixmap_data[i].height =xnav_bitmap_openmap20_height;
	  pixmap_data[i++].bits = (char *)xnav_bitmap_openmap20_bits;
	  pixmap_data[i].width =xnav_bitmap_openmap24_width;
	  pixmap_data[i].height =xnav_bitmap_openmap24_height;
	  pixmap_data[i++].bits = (char *)xnav_bitmap_openmap24_bits;

	  brow_AllocAnnotPixmap( ctx, &pixmap_data, &pixmap_openmap);

          i = 0;
	  pixmap_data[i].width =xnav_bitmap_ref8_width;
	  pixmap_data[i].height =xnav_bitmap_ref8_height;
	  pixmap_data[i++].bits = (char *)xnav_bitmap_ref8_bits;
	  pixmap_data[i].width =xnav_bitmap_ref10_width;
	  pixmap_data[i].height =xnav_bitmap_ref10_height;
	  pixmap_data[i++].bits = (char *)xnav_bitmap_ref10_bits;
	  pixmap_data[i].width =xnav_bitmap_ref12_width;
	  pixmap_data[i].height =xnav_bitmap_ref12_height;
	  pixmap_data[i++].bits = (char *)xnav_bitmap_ref12_bits;
	  pixmap_data[i].width =xnav_bitmap_ref14_width;
	  pixmap_data[i].height =xnav_bitmap_ref14_height;
	  pixmap_data[i++].bits = (char *)xnav_bitmap_ref14_bits;
	  pixmap_data[i].width =xnav_bitmap_ref16_width;
	  pixmap_data[i].height =xnav_bitmap_ref16_height;
	  pixmap_data[i++].bits = (char *)xnav_bitmap_ref16_bits;
	  pixmap_data[i].width =xnav_bitmap_ref18_width;
	  pixmap_data[i].height =xnav_bitmap_ref18_height;
	  pixmap_data[i++].bits = (char *)xnav_bitmap_ref18_bits;
	  pixmap_data[i].width =xnav_bitmap_ref20_width;
	  pixmap_data[i].height =xnav_bitmap_ref20_height;
	  pixmap_data[i++].bits = (char *)xnav_bitmap_ref20_bits;
	  pixmap_data[i].width =xnav_bitmap_ref20_width;
	  pixmap_data[i].height =xnav_bitmap_ref20_height;
	  pixmap_data[i++].bits = (char *)xnav_bitmap_ref20_bits;
	  pixmap_data[i].width =xnav_bitmap_ref24_width;
	  pixmap_data[i].height =xnav_bitmap_ref24_height;
	  pixmap_data[i++].bits = (char *)xnav_bitmap_ref24_bits;

	  brow_AllocAnnotPixmap( ctx, &pixmap_data, &pixmap_ref);

          i = 0;
	  pixmap_data[i].width =xnav_bitmap_crossref8_width;
	  pixmap_data[i].height =xnav_bitmap_crossref8_height;
	  pixmap_data[i++].bits = (char *)xnav_bitmap_crossref8_bits;
	  pixmap_data[i].width =xnav_bitmap_crossref10_width;
	  pixmap_data[i].height =xnav_bitmap_crossref10_height;
	  pixmap_data[i++].bits = (char *)xnav_bitmap_crossref10_bits;
	  pixmap_data[i].width =xnav_bitmap_crossref12_width;
	  pixmap_data[i].height =xnav_bitmap_crossref12_height;
	  pixmap_data[i++].bits = (char *)xnav_bitmap_crossref12_bits;
	  pixmap_data[i].width =xnav_bitmap_crossref14_width;
	  pixmap_data[i].height =xnav_bitmap_crossref14_height;
	  pixmap_data[i++].bits = (char *)xnav_bitmap_crossref14_bits;
	  pixmap_data[i].width =xnav_bitmap_crossref16_width;
	  pixmap_data[i].height =xnav_bitmap_crossref16_height;
	  pixmap_data[i++].bits = (char *)xnav_bitmap_crossref16_bits;
	  pixmap_data[i].width =xnav_bitmap_crossref18_width;
	  pixmap_data[i].height =xnav_bitmap_crossref18_height;
	  pixmap_data[i++].bits = (char *)xnav_bitmap_crossref18_bits;
	  pixmap_data[i].width =xnav_bitmap_crossref20_width;
	  pixmap_data[i].height =xnav_bitmap_crossref20_height;
	  pixmap_data[i++].bits = (char *)xnav_bitmap_crossref20_bits;
	  pixmap_data[i].width =xnav_bitmap_crossref20_width;
	  pixmap_data[i].height =xnav_bitmap_crossref20_height;
	  pixmap_data[i++].bits = (char *)xnav_bitmap_crossref20_bits;
	  pixmap_data[i].width =xnav_bitmap_crossref24_width;
	  pixmap_data[i].height =xnav_bitmap_crossref24_height;
	  pixmap_data[i++].bits = (char *)xnav_bitmap_crossref24_bits;

	  brow_AllocAnnotPixmap( ctx, &pixmap_data, &pixmap_crossref);

          i = 0;
	  pixmap_data[i].width =xnav_bitmap_openattr12_width;
	  pixmap_data[i].height =xnav_bitmap_openattr12_height;
	  pixmap_data[i++].bits = (char *)xnav_bitmap_openattr12_bits;
	  pixmap_data[i].width =xnav_bitmap_openattr12_width;
	  pixmap_data[i].height =xnav_bitmap_openattr12_height;
	  pixmap_data[i++].bits = (char *)xnav_bitmap_openattr12_bits;
	  pixmap_data[i].width =xnav_bitmap_openattr12_width;
	  pixmap_data[i].height =xnav_bitmap_openattr12_height;
	  pixmap_data[i++].bits = (char *)xnav_bitmap_openattr12_bits;
	  pixmap_data[i].width =xnav_bitmap_openattr12_width;
	  pixmap_data[i].height =xnav_bitmap_openattr12_height;
	  pixmap_data[i++].bits = (char *)xnav_bitmap_openattr12_bits;
	  pixmap_data[i].width =xnav_bitmap_openattr12_width;
	  pixmap_data[i].height =xnav_bitmap_openattr12_height;
	  pixmap_data[i++].bits = (char *)xnav_bitmap_openattr12_bits;
	  pixmap_data[i].width =xnav_bitmap_openattr12_width;
	  pixmap_data[i].height =xnav_bitmap_openattr12_height;
	  pixmap_data[i++].bits = (char *)xnav_bitmap_openattr12_bits;
	  pixmap_data[i].width =xnav_bitmap_openattr12_width;
	  pixmap_data[i].height =xnav_bitmap_openattr12_height;
	  pixmap_data[i++].bits = (char *)xnav_bitmap_openattr12_bits;
	  pixmap_data[i].width =xnav_bitmap_openattr12_width;
	  pixmap_data[i].height =xnav_bitmap_openattr12_height;
	  pixmap_data[i++].bits = (char *)xnav_bitmap_openattr12_bits;
	  pixmap_data[i].width =xnav_bitmap_openattr12_width;
	  pixmap_data[i].height =xnav_bitmap_openattr12_height;
	  pixmap_data[i++].bits = (char *)xnav_bitmap_openattr12_bits;

	  brow_AllocAnnotPixmap( ctx, &pixmap_data, 
		&pixmap_openattr);

          i = 0;
	  pixmap_data[i].width =xnav_bitmap_attr8_width;
	  pixmap_data[i].height =xnav_bitmap_attr8_height;
	  pixmap_data[i++].bits = (char *)xnav_bitmap_attr8_bits;
	  pixmap_data[i].width =xnav_bitmap_attr10_width;
	  pixmap_data[i].height =xnav_bitmap_attr10_height;
	  pixmap_data[i++].bits = (char *)xnav_bitmap_attr10_bits;
	  pixmap_data[i].width =xnav_bitmap_attr12_width;
	  pixmap_data[i].height =xnav_bitmap_attr12_height;
	  pixmap_data[i++].bits = (char *)xnav_bitmap_attr12_bits;
	  pixmap_data[i].width =xnav_bitmap_attr14_width;
	  pixmap_data[i].height =xnav_bitmap_attr14_height;
	  pixmap_data[i++].bits = (char *)xnav_bitmap_attr14_bits;
	  pixmap_data[i].width =xnav_bitmap_attr16_width;
	  pixmap_data[i].height =xnav_bitmap_attr16_height;
	  pixmap_data[i++].bits = (char *)xnav_bitmap_attr16_bits;
	  pixmap_data[i].width =xnav_bitmap_attr18_width;
	  pixmap_data[i].height =xnav_bitmap_attr18_height;
	  pixmap_data[i++].bits = (char *)xnav_bitmap_attr18_bits;
	  pixmap_data[i].width =xnav_bitmap_attr20_width;
	  pixmap_data[i].height =xnav_bitmap_attr20_height;
	  pixmap_data[i++].bits = (char *)xnav_bitmap_attr20_bits;
	  pixmap_data[i].width =xnav_bitmap_attr20_width;
	  pixmap_data[i].height =xnav_bitmap_attr20_height;
	  pixmap_data[i++].bits = (char *)xnav_bitmap_attr20_bits;
	  pixmap_data[i].width =xnav_bitmap_attr24_width;
	  pixmap_data[i].height =xnav_bitmap_attr24_height;
	  pixmap_data[i++].bits = (char *)xnav_bitmap_attr24_bits;

	  brow_AllocAnnotPixmap( ctx, &pixmap_data, &pixmap_attr);

          i = 0;
	  pixmap_data[i].width =xnav_bitmap_attrarra8_width;
	  pixmap_data[i].height =xnav_bitmap_attrarra8_height;
	  pixmap_data[i++].bits = (char *)xnav_bitmap_attrarra8_bits;
	  pixmap_data[i].width =xnav_bitmap_attrarra10_width;
	  pixmap_data[i].height =xnav_bitmap_attrarra10_height;
	  pixmap_data[i++].bits = (char *)xnav_bitmap_attrarra10_bits;
	  pixmap_data[i].width =xnav_bitmap_attrarra12_width;
	  pixmap_data[i].height =xnav_bitmap_attrarra12_height;
	  pixmap_data[i++].bits = (char *)xnav_bitmap_attrarra12_bits;
	  pixmap_data[i].width =xnav_bitmap_attrarra14_width;
	  pixmap_data[i].height =xnav_bitmap_attrarra14_height;
	  pixmap_data[i++].bits = (char *)xnav_bitmap_attrarra14_bits;
	  pixmap_data[i].width =xnav_bitmap_attrarra16_width;
	  pixmap_data[i].height =xnav_bitmap_attrarra16_height;
	  pixmap_data[i++].bits = (char *)xnav_bitmap_attrarra16_bits;
	  pixmap_data[i].width =xnav_bitmap_attrarra18_width;
	  pixmap_data[i].height =xnav_bitmap_attrarra18_height;
	  pixmap_data[i++].bits = (char *)xnav_bitmap_attrarra18_bits;
	  pixmap_data[i].width =xnav_bitmap_attrarra20_width;
	  pixmap_data[i].height =xnav_bitmap_attrarra20_height;
	  pixmap_data[i++].bits = (char *)xnav_bitmap_attrarra20_bits;
	  pixmap_data[i].width =xnav_bitmap_attrarra20_width;
	  pixmap_data[i].height =xnav_bitmap_attrarra20_height;
	  pixmap_data[i++].bits = (char *)xnav_bitmap_attrarra20_bits;
	  pixmap_data[i].width =xnav_bitmap_attrarra24_width;
	  pixmap_data[i].height =xnav_bitmap_attrarra24_height;
	  pixmap_data[i++].bits = (char *)xnav_bitmap_attrarra24_bits;

	  brow_AllocAnnotPixmap( ctx, &pixmap_data, &pixmap_attrarray);

          i = 0;
	  pixmap_data[i].width =xnav_bitmap_attrarel8_width;
	  pixmap_data[i].height =xnav_bitmap_attrarel8_height;
	  pixmap_data[i++].bits = (char *)xnav_bitmap_attrarel8_bits;
	  pixmap_data[i].width =xnav_bitmap_attrarel10_width;
	  pixmap_data[i].height =xnav_bitmap_attrarel10_height;
	  pixmap_data[i++].bits = (char *)xnav_bitmap_attrarel10_bits;
	  pixmap_data[i].width =xnav_bitmap_attrarel12_width;
	  pixmap_data[i].height =xnav_bitmap_attrarel12_height;
	  pixmap_data[i++].bits = (char *)xnav_bitmap_attrarel12_bits;
	  pixmap_data[i].width =xnav_bitmap_attrarel14_width;
	  pixmap_data[i].height =xnav_bitmap_attrarel14_height;
	  pixmap_data[i++].bits = (char *)xnav_bitmap_attrarel14_bits;
	  pixmap_data[i].width =xnav_bitmap_attrarel16_width;
	  pixmap_data[i].height =xnav_bitmap_attrarel16_height;
	  pixmap_data[i++].bits = (char *)xnav_bitmap_attrarel16_bits;
	  pixmap_data[i].width =xnav_bitmap_attrarel18_width;
	  pixmap_data[i].height =xnav_bitmap_attrarel18_height;
	  pixmap_data[i++].bits = (char *)xnav_bitmap_attrarel18_bits;
	  pixmap_data[i].width =xnav_bitmap_attrarel20_width;
	  pixmap_data[i].height =xnav_bitmap_attrarel20_height;
	  pixmap_data[i++].bits = (char *)xnav_bitmap_attrarel20_bits;
	  pixmap_data[i].width =xnav_bitmap_attrarel20_width;
	  pixmap_data[i].height =xnav_bitmap_attrarel20_height;
	  pixmap_data[i++].bits = (char *)xnav_bitmap_attrarel20_bits;
	  pixmap_data[i].width =xnav_bitmap_attrarel24_width;
	  pixmap_data[i].height =xnav_bitmap_attrarel24_height;
	  pixmap_data[i++].bits = (char *)xnav_bitmap_attrarel24_bits;

	  brow_AllocAnnotPixmap( ctx, &pixmap_data, &pixmap_attrarrayelem);

          i = 0;
	  pixmap_data[i].width =xnav_bitmap_action8_width;
	  pixmap_data[i].height =xnav_bitmap_action8_height;
	  pixmap_data[i++].bits = (char *)xnav_bitmap_action8_bits;
	  pixmap_data[i].width =xnav_bitmap_action10_width;
	  pixmap_data[i].height =xnav_bitmap_action10_height;
	  pixmap_data[i++].bits = (char *)xnav_bitmap_action10_bits;
	  pixmap_data[i].width =xnav_bitmap_action12_width;
	  pixmap_data[i].height =xnav_bitmap_action12_height;
	  pixmap_data[i++].bits = (char *)xnav_bitmap_action12_bits;
	  pixmap_data[i].width =xnav_bitmap_action14_width;
	  pixmap_data[i].height =xnav_bitmap_action14_height;
	  pixmap_data[i++].bits = (char *)xnav_bitmap_action14_bits;
	  pixmap_data[i].width =xnav_bitmap_action16_width;
	  pixmap_data[i].height =xnav_bitmap_action16_height;
	  pixmap_data[i++].bits = (char *)xnav_bitmap_action16_bits;
	  pixmap_data[i].width =xnav_bitmap_action18_width;
	  pixmap_data[i].height =xnav_bitmap_action18_height;
	  pixmap_data[i++].bits = (char *)xnav_bitmap_action18_bits;
	  pixmap_data[i].width =xnav_bitmap_action20_width;
	  pixmap_data[i].height =xnav_bitmap_action20_height;
	  pixmap_data[i++].bits = (char *)xnav_bitmap_action20_bits;
	  pixmap_data[i].width =xnav_bitmap_action20_width;
	  pixmap_data[i].height =xnav_bitmap_action20_height;
	  pixmap_data[i++].bits = (char *)xnav_bitmap_action20_bits;
	  pixmap_data[i].width =xnav_bitmap_action24_width;
	  pixmap_data[i].height =xnav_bitmap_action24_height;
	  pixmap_data[i++].bits = (char *)xnav_bitmap_action24_bits;

	  brow_AllocAnnotPixmap( ctx, &pixmap_data, &pixmap_action);

          for ( i = 0; i < 9; i++)
          {
	    pixmap_data[i].width =xnav_bitmap_crrread12_width;
	    pixmap_data[i].height =xnav_bitmap_crrread12_height;
	    pixmap_data[i].bits = (char *)xnav_bitmap_crrread12_bits;
          }

	  brow_AllocAnnotPixmap( ctx, &pixmap_data, &pixmap_crrread);

          for ( i = 0; i < 9; i++)
          {
	    pixmap_data[i].width =xnav_bitmap_crrwrite12_width;
	    pixmap_data[i].height =xnav_bitmap_crrwrite12_height;
	    pixmap_data[i].bits = (char *)xnav_bitmap_crrwrite12_bits;
          }

	  brow_AllocAnnotPixmap( ctx, &pixmap_data, &pixmap_crrwrite);

          for ( i = 0; i < 9; i++)
          {
	    pixmap_data[i].width =xnav_bitmap_script12_width;
	    pixmap_data[i].height =xnav_bitmap_script12_height;
	    pixmap_data[i].bits = (char *)xnav_bitmap_script12_bits;
          }

	  brow_AllocAnnotPixmap( ctx, &pixmap_data, &pixmap_script);

          for ( i = 0; i < 9; i++)
          {
	    pixmap_data[i].width =xnav_bitmap_list12_width;
	    pixmap_data[i].height =xnav_bitmap_list12_height;
	    pixmap_data[i].bits = (char *)xnav_bitmap_list12_bits;
          }

	  brow_AllocAnnotPixmap( ctx, &pixmap_data, &pixmap_list);

          for ( i = 0; i < 9; i++)
          {
	    pixmap_data[i].width =xnav_bitmap_graph12_width;
	    pixmap_data[i].height =xnav_bitmap_graph12_height;
	    pixmap_data[i].bits = (char *)xnav_bitmap_graph12_bits;
          }

	  brow_AllocAnnotPixmap( ctx, &pixmap_data, &pixmap_graph);

          for ( i = 0; i < 9; i++)
          {
	    pixmap_data[i].width =xnav_bitmap_curve12_width;
	    pixmap_data[i].height =xnav_bitmap_curve12_height;
	    pixmap_data[i].bits = (char *)xnav_bitmap_curve12_bits;
          }

	  brow_AllocAnnotPixmap( ctx, &pixmap_data, &pixmap_curve);

          for ( i = 0; i < 9; i++)
          {
	    pixmap_data[i].width =xnav_bitmap_file12_width;
	    pixmap_data[i].height =xnav_bitmap_file12_height;
	    pixmap_data[i].bits = (char *)xnav_bitmap_file12_bits;
          }

	  brow_AllocAnnotPixmap( ctx, &pixmap_data, &pixmap_file);

          for ( i = 0; i < 9; i++)
          {
	    pixmap_data[i].width =xnav_bitmap_symbol12_width;
	    pixmap_data[i].height =xnav_bitmap_symbol12_height;
	    pixmap_data[i].bits = (char *)xnav_bitmap_symbol12_bits;
          }

	  brow_AllocAnnotPixmap( ctx, &pixmap_data, &pixmap_symbol);

          for ( i = 0; i < 9; i++)
          {
	    pixmap_data[i].width =xnav_bitmap_morehelp12_width;
	    pixmap_data[i].height =xnav_bitmap_morehelp12_height;
	    pixmap_data[i].bits = (char *)xnav_bitmap_morehelp12_bits;
          }

	  brow_AllocAnnotPixmap( ctx, &pixmap_data, &pixmap_morehelp);

          for ( i = 0; i < 9; i++)
          {
	    pixmap_data[i].width =xnav_bitmap_closehelp12_width;
	    pixmap_data[i].height =xnav_bitmap_closehelp12_height;
	    pixmap_data[i].bits = (char *)xnav_bitmap_closehelp12_bits;
          }

	  brow_AllocAnnotPixmap( ctx, &pixmap_data, &pixmap_closehelp);

          i = 0;
	  pixmap_data[i].width =xnav_bitmap_object8_width;
	  pixmap_data[i].height =xnav_bitmap_object8_height;
	  pixmap_data[i++].bits = (char *)xnav_bitmap_object8_bits;
	  pixmap_data[i].width =xnav_bitmap_object10_width;
	  pixmap_data[i].height =xnav_bitmap_object10_height;
	  pixmap_data[i++].bits = (char *)xnav_bitmap_object10_bits;
	  pixmap_data[i].width =xnav_bitmap_object12_width;
	  pixmap_data[i].height =xnav_bitmap_object12_height;
	  pixmap_data[i++].bits = (char *)xnav_bitmap_object12_bits;
	  pixmap_data[i].width =xnav_bitmap_object14_width;
	  pixmap_data[i].height =xnav_bitmap_object14_height;
	  pixmap_data[i++].bits = (char *)xnav_bitmap_object14_bits;
	  pixmap_data[i].width =xnav_bitmap_object16_width;
	  pixmap_data[i].height =xnav_bitmap_object16_height;
	  pixmap_data[i++].bits = (char *)xnav_bitmap_object16_bits;
	  pixmap_data[i].width =xnav_bitmap_object18_width;
	  pixmap_data[i].height =xnav_bitmap_object18_height;
	  pixmap_data[i++].bits = (char *)xnav_bitmap_object18_bits;
	  pixmap_data[i].width =xnav_bitmap_object20_width;
	  pixmap_data[i].height =xnav_bitmap_object20_height;
	  pixmap_data[i++].bits = (char *)xnav_bitmap_object20_bits;
	  pixmap_data[i].width =xnav_bitmap_object20_width;
	  pixmap_data[i].height =xnav_bitmap_object20_height;
	  pixmap_data[i++].bits = (char *)xnav_bitmap_object20_bits;
	  pixmap_data[i].width =xnav_bitmap_object24_width;
	  pixmap_data[i].height =xnav_bitmap_object24_height;
	  pixmap_data[i++].bits = (char *)xnav_bitmap_object24_bits;

	  brow_AllocAnnotPixmap( ctx, &pixmap_data, &pixmap_object);

          for ( i = 0; i < 9; i++) {
	    pixmap_data[i].width =xnav_bitmap_attrenum12_width;
	    pixmap_data[i].height =xnav_bitmap_attrenum12_height;
	    pixmap_data[i].bits = (char *)xnav_bitmap_attrenum12_bits;
          }
	  brow_AllocAnnotPixmap( ctx, &pixmap_data, &pixmap_attrenum);

          for ( i = 0; i < 9; i++) {
	    pixmap_data[i].width =xnav_bitmap_attrmask12_width;
	    pixmap_data[i].height =xnav_bitmap_attrmask12_height;
	    pixmap_data[i].bits = (char *)xnav_bitmap_attrmask12_bits;
          }
	  brow_AllocAnnotPixmap( ctx, &pixmap_data, &pixmap_attrmask);

          i = 0;
	  pixmap_data[i].width =xnav_bitmap_block2_8_width;
	  pixmap_data[i].height =xnav_bitmap_block2_8_height;
	  pixmap_data[i++].bits = (char *)xnav_bitmap_block2_8_bits;
	  pixmap_data[i].width =xnav_bitmap_block2_10_width;
	  pixmap_data[i].height =xnav_bitmap_block2_10_height;
	  pixmap_data[i++].bits = (char *)xnav_bitmap_block2_10_bits;
	  pixmap_data[i].width =xnav_bitmap_block2_12_width;
	  pixmap_data[i].height =xnav_bitmap_block2_12_height;
	  pixmap_data[i++].bits = (char *)xnav_bitmap_block2_12_bits;
	  pixmap_data[i].width =xnav_bitmap_block2_14_width;
	  pixmap_data[i].height =xnav_bitmap_block2_14_height;
	  pixmap_data[i++].bits = (char *)xnav_bitmap_block2_14_bits;
	  pixmap_data[i].width =xnav_bitmap_block2_16_width;
	  pixmap_data[i].height =xnav_bitmap_block2_16_height;
	  pixmap_data[i++].bits = (char *)xnav_bitmap_block2_16_bits;
	  pixmap_data[i].width =xnav_bitmap_block2_18_width;
	  pixmap_data[i].height =xnav_bitmap_block2_18_height;
	  pixmap_data[i++].bits = (char *)xnav_bitmap_block2_18_bits;
	  pixmap_data[i].width =xnav_bitmap_block2_20_width;
	  pixmap_data[i].height =xnav_bitmap_block2_20_height;
	  pixmap_data[i++].bits = (char *)xnav_bitmap_block2_20_bits;
	  pixmap_data[i].width =xnav_bitmap_block2_20_width;
	  pixmap_data[i].height =xnav_bitmap_block2_20_height;
	  pixmap_data[i++].bits = (char *)xnav_bitmap_block2_20_bits;
	  pixmap_data[i].width =xnav_bitmap_block2_24_width;
	  pixmap_data[i].height =xnav_bitmap_block2_24_height;
	  pixmap_data[i++].bits = (char *)xnav_bitmap_block2_24_bits;

	  brow_AllocAnnotPixmap( ctx, &pixmap_data, &pixmap_block2);

          i = 0;
	  pixmap_data[i].width =xnav_bitmap_blockd_8_width;
	  pixmap_data[i].height =xnav_bitmap_blockd_8_height;
	  pixmap_data[i++].bits = (char *)xnav_bitmap_blockd_8_bits;
	  pixmap_data[i].width =xnav_bitmap_blockd_10_width;
	  pixmap_data[i].height =xnav_bitmap_blockd_10_height;
	  pixmap_data[i++].bits = (char *)xnav_bitmap_blockd_10_bits;
	  pixmap_data[i].width =xnav_bitmap_blockd_12_width;
	  pixmap_data[i].height =xnav_bitmap_blockd_12_height;
	  pixmap_data[i++].bits = (char *)xnav_bitmap_blockd_12_bits;
	  pixmap_data[i].width =xnav_bitmap_blockd_14_width;
	  pixmap_data[i].height =xnav_bitmap_blockd_14_height;
	  pixmap_data[i++].bits = (char *)xnav_bitmap_blockd_14_bits;
	  pixmap_data[i].width =xnav_bitmap_blockd_16_width;
	  pixmap_data[i].height =xnav_bitmap_blockd_16_height;
	  pixmap_data[i++].bits = (char *)xnav_bitmap_blockd_16_bits;
	  pixmap_data[i].width =xnav_bitmap_blockd_18_width;
	  pixmap_data[i].height =xnav_bitmap_blockd_18_height;
	  pixmap_data[i++].bits = (char *)xnav_bitmap_blockd_18_bits;
	  pixmap_data[i].width =xnav_bitmap_blockd_20_width;
	  pixmap_data[i].height =xnav_bitmap_blockd_20_height;
	  pixmap_data[i++].bits = (char *)xnav_bitmap_blockd_20_bits;
	  pixmap_data[i].width =xnav_bitmap_blockd_20_width;
	  pixmap_data[i].height =xnav_bitmap_blockd_20_height;
	  pixmap_data[i++].bits = (char *)xnav_bitmap_blockd_20_bits;
	  pixmap_data[i].width =xnav_bitmap_blockd_24_width;
	  pixmap_data[i].height =xnav_bitmap_blockd_24_height;
	  pixmap_data[i++].bits = (char *)xnav_bitmap_blockd_24_bits;

	  brow_AllocAnnotPixmap( ctx, &pixmap_data, &pixmap_blockd);

          i = 0;
	  pixmap_data[i].width =xnav_bitmap_blockr_8_width;
	  pixmap_data[i].height =xnav_bitmap_blockr_8_height;
	  pixmap_data[i++].bits = (char *)xnav_bitmap_blockr_8_bits;
	  pixmap_data[i].width =xnav_bitmap_blockr_10_width;
	  pixmap_data[i].height =xnav_bitmap_blockr_10_height;
	  pixmap_data[i++].bits = (char *)xnav_bitmap_blockr_10_bits;
	  pixmap_data[i].width =xnav_bitmap_blockr_12_width;
	  pixmap_data[i].height =xnav_bitmap_blockr_12_height;
	  pixmap_data[i++].bits = (char *)xnav_bitmap_blockr_12_bits;
	  pixmap_data[i].width =xnav_bitmap_blockr_14_width;
	  pixmap_data[i].height =xnav_bitmap_blockr_14_height;
	  pixmap_data[i++].bits = (char *)xnav_bitmap_blockr_14_bits;
	  pixmap_data[i].width =xnav_bitmap_blockr_16_width;
	  pixmap_data[i].height =xnav_bitmap_blockr_16_height;
	  pixmap_data[i++].bits = (char *)xnav_bitmap_blockr_16_bits;
	  pixmap_data[i].width =xnav_bitmap_blockr_18_width;
	  pixmap_data[i].height =xnav_bitmap_blockr_18_height;
	  pixmap_data[i++].bits = (char *)xnav_bitmap_blockr_18_bits;
	  pixmap_data[i].width =xnav_bitmap_blockr_20_width;
	  pixmap_data[i].height =xnav_bitmap_blockr_20_height;
	  pixmap_data[i++].bits = (char *)xnav_bitmap_blockr_20_bits;
	  pixmap_data[i].width =xnav_bitmap_blockr_20_width;
	  pixmap_data[i].height =xnav_bitmap_blockr_20_height;
	  pixmap_data[i++].bits = (char *)xnav_bitmap_blockr_20_bits;
	  pixmap_data[i].width =xnav_bitmap_blockr_24_width;
	  pixmap_data[i].height =xnav_bitmap_blockr_24_height;
	  pixmap_data[i++].bits = (char *)xnav_bitmap_blockr_24_bits;

	  brow_AllocAnnotPixmap( ctx, &pixmap_data, &pixmap_blockr);

          i = 0;
	  pixmap_data[i].width =xnav_bitmap_alarm2_8_width;
	  pixmap_data[i].height =xnav_bitmap_alarm2_8_height;
	  pixmap_data[i++].bits = (char *)xnav_bitmap_alarm2_8_bits;
	  pixmap_data[i].width =xnav_bitmap_alarm2_10_width;
	  pixmap_data[i].height =xnav_bitmap_alarm2_10_height;
	  pixmap_data[i++].bits = (char *)xnav_bitmap_alarm2_10_bits;
	  pixmap_data[i].width =xnav_bitmap_alarm2_12_width;
	  pixmap_data[i].height =xnav_bitmap_alarm2_12_height;
	  pixmap_data[i++].bits = (char *)xnav_bitmap_alarm2_12_bits;
	  pixmap_data[i].width =xnav_bitmap_alarm2_14_width;
	  pixmap_data[i].height =xnav_bitmap_alarm2_14_height;
	  pixmap_data[i++].bits = (char *)xnav_bitmap_alarm2_14_bits;
	  pixmap_data[i].width =xnav_bitmap_alarm2_16_width;
	  pixmap_data[i].height =xnav_bitmap_alarm2_16_height;
	  pixmap_data[i++].bits = (char *)xnav_bitmap_alarm2_16_bits;
	  pixmap_data[i].width =xnav_bitmap_alarm2_18_width;
	  pixmap_data[i].height =xnav_bitmap_alarm2_18_height;
	  pixmap_data[i++].bits = (char *)xnav_bitmap_alarm2_18_bits;
	  pixmap_data[i].width =xnav_bitmap_alarm2_20_width;
	  pixmap_data[i].height =xnav_bitmap_alarm2_20_height;
	  pixmap_data[i++].bits = (char *)xnav_bitmap_alarm2_20_bits;
	  pixmap_data[i].width =xnav_bitmap_alarm2_20_width;
	  pixmap_data[i].height =xnav_bitmap_alarm2_20_height;
	  pixmap_data[i++].bits = (char *)xnav_bitmap_alarm2_20_bits;
	  pixmap_data[i].width =xnav_bitmap_alarm2_24_width;
	  pixmap_data[i].height =xnav_bitmap_alarm2_24_height;
	  pixmap_data[i++].bits = (char *)xnav_bitmap_alarm2_24_bits;

	  brow_AllocAnnotPixmap( ctx, &pixmap_data, &pixmap_alarm2);

          i = 0;
	  pixmap_data[i].width =xnav_bitmap_arrowdown8_width;
	  pixmap_data[i].height =xnav_bitmap_arrowdown8_height;
	  pixmap_data[i++].bits = (char *)xnav_bitmap_arrowdown8_bits;
	  pixmap_data[i].width =xnav_bitmap_arrowdown10_width;
	  pixmap_data[i].height =xnav_bitmap_arrowdown10_height;
	  pixmap_data[i++].bits = (char *)xnav_bitmap_arrowdown10_bits;
	  pixmap_data[i].width =xnav_bitmap_arrowdown12_width;
	  pixmap_data[i].height =xnav_bitmap_arrowdown12_height;
	  pixmap_data[i++].bits = (char *)xnav_bitmap_arrowdown12_bits;
	  pixmap_data[i].width =xnav_bitmap_arrowdown14_width;
	  pixmap_data[i].height =xnav_bitmap_arrowdown14_height;
	  pixmap_data[i++].bits = (char *)xnav_bitmap_arrowdown14_bits;
	  pixmap_data[i].width =xnav_bitmap_arrowdown16_width;
	  pixmap_data[i].height =xnav_bitmap_arrowdown16_height;
	  pixmap_data[i++].bits = (char *)xnav_bitmap_arrowdown16_bits;
	  pixmap_data[i].width =xnav_bitmap_arrowdown18_width;
	  pixmap_data[i].height =xnav_bitmap_arrowdown18_height;
	  pixmap_data[i++].bits = (char *)xnav_bitmap_arrowdown18_bits;
	  pixmap_data[i].width =xnav_bitmap_arrowdown20_width;
	  pixmap_data[i].height =xnav_bitmap_arrowdown20_height;
	  pixmap_data[i++].bits = (char *)xnav_bitmap_arrowdown20_bits;
	  pixmap_data[i].width =xnav_bitmap_arrowdown20_width;
	  pixmap_data[i].height =xnav_bitmap_arrowdown20_height;
	  pixmap_data[i++].bits = (char *)xnav_bitmap_arrowdown20_bits;
	  pixmap_data[i].width =xnav_bitmap_arrowdown24_width;
	  pixmap_data[i].height =xnav_bitmap_arrowdown24_height;
	  pixmap_data[i++].bits = (char *)xnav_bitmap_arrowdown24_bits;

	  brow_AllocAnnotPixmap( ctx, &pixmap_data, &pixmap_arrowdown);

          i = 0;
	  pixmap_data[i].width =xnav_bitmap_arrowright8_width;
	  pixmap_data[i].height =xnav_bitmap_arrowright8_height;
	  pixmap_data[i++].bits = (char *)xnav_bitmap_arrowright8_bits;
	  pixmap_data[i].width =xnav_bitmap_arrowright10_width;
	  pixmap_data[i].height =xnav_bitmap_arrowright10_height;
	  pixmap_data[i++].bits = (char *)xnav_bitmap_arrowright10_bits;
	  pixmap_data[i].width =xnav_bitmap_arrowright12_width;
	  pixmap_data[i].height =xnav_bitmap_arrowright12_height;
	  pixmap_data[i++].bits = (char *)xnav_bitmap_arrowright12_bits;
	  pixmap_data[i].width =xnav_bitmap_arrowright14_width;
	  pixmap_data[i].height =xnav_bitmap_arrowright14_height;
	  pixmap_data[i++].bits = (char *)xnav_bitmap_arrowright14_bits;
	  pixmap_data[i].width =xnav_bitmap_arrowright16_width;
	  pixmap_data[i].height =xnav_bitmap_arrowright16_height;
	  pixmap_data[i++].bits = (char *)xnav_bitmap_arrowright16_bits;
	  pixmap_data[i].width =xnav_bitmap_arrowright18_width;
	  pixmap_data[i].height =xnav_bitmap_arrowright18_height;
	  pixmap_data[i++].bits = (char *)xnav_bitmap_arrowright18_bits;
	  pixmap_data[i].width =xnav_bitmap_arrowright20_width;
	  pixmap_data[i].height =xnav_bitmap_arrowright20_height;
	  pixmap_data[i++].bits = (char *)xnav_bitmap_arrowright20_bits;
	  pixmap_data[i].width =xnav_bitmap_arrowright20_width;
	  pixmap_data[i].height =xnav_bitmap_arrowright20_height;
	  pixmap_data[i++].bits = (char *)xnav_bitmap_arrowright20_bits;
	  pixmap_data[i].width =xnav_bitmap_arrowright24_width;
	  pixmap_data[i].height =xnav_bitmap_arrowright24_height;
	  pixmap_data[i++].bits = (char *)xnav_bitmap_arrowright24_bits;

	  brow_AllocAnnotPixmap( ctx, &pixmap_data, &pixmap_arrowright);
}

//
// Create nodeclasses
//
void XNavBrow::create_nodeclasses()
{
  allocate_pixmaps();

  // Create common-class

  brow_CreateNodeClass( ctx, "NavigatorDefault", 
		flow_eNodeGroup_Common, &nc_object);
  brow_AddFrame( nc_object, 0, 0, 20, 0.8, flow_eDrawType_Line, -1, 1);
  brow_AddAnnotPixmap( nc_object, 0, 0.2, 0.1, flow_eDrawType_Line, 2, 0);
  brow_AddAnnotPixmap( nc_object, 1, 1.1, 0.1, flow_eDrawType_Line, 2, 0);
  brow_AddAnnotPixmap( nc_object, 2, 1.1, 0.1, flow_eDrawType_LineRed, 2, 0);
  brow_AddAnnotPixmap( nc_object, 3, 1.8, 0.1, flow_eDrawType_Line, 2, 0);
  brow_AddAnnot( nc_object, 2.9, 0.6, 0,
		flow_eDrawType_TextHelveticaBold, 2, flow_eAnnotType_OneLine, 
		0);
  brow_AddAnnot( nc_object, 7.9, 0.6, 1,
		flow_eDrawType_TextHelveticaBold, 2, flow_eAnnotType_OneLine, 
		1);
  brow_AddAnnot( nc_object, 11.9, 0.6, 2,
		flow_eDrawType_TextHelveticaBold, 2, flow_eAnnotType_OneLine, 
		1);

  // Create attribute nodeclass

  brow_CreateNodeClass( ctx, "NavigatorAttr", 
		flow_eNodeGroup_Common, &nc_attr);
  brow_AddFrame( nc_attr, 0, 0, 20, 0.8, flow_eDrawType_Line, -1, 1);
  brow_AddAnnotPixmap( nc_attr, 0, 0.2, 0.1, flow_eDrawType_Line, 2, 0);
  brow_AddAnnot( nc_attr, 2.9, 0.6, 0,
		flow_eDrawType_TextHelveticaBold, 2, flow_eAnnotType_OneLine, 
		0);
  brow_AddAnnot( nc_attr, 8.9, 0.6, 1,
		flow_eDrawType_TextHelveticaBold, 2, flow_eAnnotType_OneLine, 
		1);

  // Create table nodeclass

  brow_CreateNodeClass( ctx, "NavigatorTable", 
		flow_eNodeGroup_Common, &nc_table);
  brow_AddFrame( nc_table, 0, 0, 20, 0.8, flow_eDrawType_Line, -1, 1);
  brow_AddAnnotPixmap( nc_table, 0, 0.2, 0.1, flow_eDrawType_Line, 2, 0);
  brow_AddAnnotPixmap( nc_table, 1, 1.1, 0.1, flow_eDrawType_Line, 2, 0);
  brow_AddAnnot( nc_table, 2.9, 0.6, 0,
		flow_eDrawType_TextHelveticaBold, 2, flow_eAnnotType_OneLine, 
		0);
  brow_AddAnnot( nc_table, 8.9, 0.6, 1,
		flow_eDrawType_TextHelveticaBold, 2, flow_eAnnotType_OneLine, 
		1);
  brow_AddAnnot( nc_table, 12.9, 0.6, 2,
		flow_eDrawType_TextHelveticaBold, 2, flow_eAnnotType_OneLine, 
		1);
  brow_AddAnnot( nc_table, 16.9, 0.6, 3,
		flow_eDrawType_TextHelveticaBold, 2, flow_eAnnotType_OneLine, 
		1);
  brow_AddAnnot( nc_table, 20.9, 0.6, 4,
		flow_eDrawType_TextHelveticaBold, 2, flow_eAnnotType_OneLine, 
		1);
  brow_AddAnnot( nc_table, 24.9, 0.6, 5,
		flow_eDrawType_TextHelveticaBold, 2, flow_eAnnotType_OneLine, 
		1);
  brow_AddAnnot( nc_table, 28.9, 0.6, 6,
		flow_eDrawType_TextHelveticaBold, 2, flow_eAnnotType_OneLine, 
		1);
  brow_AddAnnot( nc_table, 32.9, 0.6, 7,
		flow_eDrawType_TextHelveticaBold, 2, flow_eAnnotType_OneLine, 
		1);
  brow_AddAnnot( nc_table, 35.9, 0.6, 8,
		flow_eDrawType_TextHelveticaBold, 2, flow_eAnnotType_OneLine, 
		1);
  brow_AddAnnot( nc_table, 38.9, 0.6, 9,
		flow_eDrawType_TextHelveticaBold, 2, flow_eAnnotType_OneLine, 
		1);

  // Create text

  brow_CreateNodeClass( ctx, "NavigatorText", 
		flow_eNodeGroup_Common, &nc_text);
  brow_AddFrame( nc_text, 0, 0, 20, 0.8, flow_eDrawType_Line, -1, 1);
  brow_AddAnnotPixmap( nc_text, 0, 0.2, 0.1, flow_eDrawType_Line, 2, 0);
  brow_AddAnnotPixmap( nc_text, 1, 1.1, 0.1, flow_eDrawType_Line, 2, 0);
  brow_AddAnnot( nc_text, 2.9, 0.6, 0,
		flow_eDrawType_TextHelvetica, 2, flow_eAnnotType_OneLine, 
		0);
  brow_AddAnnot( nc_text, 7.9, 0.6, 1,
		flow_eDrawType_TextHelvetica, 2, flow_eAnnotType_OneLine, 
		1);
  brow_AddAnnot( nc_text, 11.9, 0.6, 2,
		flow_eDrawType_TextHelvetica, 2, flow_eAnnotType_OneLine, 
		1);

  // Create Header

  brow_CreateNodeClass( ctx, "NavigatorHead", 
		flow_eNodeGroup_Common, &nc_header);
  brow_AddFrame( nc_header, 0, 0, 20, 0.8, flow_eDrawType_LineGray, 2, 1);
  brow_AddAnnotPixmap( nc_header, 0, 0.2, 0.1, flow_eDrawType_Line, 2, 0);
  brow_AddAnnot( nc_header, 2.9, 0.6, 0,
		flow_eDrawType_TextHelveticaBold, 2, flow_eAnnotType_OneLine, 
		0);
  brow_AddAnnot( nc_header, 8.9, 0.6, 1,
		flow_eDrawType_TextHelveticaBold, 2, flow_eAnnotType_OneLine, 
		1);

  // Create Header2

  brow_CreateNodeClass( ctx, "NavigatorHeadLarge", 
		flow_eNodeGroup_Common, &nc_headerlarge);
  brow_AddFrame( nc_headerlarge, 0, 0, 20, 1.4, flow_eDrawType_Line, -1, 1);
  brow_AddAnnotPixmap( nc_headerlarge, 0, 0.5, 0.1, flow_eDrawType_Line, 2, 0);
  brow_AddAnnot( nc_headerlarge, 2.9, 1.0, 0,
		flow_eDrawType_TextHelveticaBold, 4, flow_eAnnotType_OneLine, 
		0);
  brow_AddAnnot( nc_headerlarge, 8.9, 1.0, 1,
		flow_eDrawType_TextHelveticaBold, 4, flow_eAnnotType_OneLine, 
		1);

  // Create TableHeader

  brow_CreateNodeClass( ctx, "NavigatorTableHead", 
		flow_eNodeGroup_Common, &nc_table_header);
  brow_AddFrame( nc_table_header, 0, 0, 20, 0.8, flow_eDrawType_LineGray, 2, 1);
  brow_AddAnnotPixmap( nc_table_header, 0, 0.2, 0.1, flow_eDrawType_Line, 2, 0);
  brow_AddAnnot( nc_table_header, 2.9, 0.6, 0,
		flow_eDrawType_TextHelveticaBold, 2, flow_eAnnotType_OneLine, 
		0);
  brow_AddAnnot( nc_table_header, 8.9, 0.6, 1,
		flow_eDrawType_TextHelveticaBold, 2, flow_eAnnotType_OneLine, 
		0);
  brow_AddAnnot( nc_table_header, 12.9, 0.6, 2,
		flow_eDrawType_TextHelveticaBold, 2, flow_eAnnotType_OneLine, 
		0);
  brow_AddAnnot( nc_table_header, 16.9, 0.6, 3,
		flow_eDrawType_TextHelveticaBold, 2, flow_eAnnotType_OneLine, 
		0);
  brow_AddAnnot( nc_table_header, 20.9, 0.6, 4,
		flow_eDrawType_TextHelveticaBold, 2, flow_eAnnotType_OneLine, 
		0);
  brow_AddAnnot( nc_table_header, 24.9, 0.6, 5,
		flow_eDrawType_TextHelveticaBold, 2, flow_eAnnotType_OneLine, 
		0);
  brow_AddAnnot( nc_table_header, 28.9, 0.6, 6,
		flow_eDrawType_TextHelveticaBold, 2, flow_eAnnotType_OneLine, 
		0);
  brow_AddAnnot( nc_table_header, 32.9, 0.6, 7,
		flow_eDrawType_TextHelveticaBold, 2, flow_eAnnotType_OneLine, 
		0);
  brow_AddAnnot( nc_table_header, 35.9, 0.6, 8,
		flow_eDrawType_TextHelveticaBold, 2, flow_eAnnotType_OneLine, 
		0);
  brow_AddAnnot( nc_table_header, 38.9, 0.6, 9,
		flow_eDrawType_TextHelveticaBold, 2, flow_eAnnotType_OneLine, 
		0);

  brow_CreateNodeClass( ctx, "NavigatorEnum", 
		flow_eNodeGroup_Common, &nc_enum);
  brow_AddRadiobutton( nc_enum, 12, 0.1, 0.7, 0.7, 0, flow_eDrawType_Line, 1);
  brow_AddFrame( nc_enum, 0, 0, 20, 0.8, flow_eDrawType_Line, -1, 1);
  brow_AddAnnotPixmap( nc_enum, 0, 0.2, 0.1, flow_eDrawType_Line, 2, 0);
  brow_AddAnnotPixmap( nc_enum, 1, 1.1, 0.1, flow_eDrawType_Line, 2, 0);
  brow_AddAnnot( nc_enum, 2.9, 0.6, 0,
		flow_eDrawType_TextHelveticaBold, 2, flow_eAnnotType_OneLine, 
		0);
 
}

void XNavBrow::brow_setup()
{
  brow_sAttributes brow_attr;
  unsigned long mask;

  mask = 0;
  mask |= brow_eAttr_indentation;
  brow_attr.indentation = 0.5;
  mask |= brow_eAttr_annotation_space;
  brow_attr.annotation_space = 0.5;
  brow_SetAttributes( ctx, &brow_attr, mask); 
  brow_SetCtxUserData( ctx, userdata);
}


