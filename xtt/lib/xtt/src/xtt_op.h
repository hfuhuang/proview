/* 
 * Proview   $Id: xtt_op.h,v 1.5 2008-10-31 12:51:36 claes Exp $
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

#ifndef xtt_op_h
#define xtt_op_h

/* xtt_op.h -- Operator window in xtt */

#include <vector>

#ifndef pwr_h
# include "pwr.h"
#endif

#ifndef rt_qcom_h
# include "rt_qcom.h"
#endif

#ifndef xtt_jop_h
# include "xtt_jop.h"
#endif

#ifndef xtt_evlist_h
# include "xtt_evlist.h"
#endif

using namespace std;

class CoWow;
class CoWowTimer;
class Op;

typedef enum {
  op_eSupColor_Gray,
  op_eSupColor_Green,
  op_eSupColor_Yellow,
  op_eSupColor_Red,
  op_eSupColor_Black
} op_eSupColor;

class OpSup {
 public:
  OpSup() : buttonw(0), imagew(0), p(0), old_color(op_eSupColor_Gray), flash(0)
    { strcpy( node_name, ""); strcpy( object_name, "");}
  
  pwr_tOid   	node_oid;
  pwr_tOName 	object_name;
  pwr_tObjName 	node_name;
  void 		*buttonw;
  void		*imagew;
  pwr_tStatus	*p;
  pwr_tRefId	refid;
  op_eSupColor	old_color;
  int		flash;
};

class Op {
 public:
  void 		*parent_ctx;
  unsigned long	balarm_prio;
  unsigned long	balarm_type;
  char		button_title[25][80];
  pwr_tAttrRef  button_aref[25];
  int	       	button_cnt;
  int	       	start_jop;
  Jop	       	*jop;
  void 		(*command_cb)( void *, char *);
  void 		(*map_cb)( void *);
  void 		(*help_cb)( void *, const char *);
  void 		(*close_cb)( void *);
  int 		(*get_alarm_info_cb)( void *, evlist_sAlarmInfo *);
  void 		(*ack_last_cb)( void *, unsigned long, unsigned long);
  CoWow		*wow;
  pwr_tMask	layout_mask;
  vector<OpSup> sup_vect;
  CoWowTimer	*sup_timerid;

  Op( void *op_parent_ctx,
      char *opplace,
      pwr_tStatus *status);
  virtual ~Op();

  virtual void	map() {}
  virtual int 	configure( char *opplace_str) {return 0;}
  virtual void 	update_alarm_info() {}
  virtual void  add_close_button() {}
  virtual int   create_menu_item( const char *name, int pixmap, int append, const char *cmd) { return 0;}
  virtual int   delete_menu_item( const char *name) { return 0;}
  virtual void  change_sup_color( void *imagew, op_eSupColor color) {}
  virtual void  set_title( char *user) {}

  void	set_jop_qid( int qix) { if ( jop) jop->set_jop_qid( qix);};
  void	scan();
  int 	appl_action( int idx);
  int 	jop_command( char *command);
  int   sup_init();
  void activate_exit();
  void activate_aalarm_ack();
  void activate_balarm_ack();
  void activate_alarmlist();
  void activate_eventlist();
  void activate_eventlog();
  void activate_blocklist();
  void activate_navigator();
  void activate_trend();
  void activate_fast();
  void activate_history();
  void activate_graph();
  void activate_help();
  void activate_help_overview();
  void activate_help_opwin();
  void activate_help_proview();
  void activate_switch_user();
  void activate_show_user();
  void activate_logout();
  void activate_cmd_menu_item( char *cmd);
  void activate_sup_node( void *id);

  static void jop_command_cb( void *op, char *command);
  static void sup_scan( void *data);
};

#endif
