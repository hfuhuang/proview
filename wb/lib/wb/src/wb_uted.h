/* 
 * Proview   $Id: wb_uted.h,v 1.5 2007-01-04 07:29:04 claes Exp $
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

#ifndef wb_uted_h
#define wb_uted_h

#ifndef wb_ldh_h
#include "wb_ldh.h"
#endif

#define UTED_MODE_EDIT 	0
#define UTED_MODE_VIEW 	1

#define	UTED_QUALS	10

#define UTED_PROC_PWRPLC	0
#define	UTED_PROC_DCL	1

#define UTED_QUAL_QUAL	0
#define UTED_QUAL_PAR	1
#define UTED_QUAL_DEFQUAL 2

#define UTED_INS_PLANT	0
#define UTED_INS_NODE		1
#define UTED_INS_PLNO		2
#define UTED_INS_NOPL		3

#define	UTED_BATCH_BATCH		0
#define UTED_BATCH_CURRSESS	1

#define UTED_MAX_COMMANDS	40

typedef struct {
  char	qual[30];
  int	insert;
  int	insert_hier;
  int	value;
  int	present;
  int	type;
} uted_sQual;

typedef struct {
  char		command[80];
  int		process;
  int		view_sensitivity;
  int		batch_sensitivity;
  uted_sQual	qualifier[10];
} uted_sCommand;

class WUted {
 public:
  void			*parent_ctx;
  char			name[80];
  int			current_index;
  int			present_sts[UTED_QUALS];
  int			batch_sts;
  ldh_tWBContext	ldhwb;
  ldh_tSesContext	ldhses;
  int			mode;
  void			(* questionbox_yes) ( WUted *);
  void			(* questionbox_no) ( WUted *);
  void			(* questionbox_cancel) ( WUted *);
  void 			(*quit_cb)(void *parent_ctx);
  int			dummy[20];
  static uted_sCommand	commands[UTED_MAX_COMMANDS];

  WUted( void	       	*wu_parent_ctx,
	 char	       	*wu_name,
	 char	       	*wu_iconname,
	 ldh_tWBContext	wu_ldhwb,
	 ldh_tSesContext wu_ldhses,
	 int	       	wu_editmode,
	 void 	       	(*wu_quit_cb)(void *),
	 pwr_tStatus  	*status);
  virtual ~WUted();

  int execute( int show);
  void message_error( pwr_tStatus sts);
  void set_editmode( int edit, ldh_tSesContext ldhses);

  virtual void remove_command_window() {}
  virtual void reset_qual() {}
  virtual void message( char *new_label) {}
  virtual void set_command_window( char *cmd) {}
  virtual void raise_window() {}
  virtual void clock_cursor() {}
  virtual void reset_cursor() {}
  virtual void configure_quals( char *label) {}
  virtual void enable_entries( int enable) {}
  virtual void get_value( int idx, char *str, int len) {}
  virtual void questionbox( char 	*question_title,
			    char	*question_text,
			    void	(* yes_procedure) (WUted *),
			    void	(* no_procedure) (WUted *),
			    void	(* cancel_procedure) (WUted *), 
			    pwr_tBoolean cancel) {}

  static void get_message_error( pwr_tStatus sts, char *str);
  static pwr_tStatus get_command_index( char *label, int *index);
  static void get_filename( char *filename);
};

#endif
