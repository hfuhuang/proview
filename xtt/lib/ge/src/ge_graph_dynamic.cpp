/* ge_graph_dynamic.cpp -- graph dynamics

   PROVIEW/R  
   Copyright (C) 1996 by Comator Process AB.

   <Description>.  */

#include "glow_std.h"

#include <stdio.h>
#include <stdlib.h>

extern "C" {
#include "co_cdh.h"
#include "co_time.h"
#include "co_ccm_msg.h"
#include "rt_gdh.h"
}

#include <Xm/Xm.h>
#include <Xm/XmP.h>
#include <Xm/Text.h>
#include <Mrm/MrmPublic.h>
#include <X11/Intrinsic.h>
#include <X11/Xlib.h>
#include <X11/Xutil.h>

#include "glow.h"
#include "glow_growctx.h"
#include "glow_growapi.h"
#include "glow_growwidget.h"

#include "ge_graph.h"
extern "C" {
#include "ge_graph_ccm.h"
#include "co_dcli.h"
}

static int			graph_gccm_func_registred = 0;
static int			graph_verify = 0;
static grow_tObject		graph_current_object;

extern "C" int graph_posit_func( 
  void *filectx,
  gccm_s_arg *arg_list, 
  int arg_count,
  int *return_decl, 
  float *return_float, 
  int *return_int, 
  char *return_string)
{
  gccm_s_arg 	*arg_p2; 

  if ( arg_count != 2)
    return CCM__ARGMISM;
  arg_p2 = arg_list->next;

  if ( arg_list->value_decl != CCM_DECL_FLOAT)
    return CCM__ARGMISM;
  if ( arg_p2->value_decl != CCM_DECL_FLOAT)
    return CCM__ARGMISM;

  grow_SetObjectPosition( graph_current_object, arg_list->value_float, 
	arg_p2->value_float);

  *return_int = 1;
  *return_decl = CCM_DECL_INT;
  
  return 1;
}

extern "C" int graph_scale_func( 
  void *filectx,
  gccm_s_arg *arg_list, 
  int arg_count,
  int *return_decl, 
  float *return_float, 
  int *return_int, 
  char *return_string)
{
  gccm_s_arg 	*arg_p2; 

  if ( arg_count != 2)
    return CCM__ARGMISM;
  arg_p2 = arg_list->next;

  if ( arg_list->value_decl != CCM_DECL_FLOAT)
    return CCM__ARGMISM;
  if ( arg_p2->value_decl != CCM_DECL_FLOAT)
    return CCM__ARGMISM;

  grow_SetObjectScale( graph_current_object, arg_list->value_float,
	arg_p2->value_float, 0, 0, glow_eScaleType_UpperLeft);

  *return_int = 1;
  *return_decl = CCM_DECL_INT;
  
  return 1;
}

extern "C" int graph_rotatecenter_func( 
  void *filectx,
  gccm_s_arg *arg_list, 
  int arg_count,
  int *return_decl, 
  float *return_float, 
  int *return_int, 
  char *return_string)
{

  if ( arg_count != 1)
    return CCM__ARGMISM;

  if ( arg_list->value_decl != CCM_DECL_FLOAT)
    return CCM__ARGMISM;

  grow_SetObjectRotation( graph_current_object, arg_list->value_float,
	0, 0, glow_eRotationPoint_Center);

  *return_int = 1;
  *return_decl = CCM_DECL_INT;
  
  return 1;
}

extern "C" int graph_rotate_func( 
  void *filectx,
  gccm_s_arg *arg_list, 
  int arg_count,
  int *return_decl, 
  float *return_float, 
  int *return_int, 
  char *return_string)
{
  gccm_s_arg 	*arg_p2, *arg_p3; 

  if ( arg_count != 3)
    return CCM__ARGMISM;
  arg_p2 = arg_list->next;
  arg_p3 = arg_p2->next;

  if ( arg_list->value_decl != CCM_DECL_FLOAT)
    return CCM__ARGMISM;

  grow_SetObjectRotation( graph_current_object, arg_list->value_float,
	arg_p2->value_float, arg_p3->value_float, glow_eRotationPoint_FixPoint);

  *return_int = 1;
  *return_decl = CCM_DECL_INT;
  
  return 1;
}

extern "C" int graph_setfillcolor_func( 
  void *filectx,
  gccm_s_arg *arg_list, 
  int arg_count,
  int *return_decl, 
  float *return_float, 
  int *return_int, 
  char *return_string)
{

  if ( arg_count != 1)
    return CCM__ARGMISM;

  if ( arg_list->value_decl != CCM_DECL_INT)
    return CCM__ARGMISM;

  grow_SetObjectFillColor( graph_current_object, 
		(glow_eDrawType) arg_list->value_int);

  *return_int = 1;
  *return_decl = CCM_DECL_INT;
  
  return 1;
}

extern "C" int graph_resetfillcolor_func( 
  void *filectx,
  gccm_s_arg *arg_list, 
  int arg_count,
  int *return_decl, 
  float *return_float, 
  int *return_int, 
  char *return_string)
{

  if ( arg_count != 0)
    return CCM__ARGMISM;

  grow_ResetObjectFillColor( graph_current_object);

  *return_int = 1;
  *return_decl = CCM_DECL_INT;
  
  return 1;
}

extern "C" int graph_setbordercolor_func( 
  void *filectx,
  gccm_s_arg *arg_list, 
  int arg_count,
  int *return_decl, 
  float *return_float, 
  int *return_int, 
  char *return_string)
{

  if ( arg_count != 1)
    return CCM__ARGMISM;

  if ( arg_list->value_decl != CCM_DECL_INT)
    return CCM__ARGMISM;

  grow_SetObjectBorderColor( graph_current_object, 
		(glow_eDrawType) arg_list->value_int);

  *return_int = 1;
  *return_decl = CCM_DECL_INT;
  
  return 1;
}

extern "C" int graph_resetbordercolor_func( 
  void *filectx,
  gccm_s_arg *arg_list, 
  int arg_count,
  int *return_decl, 
  float *return_float, 
  int *return_int, 
  char *return_string)
{

  if ( arg_count != 0)
    return CCM__ARGMISM;

  grow_ResetObjectFillColor( graph_current_object);

  *return_int = 1;
  *return_decl = CCM_DECL_INT;
  
  return 1;
}

extern "C" int graph_setcolortone_func( 
  void *filectx,
  gccm_s_arg *arg_list, 
  int arg_count,
  int *return_decl, 
  float *return_float, 
  int *return_int, 
  char *return_string)
{

  if ( arg_count != 1)
    return CCM__ARGMISM;

  if ( arg_list->value_decl != CCM_DECL_INT)
    return CCM__ARGMISM;

  grow_SetObjectColorTone( graph_current_object, 
		(glow_eDrawTone) arg_list->value_int);

  *return_int = 1;
  *return_decl = CCM_DECL_INT;
  
  return 1;
}

extern "C" int graph_resetcolortone_func( 
  void *filectx,
  gccm_s_arg *arg_list, 
  int arg_count,
  int *return_decl, 
  float *return_float, 
  int *return_int, 
  char *return_string)
{

  if ( arg_count != 0)
    return CCM__ARGMISM;

  grow_ResetObjectColorTone( graph_current_object);

  *return_int = 1;
  *return_decl = CCM_DECL_INT;
  
  return 1;
}

extern "C" int graph_setcolorshift_func( 
  void *filectx,
  gccm_s_arg *arg_list, 
  int arg_count,
  int *return_decl, 
  float *return_float, 
  int *return_int, 
  char *return_string)
{

  if ( arg_count != 1)
    return CCM__ARGMISM;

  if ( arg_list->value_decl != CCM_DECL_INT)
    return CCM__ARGMISM;

  grow_SetObjectColorShift( graph_current_object, arg_list->value_int);

  *return_int = 1;
  *return_decl = CCM_DECL_INT;
  
  return 1;
}

extern "C" int graph_resetcolorshift_func( 
  void *filectx,
  gccm_s_arg *arg_list, 
  int arg_count,
  int *return_decl, 
  float *return_float, 
  int *return_int, 
  char *return_string)
{

  if ( arg_count != 0)
    return CCM__ARGMISM;

  grow_ResetObjectColorShift( graph_current_object);

  *return_int = 1;
  *return_decl = CCM_DECL_INT;
  
  return 1;
}

extern "C" int graph_incrcolorshift_func( 
  void *filectx,
  gccm_s_arg *arg_list, 
  int arg_count,
  int *return_decl, 
  float *return_float, 
  int *return_int, 
  char *return_string)
{

  if ( arg_count != 1)
    return CCM__ARGMISM;

  if ( arg_list->value_decl != CCM_DECL_INT)
    return CCM__ARGMISM;

  grow_IncrObjectColorShift( graph_current_object, arg_list->value_int);

  *return_int = 1;
  *return_decl = CCM_DECL_INT;
  
  return 1;
}

extern "C" int graph_setcolorlight_func( 
  void *filectx,
  gccm_s_arg *arg_list, 
  int arg_count,
  int *return_decl, 
  float *return_float, 
  int *return_int, 
  char *return_string)
{

  if ( arg_count != 1)
    return CCM__ARGMISM;

  if ( arg_list->value_decl != CCM_DECL_INT)
    return CCM__ARGMISM;

  grow_SetObjectColorLightness( graph_current_object, arg_list->value_int);

  *return_int = 1;
  *return_decl = CCM_DECL_INT;
  
  return 1;
}

extern "C" int graph_resetcolorlight_func( 
  void *filectx,
  gccm_s_arg *arg_list, 
  int arg_count,
  int *return_decl, 
  float *return_float, 
  int *return_int, 
  char *return_string)
{

  if ( arg_count != 0)
    return CCM__ARGMISM;

  grow_ResetObjectColorLightness( graph_current_object);

  *return_int = 1;
  *return_decl = CCM_DECL_INT;
  
  return 1;
}

extern "C" int graph_setcolorintensity_func( 
  void *filectx,
  gccm_s_arg *arg_list, 
  int arg_count,
  int *return_decl, 
  float *return_float, 
  int *return_int, 
  char *return_string)
{

  if ( arg_count != 1)
    return CCM__ARGMISM;

  if ( arg_list->value_decl != CCM_DECL_INT)
    return CCM__ARGMISM;

  grow_SetObjectColorIntensity( graph_current_object, arg_list->value_int);

  *return_int = 1;
  *return_decl = CCM_DECL_INT;
  
  return 1;
}

extern "C" int graph_resetcolorintensity_func( 
  void *filectx,
  gccm_s_arg *arg_list, 
  int arg_count,
  int *return_decl, 
  float *return_float, 
  int *return_int, 
  char *return_string)
{

  if ( arg_count != 0)
    return CCM__ARGMISM;

  grow_ResetObjectColorIntensity( graph_current_object);

  *return_int = 1;
  *return_decl = CCM_DECL_INT;
  
  return 1;
}

extern "C" int	graph_deffilename_func( char *outfile, char *infile, void *client_data)
{
	dcli_get_defaultfilename( infile, outfile, ".pwg_com");
        return 1;
}

extern "C" int	graph_externcmd_func( char *cmd, void *client_data)
{
  printf( "%%GCCM-E-MSG, No extern functions defined\n");
  return 1;
}

extern "C" int	graph_errormessage_func( char *msg, int severity, void *client_data)
{
	if ( EVEN(severity))
	  printf( "%%GCCM-E-MSG, %s\n", msg);
	else
	  printf( "%%GCCM-I-MSG, %s\n", msg);
        return 1;
}

int Graph::get_argnames( char *code, char *argnames, int *argtypes, int *argcnt)
{
  return gccm_get_pwr_argnames( code,
		graph_errormessage_func,
		argnames, argtypes, argcnt);
}

int Graph::exec_dynamic( grow_tObject object, char *code, 
		glow_eDynamicType type)
{
  int		sts;
  int		appl_sts;

  if ( !graph_gccm_func_registred)
  {
    sts = gccm_register_function( "Posit", graph_posit_func);
    if ( EVEN(sts)) return sts;
    sts = gccm_register_function( "Scale", graph_scale_func);
    if ( EVEN(sts)) return sts;
    sts = gccm_register_function( "RotateCenter", graph_rotatecenter_func);
    if ( EVEN(sts)) return sts;
    sts = gccm_register_function( "Rotate", graph_rotate_func);
    if ( EVEN(sts)) return sts;
    sts = gccm_register_function( "SetFillColor", graph_setfillcolor_func);
    if ( EVEN(sts)) return sts;
    sts = gccm_register_function( "ResetFillColor", graph_resetfillcolor_func);
    if ( EVEN(sts)) return sts;
    sts = gccm_register_function( "SetBorderColor", graph_setbordercolor_func);
    if ( EVEN(sts)) return sts;
    sts = gccm_register_function( "ResetBorderColor", graph_resetbordercolor_func);
    if ( EVEN(sts)) return sts;
    sts = gccm_register_function( "SetColorTone", graph_setcolortone_func);
    if ( EVEN(sts)) return sts;
    sts = gccm_register_function( "ResetColorTone", graph_resetcolortone_func);
    if ( EVEN(sts)) return sts;
    sts = gccm_register_function( "SetColorShift", graph_setcolorshift_func);
    if ( EVEN(sts)) return sts;
    sts = gccm_register_function( "ResetColorShift", graph_resetcolorshift_func);
    if ( EVEN(sts)) return sts;
    sts = gccm_register_function( "IncrColorShift", graph_incrcolorshift_func);
    if ( EVEN(sts)) return sts;
    sts = gccm_register_function( "SetColorLightness", graph_setcolorlight_func);
    if ( EVEN(sts)) return sts;
    sts = gccm_register_function( "ResetColorLightness", graph_resetcolorlight_func);
    if ( EVEN(sts)) return sts;
    sts = gccm_register_function( "SetColorIntensity", graph_setcolorintensity_func);
    if ( EVEN(sts)) return sts;
    sts = gccm_register_function( "ResetColorIntensity", graph_resetcolorintensity_func);
    if ( EVEN(sts)) return sts;

    graph_gccm_func_registred = 1;
  }


  switch( type)
  {
    case glow_eDynamicType_SubGraphEnd:
      // Remove the arguments from the argument stack
      memset( &arglist_stack[arglist_cnt-1], 0, sizeof(arglist_stack[0]));
      arglist_cnt--;
      break;
    case glow_eDynamicType_SubGraph:
    {
      char 	*argnames;
      int	*argtypes;
      char 	**argvalues;
      int	arg_cnt;

      graph_current_object = object;

      // Evaluate the arguments and push them on the argument stack
      grow_GetObjectArgs( object, &argnames, &argtypes, &argvalues,
		&arg_cnt);

      if ( arglist_cnt == 0)
        sts = gccm_create_pwr_args( argnames, argtypes, argvalues,
		arg_cnt, graph_errormessage_func, &arglist_stack[arglist_cnt],
		NULL);
      else
        sts = gccm_create_pwr_args( argnames, argtypes, argvalues,
		arg_cnt, graph_errormessage_func, &arglist_stack[arglist_cnt],
		&arglist_stack[arglist_cnt - 1]);
      arglist_cnt++;

      if ( code)
      {
        // Execute the code
        if ( arglist_cnt == 0)
          sts = gccm_file_exec( code, graph_externcmd_func,
		graph_deffilename_func, graph_errormessage_func, &appl_sts,
		graph_verify, 0, NULL, 0, 0, NULL, NULL, NULL);
        else
          sts = gccm_file_exec( code, graph_externcmd_func,
		graph_deffilename_func, graph_errormessage_func, &appl_sts,
		graph_verify, 0, NULL, 0, 0, NULL, NULL, 
		&arglist_stack[arglist_cnt - 1]);
        if ( EVEN(sts)) return sts;
      }
      break;
    }
    case glow_eDynamicType_Object:
      graph_current_object = object;

      // Execute the code
      if ( arglist_cnt == 0)
        sts = gccm_file_exec( code, graph_externcmd_func,
		graph_deffilename_func, graph_errormessage_func, &appl_sts,
		graph_verify, 0, NULL, 0, 0, NULL, NULL, NULL);
      else
        sts = gccm_file_exec( code, graph_externcmd_func,
		graph_deffilename_func, graph_errormessage_func, &appl_sts,
		graph_verify, 0, NULL, 0, 0, NULL, NULL,
		&arglist_stack[arglist_cnt - 1]);
      if ( EVEN(sts)) return sts;
      break;
  }
  return 1;
}


