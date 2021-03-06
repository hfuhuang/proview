/* 
 * Proview   $Id: co_statusmon_nodelist.h,v 1.7 2008-10-31 12:51:30 claes Exp $
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

#ifndef cow_statusmon_nodelist_h
#define cow_statusmon_nodelist_h

/* cow_statusmon_nodelist.h -- Status Monitor node list */

#ifndef pwr_h
# include "pwr.h"
#endif

#ifndef cow_statusmon_nodelistnav
# include "cow_statusmon_nodelistnav.h"
#endif

class CoWow;
class MsgWindow;

class Nodelist {
  public:
    Nodelist( void *nodelist_parent_ctx,
	      const char *nodelist_name,
	      int nodelist_mode,
	      int nodelist_view_node_descr,
	      pwr_tStatus *status);
    virtual ~Nodelist();

    void 		*parent_ctx;
    char 		name[80];
    NodelistNav		*nodelistnav;
    int			nodelist_displayed;
    void 		(*help_cb)( void *, const char *);
    void 		(*close_cb)( void *);
    void 		(*add_india_ok_cb)( Nodelist *, char *, char *, char *);
    void 		(*mod_india_ok_cb)( Nodelist *, char *, char *, char *);
    CoWow		*wow;
    MsgWindow		*msg_window;
    char		remote_gui[20];
    int			mode;
    int			view_node_descr;

    virtual void pop() {}
    virtual void set_clock_cursor() {}
    virtual void reset_cursor() {}
    virtual void free_cursor() {}
    virtual void open_add_input_dialog( const char *text, const char *text2, const char *text3, 
					const char *title, const char *init_text,
					void (*ok_cb)( Nodelist *, char *, char *, char *)) {}
    virtual void open_mod_input_dialog( const char *text, const char *text2,  const char *text3,
					const char *title, const char *init_text, 
					const char *init_text2, const char *init_text3,
					void (*ok_cb)( Nodelist *, char *, char *, char *)) {}
    
    void set_scantime( float scantime) { nodelistnav->scantime = int(scantime * 1000);}
    void activate_help();
    void activate_add_node();
    void activate_modify_node();
    void activate_remove_node();
    void activate_open_xtt();
    void activate_open_opplace();
    void activate_open_rtmon();
    void activate_save();
    static void find_node_cb( void *ctx, pwr_tOid oid);
    static void add_node_ok( Nodelist *nodelist, char *node_name, char *description,
			     char *opplace);
    static void mod_node_ok( Nodelist *nodelist, char *node_name, char *description,
			     char *opplace);
};

#endif




