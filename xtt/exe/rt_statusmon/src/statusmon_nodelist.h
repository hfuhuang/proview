/* 
 * Proview   $Id: statusmon_nodelist.h,v 1.1 2007-05-11 15:04:14 claes Exp $
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

#ifndef statusmon_nodelist_h
#define statusmon_nodelist_h

/* statusmon_nodelist.h -- Status Monitor node list */

#ifndef pwr_h
# include "pwr.h"
#endif

#ifndef statusmon_nodelistnav
# include "statusmon_nodelistnav.h"
#endif

class CoWow;

class Nodelist {
  public:
    Nodelist( void *nodelist_parent_ctx,
	  char *nodelist_name,
	  pwr_tStatus *status);
    virtual ~Nodelist();

    void 		*parent_ctx;
    char 		name[80];
    NodelistNav		*nodelistnav;
    int			nodelist_displayed;
    void 		(*help_cb)( void *, char *);
    void 		(*close_cb)( void *);
    void 		(*india_ok_cb)( Nodelist *, char *);
    CoWow		*wow;

    virtual void pop() {}
    virtual void set_clock_cursor() {}
    virtual void reset_cursor() {}
    virtual void free_cursor() {}
    virtual void open_input_dialog( char *text, char *title,
				    char *init_text,
				    void (*ok_cb)( Nodelist *, char *)) {}

    void set_scantime( float scantime) { nodelistnav->scantime = int(scantime * 1000);}
    void activate_help();
    void activate_add_node();
    void activate_remove_node();
    void activate_save();
    static void find_node_cb( void *ctx, pwr_tOid oid);
    static void add_node_ok( Nodelist *nodelist, char *node_name);
};

#endif




