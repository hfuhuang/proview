/* ge_dyn.cpp -- Display plant and node hiererachy

   PROVIEW/R  
   Copyright (C) 1996 by Comator Process AB.

   <Description>.  */

#include "glow_std.h"

#include <stdio.h>
#include <stdlib.h>
#include <float.h>
#include <math.h>

extern "C" {
#include "co_cdh.h"
#include "co_time.h"
#include "co_msg.h"
#include "rt_gdh.h"
#include "co_dcli.h"
#include "ge_msg.h"
#include "rt_pwr_msg.h"
#include "pwr_baseclasses.h"
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
#include "ge_attr.h"
#include "ge_dyn.h"
#include "ge_msg.h"

extern "C" {
#include "co_mrm_util.h"
}

#include "co_lng.h"

// Until xtt_menu.h i unavailable...
#define xmenu_mUtility_Ge        (1 << 5)
#define xmenu_eItemType_Object   1

static int dyn_get_typeid( char *format)
{
  // Guess the type of the annot variable...
  if ( strchr( format, 'f'))
    return pwr_eType_Float32;
  if ( strchr( format, 'd'))
    return pwr_eType_Int32;
  if ( strchr( format, 's'))
    return pwr_eType_String;
  if ( strchr( format, 'o'))
    return pwr_eType_Objid;
  if ( strchr( format, 'm'))
    return pwr_eType_Status;
  return pwr_eType_Int32;
}

// Replace " to \"
char *GeDyn::cmd_cnv( char *instr)
{
  static char outstr[200];
  char *in;
  char *out = outstr;

  for ( in = instr; *in != 0; in++)
  {
    if ( *in == '"')
      *out++ = '\\';
    *out++ = *in;
  }
  *out = *in;
  return outstr;
}

int GeDyn::instance_to_number( int instance)
{
  int inst = 1;
  int m = instance;
  while( m > 1) {
    m = m >> 1;
    inst++;
  }
  return inst;
}

void GeDyn::replace_attribute( char *attribute, int attr_size, char *from, char *to, int *cnt, int strict)
{
  char str[200];
  char tmp[200];
  char *s;
  int offs;

  strncpy( str, attribute, sizeof(str));
  if ( !strict)
    cdh_ToLower( str, str);
  s = strstr( str, from);
  if ( s) {
    offs = (int)( s - str);
    strcpy( tmp, s + strlen(from));
    strncpy( &attribute[offs], to, attr_size - offs);
    attribute[attr_size-1] = 0;
    strncat( attribute, tmp, attr_size-strlen(attribute));

    (*cnt)++;
  }
}

GeDyn::GeDyn( const GeDyn& x) : 
  elements(0), graph(x.graph), dyn_type(x.dyn_type), total_dyn_type(x.total_dyn_type),
  action_type(x.action_type), total_action_type(x.total_action_type), access(x.access),
  cycle(x.cycle), attr_editor(x.attr_editor)
{
  GeDynElem *elem, *e;

  for ( elem = x.elements; elem; elem = elem->next) {
    e = 0;
    switch( elem->dyn_type) {
    case ge_mDynType_DigLowColor:
      e = new GeDigLowColor((const GeDigLowColor&) *elem); break;
    case ge_mDynType_DigColor:
      e = new GeDigColor((const GeDigColor&) *elem); break;
    case ge_mDynType_DigError:
      e = new GeDigError((const GeDigError&) *elem); break;
    case ge_mDynType_DigWarning:
      e = new GeDigWarning((const GeDigWarning&) *elem); break;
    case ge_mDynType_DigFlash:
      e = new GeDigFlash((const GeDigFlash&) *elem); break;
    case ge_mDynType_Invisible:
      e = new GeInvisible((const GeInvisible&) *elem); break;
    case ge_mDynType_DigBorder:
      e = new GeDigBorder((const GeDigBorder&) *elem); break;
    case ge_mDynType_DigText:
      e = new GeDigText((const GeDigText&) *elem); break;
    case ge_mDynType_Value:
      e = new GeValue((const GeValue&) *elem); break;
    case ge_mDynType_AnalogColor:
      e = new GeAnalogColor((const GeAnalogColor&) *elem); break;
    case ge_mDynType_Rotate:
      e = new GeRotate((const GeRotate&) *elem); break;
    case ge_mDynType_Move:
      e = new GeMove((const GeMove&) *elem); break;
    case ge_mDynType_DigShift:
      e = new GeDigShift((const GeDigShift&) *elem); break;
    case ge_mDynType_AnalogShift:
      e = new GeAnalogShift((const GeAnalogShift&) *elem); break;
    case ge_mDynType_Video:
      e = new GeVideo((const GeVideo&) *elem); break;
    case ge_mDynType_Animation:
      e = new GeAnimation((const GeAnimation&) *elem); break;
    case ge_mDynType_Bar:
      e = new GeBar((const GeBar&) *elem); break;
    case ge_mDynType_Trend:
      e = new GeTrend((const GeTrend&) *elem); break;
    case ge_mDynType_FillLevel:
      e = new GeFillLevel((const GeFillLevel&) *elem); break;
    case ge_mDynType_FastCurve:
      e = new GeFastCurve((const GeFastCurve&) *elem); break;
    case ge_mDynType_AnalogText:
      e = new GeAnalogText((const GeAnalogText&) *elem); break;
    case ge_mDynType_Table:
      e = new GeTable((const GeTable&) *elem); break;
    case ge_mDynType_StatusColor:
      e = new GeStatusColor((const GeStatusColor&) *elem); break;
    default: ;
    }
    switch( elem->action_type) {
    case ge_mActionType_PopupMenu:
      e = new GePopupMenu((const GePopupMenu&) *elem); break;
    case ge_mActionType_SetDig:
      e = new GeSetDig((const GeSetDig&) *elem); break;
    case ge_mActionType_ResetDig:
      e = new GeResetDig((const GeResetDig&) *elem); break;
    case ge_mActionType_ToggleDig:
      e = new GeToggleDig((const GeToggleDig&) *elem); break;
    case ge_mActionType_StoDig:
      e = new GeStoDig((const GeStoDig&) *elem); break;
    case ge_mActionType_Command:
      e = new GeCommand((const GeCommand&) *elem); break;
    case ge_mActionType_CommandDoubleClick: 
      e = new GeCommandDoubleClick((const GeCommandDoubleClick&) *elem); break;
    case ge_mActionType_Confirm:
      e = new GeConfirm((const GeConfirm&) *elem); break;
    case ge_mActionType_IncrAnalog:
      e = new GeIncrAnalog((const GeIncrAnalog&) *elem); break;
    case ge_mActionType_RadioButton:
      e = new GeRadioButton((const GeRadioButton&) *elem); break;
    case ge_mActionType_Slider:
      e = new GeSlider((const GeSlider&) *elem); break;
    case ge_mActionType_ValueInput:
      e = new GeValueInput((const GeValueInput&) *elem); break;
    case ge_mActionType_TipText:
      e = new GeTipText((const GeTipText&) *elem); break;
    case ge_mActionType_Help:
      e = new GeHelp((const GeHelp&) *elem); break;
    case ge_mActionType_OpenGraph:
      e = new GeOpenGraph((const GeOpenGraph&) *elem); break;
    case ge_mActionType_OpenURL:
      e = new GeOpenURL((const GeOpenURL&) *elem); break;
    case ge_mActionType_InputFocus:
      e = new GeInputFocus((const GeInputFocus&) *elem); break;
    case ge_mActionType_CloseGraph:
      e = new GeCloseGraph((const GeCloseGraph&) *elem); break;
    case ge_mActionType_PulldownMenu:
      e = new GePulldownMenu((const GePulldownMenu&) *elem); break;
    case ge_mActionType_OptionMenu:
      e = new GeOptionMenu((const GeOptionMenu&) *elem); break;
    default: ;
    }
    if ( e)
      insert_element( e);
  }

  // update_elements();
}

void GeDyn::save( ofstream& fp)
{
  fp << int(ge_eSave_Dyn) << endl;
  fp << int(ge_eSave_Dyn_dyn_type) << FSPACE << int(dyn_type) << endl;
  fp << int(ge_eSave_Dyn_action_type) << FSPACE << int(action_type) << endl;
  fp << int(ge_eSave_Dyn_access) << FSPACE << int(access) << endl;
  fp << int(ge_eSave_Dyn_cycle) << FSPACE << int(cycle) << endl;

  for ( GeDynElem *elem = elements; elem; elem = elem->next) {
    elem->save( fp);
  }
  fp << int(ge_eSave_End) << endl;
}

void GeDyn::open( ifstream& fp)
{
  int		type;
  int 		end_found = 0;
  int		tmp;
  char		dummy[40];
  GeDynElem    	*e;

  for (;;)
  {
    fp >> type;
    e = 0;
    switch( type) {
      case ge_eSave_Dyn: break;
      case ge_eSave_Dyn_dyn_type: fp >> tmp; dyn_type = (ge_mDynType)tmp; break;
      case ge_eSave_Dyn_action_type: fp >> tmp; action_type = (ge_mActionType)tmp; break;
      case ge_eSave_Dyn_access: fp >> tmp; access = (glow_mAccess)tmp; break;
      case ge_eSave_Dyn_cycle: fp >> tmp; cycle = (glow_eCycle)tmp; break;
      case ge_eSave_DigLowColor: e = (GeDynElem *) new GeDigLowColor(this); break;
      case ge_eSave_DigColor: e = (GeDynElem *) new GeDigColor(this); break;
      case ge_eSave_DigWarning: e = (GeDynElem *) new GeDigWarning(this); break;
      case ge_eSave_DigError: e = (GeDynElem *) new GeDigError(this); break;
      case ge_eSave_DigFlash: e = (GeDynElem *) new GeDigFlash(this); break;
      case ge_eSave_Invisible: e = (GeDynElem *) new GeInvisible(this); break;
      case ge_eSave_DigBorder: e = (GeDynElem *) new GeDigBorder(this); break;
      case ge_eSave_DigText: e = (GeDynElem *) new GeDigText(this); break;
      case ge_eSave_Value: e = (GeDynElem *) new GeValue(this); break;
      case ge_eSave_ValueInput: e = (GeDynElem *) new GeValueInput(this); break;
      case ge_eSave_AnalogColor: e = (GeDynElem *) new GeAnalogColor(this); break;
      case ge_eSave_Rotate: e = (GeDynElem *) new GeRotate(this); break;
      case ge_eSave_Move: e = (GeDynElem *) new GeMove(this); break;
      case ge_eSave_AnalogShift: e = (GeDynElem *) new GeAnalogShift(this); break;
      case ge_eSave_DigShift: e = (GeDynElem *) new GeDigShift(this); break;
      case ge_eSave_Animation: e = (GeDynElem *) new GeAnimation(this); break;
      case ge_eSave_Video: e = (GeDynElem *) new GeVideo(this); break;
      case ge_eSave_Bar: e = (GeDynElem *) new GeBar(this); break;
      case ge_eSave_Trend: e = (GeDynElem *) new GeTrend(this); break;
      case ge_eSave_FillLevel: e = (GeDynElem *) new GeFillLevel(this); break;
      case ge_eSave_FastCurve: e = (GeDynElem *) new GeFastCurve(this); break;
      case ge_eSave_AnalogText: e = (GeDynElem *) new GeAnalogText(this); break;
      case ge_eSave_Table: e = (GeDynElem *) new GeTable(this); break;
      case ge_eSave_StatusColor: e = (GeDynElem *) new GeStatusColor(this); break;
      case ge_eSave_PopupMenu: e = (GeDynElem *) new GePopupMenu(this); break;
      case ge_eSave_SetDig: e = (GeDynElem *) new GeSetDig(this); break;
      case ge_eSave_ResetDig: e = (GeDynElem *) new GeResetDig(this); break;
      case ge_eSave_ToggleDig: e = (GeDynElem *) new GeToggleDig(this); break;
      case ge_eSave_StoDig: e = (GeDynElem *) new GeStoDig(this); break;
      case ge_eSave_Command: e = (GeDynElem *) new GeCommand(this); break;
      case ge_eSave_CommandDC: e = (GeDynElem *) new GeCommandDoubleClick(this); break;
      case ge_eSave_Confirm: e = (GeDynElem *) new GeConfirm(this); break;
      case ge_eSave_IncrAnalog: e = (GeDynElem *) new GeIncrAnalog(this); break;
      case ge_eSave_RadioButton: e = (GeDynElem *) new GeRadioButton(this); break;
      case ge_eSave_Slider: e = (GeDynElem *) new GeSlider(this); break;
      case ge_eSave_TipText: e = (GeDynElem *) new GeTipText(this); break;
      case ge_eSave_Help: e = (GeDynElem *) new GeHelp(this); break;
      case ge_eSave_OpenGraph: e = (GeDynElem *) new GeOpenGraph(this); break;
      case ge_eSave_OpenURL: e = (GeDynElem *) new GeOpenURL(this); break;
      case ge_eSave_InputFocus: e = (GeDynElem *) new GeInputFocus(this); break;
      case ge_eSave_CloseGraph: e = (GeDynElem *) new GeCloseGraph(this); break;
      case ge_eSave_PulldownMenu: e = (GeDynElem *) new GePulldownMenu(this); break;
      case ge_eSave_OptionMenu: e = (GeDynElem *) new GeOptionMenu(this); break;
      case ge_eSave_End: end_found = 1; break;
      default:
        cout << "GeDyn:open syntax error" << endl;
        fp.getline( dummy, sizeof(dummy));
    }
    if ( e) {
      insert_element( e);
      e->open( fp);
    }
    if ( end_found)
      break;
  }  
}

ge_mDynType GeDyn::get_dyntype( grow_tObject object)
{
  if ( object && dyn_type & ge_mDynType_Inherit) {
    if ( grow_GetObjectType( object) == glow_eObjectType_GrowNode ||
	 grow_GetObjectType( object) == glow_eObjectType_GrowSlider ||
	 grow_GetObjectType( object) == glow_eObjectType_GrowGroup) {
      int inherit_dyn_type;
      int inherit_action_type;

      grow_GetObjectClassDynType( object, &inherit_dyn_type, &inherit_action_type);
      return (ge_mDynType) ((dyn_type & ~ge_mDynType_Inherit) | inherit_dyn_type);
    }
  }
  return dyn_type;
}

ge_mActionType GeDyn::get_actiontype( grow_tObject object)
{
  if ( object && action_type & ge_mDynType_Inherit) {
    if ( grow_GetObjectType( object) == glow_eObjectType_GrowNode ||
	 grow_GetObjectType( object) == glow_eObjectType_GrowSlider ||
	 grow_GetObjectType( object) == glow_eObjectType_GrowGroup) {
      int inherit_dyn_type;
      int inherit_action_type;

      grow_GetObjectClassDynType( object, &inherit_dyn_type, &inherit_action_type);
      return (ge_mActionType) ((action_type & ~ge_mActionType_Inherit) | inherit_action_type);
    }
  }
  return action_type;
}

void GeDyn::unset_inherit( grow_tObject object)
{
  dyn_type = get_dyntype( object);
  action_type = get_actiontype( object);
}

glow_eDrawType GeDyn::get_color1( grow_tObject object, glow_eDrawType color)
{
  if ( color == glow_eDrawType_Inherit) {
    glow_eDrawType color1;
    glow_eDrawType color2;
    
    grow_GetObjectClassTraceColor( object, &color1, &color2);
    return color1;
  }
  return color;
}

glow_eDrawType GeDyn::get_color2( grow_tObject object, glow_eDrawType color)
{
  if ( color == glow_eDrawType_Inherit) {
    glow_eDrawType color1;
    glow_eDrawType color2;
    
    grow_GetObjectClassTraceColor( object, &color1, &color2);
    return color2;
  }
  return color;
}

void GeDyn::get_attributes( grow_tObject object, attr_sItem *itemlist, int *item_count)
{
  attr_sItem *attrinfo;
  int i;

  total_dyn_type = get_dyntype( object);
  total_action_type = get_actiontype( object);
  display_access = false;

  update_elements();

  attrinfo = itemlist;

  i = *item_count;

  for ( GeDynElem *elem = elements; elem; elem = elem->next)
    elem->get_attributes( itemlist, &i);

  if ( display_access) {
    strcpy( attrinfo[i].name, "Access");
    attrinfo[i].value = &access;
    attrinfo[i].type = glow_eType_Access;
    attrinfo[i++].size = sizeof( access);
  }

  if ( attr_editor != ge_eDynAttr_Menu) {
    strcpy( attrinfo[i].name, "Cycle");
    attrinfo[i].value = &cycle;
    attrinfo[i].type = glow_eType_Cycle;
    attrinfo[i++].size = sizeof( cycle);
      
    strcpy( attrinfo[i].name, "DynType");
    attrinfo[i].value = &dyn_type;
    if ( total_dyn_type & ge_mDynType_Tone)
      attrinfo[i].type = ge_eAttrType_DynTypeTone;
    else
      attrinfo[i].type = ge_eAttrType_DynType;
    if ( total_dyn_type & ge_mDynType_Bar ||
	 total_dyn_type & ge_mDynType_Trend ||
	 total_dyn_type & ge_mDynType_Table ||
	 total_dyn_type & ge_mDynType_FastCurve)
      attrinfo[i].noedit = 1;
    attrinfo[i].mask = ~(ge_mDynType_Bar | ge_mDynType_Trend | ge_mDynType_Table | 
			 ge_mDynType_FastCurve | ge_mDynType_SliderBackground);
    attrinfo[i++].size = sizeof( dyn_type);
  }
  strcpy( attrinfo[i].name, "Action");
  attrinfo[i].value = &action_type;
  attrinfo[i].type = ge_eAttrType_ActionType;
  attrinfo[i].mask = ~ge_mActionType_Slider;
  attrinfo[i++].size = sizeof( action_type);

  *item_count = i;
}

void GeDyn::get_transtab( grow_tObject object, char **tt)
{
  int sts;
  static char transtab[][32] = {	"SubGraph",		"SubGraph",
					"Dynamic",		"",
					""};

  total_dyn_type = get_dyntype( object);
  total_action_type = get_actiontype( object);

  update_elements();
  *tt = 0;
  for ( GeDynElem *elem = elements; elem; elem = elem->next) {
    sts = elem->get_transtab( tt);
    if ( !sts)
      return;
  }
  if ( !*tt)
    *tt = (char *)transtab;
}

void GeDyn::set_attribute( grow_tObject object, char *attr_name, int second)
{
  int cnt = second + 1;

  total_dyn_type = get_dyntype( object);
  total_action_type = get_actiontype( object);

  update_elements();

  for ( GeDynElem *elem = elements; elem; elem = elem->next) {
    elem->set_attribute( object, attr_name, &cnt);
    if ( !cnt)
      break;
  }
  if ( cnt)
    graph->message( 'E', "Nothing to connect for this object");
}

void GeDyn::replace_attribute( char *from, char *to, int *cnt, int strict)
{
  for ( GeDynElem *elem = elements; elem; elem = elem->next)
    elem->replace_attribute( from, to, cnt, strict);
}

void GeDyn::set_color( grow_tObject object, glow_eDrawType color)
{
  int sts = 0;

  total_dyn_type = get_dyntype( object);
  total_action_type = get_actiontype( object);

  update_elements();

  for ( GeDynElem *elem = elements; elem; elem = elem->next) {
    sts = elem->set_color( object, color);
    if ( sts)
      break;
  }
  if ( !sts)
    graph->message( 'E', "No color to set for this object");
}

void GeDyn::set_access( glow_mAccess acc)
{
  access = acc;
}

void GeDyn::set_dyn( ge_mDynType type, ge_mActionType action)
{
  dyn_type = type;
  action_type = action;
  update_elements();
}

void GeDyn::set_command( char *cmd)
{
  for ( GeDynElem *elem = elements; elem; elem = elem->next) {
    if ( elem->action_type == ge_mActionType_Command) {
      strncpy( ((GeCommand *)elem)->command, cmd, sizeof(((GeCommand *)elem)->command));
      ((GeCommand *)elem)->command[sizeof(((GeCommand *)elem)->command)-1] = 0;
      break;
    }
  }
}

void GeDyn::set_value_input( char *format, double min_value, double max_value)
{
  for ( GeDynElem *elem = elements; elem; elem = elem->next) {
    if ( elem->action_type == ge_mActionType_ValueInput) {
      GeValueInput *e = (GeValueInput *)elem;
      if ( !e->value_element) {
	for ( GeDynElem *elem2 = elements; elem2; elem2 = elem2->next) {
	  if ( elem2->dyn_type == ge_mDynType_Value) {
	    e->value_element = (GeValue *)elem2;
	    e->annot_typeid = e->value_element->annot_typeid;
	    e->annot_size = e->value_element->annot_size;
	    break;
	  }
	}
	if ( !e->value_element)
	  return;
      }
      strcpy( e->value_element->format, format);
      e->min_value = min_value;
      e->max_value = max_value;
      break;
    }
  }
}

int GeDyn::get_attr_typeid()
{
  for ( GeDynElem *elem = elements; elem; elem = elem->next) {
    if ( elem->dyn_type == ge_mDynType_Value)
      return ((GeValue *)elem)->annot_typeid;
  }
  return 0;
}

int *GeDyn::ref_slider_disabled()
{
  for ( GeDynElem *elem = elements; elem; elem = elem->next) {
    if ( elem->action_type == ge_mActionType_Slider)
      return &((GeSlider *)elem)->slider_disabled;
  }
  return 0;
}

int *GeDyn::ref_trend_hold()
{
  for ( GeDynElem *elem = elements; elem; elem = elem->next) {
    if ( elem->dyn_type == ge_mDynType_Trend)
      return &((GeTrend *)elem)->trend_hold;
  }
  return 0;
}

double *GeDyn::ref_trend_scantime()
{
  for ( GeDynElem *elem = elements; elem; elem = elem->next) {
    if ( elem->dyn_type == ge_mDynType_Trend)
      return &((GeTrend *)elem)->scan_time;
  }
  return 0;
}

void *GeDyn::get_p()
{
  for ( GeDynElem *elem = elements; elem; elem = elem->next) {
    if ( elem->dyn_type == ge_mDynType_DigLowColor)
      return ((GeDigLowColor *)elem)->p;
    else if ( elem->dyn_type == ge_mDynType_Value)
      return ((GeValue *)elem)->p;
  }
  return 0;
}

void GeDyn::set_p( void *p)
{
  for ( GeDynElem *elem = elements; elem; elem = elem->next) {
    if ( elem->dyn_type == ge_mDynType_Value) {
      ((GeValue *)elem)->p = p;
      break;
    }
    else if ( elem->dyn_type == ge_mDynType_DigLowColor) {
      ((GeDigLowColor *)elem)->p = (pwr_tBoolean *)p;
      break;
    }
  }
}

void GeDyn::update()
{
  for ( GeDynElem *elem = elements; elem; elem = elem->next)
    elem->update();
}

void GeDyn::update_elements()
{
  GeDynElem *elem, *prev, *next;

  // Remove
  prev = 0;
  elem = elements;
  while ( elem) {
    if ( (elem->dyn_type && !(elem->dyn_type & total_dyn_type)) || 
	 (elem->action_type && !(elem->action_type & total_action_type))) {
      // Type is not valid, remove element
      if ( !prev)
	elements = elem->next;
      else
	prev->next = elem->next;
      next = elem->next;
      delete elem;
      elem = next;
    }
    else if ( elem->instance != ge_mInstance_1) {
      // Check if instance is valid
      GeDynElem *mask_elem = elem;
      while ( mask_elem) {
	if ( mask_elem->instance == ge_mInstance_1)
	  break;
	mask_elem = mask_elem->next;
      }
      if ( mask_elem && !(mask_elem->instance_mask & elem->instance)) {
	// Instance is not valid, remove element
	if ( !prev)
	  elements = elem->next;
	else
	  prev->next = elem->next;
	next = elem->next;
	delete elem;
	elem = next;
      }
      else {
	prev = elem;
	elem = elem->next;
      }
    }
    else {
      prev = elem;
      elem = elem->next;
    }
  }

  // Insert new
  int mask;
  bool found;
  GeDynElem *e;
  int i_mask;
  bool i_found;
  GeDynElem *i_elem;

  mask = 1;
  for ( int i = 0; i < 32; i++) {
    if ( mask & total_dyn_type) {
      found = false;
      for ( elem = elements; elem; elem = elem->next) {
	if ( elem->dyn_type == mask && elem->instance == ge_mInstance_1) {
	  found = true;

	  if ( elem->instance_mask > ge_mInstance_1) {
	    // Check instance
	    i_mask = ge_mInstance_1;
	    for ( int j = 0; j < 32; j++) {
	      i_found = false;
	      for ( i_elem = elements; i_elem; i_elem = i_elem->next) {
		if ( i_elem->dyn_type == mask && 
		     i_elem->instance & elem->instance_mask &&
		     i_elem->instance == i_mask) {
		  i_found = true;
		  break;
		}
	      }
	      if ( !i_found && i_mask & elem->instance_mask) {
		e = create_dyn_element( mask, i_mask);
		if ( e)
		  insert_element( e);
	      }
	      i_mask = i_mask << 1;
	    }
	  }
	  break;
	}
      }
      if ( !found) {
	// Create this element
	e = create_dyn_element( mask, ge_mInstance_1);
	if ( e)
	  insert_element( e);
      }
    }
    mask = mask << 1;
  }

  mask = 1;
  for ( int i = 0; i < 32; i++) {
    if ( mask & total_action_type) {
      found = false;
      for ( elem = elements; elem; elem  = elem->next) {
	if ( elem->action_type == mask && elem->instance == ge_mInstance_1) {
	  found = true;

	  if ( elem->instance_mask > ge_mInstance_1) {
	    // Check instance
	    i_mask = ge_mInstance_1;
	    for ( int j = 0; j < 32; j++) {
	      i_found = false;
	      for ( i_elem = elements; i_elem; i_elem = i_elem->next) {
		if ( i_elem->action_type == mask && 
		     i_elem->instance & elem->instance_mask &&
		     i_elem->instance == i_mask) {
		  i_found = true;
		  break;
		}
	      }
	      if ( !i_found && i_mask & elem->instance_mask) {
		e = create_action_element( mask, i_mask);
		if ( e)
		  insert_element( e);
	      }
	      i_mask = i_mask << 1;
	    }
	  }
	  break;
	}
      }
      if ( !found) {
	// Create this element
	e = create_action_element( mask, ge_mInstance_1);
	if ( e)
	  insert_element( e);
      }
    }
    mask = mask << 1;
  }
}

GeDynElem *GeDyn::create_action_element( int mask, int instance)
{
  GeDynElem *e = 0;

  switch ( mask) {
  case ge_mActionType_PopupMenu:
    e = (GeDynElem *) new GePopupMenu(this);
    break;
  case ge_mActionType_SetDig:
    e = (GeDynElem *) new GeSetDig(this, (ge_mInstance)instance);
    break;
  case ge_mActionType_ResetDig:
    e = (GeDynElem *) new GeResetDig(this, (ge_mInstance)instance);
    break;
  case ge_mActionType_ToggleDig:
    e = (GeDynElem *) new GeToggleDig(this);
    break;
  case ge_mActionType_StoDig:
    e = (GeDynElem *) new GeStoDig(this);
    break;
  case ge_mActionType_Command:
    e = (GeDynElem *) new GeCommand(this);
    break;
  case ge_mActionType_CommandDoubleClick:
    e = (GeDynElem *) new GeCommandDoubleClick(this);
    break;
  case ge_mActionType_Confirm:
    e = (GeDynElem *) new GeConfirm(this);
    break;
  case ge_mActionType_IncrAnalog:
    e = (GeDynElem *) new GeIncrAnalog(this);
    break;
  case ge_mActionType_RadioButton:
    e = (GeDynElem *) new GeRadioButton(this);
    break;
  case ge_mActionType_Slider:
    e = (GeDynElem *) new GeSlider(this);
    break;
  case ge_mActionType_ValueInput:
    e = (GeDynElem *) new GeValueInput(this);
    break;
  case ge_mActionType_TipText:
    e = (GeDynElem *) new GeTipText(this);
    break;
  case ge_mActionType_Help:
    e = (GeDynElem *) new GeHelp(this);
    break;
  case ge_mActionType_OpenGraph:
    e = (GeDynElem *) new GeOpenGraph(this);
    break;
  case ge_mActionType_OpenURL:
    e = (GeDynElem *) new GeOpenURL(this);
    break;
  case ge_mActionType_InputFocus:
    e = (GeDynElem *) new GeInputFocus(this);
    break;
  case ge_mActionType_CloseGraph:
    e = (GeDynElem *) new GeCloseGraph(this);
    break;
  case ge_mActionType_PulldownMenu:
    e = (GeDynElem *) new GePulldownMenu(this);
    break;
  case ge_mActionType_OptionMenu:
    e = (GeDynElem *) new GeOptionMenu(this);
    break;
  default: ;
  }
  return e;
}

GeDynElem *GeDyn::create_dyn_element( int mask, int instance)
{
  GeDynElem *e = 0;

  switch ( mask) {
  case ge_mDynType_DigLowColor:
    e = (GeDynElem *) new GeDigLowColor(this);
    break;
  case ge_mDynType_DigColor:
    e = (GeDynElem *) new GeDigColor(this, (ge_mInstance)instance);
    break;
  case ge_mDynType_DigWarning:
    e = (GeDynElem *) new GeDigWarning(this);
    break;
  case ge_mDynType_DigError:
    e = (GeDynElem *) new GeDigError(this);
    break;
  case ge_mDynType_DigFlash:
    e = (GeDynElem *) new GeDigFlash(this);
    break;
  case ge_mDynType_Invisible:
    e = (GeDynElem *) new GeInvisible(this);
    break;
  case ge_mDynType_DigBorder:
    e = (GeDynElem *) new GeDigBorder(this);
    break;
  case ge_mDynType_DigText:
    e = (GeDynElem *) new GeDigText(this);
    break;
  case ge_mDynType_Value:
    e = (GeDynElem *) new GeValue(this, (ge_mInstance)instance);
    break;
  case ge_mDynType_AnalogColor:
    e = (GeDynElem *) new GeAnalogColor(this, (ge_mInstance)instance);
    break;
  case ge_mDynType_Rotate:
    e = (GeDynElem *) new GeRotate(this);
    break;
  case ge_mDynType_Move:
    e = (GeDynElem *) new GeMove(this);
    break;
  case ge_mDynType_AnalogShift:
    e = (GeDynElem *) new GeAnalogShift(this);
    break;
  case ge_mDynType_DigShift:
    e = (GeDynElem *) new GeDigShift(this);
    break;
  case ge_mDynType_Animation:
    e = (GeDynElem *) new GeAnimation(this);
    break;
  case ge_mDynType_Video:
    e = (GeDynElem *) new GeVideo(this);
    break;
  case ge_mDynType_Bar:
    e = (GeDynElem *) new GeBar(this);
    break;
  case ge_mDynType_Trend:
    e = (GeDynElem *) new GeTrend(this);
    break;
  case ge_mDynType_FillLevel:
    e = (GeDynElem *) new GeFillLevel(this);
    break;
  case ge_mDynType_FastCurve:
    e = (GeDynElem *) new GeFastCurve(this);
    break;
  case ge_mDynType_AnalogText:
    e = (GeDynElem *) new GeAnalogText(this);
    break;
  case ge_mDynType_Table:
    e = (GeDynElem *) new GeTable(this);
    break;
  case ge_mDynType_StatusColor:
    e = (GeDynElem *) new GeStatusColor(this);
    break;
  default: ;
  }
  return e;
}

void GeDyn::insert_element( GeDynElem *e)
{
  GeDynElem *elem, *prev;

  e->dyn = this;
  elem = elements;
  prev = 0;
  while ( elem) {
    if ( elem->prio > e->prio)
      break;
    if ( elem->prio == e->prio && elem->instance < e->instance)
      break;
    prev = elem;
    elem = elem->next;
  }
  if ( prev) {
    e->next = prev->next;
    prev->next = e;
  }
  else {
    e->next = elements;
    elements = e;
  }
}

int GeDyn::disconnect( grow_tObject object)
{
  for ( GeDynElem *elem = elements; elem; elem = elem->next)
    elem->disconnect( object);
  return 1;
}

int GeDyn::connect( grow_tObject object, glow_sTraceData *trace_data)
{
  int inherit_dyn_type;
  int inherit_action_type;

  if ( grow_GetObjectType( object) == glow_eObjectType_GrowBar || 
       grow_GetObjectType( object) == glow_eObjectType_GrowTable ||
       grow_GetObjectType( object) == glow_eObjectType_GrowWindow ||
       grow_GetObjectType( object) == glow_eObjectType_GrowTrend) {
    if ( cycle == glow_eCycle_Inherit)
      cycle = glow_eCycle_Slow;
    if ( dyn_type & ge_mDynType_Inherit)
      dyn_type = ge_mDynType( dyn_type & ~ge_mDynType_Inherit);
    if ( action_type & ge_mActionType_Inherit)
      action_type = ge_mActionType( action_type & ~ge_mActionType_Inherit);
  }

  if ( dyn_type & ge_mDynType_Inherit) {
    grow_GetObjectClassDynType( object, &inherit_dyn_type, &inherit_action_type);
    total_dyn_type = ge_mDynType( dyn_type | inherit_dyn_type);
  }
  else
    total_dyn_type = dyn_type;

  if ( action_type & ge_mActionType_Inherit) {
    grow_GetObjectClassDynType( object, &inherit_dyn_type, &inherit_action_type);
    total_action_type = ge_mActionType( action_type | inherit_action_type);
  }
  else
    total_action_type = action_type;

  if ( cycle == glow_eCycle_Inherit)
    grow_GetObjectClassCycle( object, &cycle);

  for ( GeDynElem *elem = elements; elem; elem = elem->next)
    elem->connect( object, trace_data);
  return 1;
}

int GeDyn::scan( grow_tObject object)
{
  reset_color = false;
  ignore_color = false;
  for ( GeDynElem *elem = elements; elem; elem = elem->next)
    elem->scan( object);
  return 1;
}

int GeDyn::action( grow_tObject object, glow_tEvent event)
{
  int sts;

  for ( GeDynElem *elem = elements; elem; elem = elem->next) {
    sts = elem->action( object, event);
    if ( sts == GE__NO_PROPAGATE)
      return sts;
  }
  return 1;
}

int GeDyn::confirmed_action( grow_tObject object, glow_tEvent event)
{
  total_action_type = (ge_mActionType) (total_action_type & ~ge_mActionType_Confirm);

  for ( GeDynElem *elem = elements; elem; elem = elem->next)
    elem->action( object, event);

  total_action_type = (ge_mActionType) (total_action_type | ge_mActionType_Confirm);
  return 1;
}

int GeDyn::change_value( grow_tObject object, char *text)
{
  for ( GeDynElem *elem = elements; elem; elem = elem->next)
    elem->change_value( object, text);
  return 1;
}

void GeDyn::export_java( grow_tObject object, ofstream& fp, char *var_name)
{
  int inherit_dyn_type, inherit_action_type;

  if ( grow_GetObjectType( object) == glow_eObjectType_GrowBar || 
       grow_GetObjectType( object) == glow_eObjectType_GrowTable ||
       grow_GetObjectType( object) == glow_eObjectType_GrowWindow ||
       grow_GetObjectType( object) == glow_eObjectType_GrowTrend) {
    if ( cycle == glow_eCycle_Inherit)
      cycle = glow_eCycle_Slow;
    if ( dyn_type & ge_mDynType_Inherit)
      dyn_type = ge_mDynType( dyn_type & ~ge_mDynType_Inherit);
    if ( action_type & ge_mActionType_Inherit)
      action_type = ge_mActionType( action_type & ~ge_mActionType_Inherit);
  }

  if ( dyn_type & ge_mDynType_Inherit) {
    grow_GetObjectClassDynType( object, &inherit_dyn_type, &inherit_action_type);
    total_dyn_type = ge_mDynType( dyn_type | inherit_dyn_type);
  }
  else
    total_dyn_type = dyn_type;

  if ( action_type & ge_mActionType_Inherit) {
    grow_GetObjectClassDynType( object, &inherit_dyn_type, &inherit_action_type);
    total_action_type = ge_mActionType( (action_type | inherit_action_type) & ~ge_mActionType_Inherit);
  }
  else
    total_action_type = action_type;

  fp <<
"    " << var_name << ".dd.setDynType(" << total_dyn_type << ");" << endl <<
"    " << var_name << ".dd.setActionType(" << total_action_type << ");" << endl;
  if ( total_action_type)
    fp <<
"    " << var_name << ".dd.setAccess(" << access << ");" << endl;

  if ( elements) {
    fp <<
"    " << var_name << ".dd.setElements(new GeDynElemIfc[] {" << endl;
    bool first = true;
    int sts = 0;
    for ( GeDynElem *elem = elements; elem; elem = elem->next) {
      sts = elem->export_java( object, fp, first, var_name);
      if ( sts)
	first = false;
    }
    fp <<
"      });" << endl;
  }
}

void GeDyn::export_java_object( grow_tObject object, ofstream& fp, char *var_name)
{

  fp <<
      "    new GeDyn(" << var_name << "," << dyn_type << "," << action_type << "," << access << "," << "new GeDynElemIfc[] {" << endl;

  bool first = true;
  int sts = 0;
  for ( GeDynElem *elem = elements; elem; elem = elem->next) {
    sts = elem->export_java( object, fp, first, var_name);
    if ( sts)
      first = false;
  }
  fp << "      })," << endl;
}

int GeDynElem::disconnect( grow_tObject object)
{
  return 1;
}

int GeDynElem::get_transtab( char **tt)
{
  static char transtab[][32] = {	"SubGraph",		"SubGraph",
					"Dynamic",		"",
					""};

  *tt = (char *) transtab;
  return 1;
}

void GeDigLowColor::get_attributes( attr_sItem *attrinfo, int *item_count)
{
  int i = *item_count;

  if ( dyn->total_dyn_type & ge_mDynType_Tone) {
    strcpy( attrinfo[i].name, "DigLowTone.Attribute");
    attrinfo[i].value = attribute;
    attrinfo[i].type = glow_eType_String;
    attrinfo[i++].size = sizeof( attribute);

    strcpy( attrinfo[i].name, "DigLowTone.Tone");
    attrinfo[i].value = &color;
    attrinfo[i].type = glow_eType_ToneOrColor;
    attrinfo[i++].size = sizeof( color);
  }
  else {
    strcpy( attrinfo[i].name, "DigLowColor.Attribute");
    attrinfo[i].value = attribute;
    attrinfo[i].type = glow_eType_String;
    attrinfo[i++].size = sizeof( attribute);

    strcpy( attrinfo[i].name, "DigLowColor.Color");
    attrinfo[i].value = &color;
    attrinfo[i].type = glow_eType_Color;
    attrinfo[i++].size = sizeof( color);
  }
  *item_count = i;
}

void GeDigLowColor::set_attribute( grow_tObject object, char *attr_name, int *cnt)
{
  (*cnt)--;
  if ( *cnt == 0) {
    char msg[200];

    strncpy( attribute, attr_name, sizeof( attribute));
    if ( dyn->total_dyn_type & ge_mDynType_Tone)
      sprintf( msg, "DigLowTone.Attribute = %s", attr_name);
    else
      sprintf( msg, "DigLowColor.Attribute = %s", attr_name);
    dyn->graph->message( 'I', msg);
  }
}

void GeDigLowColor::replace_attribute( char *from, char *to, int *cnt, int strict)
{
  GeDyn::replace_attribute( attribute, sizeof(attribute), from, to, cnt, strict);
}

int GeDigLowColor::set_color( grow_tObject object, glow_eDrawType color)
{
  char msg[200];

  if ( dyn->total_dyn_type & ge_mDynType_Tone) {
    this->color = glow_eDrawType( color / 30);
    sprintf( msg, "DigLowTone.Tone = %s", grow_ColorToneToName( this->color));
  }
  else {
    this->color = color;
    sprintf( msg, "DigLowColor.Color = %s", grow_ColorToName(this->color));
  }
  dyn->graph->message( 'I', msg);
  return 1;
}

void GeDigLowColor::save( ofstream& fp)
{
  fp << int(ge_eSave_DigLowColor) << endl;
  fp << int(ge_eSave_DigLowColor_attribute) << FSPACE << attribute << endl;
  fp << int(ge_eSave_DigLowColor_color) << FSPACE << int(color) << endl;
  fp << int(ge_eSave_End) << endl;
}

void GeDigLowColor::open( ifstream& fp)
{
  int		type;
  int 		end_found = 0;
  int		tmp;
  char		dummy[40];

  for (;;)
  {
    fp >> type;
    switch( type) {
      case ge_eSave_DigLowColor: break;
      case ge_eSave_DigLowColor_attribute:
        fp.get();
        fp.getline( attribute, sizeof(attribute));
        break;
      case ge_eSave_DigLowColor_color: fp >> tmp; color = (glow_eDrawType)tmp; break;
      case ge_eSave_End: end_found = 1; break;
      default:
        cout << "GeDigLowColor:open syntax error" << endl;
        fp.getline( dummy, sizeof(dummy));
    }
    if ( end_found)
      break;
  }  
}

int GeDigLowColor::connect( grow_tObject object, glow_sTraceData *trace_data)
{
  int		attr_type, attr_size;
  char		parsed_name[120];
  int		sts;

  color = dyn->get_color1( object, color);
  if ( color < 0 || color >= glow_eDrawType__) {
    printf( "** Color out of range, %s\n", attribute);
    return 0;
  }

  size = 4;
  p = 0;
  db = dyn->graph->parse_attr_name( attribute, parsed_name,
				    &inverted, &attr_type, &attr_size);
  if ( strcmp( parsed_name,"") == 0)
    return 1;

  sts = dyn->graph->ref_object_info( dyn->cycle, parsed_name, (void **)&p, &subid, size);
  if ( EVEN(sts)) return sts;

  if ( p)
    trace_data->p = p;
  first_scan = true;
  return 1;
}

int GeDigLowColor::disconnect( grow_tObject object)
{
  if ( p && db == graph_eDatabase_Gdh)
    gdh_UnrefObjectInfo( subid);
  p = 0;
  return 1;
}

int GeDigLowColor::scan( grow_tObject object)
{
  if ( !p || dyn->ignore_color)
    return 1;

  if ( !first_scan) {
    if ( old_value == *p && !dyn->reset_color)
      // No change since last time
      return 1;
  }
  else
    first_scan = false;

  if ( dyn->total_dyn_type & ge_mDynType_Tone) {
    if ( (!inverted && !*p) || (inverted && *p)) {
      if ( color >= (glow_eDrawType) glow_eDrawTone__)
	grow_SetObjectFillColor( object, color);
      else
	grow_SetObjectColorTone( object, (glow_eDrawTone) color);
    }
    else {
      if ( color >= (glow_eDrawType) glow_eDrawTone__)
	grow_ResetObjectFillColor( object);
      grow_ResetObjectColorTone( object);
    }
  }
  else {
    if ( (!inverted && !*p) || (inverted && *p))
      grow_SetObjectFillColor( object, color);
    else
      grow_ResetObjectFillColor( object);
  }
  old_value = *p;
  return 1;
}

int GeDigLowColor::export_java( grow_tObject object, ofstream& fp, bool first, char *var_name)
{
  glow_eDrawType jcolor = dyn->get_color1( object, color);
  if ( first)
    fp << "      ";
  else
    fp << "      ,";
  fp << "new GeDynDigLowColor(" << var_name << ".dd, \"" << attribute << "\"," << jcolor << ")" << endl;
  return 1;
}

void GeDigColor::get_attributes( attr_sItem *attrinfo, int *item_count)
{
  int i = *item_count;

  if ( dyn->total_dyn_type & ge_mDynType_Tone) {
    if ( instance == ge_mInstance_1) {
      strcpy( attrinfo[i].name, "DigTone.Attribute");
      attrinfo[i].value = attribute;
      attrinfo[i].type = glow_eType_String;
      attrinfo[i++].size = sizeof( attribute);

      strcpy( attrinfo[i].name, "DigTone.Tone");
      attrinfo[i].value = &color;
      attrinfo[i].type = glow_eType_ToneOrColor;
      attrinfo[i++].size = sizeof( color);

      strcpy( attrinfo[i].name, "DigTone.Instances");
      attrinfo[i].value = &instance_mask;
      attrinfo[i].type = ge_eAttrType_InstanceMask;
      attrinfo[i++].size = sizeof( instance_mask);
    }
    else {
      // Get instance number
      int inst = 1;
      int m = instance;
      while( m > 1) {
	m = m >> 1;
	inst++;
      }

      sprintf( attrinfo[i].name, "DigTone%d.Attribute", inst);
      attrinfo[i].value = attribute;
      attrinfo[i].type = glow_eType_String;
      attrinfo[i++].size = sizeof( attribute);

      sprintf( attrinfo[i].name, "DigTone%d.Tone", inst);
      attrinfo[i].value = &color;
      attrinfo[i].type = glow_eType_ToneOrColor;
      attrinfo[i++].size = sizeof( color);
    }
  }
  else {
    if ( instance == ge_mInstance_1) {
      strcpy( attrinfo[i].name, "DigColor.Attribute");
      attrinfo[i].value = attribute;
      attrinfo[i].type = glow_eType_String;
      attrinfo[i++].size = sizeof( attribute);

      strcpy( attrinfo[i].name, "DigColor.Color");
      attrinfo[i].value = &color;
      attrinfo[i].type = glow_eType_Color;
      attrinfo[i++].size = sizeof( color);

      strcpy( attrinfo[i].name, "DigColor.Instances");
      attrinfo[i].value = &instance_mask;
      attrinfo[i].type = ge_eAttrType_InstanceMask;
      attrinfo[i++].size = sizeof( instance_mask);    
    }
    else {
      // Get instance number
      int inst = 1;
      int m = instance;
      while( m > 1) {
	m = m >> 1;
	inst++;
      }

      sprintf( attrinfo[i].name, "DigColor%d.Attribute", inst);
      attrinfo[i].value = attribute;
      attrinfo[i].type = glow_eType_String;
      attrinfo[i++].size = sizeof( attribute);

      sprintf( attrinfo[i].name, "DigColor%d.Color", inst);
      attrinfo[i].value = &color;
      attrinfo[i].type = glow_eType_Color;
      attrinfo[i++].size = sizeof( color);
    }
  }
  *item_count = i;
}

void GeDigColor::set_attribute( grow_tObject object, char *attr_name, int *cnt)
{
  (*cnt)--;
  if ( *cnt == 0) {
    char msg[200];

    strncpy( attribute, attr_name, sizeof( attribute));
    if ( instance == ge_mInstance_1) {
      if ( dyn->total_dyn_type & ge_mDynType_Tone)
	sprintf( msg, "DigTone.Attribute = %s", attr_name);
      else
	sprintf( msg, "DigColor.Attribute = %s", attr_name);
    }
    else {
      if ( dyn->total_dyn_type & ge_mDynType_Tone)
	sprintf( msg, "DigTone%d.Attribute = %s", GeDyn::instance_to_number( instance),
	       attr_name);
      else
	sprintf( msg, "DigColor%d.Attribute = %s", GeDyn::instance_to_number( instance),
	       attr_name);
    }
    dyn->graph->message( 'I', msg);
  }
}

void GeDigColor::replace_attribute( char *from, char *to, int *cnt, int strict)
{
  GeDyn::replace_attribute( attribute, sizeof(attribute), from, to, cnt, strict);
}

int GeDigColor::set_color( grow_tObject object, glow_eDrawType color)
{
  char msg[200];

  if ( dyn->total_dyn_type & ge_mDynType_Tone) {
    this->color = glow_eDrawType( color / 30);
    if ( instance == ge_mInstance_1)
      sprintf( msg, "DigTone.Tone = %s", grow_ColorToneToName( this->color));
    else
      sprintf( msg, "DigTone%d.Tone = %s", GeDyn::instance_to_number( instance),
	       grow_ColorToneToName( this->color));
  }
  else {
    this->color = color;
    if ( instance == ge_mInstance_1)
      sprintf( msg, "DigColor.Color = %s", grow_ColorToName( this->color));
    else
      sprintf( msg, "DigColor%d.Color = %s", GeDyn::instance_to_number( instance),
	       grow_ColorToName( this->color));
  }
  dyn->graph->message( 'I', msg);
  return 1;
}

void GeDigColor::save( ofstream& fp)
{
  fp << int(ge_eSave_DigColor) << endl;
  fp << int(ge_eSave_DigColor_attribute) << FSPACE << attribute << endl;
  fp << int(ge_eSave_DigColor_color) << FSPACE << int(color) << endl;
  fp << int(ge_eSave_DigColor_instance) << FSPACE << int(instance) << endl;
  fp << int(ge_eSave_DigColor_instance_mask) << FSPACE << int(instance_mask) << endl;
  fp << int(ge_eSave_End) << endl;
}

void GeDigColor::open( ifstream& fp)
{
  int		type;
  int 		end_found = 0;
  int		tmp;
  char		dummy[40];

  for (;;)
  {
    fp >> type;
    switch( type) {
      case ge_eSave_DigColor: break;
      case ge_eSave_DigColor_attribute:
        fp.get();
        fp.getline( attribute, sizeof(attribute));
        break;
      case ge_eSave_DigColor_color: fp >> tmp; color = (glow_eDrawType)tmp; break;
      case ge_eSave_DigColor_instance: fp >> tmp; instance = (ge_mInstance)tmp; break;
      case ge_eSave_DigColor_instance_mask: fp >> tmp; instance_mask = (ge_mInstance)tmp; break;
      case ge_eSave_End: end_found = 1; break;
      default:
        cout << "GeDigColor:open syntax error" << endl;
        fp.getline( dummy, sizeof(dummy));
    }
    if ( end_found)
      break;
  }  
}

int GeDigColor::connect( grow_tObject object, glow_sTraceData *trace_data)
{
  int		attr_type, attr_size;
  char		parsed_name[120];
  int		sts;

  color = dyn->get_color2( object, color);
  if ( color < 0 || color >= glow_eDrawType__) {
    printf( "** Color out of range, %s\n", attribute);
    return 0;
  }

  size = 4;
  p = 0;
  db = dyn->graph->parse_attr_name( attribute, parsed_name,
				    &inverted, &attr_type, &attr_size);
  if ( strcmp( parsed_name,"") == 0)
    return 1;

  sts = dyn->graph->ref_object_info( dyn->cycle, parsed_name, (void **)&p, &subid, size);
  if ( EVEN(sts)) return sts;

  if ( p)
    trace_data->p = p;
  first_scan = true;
  return 1;
}

int GeDigColor::disconnect( grow_tObject object)
{
  if ( p && db == graph_eDatabase_Gdh)
    gdh_UnrefObjectInfo( subid);
  p = 0;
  return 1;
}

int GeDigColor::scan( grow_tObject object)
{
  if ( !p || dyn->ignore_color)
    return 1;

  if ( !first_scan) {
    if ( old_value == *p && !dyn->reset_color) {
      // No change since last time
      if ( (!inverted && *p) || (inverted && !*p))
	dyn->ignore_color = true;
      return 1;
    }
  }
  else
    first_scan = false;

  if ( dyn->total_dyn_type & ge_mDynType_Tone) {
    if ( (!inverted && *p) || (inverted && !*p)) {
      if ( color >= (glow_eDrawType) glow_eDrawTone__)
	grow_SetObjectFillColor( object, color);
      else
	grow_SetObjectColorTone( object, (glow_eDrawTone) color);
      dyn->ignore_color = true;
    }
    else {
      if ( color >= (glow_eDrawType) glow_eDrawTone__)
	grow_ResetObjectFillColor( object);
      grow_ResetObjectColorTone( object);
      dyn->reset_color = true;
    }
  }
  else {
    if ( (!inverted && *p) || (inverted && !*p)) {
      grow_SetObjectFillColor( object, color);
      dyn->ignore_color = true;
    }
    else {
      grow_ResetObjectFillColor( object);
      dyn->reset_color = true;
    }
  }
  old_value = *p;
  return 1;
}

int GeDigColor::export_java( grow_tObject object, ofstream& fp, bool first, char *var_name)
{
  glow_eDrawType jcolor = dyn->get_color2( object, color);
  if ( first)
    fp << "      ";
  else
    fp << "      ,";
  fp << "new GeDynDigColor(" << var_name << ".dd, \"" << attribute << "\"," << jcolor << ")" << endl;
  return 1;
}

void GeDigWarning::get_attributes( attr_sItem *attrinfo, int *item_count)
{
  int i = *item_count;

  strcpy( attrinfo[i].name, "DigWarning.Attribute");
  attrinfo[i].value = attribute;
  attrinfo[i].type = glow_eType_String;
  attrinfo[i++].size = sizeof( attribute);

  *item_count = i;
}

void GeDigWarning::set_attribute( grow_tObject object, char *attr_name, int *cnt)
{
  (*cnt)--;
  if ( *cnt == 0) {
    char msg[200];

    strncpy( attribute, attr_name, sizeof( attribute));
    sprintf( msg, "DigWaring.Attribute = %s", attr_name);
    dyn->graph->message( 'I', msg);
  }
}

void GeDigWarning::replace_attribute( char *from, char *to, int *cnt, int strict)
{
  GeDyn::replace_attribute( attribute, sizeof(attribute), from, to, cnt, strict);
}

void GeDigWarning::save( ofstream& fp)
{
  fp << int(ge_eSave_DigWarning) << endl;
  fp << int(ge_eSave_DigWarning_attribute) << FSPACE << attribute << endl;
  fp << int(ge_eSave_End) << endl;
}

void GeDigWarning::open( ifstream& fp)
{
  int		type;
  int 		end_found = 0;
  char		dummy[40];

  for (;;)
  {
    fp >> type;
    switch( type) {
      case ge_eSave_DigWarning: break;
      case ge_eSave_DigWarning_attribute:
        fp.get();
        fp.getline( attribute, sizeof(attribute));
        break;
      case ge_eSave_End: end_found = 1; break;
      default:
        cout << "GeDigWarning:open syntax error" << endl;
        fp.getline( dummy, sizeof(dummy));
    }
    if ( end_found)
      break;
  }  
}

int GeDigWarning::connect( grow_tObject object, glow_sTraceData *trace_data)
{
  int		attr_type, attr_size;
  char		parsed_name[120];
  int		sts;

  size = 4;
  p = 0;
  db = dyn->graph->parse_attr_name( attribute, parsed_name,
				    &inverted, &attr_type, &attr_size);
  if ( strcmp( parsed_name,"") == 0)
    return 1;

  sts = dyn->graph->ref_object_info( dyn->cycle, parsed_name, (void **)&p, &subid, size);
  if ( EVEN(sts)) return sts;

  if ( p)
    trace_data->p = p;
  first_scan = true;
  return 1;
}

int GeDigWarning::disconnect( grow_tObject object)
{
  if ( p && db == graph_eDatabase_Gdh)
    gdh_UnrefObjectInfo( subid);
  p = 0;
  return 1;
}

int GeDigWarning::scan( grow_tObject object)
{
  if ( !p || dyn->ignore_color)
    return 1;

  if ( !first_scan) {
    if ( old_value == *p && !dyn->reset_color) {
      // No change since last time
      if ( (!inverted && *p) || (inverted && !*p))
	dyn->ignore_color = true;
      return 1;
    }
  }
  else
    first_scan = false;

  if ( dyn->total_dyn_type & ge_mDynType_Tone) {
    if ( (!inverted && *p) || (inverted && !*p)) {
      grow_SetObjectColorTone( object, glow_eDrawTone_Yellow);
      dyn->ignore_color = true;
    }
    else {
      grow_ResetObjectColorTone( object);
      dyn->reset_color = true;
    }
  }
  else {
    if ( (!inverted && *p) || (inverted && !*p)) {
      grow_SetObjectFillColor( object, glow_eDrawType_ColorYellow);
      dyn->ignore_color = true;
    }
    else {
      grow_ResetObjectFillColor( object);
      dyn->reset_color = true;
    }
  }
  old_value = *p;
  return 1;
}

int GeDigWarning::export_java( grow_tObject object, ofstream& fp, bool first, char *var_name)
{
  if ( first)
    fp << "      ";
  else
    fp << "      ,";
  fp << "new GeDynDigWarning(" << var_name << ".dd, \"" << attribute << "\")" << endl;
  return 1;
}

void GeDigError::get_attributes( attr_sItem *attrinfo, int *item_count)
{
  int i = *item_count;

  strcpy( attrinfo[i].name, "DigError.Attribute");
  attrinfo[i].value = attribute;
  attrinfo[i].type = glow_eType_String;
  attrinfo[i++].size = sizeof( attribute);

  *item_count = i;
}

void GeDigError::set_attribute( grow_tObject object, char *attr_name, int *cnt)
{
  (*cnt)--;
  if ( *cnt == 0) {
    char msg[200];

    strncpy( attribute, attr_name, sizeof( attribute));
    sprintf( msg, "DigError.Attribute = %s", attr_name);
    dyn->graph->message( 'I', msg);
  }
}

void GeDigError::replace_attribute( char *from, char *to, int *cnt, int strict)
{
  GeDyn::replace_attribute( attribute, sizeof(attribute), from, to, cnt, strict);
}

void GeDigError::save( ofstream& fp)
{
  fp << int(ge_eSave_DigError) << endl;
  fp << int(ge_eSave_DigError_attribute) << FSPACE << attribute << endl;
  fp << int(ge_eSave_End) << endl;
}

void GeDigError::open( ifstream& fp)
{
  int		type;
  int 		end_found = 0;
  char		dummy[40];

  for (;;)
  {
    fp >> type;
    switch( type) {
      case ge_eSave_DigError: break;
      case ge_eSave_DigError_attribute:
        fp.get();
        fp.getline( attribute, sizeof(attribute));
        break;
      case ge_eSave_End: end_found = 1; break;
      default:
        cout << "GeDigError:open syntax error" << endl;
        fp.getline( dummy, sizeof(dummy));
    }
    if ( end_found)
      break;
  }  
}

int GeDigError::connect( grow_tObject object, glow_sTraceData *trace_data)
{
  int		attr_type, attr_size;
  char		parsed_name[120];
  int		sts;

  size = 4;
  p = 0;
  db = dyn->graph->parse_attr_name( attribute, parsed_name,
				    &inverted, &attr_type, &attr_size);
  if ( strcmp( parsed_name,"") == 0)
    return 1;

  sts = dyn->graph->ref_object_info( dyn->cycle, parsed_name, (void **)&p, &subid, size);
  if ( EVEN(sts)) return sts;

  if ( p)
    trace_data->p = p;
  first_scan = true;
  return 1;
}

int GeDigError::disconnect( grow_tObject object)
{
  if ( p && db == graph_eDatabase_Gdh)
    gdh_UnrefObjectInfo( subid);
  p = 0;
  return 1;
}

int GeDigError::scan( grow_tObject object)
{
  if ( !p || dyn->ignore_color)
    return 1;

  if ( !first_scan) {
    if ( old_value == *p && !dyn->reset_color) {
      // No change since last time
      if ( (!inverted && *p) || (inverted && !*p))
	dyn->ignore_color = true;
      return 1;
    }
  }
  else
    first_scan = false;

  if ( dyn->total_dyn_type & ge_mDynType_Tone) {
    if ( (!inverted && *p) || (inverted && !*p)) {
      grow_SetObjectColorTone( object, glow_eDrawTone_Red);
      dyn->ignore_color = true;
    }
    else {
      grow_ResetObjectColorTone( object);
      dyn->reset_color = true;
    }
  }
  else {
    if ( (!inverted && *p) || (inverted && !*p)) {
      grow_SetObjectFillColor( object, glow_eDrawType_LineRed);
      dyn->ignore_color = true;
    }
    else {
      grow_ResetObjectFillColor( object);
      dyn->reset_color = true;
    }
  }
  old_value = *p;
  return 1;
}

int GeDigError::export_java( grow_tObject object, ofstream& fp, bool first, char *var_name)
{
  if ( first)
    fp << "      ";
  else
    fp << "      ,";
  fp << "new GeDynDigError(" << var_name << ".dd, \"" << attribute << "\")" << endl;
  return 1;
}

void GeDigFlash::get_attributes( attr_sItem *attrinfo, int *item_count)
{
  int i = *item_count;

  if ( dyn->total_dyn_type & ge_mDynType_Tone) {
    strcpy( attrinfo[i].name, "DigFlash.Attribute");
    attrinfo[i].value = attribute;
    attrinfo[i].type = glow_eType_String;
    attrinfo[i++].size = sizeof( attribute);

    strcpy( attrinfo[i].name, "DigFlash.Tone");
    attrinfo[i].value = &color;
    attrinfo[i].type = glow_eType_ToneOrColor;
    attrinfo[i++].size = sizeof( color);
  }
  else {
    strcpy( attrinfo[i].name, "DigFlash.Attribute");
    attrinfo[i].value = attribute;
    attrinfo[i].type = glow_eType_String;
    attrinfo[i++].size = sizeof( attribute);

    strcpy( attrinfo[i].name, "DigFlash.Color");
    attrinfo[i].value = &color;
    attrinfo[i].type = glow_eType_Color;
    attrinfo[i++].size = sizeof( color);
  }
  *item_count = i;
}

void GeDigFlash::set_attribute( grow_tObject object, char *attr_name, int *cnt)
{
  (*cnt)--;
  if ( *cnt == 0) {
    char msg[200];

    strncpy( attribute, attr_name, sizeof( attribute));
    sprintf( msg, "DigFlash.Attribute = %s", attr_name);
    dyn->graph->message( 'I', msg);
  }
}

void GeDigFlash::replace_attribute( char *from, char *to, int *cnt, int strict)
{
  GeDyn::replace_attribute( attribute, sizeof(attribute), from, to, cnt, strict);
}

int GeDigFlash::set_color( grow_tObject object, glow_eDrawType color)
{
  char msg[200];

  if ( dyn->total_dyn_type & ge_mDynType_Tone) {
    this->color = glow_eDrawType( color / 30);
    sprintf( msg, "DigFlash.Tone = %s", grow_ColorToneToName( this->color));
  }
  else {
    this->color = color;
    sprintf( msg, "DigFlash.Color = %s", grow_ColorToName(this->color));
  }
  dyn->graph->message( 'I', msg);
  return 1;
}

void GeDigFlash::save( ofstream& fp)
{
  fp << int(ge_eSave_DigFlash) << endl;
  fp << int(ge_eSave_DigFlash_attribute) << FSPACE << attribute << endl;
  fp << int(ge_eSave_DigFlash_color) << FSPACE << int(color) << endl;
  fp << int(ge_eSave_End) << endl;
}

void GeDigFlash::open( ifstream& fp)
{
  int		type;
  int 		end_found = 0;
  int		tmp;
  char		dummy[40];

  for (;;)
  {
    fp >> type;
    switch( type) {
      case ge_eSave_DigFlash: break;
      case ge_eSave_DigFlash_attribute:
        fp.get();
        fp.getline( attribute, sizeof(attribute));
        break;
      case ge_eSave_DigFlash_color: fp >> tmp; color = (glow_eDrawType)tmp; break;
      case ge_eSave_End: end_found = 1; break;
      default:
        cout << "GeDigFlash:open syntax error" << endl;
        fp.getline( dummy, sizeof(dummy));
    }
    if ( end_found)
      break;
  }
}

int GeDigFlash::connect( grow_tObject object, glow_sTraceData *trace_data)
{
  int		attr_type, attr_size;
  char		parsed_name[120];
  int		sts;

  color = dyn->get_color1( object, color);
  if ( color < 0 || color >= glow_eDrawType__) {
    printf( "** Color out of range, %s\n", attribute);
    return 0;
  }

  size = 4;
  p = 0;
  db = dyn->graph->parse_attr_name( attribute, parsed_name,
				    &inverted, &attr_type, &attr_size);
  if ( strcmp( parsed_name,"") == 0)
    return 1;

  sts = dyn->graph->ref_object_info( dyn->cycle, parsed_name, (void **)&p, &subid, size);
  if ( EVEN(sts)) return sts;

  if ( p)
    trace_data->p = p;
  first_scan = true;
  return 1;
}

int GeDigFlash::disconnect( grow_tObject object)
{
  if ( p && db == graph_eDatabase_Gdh)
    gdh_UnrefObjectInfo( subid);
  p = 0;
  return 1;
}

int GeDigFlash::scan( grow_tObject object)
{
  if ( !p || dyn->ignore_color)
    return 1;

  if ( !first_scan) {
    if ( old_value == *p && !dyn->reset_color) {
      // No change since last time
      if ( !((!inverted && *p) || (inverted && !*p)))
	return 1;
    }
  }
  else
    first_scan = false;

  if ( dyn->total_dyn_type & ge_mDynType_Tone) {
    if ( (!inverted && *p) || (inverted && !*p)) {
      if ( on) {
	if ( color >= (glow_eDrawType) glow_eDrawTone__)
	  grow_SetObjectFillColor( object, color);
	else
	  grow_SetObjectColorTone( object, (glow_eDrawTone) color);
	dyn->ignore_color = true;
      }
      else {
	if ( color >= (glow_eDrawType) glow_eDrawTone__)
	  grow_ResetObjectFillColor( object);
	grow_ResetObjectColorTone( object);
	dyn->reset_color = true;
      }
      on = !on;
    }
    else {
      if ( color >= (glow_eDrawType) glow_eDrawTone__)
	grow_ResetObjectFillColor( object);
      grow_ResetObjectColorTone( object);
      dyn->reset_color = true;
    }
  }
  else {
    if ( (!inverted && *p) || (inverted && !*p)) {
      if ( on) {
	grow_SetObjectFillColor( object, color);
	dyn->ignore_color = true;
      }
      else {
	grow_ResetObjectFillColor( object);
	dyn->reset_color = true;
      }
      on = !on;
    }
    else {
      grow_ResetObjectFillColor( object);
      dyn->reset_color = true;
    }
  }
  old_value = *p;
  return 1;
}

int GeDigFlash::export_java( grow_tObject object, ofstream& fp, bool first, char *var_name)
{
  glow_eDrawType jcolor = dyn->get_color1( object, color);

  if ( first)
    fp << "      ";
  else
    fp << "      ,";
  fp << "new GeDynDigFlash(" << var_name << ".dd, \"" << attribute << "\"," << jcolor << ")" << endl;
  return 1;
}

void GeInvisible::get_attributes( attr_sItem *attrinfo, int *item_count)
{
  int i = *item_count;

  strcpy( attrinfo[i].name, "Invisible.Attribute");
  attrinfo[i].value = attribute;
  attrinfo[i].type = glow_eType_String;
  attrinfo[i++].size = sizeof( attribute);

  *item_count = i;
}

void GeInvisible::set_attribute( grow_tObject object, char *attr_name, int *cnt)
{
  (*cnt)--;
  if ( *cnt == 0) {
    char msg[200];

    strncpy( attribute, attr_name, sizeof( attribute));
    sprintf( msg, "Invisible.Attribute = %s", attr_name);
    dyn->graph->message( 'I', msg);
  }
}

void GeInvisible::replace_attribute( char *from, char *to, int *cnt, int strict)
{
  GeDyn::replace_attribute( attribute, sizeof(attribute), from, to, cnt, strict);
}

void GeInvisible::save( ofstream& fp)
{
  fp << int(ge_eSave_Invisible) << endl;
  fp << int(ge_eSave_Invisible_attribute) << FSPACE << attribute << endl;
  fp << int(ge_eSave_End) << endl;
}

void GeInvisible::open( ifstream& fp)
{
  int		type;
  int 		end_found = 0;
  char		dummy[40];

  for (;;)
  {
    fp >> type;
    switch( type) {
      case ge_eSave_Invisible: break;
      case ge_eSave_Invisible_attribute:
        fp.get();
        fp.getline( attribute, sizeof(attribute));
        break;
      case ge_eSave_End: end_found = 1; break;
      default:
        cout << "GeInvisible:open syntax error" << endl;
        fp.getline( dummy, sizeof(dummy));
    }
    if ( end_found)
      break;
  }  
}

int GeInvisible::connect( grow_tObject object, glow_sTraceData *trace_data)
{
  int		attr_type, attr_size;
  char		parsed_name[120];
  int		sts;

  size = 4;
  p = 0;
  db = dyn->graph->parse_attr_name( attribute, parsed_name,
				    &inverted, &attr_type, &attr_size);
  if ( strcmp( parsed_name,"") == 0)
    return 1;

  sts = dyn->graph->ref_object_info( dyn->cycle, parsed_name, (void **)&p, &subid, size);
  if ( EVEN(sts)) return sts;

  if ( p)
    trace_data->p = p;
  first_scan = true;
  return 1;
}

int GeInvisible::disconnect( grow_tObject object)
{
  if ( p && db == graph_eDatabase_Gdh)
    gdh_UnrefObjectInfo( subid);
  p = 0;
  return 1;
}

int GeInvisible::scan( grow_tObject object)
{
  if ( !p)
    return 1;

  if ( !first_scan) {
    if ( old_value == *p) {
      // No change since last time
      return 1;
    }
  }
  else
    first_scan = false;

  if ( (!inverted && !*p) || (inverted && *p)) {
    grow_SetObjectVisibility( object, 1);
    dyn->reset_color = true;
  }
  else {
    grow_SetObjectVisibility( object, 0);
    dyn->ignore_color = true;
  }
  old_value = *p;
  return 1;
}

int GeInvisible::export_java( grow_tObject object, ofstream& fp, bool first, char *var_name)
{
  if ( first)
    fp << "      ";
  else
    fp << "      ,";
  fp << "new GeDynInvisible(" << var_name << ".dd, \"" << attribute << "\")" << endl;
  return 1;
}

void GeDigBorder::get_attributes( attr_sItem *attrinfo, int *item_count)
{
  int i = *item_count;

  strcpy( attrinfo[i].name, "DigBorder.Attribute");
  attrinfo[i].value = attribute;
  attrinfo[i].type = glow_eType_String;
  attrinfo[i++].size = sizeof( attribute);

  strcpy( attrinfo[i].name, "DigBorder.LowColor");
  attrinfo[i].value = &color;
  attrinfo[i].type = glow_eType_Color;
  attrinfo[i++].size = sizeof( color);

  *item_count = i;
}

void GeDigBorder::set_attribute( grow_tObject object, char *attr_name, int *cnt)
{
  (*cnt)--;
  if ( *cnt == 0) {
    char msg[200];

    strncpy( attribute, attr_name, sizeof( attribute));
    sprintf( msg, "DigBorder.Attribute = %s", attr_name);
    dyn->graph->message( 'I', msg);
  }
}

void GeDigBorder::replace_attribute( char *from, char *to, int *cnt, int strict)
{
  GeDyn::replace_attribute( attribute, sizeof(attribute), from, to, cnt, strict);
}

void GeDigBorder::save( ofstream& fp)
{
  fp << int(ge_eSave_DigBorder) << endl;
  fp << int(ge_eSave_DigBorder_attribute) << FSPACE << attribute << endl;
  fp << int(ge_eSave_DigBorder_color) << FSPACE << int(color) << endl;
  fp << int(ge_eSave_End) << endl;
}

void GeDigBorder::open( ifstream& fp)
{
  int		type;
  int 		end_found = 0;
  int		tmp;
  char		dummy[40];

  for (;;)
  {
    fp >> type;
    switch( type) {
      case ge_eSave_DigBorder: break;
      case ge_eSave_DigBorder_attribute:
        fp.get();
        fp.getline( attribute, sizeof(attribute));
        break;
      case ge_eSave_DigBorder_color: fp >> tmp; color = (glow_eDrawType)tmp; break;
      case ge_eSave_End: end_found = 1; break;
      default:
        cout << "GeDigBorder:open syntax error" << endl;
        fp.getline( dummy, sizeof(dummy));
    }
    if ( end_found)
      break;
  }  
}

int GeDigBorder::connect( grow_tObject object, glow_sTraceData *trace_data)
{
  int		attr_type, attr_size;
  char		parsed_name[120];
  int		sts;

  color = dyn->get_color1( object, color);
  if ( color < 0 || color >= glow_eDrawType__) {
    printf( "** Color out of range, %s\n", attribute);
    return 0;
  }

  size = 4;
  p = 0;
  db = dyn->graph->parse_attr_name( attribute, parsed_name,
				    &inverted, &attr_type, &attr_size);
  if ( strcmp( parsed_name,"") == 0)
    return 1;

  sts = dyn->graph->ref_object_info( dyn->cycle, parsed_name, (void **)&p, &subid, size);
  if ( EVEN(sts)) return sts;

  if ( p)
    trace_data->p = p;
  first_scan = true;
  return 1;
}

int GeDigBorder::disconnect( grow_tObject object)
{
  if ( p && db == graph_eDatabase_Gdh)
    gdh_UnrefObjectInfo( subid);
  p = 0;
  return 1;
}

int GeDigBorder::scan( grow_tObject object)
{
  if ( !p)
    return 1;

  if ( !first_scan) {
    if ( old_value == *p) {
      // No change since last time
      return 1;
    }
  }
  else
    first_scan = false;

  if ( (!inverted && !*p) || (inverted && *p)) {
    grow_SetObjectBorderColor( object, color);
  }
  else {
    grow_ResetObjectBorderColor( object);
  }
  old_value = *p;
  return 1;
}

int GeDigBorder::export_java( grow_tObject object, ofstream& fp, bool first, char *var_name)
{
  glow_eDrawType jcolor = dyn->get_color1( object, color);

  if ( first)
    fp << "      ";
  else
    fp << "      ,";
  fp << "new GeDynDigBorder(" << var_name << ".dd, \"" << attribute << "\"," << jcolor << ")" << endl;
  return 1;
}

void GeDigText::get_attributes( attr_sItem *attrinfo, int *item_count)
{
  int i = *item_count;

  strcpy( attrinfo[i].name, "DigText.Attribute");
  attrinfo[i].value = attribute;
  attrinfo[i].type = glow_eType_String;
  attrinfo[i++].size = sizeof( attribute);

  strcpy( attrinfo[i].name, "DigText.LowText");
  attrinfo[i].value = low_text;
  attrinfo[i].type = glow_eType_String;
  attrinfo[i++].size = sizeof( low_text);

  *item_count = i;
}

int GeDigText::get_transtab( char **tt)
{
  static char transtab[][32] = {	"SubGraph",		"SubGraph",
					"A1",			"Text",
					"Dynamic",		"",
					""};

  *tt = (char *) transtab;
  return 0;
}

void GeDigText::set_attribute( grow_tObject object, char *attr_name, int *cnt)
{
  (*cnt)--;
  if ( *cnt == 0) {
    char msg[200];

    strncpy( attribute, attr_name, sizeof( attribute));
    sprintf( msg, "DigText.Attribute = %s", attr_name);
    dyn->graph->message( 'I', msg);
  }
}

void GeDigText::replace_attribute( char *from, char *to, int *cnt, int strict)
{
  GeDyn::replace_attribute( attribute, sizeof(attribute), from, to, cnt, strict);
}

void GeDigText::save( ofstream& fp)
{
  fp << int(ge_eSave_DigText) << endl;
  fp << int(ge_eSave_DigText_attribute) << FSPACE << attribute << endl;
  fp << int(ge_eSave_DigText_low_text) << FSPACE << low_text << endl;
  fp << int(ge_eSave_End) << endl;
}

void GeDigText::open( ifstream& fp)
{
  int		type;
  int 		end_found = 0;
  char		dummy[40];

  for (;;)
  {
    fp >> type;
    switch( type) {
      case ge_eSave_DigText: break;
      case ge_eSave_DigText_attribute:
        fp.get();
        fp.getline( attribute, sizeof(attribute));
        break;
      case ge_eSave_DigText_low_text:
        fp.get();
        fp.getline( low_text, sizeof(low_text));
        break;
      case ge_eSave_End: end_found = 1; break;
      default:
        cout << "GeDigText:open syntax error" << endl;
        fp.getline( dummy, sizeof(dummy));
    }
    if ( end_found)
      break;
  }  
}

int GeDigText::connect( grow_tObject object, glow_sTraceData *trace_data)
{
  int		attr_type, attr_size;
  char		parsed_name[120];
  int		sts;

  size = 4;
  p = 0;
  db = dyn->graph->parse_attr_name( attribute, parsed_name,
				    &inverted, &attr_type, &attr_size);
  if ( strcmp( parsed_name,"") == 0)
    return 1;

  sts = dyn->graph->ref_object_info( dyn->cycle, parsed_name, (void **)&p, &subid, size);
  if ( EVEN(sts)) return sts;

  if ( p)
    trace_data->p = p;
  grow_GetAnnotation( object, 1, high_text, sizeof(high_text));
  first_scan = true;
  return 1;
}

int GeDigText::disconnect( grow_tObject object)
{
  if ( p && db == graph_eDatabase_Gdh)
    gdh_UnrefObjectInfo( subid);
  p = 0;
  return 1;
}

int GeDigText::scan( grow_tObject object)
{
  if ( !p || dyn->ignore_color)
    return 1;

  if ( !first_scan) {
    if ( old_value == *p) {
      // No change since last time
      return 1;
    }
  }
  else
    first_scan = false;

  if ( (!inverted && !*p) || (inverted && *p)) {
    grow_SetAnnotation( object, 1, low_text, strlen(low_text));
  }
  else {
    grow_SetAnnotation( object, 1, high_text, strlen(high_text));
  }
  old_value = *p;
  return 1;
}

int GeDigText::export_java( grow_tObject object, ofstream& fp, bool first, char *var_name)
{
  if ( first)
    fp << "      ";
  else
    fp << "      ,";
  fp << "new GeDynDigText(" << var_name << ".dd, \"" << attribute << "\",\"" << low_text << "\")" << endl;
  return 1;
}

void GeValue::get_attributes( attr_sItem *attrinfo, int *item_count)
{
  int i = *item_count;

  if ( instance == ge_mInstance_1) {
    strcpy( attrinfo[i].name, "Value.Attribute");
    attrinfo[i].value = attribute;
    attrinfo[i].type = glow_eType_String;
    attrinfo[i++].size = sizeof( attribute);

    strcpy( attrinfo[i].name, "Value.Format");
    attrinfo[i].value = format;
    attrinfo[i].type = glow_eType_String;
    attrinfo[i++].size = sizeof( format);

    strcpy( attrinfo[i].name, "Value.Instances");
    attrinfo[i].value = &instance_mask;
    attrinfo[i].type = ge_eAttrType_InstanceMask;
    attrinfo[i++].size = sizeof( instance_mask);    
  }
  else {
    // Get instance number
    int inst = 1;
    int m = instance;
    while( m > 1) {
      m = m >> 1;
      inst++;
    }

    sprintf( attrinfo[i].name, "Value[%d].Attribute", inst);
    attrinfo[i].value = attribute;
    attrinfo[i].type = glow_eType_String;
    attrinfo[i++].size = sizeof( attribute);

    sprintf( attrinfo[i].name, "Value[%d].Format", inst);
    attrinfo[i].value = format;
    attrinfo[i].type = glow_eType_String;
    attrinfo[i++].size = sizeof( format);
  }
  *item_count = i;
}

int GeValue::get_transtab( char **tt)
{
  static char transtab[][32] = {	"SubGraph",		"SubGraph",
					"A1",			"",
					"Dynamic",		"",
					""};

  *tt = (char *) transtab;
  return 0;
}

void GeValue::set_attribute( grow_tObject object, char *attr_name, int *cnt)
{
  (*cnt)--;
  if ( *cnt == 0) {
    char msg[200];

    if ( instance == ge_mInstance_1) {
      strncpy( attribute, attr_name, sizeof( attribute));
      sprintf( msg, "Value.Attribute = %s", attr_name);
      dyn->graph->message( 'I', msg);
    }
  }
}

void GeValue::replace_attribute( char *from, char *to, int *cnt, int strict)
{
  GeDyn::replace_attribute( attribute, sizeof(attribute), from, to, cnt, strict);
}

void GeValue::save( ofstream& fp)
{
  fp << int(ge_eSave_Value) << endl;
  fp << int(ge_eSave_Value_attribute) << FSPACE << attribute << endl;
  fp << int(ge_eSave_Value_format) << FSPACE << format << endl;
  fp << int(ge_eSave_Value_instance) << FSPACE << int(instance) << endl;
  fp << int(ge_eSave_Value_instance_mask) << FSPACE << int(instance_mask) << endl;
  fp << int(ge_eSave_End) << endl;
}

void GeValue::open( ifstream& fp)
{
  int		type;
  int 		end_found = 0;
  char		dummy[40];
  int		tmp;

  for (;;)
  {
    fp >> type;
    switch( type) {
      case ge_eSave_Value: break;
      case ge_eSave_Value_attribute:
        fp.get();
        fp.getline( attribute, sizeof(attribute));
        break;
      case ge_eSave_Value_format:
        fp.get();
        fp.getline( format, sizeof(format));
        break;
      case ge_eSave_Value_instance: fp >> tmp; instance = (ge_mInstance)tmp; break;
      case ge_eSave_Value_instance_mask: fp >> tmp; instance_mask = (ge_mInstance)tmp; break;
      case ge_eSave_End: end_found = 1; break;
      default:
        cout << "GeValue:open syntax error" << endl;
        fp.getline( dummy, sizeof(dummy));
    }
    if ( end_found)
      break;
  }
}

int GeValue::connect( grow_tObject object, glow_sTraceData *trace_data)
{
  int		attr_type, attr_size;
  char		parsed_name[120];
  int		sts;
  int		inverted;

  p = 0;
  db = dyn->graph->parse_attr_name( attribute, parsed_name,
				    &inverted, &attr_type, &attr_size);
  if ( strcmp( parsed_name,"") == 0)
    return 1;

  size = attr_size;
  switch ( db) {
  case graph_eDatabase_Gdh:
    sts = dyn->graph->ref_object_info( dyn->cycle, parsed_name, &p, &subid, attr_size);
    if ( EVEN(sts)) return sts;
    if ( attr_type != 0)
      annot_typeid = attr_type;
    else
      annot_typeid = dyn_get_typeid( format);
    break;
  case graph_eDatabase_Local:
    p = dyn->graph->localdb_ref_or_create( parsed_name, attr_type);
    annot_typeid = attr_type;
    if ( attr_type == pwr_eType_String)
      annot_size = 80;
    else
      annot_size = 4;
  case graph_eDatabase_User:
    annot_typeid = attr_type;
    if ( attr_type == pwr_eType_String)
      annot_size = 80;
    else
      annot_size = 4;
    break;
  default:
    ;
  }

  if ( p)
    trace_data->p = p;
  first_scan = true;
  return 1;
}

int GeValue::disconnect( grow_tObject object)
{
  if ( p && db == graph_eDatabase_Gdh)
    gdh_UnrefObjectInfo( subid);
  p = 0;
  return 1;
}

int GeValue::scan( grow_tObject object)
{
  char		buf[120];
  int		len;

  if ( !p)
    return 1;

  if ( !first_scan) {
    if ( memcmp( &old_value, p, size) == 0 )
      // No change since last time
      return 1;
  }
  else
    first_scan = false;


  switch( annot_typeid) {
  case pwr_eType_Float32:
    len = sprintf( buf, format, *(pwr_tFloat32 *) p);
    break;
  case pwr_eType_Int32:
  case pwr_eType_UInt32:
    len = sprintf( buf, format, *(pwr_tInt32 *) p);
    break;
  case pwr_eType_NetStatus:
    if ( db == graph_eDatabase_Gdh) {
      pwr_tTime t;
      pwr_tStatus sts;
      pwr_tBoolean old;

      gdh_GetSubscriptionOldness( subid, &old, &t, &sts);
      if ( old)
	*(pwr_tNetStatus *)p = PWR__NETTIMEOUT;
    }
    // No break
  case pwr_eType_Status:
    switch ( format[1]) {
    case '1':
      // Format %1m: Write only the text
      msg_GetText( *(pwr_tStatus *) p, buf, sizeof(buf));
      break;
    default:
      msg_GetMsg( *(pwr_tStatus *) p, buf, sizeof(buf));
    }
    len = strlen(buf);
    break;
  case pwr_eType_String:
    len = sprintf( buf, format, (char *)p);
    break;
  case pwr_eType_Objid: {
    int sts;
    char name[120];
    pwr_tObjid objid = *(pwr_tObjid *)p;
    switch ( format[1]) {
    case '1':
      // Format %1o, write path
      sts = gdh_ObjidToName ( objid, name, sizeof(name), 
			      cdh_mName_pathStrict);
      break;
    case '2':
      // Format %2o, write volume and path
      sts = gdh_ObjidToName ( objid, name, sizeof(name), 
			      cdh_mName_volumeStrict);
      break;
    default:
      sts = gdh_ObjidToName ( objid, name, sizeof(name), 
			      cdh_mName_object);
    }
    if ( EVEN(sts))
      strcpy( name, "");
    len = sprintf( buf, "%s", name);
    break;
  }
  case pwr_eType_Time: {
    int sts;
    char timstr[40];

    switch ( format[1]) {
    case '1': 
      // Format %1t, only time, no hundredth
      sts = time_AtoAscii( (pwr_tTime *) p, time_eFormat_Time, 
			   timstr, sizeof(timstr));
      timstr[8] = 0;
      break;
    case '2': 
      // Format %2t, only time, with hundredth
      sts = time_AtoAscii( (pwr_tTime *) p, time_eFormat_Time,
			   timstr, sizeof(timstr));
      break;
    case '3': 
      // Format %3t, compressed date and time, no hundredth
      sts = time_AtoAscii( (pwr_tTime *) p, time_eFormat_ComprDateAndTime,
			   timstr, sizeof(timstr));
      break;
    default:
      sts = time_AtoAscii( (pwr_tTime *) p, time_eFormat_DateAndTime, 
			   timstr, sizeof(timstr));
    }
    if ( EVEN(sts))
      strcpy( timstr, "-");
    len = sprintf( buf, "%s", timstr);
    break;
  }
  default: {
    int sts;
    sts = cdh_AttrValueToString( (pwr_eType) annot_typeid, 
				 p, buf, sizeof(buf));
    if ( EVEN(sts))
      sprintf( buf, "Invalid type");
    len = strlen(buf);
  }
  }
  int annot_num = GeDyn::instance_to_number( instance);
  if ( annot_num == 1)
    grow_SetAnnotationBrief( object, annot_num, buf, len);
  else
    grow_SetAnnotation( object, annot_num, buf, len);
  memcpy( &old_value, p, MIN(size, (int) sizeof(old_value)));
  return 1;
}

int GeValue::export_java( grow_tObject object, ofstream& fp, bool first, char *var_name)
{
  if ( first)
    fp << "      ";
  else
    fp << "      ,";
  fp << "new GeDynValue(" << var_name << ".dd, \"" << attribute << "\",\"" << format << "\")" << endl;
  return 1;
}

void GeValueInput::get_attributes( attr_sItem *attrinfo, int *item_count)
{
  int i = *item_count;

  strcpy( attrinfo[i].name, "ValueInput.MinValue");
  attrinfo[i].value = &min_value;
  attrinfo[i].type = glow_eType_Double;
  attrinfo[i++].size = sizeof( min_value);

  strcpy( attrinfo[i].name, "ValueInput.MaxValue");
  attrinfo[i].value = &max_value;
  attrinfo[i].type = glow_eType_Double;
  attrinfo[i++].size = sizeof( max_value);

  strcpy( attrinfo[i].name, "ValueInput.Clear");
  attrinfo[i].value = &clear;
  attrinfo[i].type = glow_eType_Boolean;
  attrinfo[i++].size = sizeof( clear);

  strcpy( attrinfo[i].name, "ValueInput.Popup");
  attrinfo[i].value = &popup;
  attrinfo[i].type = glow_eType_Boolean;
  attrinfo[i++].size = sizeof( popup);

  dyn->display_access = true;
  *item_count = i;
}

void GeValueInput::save( ofstream& fp)
{
  fp << int(ge_eSave_ValueInput) << endl;
  fp << int(ge_eSave_ValueInput_min_value) << FSPACE << min_value << endl;
  fp << int(ge_eSave_ValueInput_max_value) << FSPACE << max_value << endl;
  fp << int(ge_eSave_ValueInput_clear) << FSPACE << clear << endl;
  fp << int(ge_eSave_ValueInput_popup) << FSPACE << popup << endl;
  fp << int(ge_eSave_End) << endl;
}

void GeValueInput::open( ifstream& fp)
{
  int		type;
  int 		end_found = 0;
  char		dummy[40];

  for (;;)
  {
    fp >> type;
    switch( type) {
      case ge_eSave_ValueInput: break;
      case ge_eSave_ValueInput_min_value: fp >> min_value; break;
      case ge_eSave_ValueInput_max_value: fp >> max_value; break;
      case ge_eSave_ValueInput_clear: fp >> clear; break;
      case ge_eSave_ValueInput_popup: fp >> popup; break;
      case ge_eSave_End: end_found = 1; break;
      default:
        cout << "GeValueInput:open syntax error" << endl;
        fp.getline( dummy, sizeof(dummy));
    }
    if ( end_found)
      break;
  }
}

int GeValueInput::connect( grow_tObject object, glow_sTraceData *trace_data)
{
  // Get the Value element
  annot_typeid = annot_size = 0;
  value_element = 0;
  for ( GeDynElem *elem = dyn->elements; elem; elem = elem->next) {
    if ( elem->dyn_type == ge_mDynType_Value) {
      value_element = (GeValue *)elem;
      annot_typeid = value_element->annot_typeid;
      annot_size = value_element->annot_size;
      break;
    }
  }
  return 1;
}

int GeValueInput::action( grow_tObject object, glow_tEvent event)
{
  if ( !dyn->graph->is_authorized( dyn->access))
    return 1;
  if ( !value_element)
    return 1;

  switch ( event->event) {
  case glow_eEvent_MB1Down:
    grow_SetClickSensitivity( dyn->graph->grow->ctx, glow_mSensitivity_MB1Click);
    break;
  case glow_eEvent_MB1Click:
    if ( !(dyn->total_action_type & ge_mActionType_InputFocus)) {
	grow_SetObjectInputFocus( object, 1);
	dyn->graph->set_inputfocus( 1);
    }
    break;
  case glow_eEvent_AnnotationInput: {
    int		sts;

    sts = change_value( object, event->annot_input.text);
    if ( ODD(sts)) {
      grow_CloseAnnotationInput( object, 1);
      grow_SetObjectInputFocus( object, 0);
      if ( dyn->total_action_type & ge_mActionType_InputFocus)
	// Trigger a tab event, this is ugly but it works...
	event->event = glow_eEvent_Key_Tab;
    }
    break;
  }
  case glow_eEvent_InputFocusGained: {
    if ( !popup) {
      if ( !grow_AnnotationInputIsOpen( object, 1)) {
	// grow_CloseAnnotationInputAll( dyn->graph->grow->ctx);

      if ( clear)
	  grow_SetAnnotationBrief( object, 1, "", 0);
	grow_OpenAnnotationInput( object, 1);
      }
    }
    else {
      if ( dyn->graph->change_value_cb) {
	char str[80] = "";

	if ( !clear)
	  grow_GetAnnotation( object, 1, str, sizeof(str));
	(dyn->graph->change_value_cb)( dyn->graph->parent_ctx, object, str);
      }
    }
    break;
  }
  case glow_eEvent_InputFocusLost: {
    grow_CloseAnnotationInput( object, 1);
    value_element->first_scan = 1;
    break;
  }
  default: ;    
  }
  return 1;
}

int GeValueInput::change_value( grow_tObject object, char *text)
{
  char		buf[200];
  int		sts;
  char		parsed_name[120];
  int		inverted;
  int		attr_type, attr_size;
  graph_eDatabase db;

  db = dyn->graph->parse_attr_name( value_element->attribute, parsed_name, &inverted, 
			       &attr_type, &attr_size);
  if ( !annot_size ) {
    pwr_sAttrRef 	ar;
    pwr_tTypeId 	a_type_id;
    unsigned int 	a_size;
    unsigned int 	a_offset;
    unsigned int 	a_dim;

    sts = gdh_NameToAttrref ( pwr_cNObjid, parsed_name, &ar);
    if ( EVEN(sts)) return sts;
    sts = gdh_GetAttributeCharAttrref( &ar, &a_type_id, &a_size,
		&a_offset, &a_dim);
    if ( EVEN(sts)) return sts;
    annot_typeid = a_type_id;
    annot_size = a_size;
  }
  sts = graph_attr_string_to_value( annot_typeid, text,
	(void *)&buf, sizeof( buf), sizeof(buf));
  if ( EVEN(sts)) {
    if ( dyn->graph->message_dialog_cb)
      (dyn->graph->message_dialog_cb)( dyn->graph->parent_ctx, "Input syntax error");
    return sts;
  }

  if ( !(max_value == 0 && min_value == 0)) {
    // Max value is supplied
    int 	max_exceeded = 0;

    switch ( annot_typeid) {
    case pwr_eType_Float32:
      if ( double( *(pwr_tFloat32 *) buf) > max_value) max_exceeded = 1;
      break;
    case pwr_eType_Float64:
      if ( double( *(pwr_tFloat64 *) buf) > max_value) max_exceeded = 1;
      break;
    case pwr_eType_Int32:
      if ( double( *(pwr_tInt32 *) buf) > max_value) max_exceeded = 1;
      break;
    case pwr_eType_UInt32:
      if ( double( *(pwr_tUInt32 *) buf) > max_value) max_exceeded = 1;
      break;
    case pwr_eType_Int16:
      if ( double( *(pwr_tInt16 *) buf) > max_value) max_exceeded = 1;
      break;
    case pwr_eType_UInt16:
      if ( double( *(pwr_tUInt16 *) buf) > max_value) max_exceeded = 1;
      break;
    case pwr_eType_Int8:
      if ( double( *(pwr_tInt8 *) buf) > max_value) max_exceeded = 1;
      break;
    case pwr_eType_UInt8:
      if ( double( *(pwr_tUInt8 *) buf) > max_value) max_exceeded = 1;
      break;
    }
    if ( max_exceeded) {
      if ( dyn->graph->message_dialog_cb)
	(dyn->graph->message_dialog_cb)( dyn->graph->parent_ctx, "Maxvalue exceeded");
      return 0;
    }

    int 	min_exceeded = 0;
    switch ( annot_typeid) {
    case pwr_eType_Float32:
      if ( double( *(pwr_tFloat32 *) buf) < min_value) min_exceeded = 1;
      break;
    case pwr_eType_Float64:
      if ( double( *(pwr_tFloat64 *) buf) < min_value) min_exceeded = 1;
      break;
    case pwr_eType_Int32:
      if ( double( *(pwr_tInt32 *) buf) < min_value) min_exceeded = 1;
      break;
    case pwr_eType_UInt32:
      if ( double( *(pwr_tUInt32 *) buf) < min_value) min_exceeded = 1;
      break;
    case pwr_eType_Int16:
      if ( double( *(pwr_tInt16 *) buf) < min_value) min_exceeded = 1;
      break;
    case pwr_eType_UInt16:
      if ( double( *(pwr_tUInt16 *) buf) < min_value) min_exceeded = 1;
      break;
    case pwr_eType_Int8:
      if ( double( *(pwr_tInt8 *) buf) < min_value) min_exceeded = 1;
      break;
    case pwr_eType_UInt8:
      if ( double( *(pwr_tUInt8 *) buf) < min_value) min_exceeded = 1;
      break;
    }
    if ( min_exceeded) {
      if ( dyn->graph->message_dialog_cb)
        (dyn->graph->message_dialog_cb)( dyn->graph->parent_ctx, "Value below minvalue");
      return 0;
    }
  }

  if ( db == graph_eDatabase_Local)
    sts = dyn->graph->localdb_set_value( parsed_name, &buf, annot_size);
  else
    sts = gdh_SetObjectInfo( parsed_name, &buf, annot_size);
  if ( EVEN(sts)) printf("AnnotationInput error: %s\n", value_element->attribute);
  return 1;
}

int GeValueInput::export_java( grow_tObject object, ofstream& fp, bool first, char *var_name)
{
  int class_dyn_type, class_action_type;

  // Check that nodeclass is a valueinput, otherwise this will not be a GeTextField
  grow_GetObjectClassDynType( object, &class_dyn_type, &class_action_type);
  if ( !(class_action_type & ge_mActionType_ValueInput))
    return 1;

  if ( first)
    fp << "      ";
  else
    fp << "      ,";
  fp << "new GeDynValueInput(" << var_name << ".dd, " << min_value << "," << max_value 
     << ")" << endl;
  return 1;
}

void GeAnalogColor::get_attributes( attr_sItem *attrinfo, int *item_count)
{
  int i = *item_count;

  if ( dyn->total_dyn_type & ge_mDynType_Tone) {
    if ( instance == ge_mInstance_1) {
      strcpy( attrinfo[i].name, "AnalogTone.Limit");
      attrinfo[i].value = &limit;
      attrinfo[i].type = glow_eType_Double;
      attrinfo[i++].size = sizeof( limit);

      strcpy( attrinfo[i].name, "AnalogTone.Tone");
      attrinfo[i].value = &color;
      attrinfo[i].type = glow_eType_ToneOrColor;
      attrinfo[i++].size = sizeof( limit_type);

      strcpy( attrinfo[i].name, "AnalogTone.LimitType");
      attrinfo[i].value = &limit_type;
      attrinfo[i].type = ge_eAttrType_LimitType;
      attrinfo[i++].size = sizeof( limit_type);

      strcpy( attrinfo[i].name, "AnalogTone.Attribute");
      attrinfo[i].value = attribute;
      attrinfo[i].type = glow_eType_String;
      attrinfo[i++].size = sizeof( attribute);

      strcpy( attrinfo[i].name, "AnalogTone.Instances");
      attrinfo[i].value = &instance_mask;
      attrinfo[i].type = ge_eAttrType_InstanceMask;
      attrinfo[i++].size = sizeof( instance_mask);    
    }
    else {
      // Get instance number
      int inst = 1;
      int m = instance;
      while( m > 1) {
	m = m >> 1;
	inst++;
      }

      sprintf( attrinfo[i].name, "AnalogTone%d.Limit", inst);
      attrinfo[i].value = &limit;
      attrinfo[i].type = glow_eType_Double;
      attrinfo[i++].size = sizeof( limit);

      sprintf( attrinfo[i].name, "AnalogTone%d.Tone", inst);
      attrinfo[i].value = &color;
      attrinfo[i].type = glow_eType_ToneOrColor;
      attrinfo[i++].size = sizeof( limit_type);

      sprintf( attrinfo[i].name, "AnalogTone%d.LimitType", inst);
      attrinfo[i].value = &limit_type;
      attrinfo[i].type = ge_eAttrType_LimitType;
      attrinfo[i++].size = sizeof( limit_type);
    }
  }
  else {
    if ( instance == ge_mInstance_1) {
      strcpy( attrinfo[i].name, "AnalogColor.Limit");
      attrinfo[i].value = &limit;
      attrinfo[i].type = glow_eType_Double;
      attrinfo[i++].size = sizeof( limit);

      strcpy( attrinfo[i].name, "AnalogColor.Color");
      attrinfo[i].value = &color;
      attrinfo[i].type = glow_eType_Color;
      attrinfo[i++].size = sizeof( limit_type);

      strcpy( attrinfo[i].name, "AnalogColor.LimitType");
      attrinfo[i].value = &limit_type;
      attrinfo[i].type = ge_eAttrType_LimitType;
      attrinfo[i++].size = sizeof( limit_type);

      strcpy( attrinfo[i].name, "AnalogColor.Attribute");
      attrinfo[i].value = attribute;
      attrinfo[i].type = glow_eType_String;
      attrinfo[i++].size = sizeof( attribute);

      strcpy( attrinfo[i].name, "AnalogColor.Instances");
      attrinfo[i].value = &instance_mask;
      attrinfo[i].type = ge_eAttrType_InstanceMask;
      attrinfo[i++].size = sizeof( instance_mask);    
    }
    else {
      // Get instance number
      int inst = 1;
      int m = instance;
      while( m > 1) {
	m = m >> 1;
	inst++;
      }

      sprintf( attrinfo[i].name, "AnalogColor%d.Limit", inst);
      attrinfo[i].value = &limit;
      attrinfo[i].type = glow_eType_Double;
      attrinfo[i++].size = sizeof( limit);

      sprintf( attrinfo[i].name, "AnalogColor%d.Tone", inst);
      attrinfo[i].value = &color;
      attrinfo[i].type = glow_eType_Color;
      attrinfo[i++].size = sizeof( limit_type);

      sprintf( attrinfo[i].name, "AnalogColor%d.LimitType", inst);
      attrinfo[i].value = &limit_type;
      attrinfo[i].type = ge_eAttrType_LimitType;
      attrinfo[i++].size = sizeof( limit_type);
    }
  }
  *item_count = i;
}

void GeAnalogColor::set_attribute( grow_tObject object, char *attr_name, int *cnt)
{
  (*cnt)--;
  if ( *cnt == 0) {
    char msg[200];

    strncpy( attribute, attr_name, sizeof( attribute));
    if ( instance == ge_mInstance_1) {
      if ( dyn->total_dyn_type & ge_mDynType_Tone)
	sprintf( msg, "AnalogTone.Attribute = %s", attr_name);
      else
	sprintf( msg, "AnalogColor.Attribute = %s", attr_name);
    }
    else {
      if ( dyn->total_dyn_type & ge_mDynType_Tone)
	sprintf( msg, "AnalogTone%d.Attribute = %s", GeDyn::instance_to_number( instance),
	       attr_name);
      else
	sprintf( msg, "AnalogColor%d.Attribute = %s", GeDyn::instance_to_number( instance),
	       attr_name);
    }
    dyn->graph->message( 'I', msg);
  }
}

void GeAnalogColor::replace_attribute( char *from, char *to, int *cnt, int strict)
{
  GeDyn::replace_attribute( attribute, sizeof(attribute), from, to, cnt, strict);
}

int GeAnalogColor::set_color( grow_tObject object, glow_eDrawType color)
{
  char msg[200];

  if ( dyn->total_dyn_type & ge_mDynType_Tone) {
    this->color = glow_eDrawType( color / 30);
    if ( instance == ge_mInstance_1)
      sprintf( msg, "AnalogTone.Tone = %s", grow_ColorToneToName( this->color));
    else
      sprintf( msg, "AnalogTone%d.Tone = %s", GeDyn::instance_to_number( instance),
	       grow_ColorToneToName( this->color));
  }
  else {
    this->color = color;
    if ( instance == ge_mInstance_1)
      sprintf( msg, "AnalogColor.Color = %s", grow_ColorToName( this->color));
    else
      sprintf( msg, "AnalogColor%d.Color = %s", GeDyn::instance_to_number( instance),
	       grow_ColorToName( this->color));
  }
  dyn->graph->message( 'I', msg);
  return 1;
}

void GeAnalogColor::save( ofstream& fp)
{
  fp << int(ge_eSave_AnalogColor) << endl;
  fp << int(ge_eSave_AnalogColor_attribute) << FSPACE << attribute << endl;
  fp << int(ge_eSave_AnalogColor_limit) << FSPACE << limit << endl;
  fp << int(ge_eSave_AnalogColor_limit_type) << FSPACE << (int)limit_type << endl;
  fp << int(ge_eSave_AnalogColor_color) << FSPACE << (int)color << endl;
  fp << int(ge_eSave_AnalogColor_instance) << FSPACE << int(instance) << endl;
  fp << int(ge_eSave_AnalogColor_instance_mask) << FSPACE << int(instance_mask) << endl;
  fp << int(ge_eSave_End) << endl;
}

void GeAnalogColor::open( ifstream& fp)
{
  int		type;
  int 		end_found = 0;
  char		dummy[40];
  int		tmp;

  for (;;)
  {
    fp >> type;
    switch( type) {
      case ge_eSave_AnalogColor: break;
      case ge_eSave_AnalogColor_attribute:
        fp.get();
        fp.getline( attribute, sizeof(attribute));
        break;
      case ge_eSave_AnalogColor_limit: fp >> limit; break;
      case ge_eSave_AnalogColor_limit_type: fp >> tmp; limit_type = (ge_eLimitType)tmp; break;
      case ge_eSave_AnalogColor_color: fp >> tmp; color = (glow_eDrawType)tmp; break;
      case ge_eSave_AnalogColor_instance: fp >> tmp; instance = (ge_mInstance)tmp; break;
      case ge_eSave_AnalogColor_instance_mask: fp >> tmp; instance_mask = (ge_mInstance)tmp; break;
      case ge_eSave_End: end_found = 1; break;
      default:
        cout << "GeAnalogColor:open syntax error" << endl;
        fp.getline( dummy, sizeof(dummy));
    }
    if ( end_found)
      break;
  }
}

int GeAnalogColor::connect( grow_tObject object, glow_sTraceData *trace_data)
{
  char		parsed_name[120];
  int		inverted;
  int		sts;
  bool		found = false;
  GeAnalogColor	*e;
  GeDynElem	*elem;

  // Get attribute for instance 1
  if ( instance == ge_mInstance_1)
    e = this;
  else {
    for ( elem = dyn->elements; elem; elem = elem->next) {
      if ( elem->dyn_type == ge_mDynType_AnalogColor &&
	   elem->instance == ge_mInstance_1) {
	found = true;
	break;
      }
    }
    if ( !found)
      return 1;
    e =  (GeAnalogColor *)elem;
  }

  if ( e->p == 0) {
    e->size = 4;
    db = dyn->graph->parse_attr_name( e->attribute, parsed_name,
				    &inverted, &e->type, &e->size);
    if ( strcmp( parsed_name,"") == 0)
      return 1;

    switch ( e->type) {
    case pwr_eType_Float32:
    case pwr_eType_Int32:
      break;
    default:
      return 1;
    }

    sts = dyn->graph->ref_object_info( dyn->cycle, parsed_name, (void **)&e->p,
				       &e->subid, e->size);
    if ( EVEN(sts)) return sts;

    if ( e->p)
      trace_data->p = e->p;
  }
  type = e->type;
  size = e->size;
  p = e->p;

  first_scan = true;
  return 1;
}

int GeAnalogColor::disconnect( grow_tObject object)
{
  if ( instance == ge_mInstance_1 && p && db == graph_eDatabase_Gdh)
    gdh_UnrefObjectInfo( subid);
  p = 0;
  return 1;
}

int GeAnalogColor::scan( grow_tObject object)
{
  if ( !p || dyn->ignore_color)
    return 1;
  
  if ( !first_scan) {
    switch ( type) {
    case pwr_eType_Float32:
      if ( !dyn->reset_color && fabs( old_value - *p) < FLT_EPSILON) {
	// No change since last time
	if ( old_state)
	  dyn->ignore_color = true;
	return 1;
      }
      break;
    default:
      if ( !dyn->reset_color && memcmp( &old_value, p, size) == 0) {
	// No change since last time
	if ( old_state)
	  dyn->ignore_color = true;
	return 1;
      }
    }
  }
  else
    first_scan = false;

  bool state;
  bool set_color = false;
  bool reset_color = false;

  switch ( limit_type) {
  case ge_eLimitType_Gt: {
    switch ( type) {
    case pwr_eType_Float32:
      state = *p > limit;
      break;
    case pwr_eType_Int32:
      state = *(pwr_tInt32 *)p > limit;
      break;
    default: ;
    }
    break;
  }
  case ge_eLimitType_Ge: {
    switch ( type) {
    case pwr_eType_Float32:
      state = *p >= (limit - FLT_EPSILON);
      break;
    case pwr_eType_Int32:
      state = *(pwr_tInt32 *)p >= limit;
      break;
    default: ;
    }
    break;
  }
  case ge_eLimitType_Lt: {
    switch ( type) {
    case pwr_eType_Float32:
      state = *p < limit;
      break;
    case pwr_eType_Int32:
      state = *(pwr_tInt32 *)p < limit;
      break;
    default: ;
    }
    break;
  }
  case ge_eLimitType_Le: {
    switch ( type) {
    case pwr_eType_Float32:
      state = *p <= (limit + FLT_EPSILON);
      break;
    case pwr_eType_Int32:
      state = *(pwr_tInt32 *)p <= limit;
      break;
    default: ;
    }
    break;
  }
  case ge_eLimitType_Eq: {
    switch ( type) {
    case pwr_eType_Float32:
      state = fabs(*p - limit) < FLT_EPSILON;
      break;
    case pwr_eType_Int32:
      state = *(pwr_tInt32 *)p == limit;
      break;
    default: ;
    }
    break;
  }
  }

  if ( state != old_state || dyn->reset_color || first_scan) {
    if ( state) {
      set_color = true;
      dyn->ignore_color = true;
    }
    else {
      reset_color = true;
      dyn->reset_color = true;
    }
    old_state = state;
  }
  else if ( state)
    dyn->ignore_color = true;

  memcpy( &old_value, p, size);

  if ( !set_color && !reset_color) {
    return 1;
  }
  if ( dyn->total_dyn_type & ge_mDynType_Tone) {
    if ( set_color) {
      if ( color >= (glow_eDrawType) glow_eDrawTone__)
	grow_SetObjectFillColor( object, color);
      else
	grow_SetObjectColorTone( object, (glow_eDrawTone) color);
      dyn->ignore_color = true;
    }
    else {
      if ( color >= (glow_eDrawType) glow_eDrawTone__)
	grow_ResetObjectFillColor( object);
      grow_ResetObjectColorTone( object);
      dyn->reset_color = true;
    }
  }
  else {
    if ( set_color) {
      grow_SetObjectFillColor( object, color);
      dyn->ignore_color = true;
    }
    else {
      grow_ResetObjectFillColor( object);
      dyn->reset_color = true;
    }
  }

  return 1;
}

int GeAnalogColor::export_java( grow_tObject object, ofstream& fp, bool first, char *var_name)
{
  if ( first)
    fp << "      ";
  else
    fp << "      ,";
  fp << "new GeDynAnalogColor(" << var_name << ".dd, \"" << attribute << "\"," 
     << limit << "," << limit_type << "," << color << ")" << endl;
  return 1;
}

void GeRotate::get_attributes( attr_sItem *attrinfo, int *item_count)
{
  int i = *item_count;

  strcpy( attrinfo[i].name, "Rotate.Attribute");
  attrinfo[i].value = attribute;
  attrinfo[i].type = glow_eType_String;
  attrinfo[i++].size = sizeof( attribute);

  strcpy( attrinfo[i].name, "Rotate.x0");
  attrinfo[i].value = &x0;
  attrinfo[i].type = glow_eType_Double;
  attrinfo[i++].size = sizeof( x0);

  strcpy( attrinfo[i].name, "Rotate.y0");
  attrinfo[i].value = &y0;
  attrinfo[i].type = glow_eType_Double;
  attrinfo[i++].size = sizeof( y0);

  strcpy( attrinfo[i].name, "Rotate.Factor");
  attrinfo[i].value = &factor;
  attrinfo[i].type = glow_eType_Double;
  attrinfo[i++].size = sizeof( factor);

  *item_count = i;
}

void GeRotate::set_attribute( grow_tObject object, char *attr_name, int *cnt)
{
  (*cnt)--;
  if ( *cnt == 0) {
    char msg[200];

    strncpy( attribute, attr_name, sizeof( attribute));
    sprintf( msg, "Rotate.Attribute = %s", attr_name);
    dyn->graph->message( 'I', msg);
  }
}

void GeRotate::replace_attribute( char *from, char *to, int *cnt, int strict)
{
  GeDyn::replace_attribute( attribute, sizeof(attribute), from, to, cnt, strict);
}

void GeRotate::save( ofstream& fp)
{
  fp << int(ge_eSave_Rotate) << endl;
  fp << int(ge_eSave_Rotate_attribute) << FSPACE << attribute << endl;
  fp << int(ge_eSave_Rotate_x0) << FSPACE << x0 << endl;
  fp << int(ge_eSave_Rotate_y0) << FSPACE << y0 << endl;
  fp << int(ge_eSave_Rotate_factor) << FSPACE << factor << endl;
  fp << int(ge_eSave_End) << endl;
}

void GeRotate::open( ifstream& fp)
{
  int		type;
  int 		end_found = 0;
  char		dummy[40];

  for (;;)
  {
    fp >> type;
    switch( type) {
      case ge_eSave_Rotate: break;
      case ge_eSave_Rotate_attribute:
        fp.get();
        fp.getline( attribute, sizeof(attribute));
        break;
      case ge_eSave_Rotate_x0: fp >> x0; break;
      case ge_eSave_Rotate_y0: fp >> y0; break;
      case ge_eSave_Rotate_factor: fp >> factor; break;
      case ge_eSave_End: end_found = 1; break;
      default:
        cout << "GeRotate:open syntax error" << endl;
        fp.getline( dummy, sizeof(dummy));
    }
    if ( end_found)
      break;
  }
}

int GeRotate::connect( grow_tObject object, glow_sTraceData *trace_data)
{
  int		attr_type, attr_size;
  char		parsed_name[120];
  int		inverted;
  int		sts;

  size = 4;
  p = 0;
  db = dyn->graph->parse_attr_name( attribute, parsed_name,
				    &inverted, &attr_type, &attr_size);
  if ( strcmp( parsed_name,"") == 0)
    return 1;

  sts = dyn->graph->ref_object_info( dyn->cycle, parsed_name, (void **)&p, &subid, size);
  if ( EVEN(sts)) return sts;

  if ( p)
    trace_data->p = p;
  first_scan = true;
  if ( !grow_TransformIsStored( object))
    grow_StoreTransform( object);
  if ( x0 != 0 || y0 != 0)
    rotation_point = glow_eRotationPoint_FixPoint;
  else
    rotation_point = glow_eRotationPoint_Zero;

  return 1;
}

int GeRotate::disconnect( grow_tObject object)
{
  if ( p && db == graph_eDatabase_Gdh)
    gdh_UnrefObjectInfo( subid);
  p = 0;
  return 1;
}

int GeRotate::scan( grow_tObject object)
{
  
  if ( !first_scan) {
    if ( fabs( old_value - *p) < FLT_EPSILON)
      // No change since last time
      return 1;
  }
  else
    first_scan = false;

  double value = *p * factor;

  grow_SetObjectRotation( object, value, x0, y0, rotation_point);
  old_value = *p;
  return 1;
}

int GeRotate::export_java( grow_tObject object, ofstream& fp, bool first, char *var_name)
{
  double x1, x2, y1, y2;
  double rotation_x, rotation_y;

  if ( x0 != 0  || y0 != 0) { 
    rotation_x = x0;
    rotation_y = y0;

    grow_ToPixel( dyn->graph->grow->ctx, rotation_x, rotation_y, 
		  &rotation_x, &rotation_y);
    grow_MeasureJavaBean( dyn->graph->grow->ctx, &x2, &x1, &y2, &y1);

    rotation_x -= x1 - glow_cJBean_Offset;
    rotation_y -= y1 - glow_cJBean_Offset;
  }
  else {
    // Zero point for nodeclass is rotation point 
    grow_GetNodeClassOrigo( object, &rotation_x, &rotation_y);
    grow_MeasureNode( object, &x1, &y1, &x2, &y2);
    rotation_x += x1;
    rotation_y += y1;

    grow_ToPixel( dyn->graph->grow->ctx, rotation_x, rotation_y, 
		  &rotation_x, &rotation_y);

    grow_MeasureJavaBean( dyn->graph->grow->ctx, &x2, &x1, &y2, &y1);

    rotation_x -= x1 - glow_cJBean_Offset;
    rotation_y -= y1 - glow_cJBean_Offset;
  }


  if ( first)
    fp << "      ";
  else
    fp << "      ,";
  fp << "new GeDynRotate(" << var_name << ".dd, \"" << attribute << "\"," << rotation_x 
     << "," << rotation_y << "," << factor << ")" << endl;
  return 1;
}

void GeMove::get_attributes( attr_sItem *attrinfo, int *item_count)
{
  int i = *item_count;

  strcpy( attrinfo[i].name, "Move.XAttribute");
  attrinfo[i].value = move_x_attribute;
  attrinfo[i].type = glow_eType_String;
  attrinfo[i++].size = sizeof( move_x_attribute);

  strcpy( attrinfo[i].name, "Move.YAttribute");
  attrinfo[i].value = move_y_attribute;
  attrinfo[i].type = glow_eType_String;
  attrinfo[i++].size = sizeof( move_y_attribute);

  strcpy( attrinfo[i].name, "Move.XOffset");
  attrinfo[i].value = &x_offset;
  attrinfo[i].type = glow_eType_Double;
  attrinfo[i++].size = sizeof( x_offset);

  strcpy( attrinfo[i].name, "Move.YOffset");
  attrinfo[i].value = &y_offset;
  attrinfo[i].type = glow_eType_Double;
  attrinfo[i++].size = sizeof( y_offset);

  strcpy( attrinfo[i].name, "Move.Factor");
  attrinfo[i].value = &factor;
  attrinfo[i].type = glow_eType_Double;
  attrinfo[i++].size = sizeof( factor);

  strcpy( attrinfo[i].name, "Move.ScaleXAttribute");
  attrinfo[i].value = scale_x_attribute;
  attrinfo[i].type = glow_eType_String;
  attrinfo[i++].size = sizeof( scale_x_attribute);

  strcpy( attrinfo[i].name, "Move.ScaleYAttribute");
  attrinfo[i].value = scale_y_attribute;
  attrinfo[i].type = glow_eType_String;
  attrinfo[i++].size = sizeof( scale_y_attribute);

  strcpy( attrinfo[i].name, "Move.ScaleFactor");
  attrinfo[i].value = &scale_factor;
  attrinfo[i].type = glow_eType_Double;
  attrinfo[i++].size = sizeof( scale_factor);

  strcpy( attrinfo[i].name, "Move.ScaleType");
  attrinfo[i].value = &scale_type;
  attrinfo[i].type = ge_eAttrType_ScaleType;
  attrinfo[i++].size = sizeof( scale_type);

  *item_count = i;
}

void GeMove::set_attribute( grow_tObject object, char *attr_name, int *cnt)
{
  (*cnt)--;
  if ( *cnt == 0) {
    char msg[200];

    strncpy( move_x_attribute, attr_name, sizeof( move_x_attribute));
    sprintf( msg, "Move.XAttribute = %s", attr_name);
    dyn->graph->message( 'I', msg);
  }
  else if ( *cnt == 1) {
    char msg[200];

    strncpy( move_y_attribute, attr_name, sizeof( move_y_attribute));
    sprintf( msg, "Move.YAttribute = %s", attr_name);
    dyn->graph->message( 'I', msg);
    (*cnt)--;
  }
}

void GeMove::replace_attribute( char *from, char *to, int *cnt, int strict)
{
  GeDyn::replace_attribute( move_x_attribute, sizeof(move_x_attribute), from, to, cnt, strict);
  GeDyn::replace_attribute( move_y_attribute, sizeof(move_y_attribute), from, to, cnt, strict);
  GeDyn::replace_attribute( scale_x_attribute, sizeof(scale_x_attribute), from, to, cnt, strict);
  GeDyn::replace_attribute( scale_y_attribute, sizeof(scale_y_attribute), from, to, cnt, strict);
}

void GeMove::save( ofstream& fp)
{
  fp << int(ge_eSave_Move) << endl;
  fp << int(ge_eSave_Move_move_x_attribute) << FSPACE << move_x_attribute << endl;
  fp << int(ge_eSave_Move_move_y_attribute) << FSPACE << move_y_attribute << endl;
  fp << int(ge_eSave_Move_scale_x_attribute) << FSPACE << scale_x_attribute << endl;
  fp << int(ge_eSave_Move_scale_y_attribute) << FSPACE << scale_y_attribute << endl;
  fp << int(ge_eSave_Move_x_offset) << FSPACE << x_offset << endl;
  fp << int(ge_eSave_Move_y_offset) << FSPACE << y_offset << endl;
  fp << int(ge_eSave_Move_factor) << FSPACE << factor << endl;
  fp << int(ge_eSave_Move_scale_factor) << FSPACE << scale_factor << endl;
  fp << int(ge_eSave_Move_scale_type) << FSPACE << scale_type << endl;
  fp << int(ge_eSave_End) << endl;
}

void GeMove::open( ifstream& fp)
{
  int		type;
  int 		end_found = 0;
  char		dummy[40];
  int		tmp;

  for (;;)
  {
    fp >> type;
    switch( type) {
      case ge_eSave_Move: break;
      case ge_eSave_Move_move_x_attribute:
        fp.get();
        fp.getline( move_x_attribute, sizeof(move_x_attribute));
        break;
      case ge_eSave_Move_move_y_attribute:
        fp.get();
        fp.getline( move_y_attribute, sizeof(move_y_attribute));
        break;
      case ge_eSave_Move_scale_x_attribute:
        fp.get();
        fp.getline( scale_x_attribute, sizeof(scale_x_attribute));
        break;
      case ge_eSave_Move_scale_y_attribute:
        fp.get();
        fp.getline( scale_y_attribute, sizeof(scale_y_attribute));
        break;
      case ge_eSave_Move_x_offset: fp >> x_offset; break;
      case ge_eSave_Move_y_offset: fp >> y_offset; break;
      case ge_eSave_Move_factor: fp >> factor; break;
      case ge_eSave_Move_scale_factor: fp >> scale_factor; break;
      case ge_eSave_Move_scale_type: fp >> tmp; scale_type = (glow_eScaleType)tmp; break;
      case ge_eSave_End: end_found = 1; break;
      default:
        cout << "GeMove:open syntax error" << endl;
        fp.getline( dummy, sizeof(dummy));
    }
    if ( end_found)
      break;
  }
}

int GeMove::connect( grow_tObject object, glow_sTraceData *trace_data)
{
  char		parsed_name[120];
  int		inverted;
  int		sts;
  double	ur_x, ur_y;

  move_x_size = 4;
  move_x_type = pwr_eType_Float32;
  move_x_p = 0;
  move_x_db = dyn->graph->parse_attr_name( move_x_attribute, parsed_name,
				    &inverted, &move_x_type, &move_x_size);
  if ( strcmp( parsed_name,"") != 0) {
    sts = dyn->graph->ref_object_info( dyn->cycle, parsed_name, (void **)&move_x_p, 
				       &move_x_subid, move_x_size);
    if ( EVEN(sts)) return sts;
  }
  move_y_size = 4;
  move_y_type = pwr_eType_Float32;
  move_y_p = 0;
  move_y_db = dyn->graph->parse_attr_name( move_y_attribute, parsed_name,
				    &inverted, &move_y_type, &move_y_size);
  if ( strcmp( parsed_name,"") != 0) {
    sts = dyn->graph->ref_object_info( dyn->cycle, parsed_name, (void **)&move_y_p, 
				       &move_y_subid, move_y_size);
    if ( EVEN(sts)) return sts;
  }
  scale_x_size = 4;
  scale_x_type = pwr_eType_Float32;
  scale_x_p = 0;
  scale_x_db = dyn->graph->parse_attr_name( scale_x_attribute, parsed_name,
				    &inverted, &scale_x_type, &scale_x_size);
  if ( strcmp( parsed_name,"") != 0) {
    sts = dyn->graph->ref_object_info( dyn->cycle, parsed_name, (void **)&scale_x_p, 
				       &scale_x_subid, scale_x_size);
    if ( EVEN(sts)) return sts;
  }
  scale_y_size = 4;
  scale_y_type = pwr_eType_Float32;
  scale_y_p = 0;
  scale_y_db = dyn->graph->parse_attr_name( scale_y_attribute, parsed_name,
				    &inverted, &scale_y_type, &scale_y_size);
  if ( strcmp( parsed_name,"") != 0) {
    sts = dyn->graph->ref_object_info( dyn->cycle, parsed_name, (void **)&scale_y_p, 
				       &scale_y_subid, scale_y_size);
    if ( EVEN(sts)) return sts;
  }
  if ( move_x_p)
    trace_data->p = move_x_p;
  else if ( move_y_p)
    trace_data->p = move_y_p;
  else if ( scale_x_p)
    trace_data->p = scale_x_p;
  else if ( scale_y_p)
    trace_data->p = scale_y_p;

  first_scan = true;
  if ( !grow_TransformIsStored( object)) {
    grow_StoreTransform( object);
    grow_MeasureNode( object, &x_orig, &y_orig, &ur_x, &ur_y);
    width_orig = ur_x - x_orig;
    height_orig = ur_y - y_orig;
  }
  return 1;
}

int GeMove::disconnect( grow_tObject object)
{
  if ( move_x_p && move_x_db == graph_eDatabase_Gdh)
    gdh_UnrefObjectInfo( move_x_subid);
  if ( move_y_p && move_y_db == graph_eDatabase_Gdh)
    gdh_UnrefObjectInfo( move_y_subid);
  if ( scale_x_p && scale_x_db == graph_eDatabase_Gdh)
    gdh_UnrefObjectInfo( scale_x_subid);
  if ( scale_y_p && scale_y_db == graph_eDatabase_Gdh)
    gdh_UnrefObjectInfo( scale_y_subid);
  move_x_p = move_y_p = scale_x_p = scale_y_p = 0;
  return 1;
}

int GeMove::scan( grow_tObject object)
{
  bool update = false;
  
  if ( !first_scan) {
    if ( move_x_p && memcmp( &move_x_old_value, move_x_p, move_x_size) != 0)
      update = true;
    else if ( move_y_p && memcmp( &move_y_old_value, move_y_p, move_y_size) != 0)
      update = true;
    else if ( scale_x_p && memcmp( &scale_x_old_value, scale_x_p, scale_x_size) != 0)
      update = true;
    else if ( scale_y_p && memcmp( &scale_y_old_value, scale_y_p, scale_y_size) != 0)
      update = true;
    if ( !update)
      return 1;
  }
  else
    first_scan = false;

  double move_x, move_y, scale_x, scale_y;

  if ( scale_x_p || scale_y_p) {
    if ( scale_x_p) {
      switch ( scale_x_type) {
      case pwr_eType_Float32: scale_x = *scale_x_p * scale_factor; break;
      case pwr_eType_Float64: scale_x = *(pwr_tFloat64 *)scale_x_p * scale_factor; break;
      case pwr_eType_Int32:   scale_x = *(pwr_tInt32 *)scale_x_p * scale_factor; break;
      case pwr_eType_UInt32:  scale_x = *(pwr_tUInt32 *)scale_x_p * scale_factor; break;
      default: scale_x = 1;
      }
    }
    else
      scale_x = 1;

    if ( scale_y_p) {
      switch ( scale_y_type) {
      case pwr_eType_Float32: scale_y = *scale_y_p * scale_factor; break;
      case pwr_eType_Float64: scale_y = *(pwr_tFloat64 *)scale_y_p * scale_factor; break;
      case pwr_eType_Int32:   scale_y = *(pwr_tInt32 *)scale_y_p * scale_factor; break;
      case pwr_eType_UInt32:  scale_y = *(pwr_tUInt32 *)scale_y_p * scale_factor; break;
      default: scale_y = 1;
      }
    }
    else
      scale_y = 1;

      
    if ( !(move_x_p || move_y_p))
      grow_SetObjectScale( object, scale_x, scale_y, 0, 0,
			   scale_type);
    if ( scale_x_p)
      memcpy( &scale_x_old_value, scale_x_p, scale_x_size);
    if ( scale_y_p)
      memcpy( &scale_y_old_value, scale_y_p, scale_y_size);


    if ( move_x_p || move_y_p) {
      if ( move_x_p) {
	double scale_offs = 0;
	// Adjust position for different scaletypes
	switch ( scale_type) {
	case glow_eScaleType_LowerRight:
	case glow_eScaleType_UpperRight:
	  scale_offs = width_orig * ( 1 - scale_x);
	  break;
	case glow_eScaleType_Center:
	  scale_offs = width_orig * ( 1 - scale_x) / 2;
	  break;
	default: ;
	}
	switch ( move_x_type) {
	case pwr_eType_Float32: move_x = x_orig + scale_offs + (*move_x_p - x_offset) * factor; break;
	case pwr_eType_Float64: move_x = x_orig + scale_offs + (*(pwr_tFloat64 *) move_x_p - x_offset) * factor; break;
	case pwr_eType_Int32:   move_x = x_orig + scale_offs + (*(pwr_tInt32 *) move_x_p - x_offset) * factor; break;
	case pwr_eType_UInt32:  move_x = x_orig + scale_offs + (*(pwr_tUInt32 *) move_x_p - x_offset) * factor; break;
	default: move_x = x_orig + scale_offs;
	}
      }
      else
	move_x = x_orig;

      if ( move_y_p) {
	double scale_offs = 0;
	// Adjust position for different scaletypes
	switch ( scale_type) {
	case glow_eScaleType_LowerRight:
	case glow_eScaleType_UpperRight:
	  scale_offs = height_orig * ( 1 - scale_y);
	  break;
	case glow_eScaleType_Center:
	  scale_offs = height_orig * ( 1 - scale_y) / 2;
	  break;
	default: ;
	}

	switch ( move_y_type) {
	case pwr_eType_Float32: move_y = y_orig + scale_offs + (*move_y_p - y_offset) * factor; break;
	case pwr_eType_Float64: move_y = y_orig + scale_offs + (*(pwr_tFloat64 *) move_y_p - y_offset) * factor; break;
	case pwr_eType_Int32:   move_y = y_orig + scale_offs + (*(pwr_tInt32 *) move_y_p - y_offset) * factor; break;
	case pwr_eType_UInt32:  move_y = y_orig + scale_offs + (*(pwr_tUInt32 *) move_y_p - y_offset) * factor; break;
	default: move_y = y_orig + scale_offs;
	}
      }
      else
	move_y = y_orig;

      grow_SetObjectScalePos( object, move_x, move_y, 
			      scale_x, scale_y, 0, 0,
			      scale_type);
      if ( move_x_p)
	memcpy( &move_x_old_value, move_x_p, move_x_size);
      if ( move_y_p)
	memcpy( &move_y_old_value, move_y_p, move_y_size);
    }
  }
  else {
    if ( move_x_p) {
      switch ( move_x_type) {
      case pwr_eType_Float32: move_x = (*move_x_p - x_offset) * factor; break;
      case pwr_eType_Float64: move_x = (*(pwr_tFloat64 *) move_x_p - x_offset) * factor; break;
      case pwr_eType_Int32:   move_x = (*(pwr_tInt32 *) move_x_p - x_offset) * factor; break;
      case pwr_eType_UInt32:  move_x = (*(pwr_tUInt32 *) move_x_p - x_offset) * factor; break;
      default: move_x = 0;
      }
    }
    else
      move_x = 0;

    if ( move_y_p) {
      switch ( move_y_type) {
      case pwr_eType_Float32: move_y = (*move_y_p - y_offset) * factor; break;
      case pwr_eType_Float64: move_y = (*(pwr_tFloat64 *) move_y_p - y_offset) * factor; break;
      case pwr_eType_Int32:   move_y = (*(pwr_tInt32 *) move_y_p - y_offset) * factor; break;
      case pwr_eType_UInt32:  move_y = (*(pwr_tUInt32 *) move_y_p - y_offset) * factor; break;
      default: move_y = 0;
      }
    }
    else
      move_y = 0;
  
    grow_SetObjectPosition( object, move_x, move_y);

    if ( move_x_p)
      memcpy( &move_x_old_value, move_x_p, move_x_size);
    if ( move_y_p)
      memcpy( &move_y_old_value, move_y_p, move_y_size);
  }
  return 1;
}

int GeMove::export_java( grow_tObject object, ofstream& fp, bool first, char *var_name)
{
  double zoom_factor;

  grow_GetZoom( dyn->graph->grow->ctx, &zoom_factor);

  if ( first)
    fp << "      ";
  else
    fp << "      ,";
  fp << "new GeDynMove(" << var_name << ".dd, \"" 
     << move_x_attribute << "\",\"" << move_y_attribute << "\",\""
     << scale_x_attribute << "\",\"" << scale_y_attribute << "\","
     << x_offset << "," << y_offset << "," << factor * zoom_factor << "," 
     << scale_factor << ")" << endl;
  return 1;
}

void GeAnalogShift::get_attributes( attr_sItem *attrinfo, int *item_count)
{
  int i = *item_count;

  strcpy( attrinfo[i].name, "AnalogShift.Attribute");
  attrinfo[i].value = attribute;
  attrinfo[i].type = glow_eType_String;
  attrinfo[i++].size = sizeof( attribute);

  *item_count = i;
}

void GeAnalogShift::set_attribute( grow_tObject object, char *attr_name, int *cnt)
{
  (*cnt)--;
  if ( *cnt == 0) {
    char msg[200];

    strncpy( attribute, attr_name, sizeof( attribute));
    sprintf( msg, "AnalogShift.Attribute = %s", attr_name);
    dyn->graph->message( 'I', msg);
  }
}

void GeAnalogShift::replace_attribute( char *from, char *to, int *cnt, int strict)
{
  GeDyn::replace_attribute( attribute, sizeof(attribute), from, to, cnt, strict);
}

void GeAnalogShift::save( ofstream& fp)
{
  fp << int(ge_eSave_AnalogShift) << endl;
  fp << int(ge_eSave_AnalogShift_attribute) << FSPACE << attribute << endl;
  fp << int(ge_eSave_End) << endl;
}

void GeAnalogShift::open( ifstream& fp)
{
  int		type;
  int 		end_found = 0;
  char		dummy[40];

  for (;;)
  {
    fp >> type;
    switch( type) {
      case ge_eSave_AnalogShift: break;
      case ge_eSave_AnalogShift_attribute:
        fp.get();
        fp.getline( attribute, sizeof(attribute));
        break;
      case ge_eSave_End: end_found = 1; break;
      default:
        cout << "GeAnalogShift:open syntax error" << endl;
        fp.getline( dummy, sizeof(dummy));
    }
    if ( end_found)
      break;
  }  
}

int GeAnalogShift::connect( grow_tObject object, glow_sTraceData *trace_data)
{
  char		parsed_name[120];
  int		inverted;
  int		sts;

  size = 4;
  p = 0;
  db = dyn->graph->parse_attr_name( attribute, parsed_name,
				    &inverted, &type, &size);
  if ( strcmp( parsed_name,"") == 0)
    return 1;

  switch ( type) {
  case pwr_eType_Float32:
  case pwr_eType_Int32:
    break;
  default:
    return 1;
  }

  sts = dyn->graph->ref_object_info( dyn->cycle, parsed_name, (void **)&p, &subid, size);
  if ( EVEN(sts)) return sts;

  if ( p)
    trace_data->p = p;
  first_scan = true;
  return 1;
}

int GeAnalogShift::disconnect( grow_tObject object)
{
  if ( p && db == graph_eDatabase_Gdh)
    gdh_UnrefObjectInfo( subid);
  p = 0;
  return 1;
}

int GeAnalogShift::scan( grow_tObject object)
{
  if ( !first_scan) {
    switch ( type) {
    case pwr_eType_Float32:
      if ( fabs( old_value - *p) < FLT_EPSILON)
	return 1;
      break;
    default:
      if ( memcmp( &old_value, p, size) == 0)
	return 1;
    }
  }
  else
    first_scan = false;

  int index;

  switch ( type) {
  case pwr_eType_Float32:
    index = int( *p + 0.5);
    break;
  default:
    index = *(pwr_tInt32 *)p;
  }
  grow_SetObjectNodeClassByIndex( object, index);
  old_value = *p;
  return 1;
}

int GeAnalogShift::export_java( grow_tObject object, ofstream& fp, bool first, char *var_name)
{
  if ( first)
    fp << "      ";
  else
    fp << "      ,";
  fp << "new GeDynAnalogShift(" << var_name << ".dd, \"" << attribute << "\")" << endl;
  return 1;
}

void GeDigShift::get_attributes( attr_sItem *attrinfo, int *item_count)
{
  int i = *item_count;

  strcpy( attrinfo[i].name, "DigShift.Attribute");
  attrinfo[i].value = attribute;
  attrinfo[i].type = glow_eType_String;
  attrinfo[i++].size = sizeof( attribute);

  *item_count = i;
}

void GeDigShift::set_attribute( grow_tObject object, char *attr_name, int *cnt)
{
  (*cnt)--;
  if ( *cnt == 0) {
    char msg[200];

    strncpy( attribute, attr_name, sizeof( attribute));
    sprintf( msg, "DigShift.Attribute = %s", attr_name);
    dyn->graph->message( 'I', msg);
  }
}

void GeDigShift::replace_attribute( char *from, char *to, int *cnt, int strict)
{
  GeDyn::replace_attribute( attribute, sizeof(attribute), from, to, cnt, strict);
}

void GeDigShift::save( ofstream& fp)
{
  fp << int(ge_eSave_DigShift) << endl;
  fp << int(ge_eSave_DigShift_attribute) << FSPACE << attribute << endl;
  fp << int(ge_eSave_End) << endl;
}

void GeDigShift::open( ifstream& fp)
{
  int		type;
  int 		end_found = 0;
  char		dummy[40];

  for (;;)
  {
    fp >> type;
    switch( type) {
      case ge_eSave_DigShift: break;
      case ge_eSave_DigShift_attribute:
        fp.get();
        fp.getline( attribute, sizeof(attribute));
        break;
      case ge_eSave_End: end_found = 1; break;
      default:
        cout << "GeDigShift:open syntax error" << endl;
        fp.getline( dummy, sizeof(dummy));
    }
    if ( end_found)
      break;
  }  
}

int GeDigShift::connect( grow_tObject object, glow_sTraceData *trace_data)
{
  int		attr_type, attr_size;
  char		parsed_name[120];
  int		sts;

  size = 4;
  p = 0;
  db = dyn->graph->parse_attr_name( attribute, parsed_name,
				    &inverted, &attr_type, &attr_size);
  if ( strcmp( parsed_name,"") == 0)
    return 1;

  sts = dyn->graph->ref_object_info( dyn->cycle, parsed_name, (void **)&p, &subid, size);
  if ( EVEN(sts)) return sts;

  if ( p)
    trace_data->p = p;
  first_scan = true;
  return 1;
}

int GeDigShift::disconnect( grow_tObject object)
{
  if ( p && db == graph_eDatabase_Gdh)
    gdh_UnrefObjectInfo( subid);
  p = 0;
  return 1;
}

int GeDigShift::scan( grow_tObject object)
{
  if ( !p)
    return 1;
  if ( !first_scan) {
    if ( old_value == *p) {
      // No change since last time
      return 1;
    }
  }
  else
    first_scan = false;

  if ( (!inverted && *p) || (inverted && !*p)) {
    grow_SetObjectLastNodeClass( object);
  }
  else {
    grow_SetObjectFirstNodeClass( object);
  }
  old_value = *p;
  return 1;
}

int GeDigShift::export_java( grow_tObject object, ofstream& fp, bool first, char *var_name)
{
  if ( first)
    fp << "      ";
  else
    fp << "      ,";
  fp << "new GeDynDigShift(" << var_name << ".dd, \"" << attribute << "\")" << endl;
  return 1;
}

void GeAnimation::get_attributes( attr_sItem *attrinfo, int *item_count)
{
  int i = *item_count;

  strcpy( attrinfo[i].name, "Animation.Attribute");
  attrinfo[i].value = attribute;
  attrinfo[i].type = glow_eType_String;
  attrinfo[i++].size = sizeof( attribute);

  strcpy( attrinfo[i].name, "Animation.Sequence");
  attrinfo[i].value = (void *) &sequence;
  attrinfo[i].type = ge_eAttrType_AnimSequence;
  attrinfo[i++].size = sizeof( sequence);

  *item_count = i;
}

void GeAnimation::save( ofstream& fp)
{
  fp << int(ge_eSave_Animation) << endl;
  fp << int(ge_eSave_Animation_attribute) << FSPACE << attribute << endl;
  fp << int(ge_eSave_Animation_sequence) << FSPACE << int(sequence) << endl;
  fp << int(ge_eSave_End) << endl;
}

void GeAnimation::set_attribute( grow_tObject object, char *attr_name, int *cnt)
{
  (*cnt)--;
  if ( *cnt == 0) {
    char msg[200];

    strncpy( attribute, attr_name, sizeof( attribute));
    sprintf( msg, "Animation.Attribute = %s", attr_name);
    dyn->graph->message( 'I', msg);
  }
}

void GeAnimation::replace_attribute( char *from, char *to, int *cnt, int strict)
{
  GeDyn::replace_attribute( attribute, sizeof(attribute), from, to, cnt, strict);
}

void GeAnimation::open( ifstream& fp)
{
  int		type;
  int 		end_found = 0;
  int		tmp;
  char		dummy[40];

  for (;;)
  {
    fp >> type;
    switch( type) {
      case ge_eSave_Animation: break;
      case ge_eSave_Animation_attribute:
        fp.get();
        fp.getline( attribute, sizeof(attribute));
        break;
      case ge_eSave_Animation_sequence: fp >> tmp; sequence = (ge_eAnimSequence)tmp; break;
      case ge_eSave_End: end_found = 1; break;
      default:
        cout << "GeAnimation:open syntax error" << endl;
        fp.getline( dummy, sizeof(dummy));
    }
    if ( end_found)
      break;
  }  
}

int GeAnimation::connect( grow_tObject object, glow_sTraceData *trace_data)
{
  int		attr_type, attr_size;
  char		parsed_name[120];
  int		sts;
  int		attr2;

  size = 4;
  p = 0;
  db = dyn->graph->parse_attr_name( attribute, parsed_name,
				    &inverted, &attr_type, &attr_size);
  if ( strcmp( parsed_name,"") == 0)
    return 1;

  sts = dyn->graph->ref_object_info( dyn->cycle, parsed_name, (void **)&p, &subid, size);
  if ( EVEN(sts)) return sts;

  if ( p)
    trace_data->p = p;
  first_scan = true;

  if ( sequence == ge_eAnimSequence_Inherit)
    grow_GetObjectClassDynAttr( object, (int *)&sequence, &attr2);

  return 1;
}

int GeAnimation::disconnect( grow_tObject object)
{
  if ( p && db == graph_eDatabase_Gdh)
    gdh_UnrefObjectInfo( subid);
  p = 0;
  return 1;
}

int GeAnimation::scan( grow_tObject object)
{
  int max_count;
  int sts;

  if ( first_scan) {
    animation_count = 0;
    animation_direction = 0;
    first_scan = false;
    if ( sequence == ge_eAnimSequence_Dig) {
      if ( (!inverted && *p) || (inverted && !*p))
	grow_SetObjectLastNodeClass( object);
      old_value = *p;
    }
  }

  if ( sequence == ge_eAnimSequence_Dig) {
    if ( old_value != *p) {
      // Start animation
      if ( (!inverted && *p) || (inverted && !*p)) {
	animation_count = 0;
	animation_direction = 1;
      }
      else if ( (!inverted && !*p) || (inverted && *p)) {
	animation_direction = 2;
	animation_count = 0;
      }
    }

    if ( animation_direction != 0) {
      grow_GetObjectAnimationCount( object, &max_count);
      animation_count++;
      if ( animation_count >= max_count) {
	// Shift nodeclass
	if ( animation_direction == 1) {
	  // Shift forward

	  sts = grow_SetObjectNextNodeClass( object);
	  if ( EVEN(sts)) {
	    // End of animation
	    animation_count = 0;
	    animation_direction = 0;
	  }
	  animation_count = 0;
	}
	else {
	  // Shift backward

	  sts = grow_SetObjectPrevNodeClass( object);
	  if ( EVEN(sts)) {
	    // End of animation
	    animation_count = 0;
	    animation_direction = 0;
	  }
	  animation_count = 0;
	}
      }
    }
  }
  else {
    if ( (!inverted && *p) || (inverted && !*p)) {
      if ( animation_direction == 0) {
	// Animation has been stopped
	animation_count = 0;
	animation_direction = 1;
      }

      grow_GetObjectAnimationCount( object, &max_count);
      animation_count++;
      if ( animation_count >= max_count) {
	// Shift nodeclass
	if ( animation_direction == 1) {
	  // Shift forward

	  sts = grow_SetObjectNextNodeClass( object);
	  if ( EVEN(sts)) {
	    if ( sequence == ge_eAnimSequence_Cycle) {
	      // Start from the beginning again
	      grow_SetObjectNodeClassByIndex( object, 1);
	    }
	    else {
	      // Change direction
	      animation_direction = 2;
	      sts = grow_SetObjectPrevNodeClass( object);
	    }
	  }
	  animation_count = 0;
	}
	else {
	  // Shift backward

	  sts = grow_SetObjectPrevNodeClass( object);
	  if ( EVEN(sts)) {
	    // Change direction
	    animation_direction = 1;
	    sts = grow_SetObjectNextNodeClass( object);
	  }
	  animation_count = 0;
	}
      }
    }
    else {
      if ( animation_direction != 0) {
	// Stop and reset animation
	animation_direction = 0;
	grow_SetObjectFirstNodeClass( object);
      }
    }
  }
  old_value = *p;
  return 1;
}

int GeAnimation::export_java( grow_tObject object, ofstream& fp, bool first, char *var_name)
{
  int seq, attr2;

  if ( sequence == ge_eAnimSequence_Inherit)
    grow_GetObjectClassDynAttr( object, &seq, &attr2);
  else
    seq = (int)sequence;

  if ( first)
    fp << "      ";
  else
    fp << "      ,";
  fp << "new GeDynAnimation(" << var_name << ".dd, \"" << attribute << "\"," << 
      seq << ")" << endl;
  return 1;
}

void GeVideo::save( ofstream& fp)
{
  fp << int(ge_eSave_Video) << endl;
  fp << int(ge_eSave_End) << endl;
}

void GeVideo::open( ifstream& fp)
{
  int		type;
  int 		end_found = 0;
  char		dummy[40];

  for (;;)
  {
    fp >> type;
    switch( type) {
      case ge_eSave_Video: break;
      case ge_eSave_End: end_found = 1; break;
      default:
        cout << "GeVideo:open syntax error" << endl;
        fp.getline( dummy, sizeof(dummy));
    }
    if ( end_found)
      break;
  }  
}

int GeVideo::connect( grow_tObject object, glow_sTraceData *trace_data)
{
  trace_data->p = (void *) 1;
  return 1;
}

int GeVideo::disconnect( grow_tObject object)
{
  return 1;
}

int GeVideo::scan( grow_tObject object)
{
  grow_tObject 	*objectlist;
  int 		object_cnt;

  grow_GetGroupObjectList( object, &objectlist, &object_cnt);

  for ( int i = 0; i < object_cnt; i++) {
    if ( grow_GetObjectType( objectlist[i]) == glow_eObjectType_GrowImage) {
      grow_ImageUpdate( objectlist[i]);
      break;
    }	
  }
  return 1;
}

void GeBar::get_attributes( attr_sItem *attrinfo, int *item_count)
{
  int i = *item_count;

  strcpy( attrinfo[i].name, "Bar.Attribute");
  attrinfo[i].value = attribute;
  attrinfo[i].type = glow_eType_String;
  attrinfo[i++].size = sizeof( attribute);

  *item_count = i;
}

void GeBar::set_attribute( grow_tObject object, char *attr_name, int *cnt)
{
  (*cnt)--;
  if ( *cnt == 0) {
    char msg[200];

    strncpy( attribute, attr_name, sizeof( attribute));
    sprintf( msg, "Bar.Attribute = %s", attr_name);
    dyn->graph->message( 'I', msg);
  }
}

void GeBar::replace_attribute( char *from, char *to, int *cnt, int strict)
{
  GeDyn::replace_attribute( attribute, sizeof(attribute), from, to, cnt, strict);
}

void GeBar::save( ofstream& fp)
{
  fp << int(ge_eSave_Bar) << endl;
  fp << int(ge_eSave_Bar_attribute) << FSPACE << attribute << endl;
  fp << int(ge_eSave_End) << endl;
}

void GeBar::open( ifstream& fp)
{
  int		type;
  int 		end_found = 0;
  char		dummy[40];

  for (;;)
  {
    fp >> type;
    switch( type) {
      case ge_eSave_Bar: break;
      case ge_eSave_Bar_attribute:
        fp.get();
        fp.getline( attribute, sizeof(attribute));
        break;
      case ge_eSave_End: end_found = 1; break;
      default:
        cout << "GeBar:open syntax error" << endl;
        fp.getline( dummy, sizeof(dummy));
    }
    if ( end_found)
      break;
  }
}

int GeBar::connect( grow_tObject object, glow_sTraceData *trace_data)
{
  int		attr_type, attr_size;
  char		parsed_name[120];
  int		sts;
  int		inverted;

  size = 4;
  p = 0;
  db = dyn->graph->parse_attr_name( attribute, parsed_name,
				    &inverted, &attr_type, &attr_size);
  if ( strcmp( parsed_name,"") == 0)
    return 1;

  switch ( db) {
  case graph_eDatabase_Gdh:
    sts = dyn->graph->ref_object_info( dyn->cycle, parsed_name, (void **)&p, 
				       &subid, attr_size);
    if ( EVEN(sts)) return sts;
    bar_typeid = attr_type;
    break;
  case graph_eDatabase_Local:
    p = (pwr_tFloat32 *) dyn->graph->localdb_ref_or_create( parsed_name, attr_type);
    bar_typeid = attr_type;
  case graph_eDatabase_User:
    bar_typeid = attr_type;
    break;
  default:
    ;
  }

  if ( p)
    trace_data->p = p;
  first_scan = true;
  return 1;
}

int GeBar::disconnect( grow_tObject object)
{
  if ( p && db == graph_eDatabase_Gdh)
    gdh_UnrefObjectInfo( subid);
  p = 0;
  return 1;
}

int GeBar::scan( grow_tObject object)
{
  if ( !p)
    return 1;

  if ( !first_scan) {
    if ( memcmp( &old_value, p, size) == 0 )
      // No change since last time
      return 1;
  }
  else
    first_scan = false;

  switch ( bar_typeid) {
  case pwr_eType_Float32:
    grow_SetBarValue( object, double( *(pwr_tFloat32 *) p));
    break;
  case pwr_eType_UInt32:
    grow_SetBarValue( object, double( *(pwr_tUInt32 *) p));
    break;
  case pwr_eType_Int32:
    grow_SetBarValue( object, double( *(pwr_tInt32 *) p));
    break;
  default: ;
  }

  memcpy( &old_value, p, size);
  return 1;
}

void GeTrend::get_attributes( attr_sItem *attrinfo, int *item_count)
{
  int i = *item_count;

  strcpy( attrinfo[i].name, "Trend.Attribute1");
  attrinfo[i].value = attribute1;
  attrinfo[i].type = glow_eType_String;
  attrinfo[i++].size = sizeof( attribute1);

  strcpy( attrinfo[i].name, "Trend.Attribute2");
  attrinfo[i].value = attribute2;
  attrinfo[i].type = glow_eType_String;
  attrinfo[i++].size = sizeof( attribute2);

  *item_count = i;
}

void GeTrend::set_attribute( grow_tObject object, char *attr_name, int *cnt)
{
  (*cnt)--;
  if ( *cnt == 0) {
    char msg[200];

    strncpy( attribute1, attr_name, sizeof( attribute1));
    sprintf( msg, "Trend.Attribute1 = %s", attr_name);
    dyn->graph->message( 'I', msg);
  }
  else if ( *cnt == 1) {
    char msg[200];

    strncpy( attribute2, attr_name, sizeof( attribute2));
    sprintf( msg, "Trend.YAttribute2 = %s", attr_name);
    dyn->graph->message( 'I', msg);
    (*cnt)--;
  }
}

void GeTrend::replace_attribute( char *from, char *to, int *cnt, int strict)
{
  GeDyn::replace_attribute( attribute1, sizeof(attribute1), from, to, cnt, strict);
  GeDyn::replace_attribute( attribute2, sizeof(attribute2), from, to, cnt, strict);
}

void GeTrend::save( ofstream& fp)
{
  fp << int(ge_eSave_Trend) << endl;
  fp << int(ge_eSave_Trend_attribute1) << FSPACE << attribute1 << endl;
  fp << int(ge_eSave_Trend_attribute2) << FSPACE << attribute2 << endl;
  fp << int(ge_eSave_End) << endl;
}

void GeTrend::open( ifstream& fp)
{
  int		type;
  int 		end_found = 0;
  char		dummy[40];

  for (;;)
  {
    fp >> type;
    switch( type) {
      case ge_eSave_Trend: break;
      case ge_eSave_Trend_attribute1:
        fp.get();
        fp.getline( attribute1, sizeof(attribute1));
        break;
      case ge_eSave_Trend_attribute2:
        fp.get();
        fp.getline( attribute2, sizeof(attribute2));
        break;
      case ge_eSave_End: end_found = 1; break;
      default:
        cout << "GeTrend:open syntax error" << endl;
        fp.getline( dummy, sizeof(dummy));
    }
    if ( end_found)
      break;
  }
}

int GeTrend::connect( grow_tObject object, glow_sTraceData *trace_data)
{
  int		attr_type, attr_size;
  char		parsed_name[120];
  int		sts;
  int		inverted;

  size1 = 4;
  p1 = 0;
  db1 = dyn->graph->parse_attr_name( attribute1, parsed_name,
				    &inverted, &attr_type, &attr_size);
  if ( strcmp( parsed_name,"") != 0) {

    switch ( db1) {
    case graph_eDatabase_Gdh:
      sts = dyn->graph->ref_object_info( dyn->cycle, parsed_name, (void **)&p1, 
				       &subid1, attr_size);
      if ( EVEN(sts)) return sts;
      trend_typeid1 = attr_type;
      break;
    case graph_eDatabase_Local:
      p1 = (pwr_tFloat32 *) dyn->graph->localdb_ref_or_create( parsed_name, attr_type);
      trend_typeid1 = attr_type;
    case graph_eDatabase_User:
      trend_typeid1 = attr_type;
      break;
    default:
      ;
    }
  }
  size2 = 4;
  p2 = 0;
  db2 = dyn->graph->parse_attr_name( attribute2, parsed_name,
				    &inverted, &attr_type, &attr_size);
  if ( strcmp( parsed_name,"") != 0) {
    switch ( db2) {
    case graph_eDatabase_Gdh:
      sts = dyn->graph->ref_object_info( dyn->cycle, parsed_name, (void **)&p2, 
					 &subid2, attr_size);
      if ( EVEN(sts)) return sts;
      trend_typeid2 = attr_type;
      break;
    case graph_eDatabase_Local:
      p2 = (pwr_tFloat32 *) dyn->graph->localdb_ref_or_create( parsed_name, attr_type);
      trend_typeid2 = attr_type;
    case graph_eDatabase_User:
      trend_typeid2 = attr_type;
      break;
    default:
      ;
    }
  }
  grow_GetTrendScanTime( object, &scan_time);
  acc_time = scan_time;
  trend_hold = 0;

  if ( p1)
    trace_data->p = p1;
  else if ( p2)
    trace_data->p = p2;
  first_scan = true;
  return 1;
}

int GeTrend::disconnect( grow_tObject object)
{
  if ( p1 && db1 == graph_eDatabase_Gdh)
    gdh_UnrefObjectInfo( subid1);
  p1 = 0;
  if ( p2 && db2 == graph_eDatabase_Gdh)
    gdh_UnrefObjectInfo( subid2);
  p2 = 0;
  return 1;
}

int GeTrend::scan( grow_tObject object)
{
  if ( !p1 && !p2)
    return 1;
  if ( trend_hold)
    return 1;

  if ( first_scan)
    first_scan = false;

  acc_time += dyn->graph->scan_time;
  if ( acc_time + DBL_EPSILON >= scan_time) {
    if ( p1) {
      switch ( trend_typeid1) {
      case pwr_eType_Boolean:
	grow_AddTrendValue( object, double( *(pwr_tBoolean *) p1), 0);
	break;
      case pwr_eType_Float32:
	grow_AddTrendValue( object, double( *(pwr_tFloat32 *) p1), 0);
	break;
      case pwr_eType_Int32:
	grow_AddTrendValue( object, double( *(pwr_tInt32 *) p1), 0);
	break;
      case pwr_eType_UInt32:
	grow_AddTrendValue( object, double( *(pwr_tUInt32 *) p1), 0);
	break;
      default: ;
      }
    }
    if ( p2) {
      switch ( trend_typeid2) {
      case pwr_eType_Boolean:
	grow_AddTrendValue( object, double( *(pwr_tBoolean *) p2), 1);
	break;
      case pwr_eType_Float32:
	grow_AddTrendValue( object, double( *(pwr_tFloat32 *) p2), 1);
	break;
      case pwr_eType_Int32:
	grow_AddTrendValue( object, double( *(pwr_tInt32 *) p2), 1);
	break;
      case pwr_eType_UInt32:
	grow_AddTrendValue( object, double( *(pwr_tUInt32 *) p2), 1);
	break;
      default: ;
      }
    }
    acc_time = 0;
  }
  return 1;
}

void GeTable::get_attributes( attr_sItem *attrinfo, int *item_count)
{
  int i = *item_count;

  strcpy( attrinfo[i].name, "Column1.Attribute");
  attrinfo[i].value = attribute[0];
  attrinfo[i].type = glow_eType_String;
  attrinfo[i++].size = sizeof( attribute[0]);

  strcpy( attrinfo[i].name, "Column1.Format");
  attrinfo[i].value = format[0];
  attrinfo[i].type = glow_eType_String;
  attrinfo[i++].size = sizeof( format[0]);

  strcpy( attrinfo[i].name, "Column1.SelectAttribute");
  attrinfo[i].value = sel_attribute[0];
  attrinfo[i].type = glow_eType_String;
  attrinfo[i++].size = sizeof( sel_attribute[0]);

  strcpy( attrinfo[i].name, "Column2.Attribute");
  attrinfo[i].value = attribute[1];
  attrinfo[i].type = glow_eType_String;
  attrinfo[i++].size = sizeof( attribute[0]);

  strcpy( attrinfo[i].name, "Column2.Format");
  attrinfo[i].value = format[1];
  attrinfo[i].type = glow_eType_String;
  attrinfo[i++].size = sizeof( format[0]);

  strcpy( attrinfo[i].name, "Column2.SelectAttribute");
  attrinfo[i].value = sel_attribute[1];
  attrinfo[i].type = glow_eType_String;
  attrinfo[i++].size = sizeof( sel_attribute[0]);

  strcpy( attrinfo[i].name, "Column3.Attribute");
  attrinfo[i].value = attribute[2];
  attrinfo[i].type = glow_eType_String;
  attrinfo[i++].size = sizeof( attribute[0]);

  strcpy( attrinfo[i].name, "Column3.Format");
  attrinfo[i].value = format[2];
  attrinfo[i].type = glow_eType_String;
  attrinfo[i++].size = sizeof( format[0]);

  strcpy( attrinfo[i].name, "Column3.SelectAttribute");
  attrinfo[i].value = sel_attribute[2];
  attrinfo[i].type = glow_eType_String;
  attrinfo[i++].size = sizeof( sel_attribute[0]);

  strcpy( attrinfo[i].name, "Column4.Attribute");
  attrinfo[i].value = attribute[3];
  attrinfo[i].type = glow_eType_String;
  attrinfo[i++].size = sizeof( attribute[0]);

  strcpy( attrinfo[i].name, "Column4.Format");
  attrinfo[i].value = format[3];
  attrinfo[i].type = glow_eType_String;
  attrinfo[i++].size = sizeof( format[0]);

  strcpy( attrinfo[i].name, "Column4.SelectAttribute");
  attrinfo[i].value = sel_attribute[3];
  attrinfo[i].type = glow_eType_String;
  attrinfo[i++].size = sizeof( sel_attribute[0]);

  strcpy( attrinfo[i].name, "Column5.Attribute");
  attrinfo[i].value = attribute[4];
  attrinfo[i].type = glow_eType_String;
  attrinfo[i++].size = sizeof( attribute[0]);

  strcpy( attrinfo[i].name, "Column5.Format");
  attrinfo[i].value = format[4];
  attrinfo[i].type = glow_eType_String;
  attrinfo[i++].size = sizeof( format[0]);

  strcpy( attrinfo[i].name, "Column5.SelectAttribute");
  attrinfo[i].value = sel_attribute[4];
  attrinfo[i].type = glow_eType_String;
  attrinfo[i++].size = sizeof( sel_attribute[0]);

  strcpy( attrinfo[i].name, "Column6.Attribute");
  attrinfo[i].value = attribute[5];
  attrinfo[i].type = glow_eType_String;
  attrinfo[i++].size = sizeof( attribute[0]);

  strcpy( attrinfo[i].name, "Column6.Format");
  attrinfo[i].value = format[5];
  attrinfo[i].type = glow_eType_String;
  attrinfo[i++].size = sizeof( format[0]);

  strcpy( attrinfo[i].name, "Column6.SelectAttribute");
  attrinfo[i].value = sel_attribute[5];
  attrinfo[i].type = glow_eType_String;
  attrinfo[i++].size = sizeof( sel_attribute[0]);

  strcpy( attrinfo[i].name, "Column7.Attribute");
  attrinfo[i].value = attribute[6];
  attrinfo[i].type = glow_eType_String;
  attrinfo[i++].size = sizeof( attribute[0]);

  strcpy( attrinfo[i].name, "Column7.Format");
  attrinfo[i].value = format[6];
  attrinfo[i].type = glow_eType_String;
  attrinfo[i++].size = sizeof( format[0]);

  strcpy( attrinfo[i].name, "Column7.SelectAttribute");
  attrinfo[i].value = sel_attribute[6];
  attrinfo[i].type = glow_eType_String;
  attrinfo[i++].size = sizeof( sel_attribute[0]);

  strcpy( attrinfo[i].name, "Column8.Attribute");
  attrinfo[i].value = attribute[7];
  attrinfo[i].type = glow_eType_String;
  attrinfo[i++].size = sizeof( attribute[0]);

  strcpy( attrinfo[i].name, "Column8.Format");
  attrinfo[i].value = format[7];
  attrinfo[i].type = glow_eType_String;
  attrinfo[i++].size = sizeof( format[0]);

  strcpy( attrinfo[i].name, "Column8.SelectAttribute");
  attrinfo[i].value = sel_attribute[7];
  attrinfo[i].type = glow_eType_String;
  attrinfo[i++].size = sizeof( sel_attribute[0]);

  strcpy( attrinfo[i].name, "Column9.Attribute");
  attrinfo[i].value = attribute[8];
  attrinfo[i].type = glow_eType_String;
  attrinfo[i++].size = sizeof( attribute[0]);

  strcpy( attrinfo[i].name, "Column9.Format");
  attrinfo[i].value = format[8];
  attrinfo[i].type = glow_eType_String;
  attrinfo[i++].size = sizeof( format[0]);

  strcpy( attrinfo[i].name, "Column9.SelectAttribute");
  attrinfo[i].value = sel_attribute[8];
  attrinfo[i].type = glow_eType_String;
  attrinfo[i++].size = sizeof( sel_attribute[0]);

  strcpy( attrinfo[i].name, "Column10.Attribute");
  attrinfo[i].value = attribute[9];
  attrinfo[i].type = glow_eType_String;
  attrinfo[i++].size = sizeof( attribute[0]);

  strcpy( attrinfo[i].name, "Column10.Format");
  attrinfo[i].value = format[9];
  attrinfo[i].type = glow_eType_String;
  attrinfo[i++].size = sizeof( format[0]);

  strcpy( attrinfo[i].name, "Column10.SelectAttribute");
  attrinfo[i].value = sel_attribute[9];
  attrinfo[i].type = glow_eType_String;
  attrinfo[i++].size = sizeof( sel_attribute[0]);

  strcpy( attrinfo[i].name, "Column11.Attribute");
  attrinfo[i].value = attribute[10];
  attrinfo[i].type = glow_eType_String;
  attrinfo[i++].size = sizeof( attribute[0]);

  strcpy( attrinfo[i].name, "Column11.Format");
  attrinfo[i].value = format[10];
  attrinfo[i].type = glow_eType_String;
  attrinfo[i++].size = sizeof( format[0]);

  strcpy( attrinfo[i].name, "Column11.SelectAttribute");
  attrinfo[i].value = sel_attribute[10];
  attrinfo[i].type = glow_eType_String;
  attrinfo[i++].size = sizeof( sel_attribute[0]);

  strcpy( attrinfo[i].name, "Column12.Attribute");
  attrinfo[i].value = attribute[11];
  attrinfo[i].type = glow_eType_String;
  attrinfo[i++].size = sizeof( attribute[0]);

  strcpy( attrinfo[i].name, "Column12.Format");
  attrinfo[i].value = format[11];
  attrinfo[i].type = glow_eType_String;
  attrinfo[i++].size = sizeof( format[0]);

  strcpy( attrinfo[i].name, "Column12.SelectAttribute");
  attrinfo[i].value = sel_attribute[11];
  attrinfo[i].type = glow_eType_String;
  attrinfo[i++].size = sizeof( sel_attribute[0]);

  *item_count = i;
}

void GeTable::set_attribute( grow_tObject object, char *attr_name, int *cnt)
{
  (*cnt)--;
  if ( *cnt == 0) {
    char msg[200];

    strncpy( attribute[0], attr_name, sizeof( attribute));
    sprintf( msg, "Column1.Attribute = %s", attr_name);
    dyn->graph->message( 'I', msg);
  }
}

void GeTable::replace_attribute( char *from, char *to, int *cnt, int strict)
{
  for ( int i = 0; i < TABLE_MAX_COL; i++)
    GeDyn::replace_attribute( attribute[i], sizeof(attribute[0]), from, to, cnt, strict);
}

void GeTable::save( ofstream& fp)
{
  fp << int(ge_eSave_Table) << endl;
  for ( int i = 0; i < TABLE_MAX_COL; i++) {
    fp << int(ge_eSave_Table_attribute1)+3*i << FSPACE << attribute[i] << endl;
    fp << int(ge_eSave_Table_format1)+3*i << FSPACE << format[i] << endl;
    fp << int(ge_eSave_Table_sel_attribute1)+3*i << FSPACE << sel_attribute[i] << endl;
  }
  fp << int(ge_eSave_End) << endl;
}

void GeTable::open( ifstream& fp)
{
  int		type;
  int 		end_found = 0;
  char		dummy[40];

  for (;;)
  {
    fp >> type;
    switch( type) {
      case ge_eSave_Table: break;
      case ge_eSave_Table_attribute1:
        fp.get(); fp.getline( attribute[0], sizeof(attribute[0])); break;
      case ge_eSave_Table_format1:
        fp.get(); fp.getline( format[0], sizeof(format[0])); break;
      case ge_eSave_Table_sel_attribute1:
        fp.get(); fp.getline( sel_attribute[0], sizeof(sel_attribute[0])); break;
      case ge_eSave_Table_attribute2:
        fp.get(); fp.getline( attribute[1], sizeof(attribute[0])); break;
      case ge_eSave_Table_format2:
        fp.get(); fp.getline( format[1], sizeof(format[0])); break;
      case ge_eSave_Table_sel_attribute2:
        fp.get(); fp.getline( sel_attribute[1], sizeof(sel_attribute[0])); break;
      case ge_eSave_Table_attribute3:
        fp.get(); fp.getline( attribute[2], sizeof(attribute[0])); break;
      case ge_eSave_Table_format3:
        fp.get(); fp.getline( format[2], sizeof(format[0])); break;
      case ge_eSave_Table_sel_attribute3:
        fp.get(); fp.getline( sel_attribute[2], sizeof(sel_attribute[0])); break;
      case ge_eSave_Table_attribute4:
        fp.get(); fp.getline( attribute[3], sizeof(attribute[0])); break;
      case ge_eSave_Table_format4:
        fp.get(); fp.getline( format[3], sizeof(format[0])); break;
      case ge_eSave_Table_sel_attribute4:
        fp.get(); fp.getline( sel_attribute[3], sizeof(sel_attribute[0])); break;
      case ge_eSave_Table_attribute5:
        fp.get(); fp.getline( attribute[4], sizeof(attribute[0])); break;
      case ge_eSave_Table_format5:
        fp.get(); fp.getline( format[4], sizeof(format[0])); break;
      case ge_eSave_Table_sel_attribute5:
        fp.get(); fp.getline( sel_attribute[4], sizeof(sel_attribute[0])); break;
      case ge_eSave_Table_attribute6:
        fp.get(); fp.getline( attribute[5], sizeof(attribute[0])); break;
      case ge_eSave_Table_format6:
        fp.get(); fp.getline( format[5], sizeof(format[0])); break;
      case ge_eSave_Table_sel_attribute6:
        fp.get(); fp.getline( sel_attribute[5], sizeof(sel_attribute[0])); break;
      case ge_eSave_Table_attribute7:
        fp.get(); fp.getline( attribute[6], sizeof(attribute[0])); break;
      case ge_eSave_Table_format7:
        fp.get(); fp.getline( format[6], sizeof(format[0])); break;
      case ge_eSave_Table_sel_attribute7:
        fp.get(); fp.getline( sel_attribute[6], sizeof(sel_attribute[0])); break;
      case ge_eSave_Table_attribute8:
        fp.get(); fp.getline( attribute[7], sizeof(attribute[0])); break;
      case ge_eSave_Table_format8:
        fp.get(); fp.getline( format[7], sizeof(format[0])); break;
      case ge_eSave_Table_sel_attribute8:
        fp.get(); fp.getline( sel_attribute[7], sizeof(sel_attribute[0])); break;
      case ge_eSave_Table_attribute9:
        fp.get(); fp.getline( attribute[8], sizeof(attribute[0])); break;
      case ge_eSave_Table_format9:
        fp.get(); fp.getline( format[8], sizeof(format[0])); break;
      case ge_eSave_Table_sel_attribute9:
        fp.get(); fp.getline( sel_attribute[8], sizeof(sel_attribute[0])); break;
      case ge_eSave_Table_attribute10:
        fp.get(); fp.getline( attribute[9], sizeof(attribute[0])); break;
      case ge_eSave_Table_format10:
        fp.get(); fp.getline( format[9], sizeof(format[0])); break;
      case ge_eSave_Table_sel_attribute10:
        fp.get(); fp.getline( sel_attribute[9], sizeof(sel_attribute[0])); break;
      case ge_eSave_Table_attribute11:
        fp.get(); fp.getline( attribute[10], sizeof(attribute[0])); break;
      case ge_eSave_Table_format11:
        fp.get(); fp.getline( format[10], sizeof(format[0])); break;
      case ge_eSave_Table_sel_attribute11:
        fp.get(); fp.getline( sel_attribute[10], sizeof(sel_attribute[0])); break;
      case ge_eSave_Table_attribute12:
        fp.get(); fp.getline( attribute[11], sizeof(attribute[0])); break;
      case ge_eSave_Table_format12:
        fp.get(); fp.getline( format[11], sizeof(format[0])); break;
     case ge_eSave_Table_sel_attribute12:
        fp.get(); fp.getline( sel_attribute[11], sizeof(sel_attribute[0])); break;
      case ge_eSave_End: end_found = 1; break;
      default:
        cout << "GeTable:open syntax error" << endl;
        fp.getline( dummy, sizeof(dummy));
    }
    if ( end_found)
      break;
  }
}

int GeTable::connect( grow_tObject object, glow_sTraceData *trace_data)
{
  int		attr_type;
  int		attr_size;
  char		parsed_name[120];
  int		sts;
  int		inverted;
  glow_sTableInfo info;

  grow_GetTableInfo( object, &info);
  columns = info.columns;
  rows = info.rows;

  for ( int i = 0; i < columns; i++) {
    p[i] = 0;
    db[i] = dyn->graph->parse_attr_name( attribute[i], (char *)parsed_name,
				    &inverted, &attr_type, &size[i], &elements[i]);
    if ( strcmp( parsed_name,"") == 0)
      continue;

    if ( !elements[i])
      continue;

    size[i] = size[i] / elements[i];
    int col_size = size[i] * MIN(rows, elements[i]);

    elements[i] = MIN( elements[i], rows);

    if ( strncmp( parsed_name, "$header.", 8) != 0) { 
      switch ( db[i]) {
      case graph_eDatabase_Gdh:
	sts = dyn->graph->ref_object_info( dyn->cycle, parsed_name, (void **)&p[i], 
					   &subid[i], col_size);
	if ( EVEN(sts)) return sts;
	type_id[i] = attr_type;
	break;
      default:
	;
      }
    }
    else {
      int		h_attr_type;
      int		h_attr_size;
      char		h_parsed_name[120];
      int		h_inverted;
      int		h_elements;
      pwr_tObjid	*objid_value;
      char		name[120];

      // Replace $header with the object in the header column
      is_headerref[i] = 1;
      elements[i] = elements[0];
      type_id[i] = attr_type;
      headerref_p[i] = (char **)calloc( elements[i], sizeof(char*));
      headerref_subid[i] = (pwr_tSubid *)calloc( elements[i], sizeof(pwr_tSubid));

	
      dyn->graph->parse_attr_name( attribute[0], (char *)h_parsed_name,
				   &h_inverted, &h_attr_type, &h_attr_size, &h_elements);
      objid_value = (pwr_tObjid *) calloc( h_elements, sizeof(pwr_tObjid));
      sts = gdh_GetObjectInfo( h_parsed_name, objid_value, 
			       h_elements * sizeof(pwr_tObjid));
      if ( EVEN(sts)) continue;

      for ( int j = 0; j < elements[i]; j++) {
	if ( cdh_ObjidIsNotNull( objid_value[j])) {
	  sts = gdh_ObjidToName( objid_value[j], name, sizeof(name), cdh_mName_volumeStrict);
	  if ( EVEN(sts)) continue;

	  strcat( name, &parsed_name[7]);
	  type_id[i] = attr_type;
	  sts = dyn->graph->ref_object_info( dyn->cycle, name, (void **)&headerref_p[i][j],
					   &headerref_subid[i][j], size[i]);
	  if ( EVEN(sts)) return sts;
	}
      }
    }

    switch ( type_id[i]) {
    case pwr_eType_String:
      info.column_size[i] = size[i];
      break;
    default:
      info.column_size[i] = 10;
    }

    old_value[i] = (char *)calloc( elements[i], size[i]);


    // Connect select array
    sel_p[i] = 0;
    sel_db[i] = dyn->graph->parse_attr_name( sel_attribute[i], (char *)parsed_name,
				    &inverted, &attr_type, &attr_size, &sel_elements[i]);
    if ( strcmp( parsed_name,"") == 0)
      continue;

    if ( sel_elements[i] > elements[i])
      sel_elements[i] = elements[i];
    if ( attr_type != pwr_eType_Boolean ||
	 !sel_elements[i])
      continue;

    switch ( sel_db[i]) {
    case graph_eDatabase_Gdh:
      sts = dyn->graph->ref_object_info( dyn->cycle, parsed_name, (void **)&sel_p[i], 
				       &sel_subid[i], sel_elements[i] * sizeof(pwr_tBoolean));
      if ( EVEN(sts)) return sts;
      break;
    default:
      ;
    }
  }
    
  grow_SetTableInfo( object, &info);

  if ( p[0])
    trace_data->p = p[0];
  first_scan = true;
  return 1;
}

int GeTable::disconnect( grow_tObject object)
{
  for ( int i = 0; i < columns; i++) {
    if ( p[i] && db[i] == graph_eDatabase_Gdh)
      gdh_UnrefObjectInfo( subid[i]);
    p[i] = 0;
    if ( old_value[i]) {
      free( old_value[i]);
      old_value[i] = 0;
    }

    if ( sel_p[i] && sel_db[i] == graph_eDatabase_Gdh)
      gdh_UnrefObjectInfo( subid[i]);
    sel_p[i] = 0;

    if ( is_headerref[i]) {
      for ( int j = 0; j < elements[i]; j++) {
	if ( headerref_p[i][j])
	  gdh_UnrefObjectInfo( headerref_subid[i][j]);
      }
      free( headerref_p[i]);
      free( headerref_subid[i]);
    }
  }
  return 1;
}

int GeTable::scan( grow_tObject object)
{
  if ( !p[0])
    return 1;

  int i, j, offs;
  char		buf[120];
  int		len;

  grow_SetDeferedRedraw( dyn->graph->grow->ctx);
  for ( i = 0; i < columns; i++) {
    if ( is_headerref[i]) {
      for  ( j = 0; j < elements[i]; j++) {
	if ( !headerref_p[i][j])
	  continue;

	offs = j * size[i];
	if ( !first_scan) {
	  if ( memcmp( old_value[i] + offs, headerref_p[i][j], size[i]) == 0 )
	    // No change since last time
	    continue;
	}

	switch ( type_id[i]) {
	case pwr_eType_Float32:
	  len = sprintf( buf, format[i], *(pwr_tFloat32 *) headerref_p[i][j]);
	  break;
	case pwr_eType_Int32:
	case pwr_eType_UInt32:
	  len = sprintf( buf, format[i], *(pwr_tInt32 *) headerref_p[i][j]);
	  break;
	case pwr_eType_String:
	  len = sprintf( buf, format[i], (char *)headerref_p[i][j]);
	  break;
	case pwr_eType_Objid: {
	  int sts;
	  char name[120];
	  pwr_tObjid objid = *(pwr_tObjid *)headerref_p[i][j];
	  sts = gdh_ObjidToName ( objid, name, sizeof(name), 
				  cdh_mName_object);
	  if ( EVEN(sts))
	    strcpy( name, "");
	  len = sprintf( buf, "%s", name);
	  break;
	}
	default: {
	  int sts;
	  sts = cdh_AttrValueToString( (pwr_eType) type_id[i], 
				       headerref_p[i][j], buf, sizeof(buf));
	  if ( EVEN(sts))
	    sprintf( buf, "Invalid type");
	  len = strlen(buf);
	}
	}

	grow_SetCellValue( object, i, j, buf);
	memcpy( old_value[i] + offs, headerref_p[i][j], size[i]);
      }
    }
    else {
      if ( !p[i])
	continue;
      for ( j = 0; j < elements[i]; j++) {
	offs = j * size[i];
	if ( !first_scan) {
	  if ( memcmp( old_value[i] + offs, p[i] + offs, size[i]) == 0 )
	    // No change since last time
	    continue;
	}

	switch ( type_id[i]) {
	case pwr_eType_Float32:
	  len = sprintf( buf, format[i], *(pwr_tFloat32 *) (p[i] + offs));
	  break;
	case pwr_eType_Int32:
	case pwr_eType_UInt32:
	  len = sprintf( buf, format[i], *(pwr_tInt32 *) (p[i] + offs));
	  break;
	case pwr_eType_String:
	  len = sprintf( buf, format[i], (char *)(p[i] + offs));
	  break;
	case pwr_eType_Objid: {
	  int sts;
	  char name[120];
	  pwr_tObjid objid = *(pwr_tObjid *)(p[i] + offs);
	  sts = gdh_ObjidToName ( objid, name, sizeof(name), 
				  cdh_mName_object);
	  if ( EVEN(sts))
	    strcpy( name, "");
	  len = sprintf( buf, "%s", name);
	  break;
	}
	default: {
	  int sts;
	  sts = cdh_AttrValueToString( (pwr_eType) type_id[i], 
				       p + offs, buf, sizeof(buf));
	  if ( EVEN(sts))
	    sprintf( buf, "Invalid type");
	  len = strlen(buf);
	}
	}

	grow_SetCellValue( object, i, j, buf);
	memcpy( old_value[i] + offs, p[i] + offs, size[i]);
      }
    }
  }

  // Examine select array
  int sel_found = 0;
  for ( i = 0; i < columns; i++) {
    if ( !sel_p[i])
      continue;
    for ( j = 0; j < sel_elements[i]; j++) {
      if ( sel_p[i][j]) {
	sel_found = 1;
	grow_SetSelectedCell( object, i, j);
      }
    }
  }
  if ( !sel_found)
    grow_SetSelectedCell( object, -1, -1);
  grow_RedrawDefered( dyn->graph->grow->ctx);
  if ( first_scan)
    first_scan = false;
  return 1;
}

int GeTable::action( grow_tObject object, glow_tEvent event)
{
  if ( !dyn->graph->is_authorized( dyn->access))
    return 1;

  switch ( event->event) {
  case glow_eEvent_MB1Down:
    grow_SetClickSensitivity( dyn->graph->grow->ctx, glow_mSensitivity_MB1Click);
    break;
  case glow_eEvent_MB1Up:
    break;
  case glow_eEvent_MB1Click: {
    int column, row;
    pwr_tBoolean	value;
    int			sts;
    char       		parsed_name[120];
    int			inverted;
    int			attr_type, attr_size;
    graph_eDatabase 	db;

    if ( event->any.type != glow_eEventType_Table)
      break;

    sts = grow_GetSelectedCell( object, &column, &row);
    if ( ODD(sts) && sel_p[column]) {
      db = dyn->graph->parse_attr_name( sel_attribute[column], parsed_name, &inverted, &attr_type, 
				      &attr_size);
      value = 0;
      sprintf( &parsed_name[strlen(parsed_name)], "[%d]", row);
      sts = gdh_SetObjectInfo( parsed_name, &value, sizeof(value));
      if ( EVEN(sts)) printf("Table error: %s\n", parsed_name);
    }
    if ( sel_p[event->table.column]) {
      db = dyn->graph->parse_attr_name( sel_attribute[event->table.column], parsed_name, &inverted, &attr_type, 
				      &attr_size);
      value = 1;
      sprintf( &parsed_name[strlen(parsed_name)], "[%d]", event->table.row);
      sts = gdh_SetObjectInfo( parsed_name, &value, sizeof(value));
      if ( EVEN(sts)) 
	printf("Table error: %s\n", parsed_name);
      else
	grow_SetSelectedCell( object, event->table.column, event->table.row);
    }
    break;
  }
  case glow_eEvent_Key_Up:
  case glow_eEvent_Key_Down:
  case glow_eEvent_Key_Left:
  case glow_eEvent_Key_Right: {
    int column, row, new_column, new_row;
    pwr_tBoolean	value;
    int			sts;
    char       		parsed_name[120];
    int			inverted;
    int			attr_type, attr_size;
    graph_eDatabase 	db;

    sts = grow_GetSelectedCell( object, &column, &row);

    switch ( event->event) {
    case glow_eEvent_Key_Up:
      if ( EVEN(sts))
	return GE__NO_PROPAGATE;
      new_row = row - 1;
      new_column = column;
      if ( new_row < 0)
	return GE__NO_PROPAGATE;
      break;
    case glow_eEvent_Key_Down:
      if ( EVEN(sts)) {
	column = 0;
	row = -1;
      }
      new_row = row + 1;
      new_column = column;
      if ( new_row >= sel_elements[new_column])
	return GE__NO_PROPAGATE;
      break;
    case glow_eEvent_Key_Left:
      if ( EVEN(sts))
	return GE__NO_PROPAGATE;
      new_row = row;
      new_column = column - 1;
      if ( new_column < 0 || !sel_p[new_column])
	return GE__NO_PROPAGATE;
      break;
    case glow_eEvent_Key_Right:
      if ( EVEN(sts)) {
	column = -1;
	row = 0;
      }
      new_row = row;
      new_column = column + 1;
      if ( new_column >= columns || !sel_p[new_column])
	return GE__NO_PROPAGATE;
      break;
    default: ;  
    }

    if ( ODD(sts) && sel_p[column]) {
      db = dyn->graph->parse_attr_name( sel_attribute[column], parsed_name, &inverted, &attr_type, 
				      &attr_size);
      value = 0;
      sprintf( &parsed_name[strlen(parsed_name)], "[%d]", row);
      sts = gdh_SetObjectInfo( parsed_name, &value, sizeof(value));
      if ( EVEN(sts)) printf("Table error: %s\n", parsed_name);
    }
    if ( sel_p[new_column]) {
      db = dyn->graph->parse_attr_name( sel_attribute[new_column], parsed_name, &inverted, &attr_type, 
				      &attr_size);
      value = 1;
      sprintf( &parsed_name[strlen(parsed_name)], "[%d]", new_row);
      sts = gdh_SetObjectInfo( parsed_name, &value, sizeof(value));
      if ( EVEN(sts)) 
	printf("Table error: %s\n", parsed_name);
      else {
	grow_SetSelectedCell( object, new_column, new_row);
	grow_TableMakeCellVisible( object, new_column, new_row);
      }
    }
    return GE__NO_PROPAGATE;
    break;
  }
  case glow_eEvent_MB3Press: {
    int			sts;
    char       		parsed_name[120];
    int			inverted;
    int			attr_type, attr_size;
    pwr_sAttrRef    	attrref;
    char            	name[80];
    Widget          	popup;

    if ( event->any.type != glow_eEventType_Table)
      break;
    
    if ( type_id[event->table.column] == pwr_eType_Objid) {
      dyn->graph->parse_attr_name( attribute[event->table.column], parsed_name, &inverted,
				 &attr_type, &attr_size);

      sprintf( &parsed_name[strlen(parsed_name)], "[%d]", event->table.row);
      memset( &attrref, 0, sizeof(attrref));
      sts = gdh_GetObjectInfo( parsed_name, &attrref.Objid, 
				 sizeof(attrref.Objid));
      if ( EVEN(sts)) break;
      if ( cdh_ObjidIsNull( attrref.Objid))
	break;

      if ( dyn->graph->popup_menu_cb) {
	// Display popup menu
	grow_GetName( dyn->graph->grow->ctx, name);

	(dyn->graph->popup_menu_cb)( dyn->graph->parent_ctx, attrref, 
				     xmenu_eItemType_Object,
				     xmenu_mUtility_Ge, name, &popup);
	if ( !popup)
	  break;

	mrm_PositionPopup( popup, dyn->graph->grow_widget, 
			   event->any.x_pixel + 8, event->any.y_pixel);
	XtManageChild(popup);
      }
    }
    break;
  }
  default: ;    
  }
  return 1;
}

int GeTable::export_java( grow_tObject object, ofstream& fp, bool first, char *var_name)
{
  glow_sTableInfo info;

  grow_GetTableInfo( object, &info);
  columns = info.columns;
  rows = info.rows;

  if ( first)
    fp << "      ";
  else
    fp << "      ,";
  fp << "new GeDynTable(" << var_name << ".dd, new String[] {";
  for ( int i = 0; i < columns; i++) {
    if ( i != 0)
      fp << ",";
    fp << "\"" << attribute[i] << "\"";
  }
  fp << "}, new String[] {";
  for ( int i = 0; i < columns; i++) {
    if ( i != 0)
      fp << ",";
    fp << "\"" << format[i] << "\"";
  }
  fp << "}, new String[] {";
  for ( int i = 0; i < columns; i++) {
    if ( i != 0)
      fp << ",";
    fp << "\"" << sel_attribute[i] << "\"";
  }
  fp << "}," << rows << "," << columns << ")" << endl;
  return 1;
}

void GeStatusColor::get_attributes( attr_sItem *attrinfo, int *item_count)
{
  int i = *item_count;

  if ( dyn->total_dyn_type & ge_mDynType_Tone) {
    strcpy( attrinfo[i].name, "StatusTone.Attribute");
    attrinfo[i].value = attribute;
    attrinfo[i].type = glow_eType_String;
    attrinfo[i++].size = sizeof( attribute);

    strcpy( attrinfo[i].name, "StatusTone.NoStatusTone");
    attrinfo[i].value = &nostatus_color;
    attrinfo[i].type = glow_eType_ToneOrColor;
    attrinfo[i++].size = sizeof( nostatus_color);
  }
  else {
    strcpy( attrinfo[i].name, "StatusColor.Attribute");
    attrinfo[i].value = attribute;
    attrinfo[i].type = glow_eType_String;
    attrinfo[i++].size = sizeof( attribute);

    strcpy( attrinfo[i].name, "StatusColor.NoStatusColor");
    attrinfo[i].value = &nostatus_color;
    attrinfo[i].type = glow_eType_Color;
    attrinfo[i++].size = sizeof( nostatus_color);
  }
  *item_count = i;
}

void GeStatusColor::set_attribute( grow_tObject object, char *attr_name, int *cnt)
{
  (*cnt)--;
  if ( *cnt == 0) {
    char msg[200];

    strncpy( attribute, attr_name, sizeof( attribute));
    if ( dyn->total_dyn_type & ge_mDynType_Tone)
      sprintf( msg, "StatusTone.Attribute = %s", attr_name);
    else
      sprintf( msg, "StatusColor.Attribute = %s", attr_name);
    dyn->graph->message( 'I', msg);
  }
}

void GeStatusColor::replace_attribute( char *from, char *to, int *cnt, int strict)
{
  GeDyn::replace_attribute( attribute, sizeof(attribute), from, to, cnt, strict);
}

int GeStatusColor::set_color( grow_tObject object, glow_eDrawType color)
{
  char msg[200];

  if ( dyn->total_dyn_type & ge_mDynType_Tone) {
    this->nostatus_color = glow_eDrawType( color / 30);
    sprintf( msg, "StatusTone.NoStatusTone = %s", grow_ColorToneToName( this->nostatus_color));
  }
  else {
    this->nostatus_color = color;
    sprintf( msg, "StatusColor.NoStatusColor = %s", grow_ColorToName(this->nostatus_color));
  }
  dyn->graph->message( 'I', msg);
  return 1;
}

void GeStatusColor::save( ofstream& fp)
{
  fp << int(ge_eSave_StatusColor) << endl;
  fp << int(ge_eSave_StatusColor_attribute) << FSPACE << attribute << endl;
  fp << int(ge_eSave_StatusColor_nostatus_color) << FSPACE << int(nostatus_color) << endl;
  fp << int(ge_eSave_End) << endl;
}

void GeStatusColor::open( ifstream& fp)
{
  int		type;
  int 		end_found = 0;
  int		tmp;
  char		dummy[40];

  for (;;)
  {
    fp >> type;
    switch( type) {
      case ge_eSave_StatusColor: break;
      case ge_eSave_StatusColor_attribute:
        fp.get();
        fp.getline( attribute, sizeof(attribute));
        break;
      case ge_eSave_StatusColor_nostatus_color: fp >> tmp; nostatus_color = (glow_eDrawType)tmp; break;
      case ge_eSave_End: end_found = 1; break;
      default:
        cout << "GeStatusColor:open syntax error" << endl;
        fp.getline( dummy, sizeof(dummy));
    }
    if ( end_found)
      break;
  }  
}

int GeStatusColor::connect( grow_tObject object, glow_sTraceData *trace_data)
{
  int		attr_size;
  char		parsed_name[120];
  int		sts;
  int		inverted;

  nostatus_color = dyn->get_color1( object, nostatus_color);
  if ( nostatus_color < 0 || nostatus_color >= glow_eDrawType__) {
    printf( "** Color out of range, %s\n", attribute);
    return 0;
  }

  size = 4;
  p = 0;
  db = dyn->graph->parse_attr_name( attribute, parsed_name,
				    &inverted, &attr_type, &attr_size);
  if ( strcmp( parsed_name,"") == 0)
    return 1;

  sts = dyn->graph->ref_object_info( dyn->cycle, parsed_name, (void **)&p, &subid, size);
  if ( EVEN(sts)) return sts;

  if ( p)
    trace_data->p = p;
  first_scan = true;
  return 1;
}

int GeStatusColor::disconnect( grow_tObject object)
{
  if ( p && db == graph_eDatabase_Gdh)
    gdh_UnrefObjectInfo( subid);
  p = 0;
  return 1;
}

int GeStatusColor::scan( grow_tObject object)
{
  if ( !p)
    return 1;

  if ( db == graph_eDatabase_Gdh && attr_type == pwr_eType_NetStatus) {
    pwr_tTime t;
    pwr_tStatus sts;
    pwr_tBoolean old;

    gdh_GetSubscriptionOldness( subid, &old, &t, &sts);
    if ( old)
      *p = PWR__NETTIMEOUT;
  }

  if ( dyn->ignore_color)
    return 1;

  if ( !first_scan && old_status != ge_ePwrStatus_Fatal) {
    if ( old_value == *p && !dyn->reset_color)
      // No change since last time
      return 1;
  }

  old_value = *p;

  ge_ePwrStatus value;
  if ( *p == pwr_cNStatus)
    value = ge_ePwrStatus_No;
  else {
    switch ( *p & 7) {
    case 3:
    case 1:
      value = ge_ePwrStatus_Success;
      break;
    case 0:
      value = ge_ePwrStatus_Warning;
      break;
    case 2:
      value = ge_ePwrStatus_Error;
      break;
    case 4:
      value = ge_ePwrStatus_Fatal;
      break;
    default:
      value = ge_ePwrStatus_No;
    }
  }
  if ( !first_scan && old_status == value && old_status != ge_ePwrStatus_Fatal)
    return 1;
  else
    first_scan = false;

  if ( dyn->total_dyn_type & ge_mDynType_Tone) {
    switch ( value) {
    case ge_ePwrStatus_No:
      if ( nostatus_color >= (glow_eDrawType) glow_eDrawTone__)
	grow_SetObjectFillColor( object, nostatus_color);
      else
	grow_SetObjectColorTone( object, (glow_eDrawTone) nostatus_color);
      break;
    case ge_ePwrStatus_Success:
      if ( nostatus_color >= (glow_eDrawType) glow_eDrawTone__)
	grow_ResetObjectFillColor( object);
      grow_ResetObjectColorTone( object);
      break;
    case ge_ePwrStatus_Warning:
      grow_SetObjectColorTone( object, glow_eDrawTone_Yellow);
      break;
    case ge_ePwrStatus_Error:
      grow_SetObjectColorTone( object, glow_eDrawTone_Red);
      break;
    case ge_ePwrStatus_Fatal:
      on = !on;
      if ( on)
	grow_SetObjectColorTone( object, glow_eDrawTone_Red);
      else {
	if ( nostatus_color >= (glow_eDrawType) glow_eDrawTone__)
	  grow_SetObjectFillColor( object, nostatus_color);
	else
	  grow_SetObjectColorTone( object, (glow_eDrawTone) nostatus_color);
      }
      break;
    }
  }
  else {
    switch ( value) {
    case ge_ePwrStatus_No:
      grow_SetObjectFillColor( object, nostatus_color);
      break;
    case ge_ePwrStatus_Success:
      grow_ResetObjectFillColor( object);
      break;
    case ge_ePwrStatus_Warning:
      grow_SetObjectFillColor( object, glow_eDrawType_ColorYellow);
      break;
    case ge_ePwrStatus_Error:
      grow_SetObjectFillColor( object, glow_eDrawType_LineRed);
      break;
    case ge_ePwrStatus_Fatal:
      on = !on;
      if ( on)
	grow_SetObjectFillColor( object, glow_eDrawType_LineRed);
      else
	grow_SetObjectFillColor( object, nostatus_color);
      break;
    }
  }
  old_status = value;
  return 1;
}

int GeStatusColor::export_java( grow_tObject object, ofstream& fp, bool first, char *var_name)
{
#if 0
  glow_eDrawType jcolor = dyn->get_color1( object, nostatus_color);
  if ( first)
    fp << "      ";
  else
    fp << "      ,";
  fp << "new GeDynStatusColor(" << var_name << ".dd, \"" << attribute << "\"," << jcolor << ")" << endl;
#endif
  return 1;
}

void GeFillLevel::get_attributes( attr_sItem *attrinfo, int *item_count)
{
  int i = *item_count;
  
  strcpy( attrinfo[i].name, "FillLevel.Attribute");
  attrinfo[i].value = attribute;
  attrinfo[i].type = glow_eType_String;
  attrinfo[i++].size = sizeof( attribute);

  if ( dyn->total_dyn_type & ge_mDynType_Tone) {
    strcpy( attrinfo[i].name, "FillLevel.BackgroundTone");
    attrinfo[i].value = &color;
    attrinfo[i].type = glow_eType_ToneOrColor;
    attrinfo[i++].size = sizeof( color);
  }
  else {
    strcpy( attrinfo[i].name, "FillLevel.BackgroundColor");
    attrinfo[i].value = &color;
    attrinfo[i].type = glow_eType_Color;
    attrinfo[i++].size = sizeof( color);
  }
  strcpy( attrinfo[i].name, "FillLevel.Direction");
  attrinfo[i].value = &direction;
  attrinfo[i].type = glow_eType_Direction;
  attrinfo[i++].size = sizeof( direction);

  strcpy( attrinfo[i].name, "FillLevel.MinValue");
  attrinfo[i].value = &min_value;
  attrinfo[i].type = glow_eType_Double;
  attrinfo[i++].size = sizeof( min_value);

  strcpy( attrinfo[i].name, "FillLevel.MaxValue");
  attrinfo[i].value = &max_value;
  attrinfo[i].type = glow_eType_Double;
  attrinfo[i++].size = sizeof( max_value);

  *item_count = i;
}

void GeFillLevel::set_attribute( grow_tObject object, char *attr_name, int *cnt)
{
  (*cnt)--;
  if ( *cnt == 0) {
    char msg[200];

    strncpy( attribute, attr_name, sizeof( attribute));
    sprintf( msg, "FillLevel.Attribute = %s", attr_name);
    dyn->graph->message( 'I', msg);
  }
}

void GeFillLevel::replace_attribute( char *from, char *to, int *cnt, int strict)
{
  GeDyn::replace_attribute( attribute, sizeof(attribute), from, to, cnt, strict);
}

void GeFillLevel::save( ofstream& fp)
{
  fp << int(ge_eSave_FillLevel) << endl;
  fp << int(ge_eSave_FillLevel_attribute) << FSPACE << attribute << endl;
  fp << int(ge_eSave_FillLevel_color) << FSPACE << int(color) << endl;
  fp << int(ge_eSave_FillLevel_direction) << FSPACE << int(direction) << endl;
  fp << int(ge_eSave_FillLevel_max_value) << FSPACE << max_value << endl;
  fp << int(ge_eSave_FillLevel_min_value) << FSPACE << min_value << endl;
  fp << int(ge_eSave_End) << endl;
}

void GeFillLevel::open( ifstream& fp)
{
  int		type;
  int 		end_found = 0;
  int		tmp;
  char		dummy[40];

  for (;;)
  {
    fp >> type;
    switch( type) {
      case ge_eSave_FillLevel: break;
      case ge_eSave_FillLevel_attribute:
        fp.get();
        fp.getline( attribute, sizeof(attribute));
        break;
      case ge_eSave_FillLevel_color: fp >> tmp; color = (glow_eDrawType)tmp; break;
      case ge_eSave_FillLevel_direction: fp >> tmp; direction = (glow_eDirection)tmp; break;
      case ge_eSave_FillLevel_max_value: fp >> max_value; break;
      case ge_eSave_FillLevel_min_value: fp >> min_value; break;
      case ge_eSave_End: end_found = 1; break;
      default:
        cout << "GeFillLevel:open syntax error" << endl;
        fp.getline( dummy, sizeof(dummy));
    }
    if ( end_found)
      break;
  }  
}

int GeFillLevel::connect( grow_tObject object, glow_sTraceData *trace_data)
{
  int		attr_type, attr_size;
  char		parsed_name[120];
  int		sts;
  int		inverted;
  glow_eDirection dir;

  color = dyn->get_color2( object, color);
  if ( color < 0 || color >= glow_eDrawType__) {
    printf( "** Color out of range, %s\n", attribute);
    return 0;
  }

  if ( max_value == min_value)
    return 0;

  size = 4;
  p = 0;
  db = dyn->graph->parse_attr_name( attribute, parsed_name,
				    &inverted, &attr_type, &attr_size);
  if ( strcmp( parsed_name,"") == 0)
    return 1;

  sts = dyn->graph->ref_object_info( dyn->cycle, parsed_name, (void **)&p, &subid, size);
  if ( EVEN(sts)) return sts;

  if ( p)
    trace_data->p = p;
  first_scan = true;

  if ( dyn->total_dyn_type & ge_mDynType_Tone) {
    if ( color >= (glow_eDrawType) glow_eDrawTone__)
      grow_SetObjectLevelFillColor( object, color);
    else
      grow_SetObjectLevelColorTone( object, (glow_eDrawTone)color);
  }
  else
    grow_SetObjectLevelFillColor( object, color);

  sts = grow_GetObjectLimits( object, &limit_min, &limit_max, &dir);
  if ( ODD(sts)) {
    limits_found = true;
    direction = dir;
  }
  grow_SetObjectLevelDirection( object, direction);
  return 1;
}

int GeFillLevel::disconnect( grow_tObject object)
{
  if ( p && db == graph_eDatabase_Gdh)
    gdh_UnrefObjectInfo( subid);
  p = 0;
  return 1;
}

int GeFillLevel::scan( grow_tObject object)
{
  
  if ( !first_scan) {
    if ( fabs( old_value - *p) < FLT_EPSILON)
      // No change since last time
      return 1;
  }
  else
    first_scan = false;

  double value;
  if ( !limits_found)
    value = (*p - min_value) / (max_value - min_value);
  else {
    double		ll_x, ll_y, ur_x, ur_y;

    grow_MeasureNode( object, &ll_x, &ll_y, &ur_x, &ur_y);
    
    switch ( direction) {
    case glow_eDirection_Right:
      value = ((*p - min_value) / (max_value - min_value) * ( limit_max - limit_min) 
	       + (limit_min - ll_x)) / (ur_x - ll_x);
      break;
    case glow_eDirection_Left:
      value = ((*p - min_value) / (max_value - min_value) * ( limit_max - limit_min) 
	       + (ur_x - limit_max)) / (ur_x - ll_x);
      break;
    case glow_eDirection_Up:
      value = ((*p - min_value) / (max_value - min_value) * ( limit_max - limit_min) 
	       + (limit_min - ll_y)) / (ur_y - ll_y);
      break;
    case glow_eDirection_Down:
      value = ((*p - min_value) / (max_value - min_value) * ( limit_max - limit_min) 
	       + (ur_y - limit_max)) / (ur_y - ll_y);
      break;
    default: ;
    }
  }
  grow_SetObjectFillLevel( object, value);
  old_value = *p;
  return 1;
}

int GeFillLevel::export_java( grow_tObject object, ofstream& fp, bool first, char *var_name)
{
  glow_eDrawType jcolor = dyn->get_color2( object, color);
  int sts;
  double min_limit, max_limit;
  glow_eDirection dir;

  sts = grow_GetObjectLimitsPixel( object, &min_limit, &max_limit, &dir);
  if ( EVEN(sts)) {
    min_limit = max_limit = 0;
    dir = direction;
  }

  if ( first)
    fp << "      ";
  else
    fp << "      ,";
  fp << "new GeDynFillLevel(" << var_name << ".dd, \"" << attribute << "\"," << jcolor << "," <<
    dir << "," << min_value << "," << max_value << "," << min_limit << "," << max_limit << ")" << endl;

  return 1;
}

void GePopupMenu::get_attributes( attr_sItem *attrinfo, int *item_count)
{
  int i = *item_count;

  strcpy( attrinfo[i].name, "PopupMenu.ReferenceObject");
  attrinfo[i].value = ref_object;
  attrinfo[i].type = glow_eType_String;
  attrinfo[i++].size = sizeof( ref_object);

  dyn->display_access = true;
  *item_count = i;
}

void GePopupMenu::set_attribute( grow_tObject object, char *attr_name, int *cnt)
{
  (*cnt)--;
  if ( *cnt == 0) {
    char msg[200];
    char *s;

    strncpy( ref_object, attr_name, sizeof( ref_object));
    if ( (s = strchr( ref_object, '.')))
      *s = 0;
    sprintf( msg, "PopupMenu.ReferenceObject = %s", ref_object);
    dyn->graph->message( 'I', msg);
  }
}

void GePopupMenu::replace_attribute( char *from, char *to, int *cnt, int strict)
{
  GeDyn::replace_attribute( ref_object, sizeof(ref_object), from, to, cnt, strict);
}

void GePopupMenu::save( ofstream& fp)
{
  fp << int(ge_eSave_PopupMenu) << endl;
  fp << int(ge_eSave_PopupMenu_ref_object) << FSPACE << ref_object << endl;
  fp << int(ge_eSave_End) << endl;
}

void GePopupMenu::open( ifstream& fp)
{
  int		type;
  int 		end_found = 0;
  char		dummy[40];

  for (;;)
  {
    fp >> type;
    switch( type) {
      case ge_eSave_PopupMenu: break;
      case ge_eSave_PopupMenu_ref_object:
        fp.get();
        fp.getline( ref_object, sizeof(ref_object));
        break;
      case ge_eSave_End: end_found = 1; break;
      default:
        cout << "GePopupMenu:open syntax error" << endl;
        fp.getline( dummy, sizeof(dummy));
    }
    if ( end_found)
      break;
  }  
}

int GePopupMenu::action( grow_tObject object, glow_tEvent event)
{
  if ( !dyn->graph->is_authorized( dyn->access))
    return 1;

  switch ( event->event) {
  case glow_eEvent_MB3Press: {
    int			sts;
    char       		parsed_name[120];
    int			inverted;
    int			attr_type, attr_size;
    char            	*s;
    pwr_sAttrRef    	attrref;
    char            	name[80];
    Widget          	popup;
    int			reference = 0;

    if ( ref_object[0] == '&') {
      // Refobject starting with '&' indicates reference
      dyn->graph->parse_attr_name( &ref_object[1], parsed_name, &inverted,
				 &attr_type, &attr_size);
      reference = 1;
    }
    else {
      dyn->graph->parse_attr_name( ref_object, parsed_name, &inverted,
				 &attr_type, &attr_size);
      if ( inverted) // Old syntax for reference
	reference = 1;
    }
    if ( reference) {
      // The ref_object is an objid-attribute that containts the object 
      memset( &attrref, 0, sizeof(attrref));
      sts = gdh_GetObjectInfo( parsed_name, &attrref.Objid, 
				 sizeof(attrref.Objid));
      if ( EVEN(sts)) break;
      if ( cdh_ObjidIsNull( attrref.Objid))
	break;
    }
    else {
      if ( (s = strrchr( parsed_name, '.')))
	*s = 0;

      memset( &attrref, 0, sizeof(attrref));
      sts = gdh_NameToObjid( parsed_name, &attrref.Objid);
      if ( EVEN(sts)) break;
    }
    if ( dyn->graph->popup_menu_cb) {
      // Display popup menu
      grow_GetName( dyn->graph->grow->ctx, name);

      (dyn->graph->popup_menu_cb)( dyn->graph->parent_ctx, attrref, 
			      xmenu_eItemType_Object,
			      xmenu_mUtility_Ge, name, &popup);
      if ( !popup)
	break;

      mrm_PositionPopup( popup, dyn->graph->grow_widget, 
			 event->any.x_pixel + 8, event->any.y_pixel);
      XtManageChild(popup);
    }
    break;
  }
  case glow_eEvent_Key_CtrlAscii: {
    char method[40];
    char filter[40];

    switch ( event->key.ascii) {
    case 1:
      strcpy( method, "$Object-OpenObject");
      strcpy( filter, "$Object-OpenObjectFilter");
      break;
    case 7:
      strcpy( method, "$Object-OpenClassGraph");
      strcpy( filter, "$Object-OpenClassGraphFilter");
      break;
    case 18:
      strcpy( method, "$Object-OpenCrossref");
      strcpy( filter, "$Object-OpenCrossrefFilter");
      break;
    case 4:
      strcpy( method, "$Object-RtNavigator");
      strcpy( filter, "$Object-RtNavigatorFilter");
      break;
    case 12:
      strcpy( method, "$Object-OpenTrace");
      strcpy( filter, "$Object-OpenTraceFilter");
      break;
    case 8:
      strcpy( method, "$Object-Help");
      strcpy( filter, "$Object-HelpFilter");
      break;
    case 10:
      strcpy( method, "$Object-HelpClass");
      strcpy( filter, "$Object-HelpClassFilter");
      break;
    default:
      ;
    }

    if ( dyn->graph->call_method_cb) {
      int	       	sts;
      char              parsed_name[120];
      int	       	inverted;
      int	       	attr_type, attr_size;
      char            	*s;
      pwr_sAttrRef    	attrref;

      dyn->graph->parse_attr_name( ref_object, parsed_name, &inverted,
				   &attr_type, &attr_size);
      if ( inverted) {
	// The ref_object is an objid-attribute that containts the object 
	memset( &attrref, 0, sizeof(attrref));
	sts = gdh_GetObjectInfo( parsed_name, &attrref.Objid, 
				 sizeof(attrref.Objid));
	if ( EVEN(sts)) break;
	if ( cdh_ObjidIsNull( attrref.Objid))
	  break;
      }
      else {
	if ( (s = strrchr( parsed_name, '.')))
	  *s = 0;
	
	memset( &attrref, 0, sizeof(attrref));
	sts = gdh_NameToObjid( parsed_name, &attrref.Objid);
	if ( EVEN(sts)) break;
      }
      (dyn->graph->call_method_cb)(dyn->graph->parent_ctx,
				   method, filter, attrref, 
				   xmenu_eItemType_Object,
				   xmenu_mUtility_Ge, NULL);
    }
    break;
  }  
  default: ;    
  }
  return 1;
}

int GePopupMenu::export_java( grow_tObject object, ofstream& fp, bool first, char *var_name)
{
  if ( first)
    fp << "      ";
  else
    fp << "      ,";
  fp << "new GeDynPopupMenu(" << var_name << ".dd, \"" << ref_object << "\")" << endl;
  return 1;
}

void GeSetDig::get_attributes( attr_sItem *attrinfo, int *item_count)
{
  int i = *item_count;

  if ( instance == ge_mInstance_1) {
    strcpy( attrinfo[i].name, "SetDig.Attribute");
    attrinfo[i].value = attribute;
    attrinfo[i].type = glow_eType_String;
    attrinfo[i++].size = sizeof( attribute);

    strcpy( attrinfo[i].name, "SetDig.Instances");
    attrinfo[i].value = &instance_mask;
    attrinfo[i].type = ge_eAttrType_InstanceMask;
    attrinfo[i++].size = sizeof( instance_mask);
  }
  else {
    // Get instance number
    int inst = 1;
    int m = instance;
    while( m > 1) {
      m = m >> 1;
      inst++;
    }

    sprintf( attrinfo[i].name, "SetDig%d.Attribute", inst);
    attrinfo[i].value = attribute;
    attrinfo[i].type = glow_eType_String;
    attrinfo[i++].size = sizeof( attribute);
  }

  dyn->display_access = true;
  *item_count = i;
}

int GeSetDig::get_transtab( char **tt)
{
  static char transtab[][32] = {	"SubGraph",		"SubGraph",
					"A1",			"Text",
					"Dynamic",		"",
					""};

  *tt = (char *) transtab;
  return 0;
}

void GeSetDig::set_attribute( grow_tObject object, char *attr_name, int *cnt)
{
  (*cnt)--;
  if ( *cnt == 0) {
    char msg[200];

    strncpy( attribute, attr_name, sizeof( attribute));
    if ( instance == ge_mInstance_1)
      sprintf( msg, "SetDig.Attribute = %s", attr_name);
    else
      sprintf( msg, "SetDig%d.Attribute = %s", GeDyn::instance_to_number( instance),
	       attr_name);
    dyn->graph->message( 'I', msg);
  }
}

void GeSetDig::replace_attribute( char *from, char *to, int *cnt, int strict)
{
  GeDyn::replace_attribute( attribute, sizeof(attribute), from, to, cnt, strict);
}

void GeSetDig::save( ofstream& fp)
{
  fp << int(ge_eSave_SetDig) << endl;
  fp << int(ge_eSave_SetDig_attribute) << FSPACE << attribute << endl;
  fp << int(ge_eSave_SetDig_instance) << FSPACE << int(instance) << endl;
  fp << int(ge_eSave_SetDig_instance_mask) << FSPACE << int(instance_mask) << endl;
  fp << int(ge_eSave_End) << endl;
}

void GeSetDig::open( ifstream& fp)
{
  int		type;
  int 		end_found = 0;
  char		dummy[40];
  int		tmp;

  for (;;)
  {
    fp >> type;
    switch( type) {
      case ge_eSave_SetDig: break;
      case ge_eSave_SetDig_attribute:
        fp.get();
        fp.getline( attribute, sizeof(attribute));
        break;
      case ge_eSave_SetDig_instance: fp >> tmp; instance = (ge_mInstance)tmp; break;
      case ge_eSave_SetDig_instance_mask: fp >> tmp; instance_mask = (ge_mInstance)tmp; break;
      case ge_eSave_End: end_found = 1; break;
      default:
        cout << "GeSetDig:open syntax error" << endl;
        fp.getline( dummy, sizeof(dummy));
    }
    if ( end_found)
      break;
  }  
}

int GeSetDig::action( grow_tObject object, glow_tEvent event)
{
  if ( !dyn->graph->is_authorized( dyn->access))
    return 1;

  switch ( event->event) {
  case glow_eEvent_MB1Down:
    grow_SetClickSensitivity( dyn->graph->grow->ctx, glow_mSensitivity_MB1Click);
    grow_SetObjectColorInverse( object, 1);
    break;
  case glow_eEvent_MB1Up:
    grow_SetObjectColorInverse( object, 0);
    break;
  case glow_eEvent_Key_Return:
  case glow_eEvent_MB1Click: {
    pwr_tBoolean	value = 1;
    int			sts;
    char       		parsed_name[120];
    int			inverted;
    int			attr_type, attr_size;
    graph_eDatabase 	db;
    
    if ( dyn->total_action_type & ge_mActionType_Confirm)
      break;

    db = dyn->graph->parse_attr_name( attribute, parsed_name, &inverted, &attr_type, 
				      &attr_size);
    switch ( db) {
    case graph_eDatabase_Local:
      sts = dyn->graph->localdb_set_value( parsed_name, &value, sizeof(value));
      if ( EVEN(sts)) printf("SetDig error: %s\n", attribute);
      break;
    case graph_eDatabase_Gdh:
      sts = gdh_SetObjectInfo( parsed_name, &value, sizeof(value));
      if ( EVEN(sts)) printf("SetDig error: %s\n", attribute);
      break;
    default:
      ;
    }
    break;
  }
  default: ;    
  }
  return 1;
}

int GeSetDig::export_java( grow_tObject object, ofstream& fp, bool first, char *var_name)
{
  if ( first)
    fp << "      ";
  else
    fp << "      ,";
  fp << "new GeDynSetDig(" << var_name << ".dd, \"" << attribute << "\")" << endl;
  return 1;
}

void GeResetDig::get_attributes( attr_sItem *attrinfo, int *item_count)
{
  int i = *item_count;

  if ( instance == ge_mInstance_1) {
    strcpy( attrinfo[i].name, "ResetDig.Attribute");
    attrinfo[i].value = attribute;
    attrinfo[i].type = glow_eType_String;
    attrinfo[i++].size = sizeof( attribute);

    strcpy( attrinfo[i].name, "ResetDig.Instances");
    attrinfo[i].value = &instance_mask;
    attrinfo[i].type = ge_eAttrType_InstanceMask;
    attrinfo[i++].size = sizeof( instance_mask);
  }
  else {
    // Get instance number
    int inst = 1;
    int m = instance;
    while( m > 1) {
      m = m >> 1;
      inst++;
    }

    sprintf( attrinfo[i].name, "ResetDig%d.Attribute", inst);
    attrinfo[i].value = attribute;
    attrinfo[i].type = glow_eType_String;
    attrinfo[i++].size = sizeof( attribute);
  }

  dyn->display_access = true;
  *item_count = i;
}

int GeResetDig::get_transtab( char **tt)
{
  static char transtab[][32] = {	"SubGraph",		"SubGraph",
					"A1",			"Text",
					"Dynamic",		"",
					""};

  *tt = (char *) transtab;
  return 0;
}

void GeResetDig::set_attribute( grow_tObject object, char *attr_name, int *cnt)
{
  (*cnt)--;
  if ( *cnt == 0) {
    char msg[200];

    strncpy( attribute, attr_name, sizeof( attribute));
    if ( instance == ge_mInstance_1)
      sprintf( msg, "ResetDig.Attribute = %s", attr_name);
    else
      sprintf( msg, "ResetDig%d.Attribute = %s", GeDyn::instance_to_number( instance),
	       attr_name);
    dyn->graph->message( 'I', msg);
  }
}

void GeResetDig::replace_attribute( char *from, char *to, int *cnt, int strict)
{
  GeDyn::replace_attribute( attribute, sizeof(attribute), from, to, cnt, strict);
}

void GeResetDig::save( ofstream& fp)
{
  fp << int(ge_eSave_ResetDig) << endl;
  fp << int(ge_eSave_ResetDig_attribute) << FSPACE << attribute << endl;
  fp << int(ge_eSave_ResetDig_instance) << FSPACE << int(instance) << endl;
  fp << int(ge_eSave_ResetDig_instance_mask) << FSPACE << int(instance_mask) << endl;
  fp << int(ge_eSave_End) << endl;
}

void GeResetDig::open( ifstream& fp)
{
  int		type;
  int 		end_found = 0;
  char		dummy[40];
  int		tmp;

  for (;;)
  {
    fp >> type;
    switch( type) {
      case ge_eSave_ResetDig: break;
      case ge_eSave_ResetDig_attribute:
        fp.get();
        fp.getline( attribute, sizeof(attribute));
        break;
      case ge_eSave_End: end_found = 1; break;
      case ge_eSave_ResetDig_instance: fp >> tmp; instance = (ge_mInstance)tmp; break;
      case ge_eSave_ResetDig_instance_mask: fp >> tmp; instance_mask = (ge_mInstance)tmp; break;
      default:
        cout << "GeResetDig:open syntax error" << endl;
        fp.getline( dummy, sizeof(dummy));
    }
    if ( end_found)
      break;
  }  
}

int GeResetDig::action( grow_tObject object, glow_tEvent event)
{
  if ( !dyn->graph->is_authorized( dyn->access))
    return 1;

  switch ( event->event) {
  case glow_eEvent_MB1Down:
    grow_SetClickSensitivity( dyn->graph->grow->ctx, glow_mSensitivity_MB1Click);
    grow_SetObjectColorInverse( object, 1);
    break;
  case glow_eEvent_MB1Up:
    grow_SetObjectColorInverse( object, 0);
    break;
  case glow_eEvent_Key_Return:
  case glow_eEvent_MB1Click: {
    pwr_tBoolean	value = 0;
    int			sts;
    char       		parsed_name[120];
    int			inverted;
    int			attr_type, attr_size;
    
    if ( dyn->total_action_type & ge_mActionType_Confirm)
      break;

    dyn->graph->parse_attr_name( attribute, parsed_name, &inverted, &attr_type, &attr_size);
    sts = gdh_SetObjectInfo( parsed_name, &value, sizeof(value));
    if ( EVEN(sts)) printf("ResetDig error: %s\n", attribute);
    break;
  }
  default: ;    
  }
  return 1;
}

int GeResetDig::export_java( grow_tObject object, ofstream& fp, bool first, char *var_name)
{
  if ( first)
    fp << "      ";
  else
    fp << "      ,";
  fp << "new GeDynResetDig(" << var_name << ".dd, \"" << attribute << "\")" << endl;
  return 1;
}

void GeToggleDig::get_attributes( attr_sItem *attrinfo, int *item_count)
{
  int i = *item_count;

  strcpy( attrinfo[i].name, "ToggleDig.Attribute");
  attrinfo[i].value = attribute;
  attrinfo[i].type = glow_eType_String;
  attrinfo[i++].size = sizeof( attribute);

  dyn->display_access = true;
  *item_count = i;
}

int GeToggleDig::get_transtab( char **tt)
{
  static char transtab[][32] = {	"SubGraph",		"SubGraph",
					"A1",			"Text",
					"Dynamic",		"",
					""};

  *tt = (char *) transtab;
  return 0;
}

void GeToggleDig::set_attribute( grow_tObject object, char *attr_name, int *cnt)
{
  (*cnt)--;
  if ( *cnt == 0) {
    char msg[200];

    strncpy( attribute, attr_name, sizeof( attribute));
    sprintf( msg, "ToggleDig.Attribute = %s", attr_name);
    dyn->graph->message( 'I', msg);
  }
}

void GeToggleDig::replace_attribute( char *from, char *to, int *cnt, int strict)
{
  GeDyn::replace_attribute( attribute, sizeof(attribute), from, to, cnt, strict);
}

void GeToggleDig::save( ofstream& fp)
{
  fp << int(ge_eSave_ToggleDig) << endl;
  fp << int(ge_eSave_ToggleDig_attribute) << FSPACE << attribute << endl;
  fp << int(ge_eSave_End) << endl;
}

void GeToggleDig::open( ifstream& fp)
{
  int		type;
  int 		end_found = 0;
  char		dummy[40];

  for (;;)
  {
    fp >> type;
    switch( type) {
      case ge_eSave_ToggleDig: break;
      case ge_eSave_ToggleDig_attribute:
        fp.get();
        fp.getline( attribute, sizeof(attribute));
        break;
      case ge_eSave_End: end_found = 1; break;
      default:
        cout << "GeToggleDig:open syntax error" << endl;
        fp.getline( dummy, sizeof(dummy));
    }
    if ( end_found)
      break;
  }  
}

int GeToggleDig::action( grow_tObject object, glow_tEvent event)
{
  if ( !dyn->graph->is_authorized( dyn->access))
    return 1;

  switch ( event->event) {
  case glow_eEvent_MB1Down:
    grow_SetClickSensitivity( dyn->graph->grow->ctx, glow_mSensitivity_MB1Click);
    grow_SetObjectColorInverse( object, 1);
    break;
  case glow_eEvent_MB1Up:
    grow_SetObjectColorInverse( object, 0);
    break;
  case glow_eEvent_Key_Return:
  case glow_eEvent_MB1Click: {
    pwr_tBoolean	value;
    int			sts;
    char       		parsed_name[120];
    int			inverted;
    int			attr_type, attr_size;
    
    if ( dyn->total_action_type & ge_mActionType_Confirm)
      break;

    dyn->graph->parse_attr_name( attribute, parsed_name, &inverted, &attr_type, &attr_size);
    sts = gdh_GetObjectInfo( parsed_name, &value, sizeof(value));
    if ( EVEN(sts)) {
      printf("ToggleDig error: %s\n", attribute);
      break;
    }
    value = !value;
    sts = gdh_SetObjectInfo( parsed_name, &value, sizeof(value));
    if ( EVEN(sts)) printf("ToggleDig error: %s\n", attribute);
    break;
  }
  default: ;    
  }
  return 1;
}

int GeToggleDig::export_java( grow_tObject object, ofstream& fp, bool first, char *var_name)
{
  if ( first)
    fp << "      ";
  else
    fp << "      ,";
  fp << "new GeDynToggleDig(" << var_name << ".dd, \"" << attribute << "\")" << endl;
  return 1;
}

void GeStoDig::get_attributes( attr_sItem *attrinfo, int *item_count)
{
  int i = *item_count;

  strcpy( attrinfo[i].name, "StoDig.Attribute");
  attrinfo[i].value = attribute;
  attrinfo[i].type = glow_eType_String;
  attrinfo[i++].size = sizeof( attribute);

  dyn->display_access = true;
  *item_count = i;
}

int GeStoDig::get_transtab( char **tt)
{
  static char transtab[][32] = {	"SubGraph",		"SubGraph",
					"A1",			"Text",
					"Dynamic",		"",
					""};

  *tt = (char *) transtab;
  return 0;
}

void GeStoDig::set_attribute( grow_tObject object, char *attr_name, int *cnt)
{
  (*cnt)--;
  if ( *cnt == 0) {
    char msg[200];

    strncpy( attribute, attr_name, sizeof( attribute));
    sprintf( msg, "StoDig.Attribute = %s", attr_name);
    dyn->graph->message( 'I', msg);
  }
}

void GeStoDig::replace_attribute( char *from, char *to, int *cnt, int strict)
{
  GeDyn::replace_attribute( attribute, sizeof(attribute), from, to, cnt, strict);
}

void GeStoDig::save( ofstream& fp)
{
  fp << int(ge_eSave_StoDig) << endl;
  fp << int(ge_eSave_StoDig_attribute) << FSPACE << attribute << endl;
  fp << int(ge_eSave_End) << endl;
}

void GeStoDig::open( ifstream& fp)
{
  int		type;
  int 		end_found = 0;
  char		dummy[40];

  for (;;)
  {
    fp >> type;
    switch( type) {
      case ge_eSave_StoDig: break;
      case ge_eSave_StoDig_attribute:
        fp.get();
        fp.getline( attribute, sizeof(attribute));
        break;
      case ge_eSave_End: end_found = 1; break;
      default:
        cout << "GeStoDig:open syntax error" << endl;
        fp.getline( dummy, sizeof(dummy));
    }
    if ( end_found)
      break;
  }  
}

int GeStoDig::action( grow_tObject object, glow_tEvent event)
{
  pwr_tBoolean	value = 0;
  int			sts;
  char       		parsed_name[120];
  int			inverted;
  int			attr_type, attr_size;

  if ( !dyn->graph->is_authorized( dyn->access))
    return 1;

  switch ( event->event) {
  case glow_eEvent_MB1Down:
    grow_SetObjectColorInverse( object, 1);
    value = 1;
    dyn->graph->parse_attr_name( attribute, parsed_name, &inverted, &attr_type, &attr_size);
    sts = gdh_SetObjectInfo( parsed_name, &value, sizeof(value));
    if ( EVEN(sts)) {
      printf("StoDig error: %s\n", attribute);
      break;
    }
    break;
  case glow_eEvent_MB1Up:
    grow_SetObjectColorInverse( object, 0);
    value = 0;
    dyn->graph->parse_attr_name( attribute, parsed_name, &inverted, &attr_type, &attr_size);
    sts = gdh_SetObjectInfo( parsed_name, &value, sizeof(value));
    if ( EVEN(sts)) break;

    break;
  default: ;    
  }
  return 1;
}

int GeStoDig::export_java( grow_tObject object, ofstream& fp, bool first, char *var_name)
{
  if ( first)
    fp << "      ";
  else
    fp << "      ,";
  fp << "new GeDynStoDig(" << var_name << ".dd, \"" << attribute << "\")" << endl;
  return 1;
}

void GeCommand::get_attributes( attr_sItem *attrinfo, int *item_count)
{
  int i = *item_count;

  strcpy( attrinfo[i].name, "Command.Command");
  attrinfo[i].value = command;
  attrinfo[i].type = glow_eType_String;
  attrinfo[i++].size = sizeof( command);

  dyn->display_access = true;
  *item_count = i;
}

int GeCommand::get_transtab( char **tt)
{
  static char transtab[][32] = {	"SubGraph",		"SubGraph",
					"A1",			"Text",
					"Dynamic",		"",
					""};

  *tt = (char *) transtab;
  return 0;
}

void GeCommand::save( ofstream& fp)
{
  fp << int(ge_eSave_Command) << endl;
  fp << int(ge_eSave_Command_command) << FSPACE << command << endl;
  fp << int(ge_eSave_End) << endl;
}

void GeCommand::open( ifstream& fp)
{
  int		type;
  int 		end_found = 0;
  char		dummy[40];

  for (;;)
  {
    fp >> type;
    switch( type) {
      case ge_eSave_Command: break;
      case ge_eSave_Command_command:
        fp.get();
        fp.getline( command, sizeof(command));
        break;
      case ge_eSave_End: end_found = 1; break;
      default:
        cout << "GeCommand:open syntax error" << endl;
        fp.getline( dummy, sizeof(dummy));
    }
    if ( end_found)
      break;
  }  
}

int GeCommand::action( grow_tObject object, glow_tEvent event)
{
  if ( !dyn->graph->is_authorized( dyn->access))
    return 1;

  switch ( event->event) {
  case glow_eEvent_MB1Down:
    grow_SetClickSensitivity( dyn->graph->grow->ctx, glow_mSensitivity_MB1Click);
    grow_SetObjectColorInverse( object, 1);
    break;
  case glow_eEvent_MB1Up:
    grow_SetObjectColorInverse( object, 0);
    break;
  case glow_eEvent_Key_Return:
  case glow_eEvent_MB1Click:
    if ( dyn->total_action_type & ge_mActionType_Confirm)
      break;

    if ( dyn->graph->command_cb) {
      char cmd[200];

      dyn->graph->get_command( command, cmd);
      (dyn->graph->command_cb)( dyn->graph->parent_ctx, cmd);
    }
    break;
  default: ;    
  }
  return 1;
}

int GeCommand::export_java( grow_tObject object, ofstream& fp, bool first, char *var_name)
{
  if ( first)
    fp << "      ";
  else
    fp << "      ,";
  fp << "new GeDynCommand(" << var_name << ".dd, \"" << GeDyn::cmd_cnv( command) << "\")" << endl;
  return 1;
}

void GeCommandDoubleClick::get_attributes( attr_sItem *attrinfo, int *item_count)
{
  int i = *item_count;

  strcpy( attrinfo[i].name, "CommandDoubleClick.Command");
  attrinfo[i].value = command;
  attrinfo[i].type = glow_eType_String;
  attrinfo[i++].size = sizeof( command);

  dyn->display_access = true;
  *item_count = i;
}

int GeCommandDoubleClick::get_transtab( char **tt)
{
  static char transtab[][32] = {	"SubGraph",		"SubGraph",
					"A1",			"Text",
					"Dynamic",		"",
					""};

  *tt = (char *) transtab;
  return 0;
}

void GeCommandDoubleClick::save( ofstream& fp)
{
  fp << int(ge_eSave_CommandDC) << endl;
  fp << int(ge_eSave_CommandDC_command) << FSPACE << command << endl;
  fp << int(ge_eSave_End) << endl;
}

void GeCommandDoubleClick::open( ifstream& fp)
{
  int		type;
  int 		end_found = 0;
  char		dummy[40];

  for (;;)
  {
    fp >> type;
    switch( type) {
      case ge_eSave_CommandDC: break;
      case ge_eSave_CommandDC_command:
        fp.get();
        fp.getline( command, sizeof(command));
        break;
      case ge_eSave_End: end_found = 1; break;
      default:
        cout << "GeCommandDC:open syntax error" << endl;
        fp.getline( dummy, sizeof(dummy));
    }
    if ( end_found)
      break;
  }  
}

int GeCommandDoubleClick::action( grow_tObject object, glow_tEvent event)
{
  if ( !dyn->graph->is_authorized( dyn->access))
    return 1;

  switch ( event->event) {
  case glow_eEvent_MB1Down:
    grow_SetClickSensitivity( dyn->graph->grow->ctx, 
			glow_mSensitivity_MB1Click | glow_mSensitivity_MB1DoubleClick);
    grow_SetObjectColorInverse( object, 1);
    break;
  case glow_eEvent_MB1Up:
    grow_SetObjectColorInverse( object, 0);
    break;
  case glow_eEvent_MB1DoubleClick:
    if ( dyn->graph->command_cb) {
      char cmd[200];

      dyn->graph->get_command( command, cmd);
      (dyn->graph->command_cb)( dyn->graph->parent_ctx, cmd);
    }
    break;
  default: ;    
  }
  return 1;
}

void GeConfirm::get_attributes( attr_sItem *attrinfo, int *item_count)
{
  int i = *item_count;

  strcpy( attrinfo[i].name, "Confirm.Text");
  attrinfo[i].value = text;
  attrinfo[i].type = glow_eType_String;
  attrinfo[i++].size = sizeof( text);

  dyn->display_access = true;
  *item_count = i;
}

void GeConfirm::save( ofstream& fp)
{
  fp << int(ge_eSave_Confirm) << endl;
  fp << int(ge_eSave_Confirm_text) << FSPACE << text << endl;
  fp << int(ge_eSave_End) << endl;
}

void GeConfirm::open( ifstream& fp)
{
  int		type;
  int 		end_found = 0;
  char		dummy[40];

  for (;;)
  {
    fp >> type;
    switch( type) {
      case ge_eSave_Confirm: break;
      case ge_eSave_Confirm_text:
        fp.get();
        fp.getline( text, sizeof(text));
        break;
      case ge_eSave_End: end_found = 1; break;
      default:
        cout << "GeConfirm:open syntax error" << endl;
        fp.getline( dummy, sizeof(dummy));
    }
    if ( end_found)
      break;
  }  
}

int GeConfirm::action( grow_tObject object, glow_tEvent event)
{
  if ( !dyn->graph->is_authorized( dyn->access))
    return 1;

  if ( !(dyn->total_action_type & ge_mActionType_Confirm))
    return 1;

  switch ( event->event) {
  case glow_eEvent_Key_Return:
  case glow_eEvent_MB1Click: {
    if ( dyn->total_action_type & ge_mActionType_ValueInput)
      return 1;
    if ( dyn->graph->confirm_cb) {
      (dyn->graph->confirm_cb)( dyn->graph->parent_ctx, 
			   object, text);
    }
    break;
  }
  default: ;    
  }
  return 1;
}

int GeConfirm::export_java( grow_tObject object, ofstream& fp, bool first, char *var_name)
{
  if ( first)
    fp << "      ";
  else
    fp << "      ,";
  fp << "new GeDynConfirm(" << var_name << ".dd, \"" << text << "\")" << endl;
  return 1;
}

void GeIncrAnalog::get_attributes( attr_sItem *attrinfo, int *item_count)
{
  int i = *item_count;

  strcpy( attrinfo[i].name, "IncrAnalog.Attribute");
  attrinfo[i].value = attribute;
  attrinfo[i].type = glow_eType_String;
  attrinfo[i++].size = sizeof( attribute);

  strcpy( attrinfo[i].name, "IncrAnalog.Increment");
  attrinfo[i].value = &increment;
  attrinfo[i].type = glow_eType_Double;
  attrinfo[i++].size = sizeof( increment);

  strcpy( attrinfo[i].name, "IncrAnalog.MinValue");
  attrinfo[i].value = &min_value;
  attrinfo[i].type = glow_eType_Double;
  attrinfo[i++].size = sizeof( min_value);

  strcpy( attrinfo[i].name, "IncrAnalog.MaxValue");
  attrinfo[i].value = &max_value;
  attrinfo[i].type = glow_eType_Double;
  attrinfo[i++].size = sizeof( max_value);

  dyn->display_access = true;
  *item_count = i;
}

void GeIncrAnalog::set_attribute( grow_tObject object, char *attr_name, int *cnt)
{
  (*cnt)--;
  if ( *cnt == 0) {
    char msg[200];

    strncpy( attribute, attr_name, sizeof( attribute));
    sprintf( msg, "IncrAnalog.Attribute = %s", attr_name);
    dyn->graph->message( 'I', msg);
  }
}

void GeIncrAnalog::replace_attribute( char *from, char *to, int *cnt, int strict)
{
  GeDyn::replace_attribute( attribute, sizeof(attribute), from, to, cnt, strict);
}

void GeIncrAnalog::save( ofstream& fp)
{
  fp << int(ge_eSave_IncrAnalog) << endl;
  fp << int(ge_eSave_IncrAnalog_attribute) << FSPACE << attribute << endl;
  fp << int(ge_eSave_IncrAnalog_increment) << FSPACE << increment << endl;
  fp << int(ge_eSave_IncrAnalog_min_value) << FSPACE << min_value << endl;
  fp << int(ge_eSave_IncrAnalog_max_value) << FSPACE << max_value << endl;
  fp << int(ge_eSave_End) << endl;
}

void GeIncrAnalog::open( ifstream& fp)
{
  int		type;
  int 		end_found = 0;
  char		dummy[40];

  for (;;)
  {
    fp >> type;
    switch( type) {
      case ge_eSave_IncrAnalog: break;
      case ge_eSave_IncrAnalog_attribute:
        fp.get();
        fp.getline( attribute, sizeof(attribute));
        break;
      case ge_eSave_IncrAnalog_increment: fp >> increment; break;
      case ge_eSave_IncrAnalog_min_value: fp >> min_value; break;
      case ge_eSave_IncrAnalog_max_value: fp >> max_value; break;
      case ge_eSave_End: end_found = 1; break;
      default:
        cout << "GeIncrAnalog:open syntax error" << endl;
        fp.getline( dummy, sizeof(dummy));
    }
    if ( end_found)
      break;
  }
}

int GeIncrAnalog::action( grow_tObject object, glow_tEvent event)
{
  if ( !dyn->graph->is_authorized( dyn->access))
    return 1;

  switch ( event->event) {
  case glow_eEvent_MB1Down:
    grow_SetClickSensitivity( dyn->graph->grow->ctx, glow_mSensitivity_MB1Click);
    grow_SetObjectColorInverse( object, 1);
    break;
  case glow_eEvent_MB1Up:
    grow_SetObjectColorInverse( object, 0);
    break;
  case glow_eEvent_Key_Return:
  case glow_eEvent_MB1Click: {
    int			sts;
    char       		parsed_name[120];
    int			inverted;
    int			attr_type, attr_size;
    pwr_tFloat32 	value;

    dyn->graph->parse_attr_name( attribute, parsed_name, &inverted, &attr_type, &attr_size);
    sts = gdh_GetObjectInfo( parsed_name, &value, sizeof(value));
    if ( EVEN(sts)) {
      printf("IncrAnalog error: %s\n", attribute);
      break;
    }

    value += increment;
    if ( !( min_value == 0 && max_value == 0)) {
      value = MAX( value, min_value);
      value = MIN( value, max_value);
    }
    sts = gdh_SetObjectInfo( parsed_name, &value, sizeof(value));
    if ( EVEN(sts)) printf("IncrAnalog error: %s\n", attribute);

    break;
  }
  default: ;    
  }
  return 1;
}

int GeIncrAnalog::export_java( grow_tObject object, ofstream& fp, bool first, char *var_name)
{
  if ( first)
    fp << "      ";
  else
    fp << "      ,";
  fp << "new GeDynIncrAnalog(" << var_name << ".dd, \"" << attribute << "\"," 
     << increment << "," << min_value << "," << max_value << ")" << endl;
  return 1;
}

void GeRadioButton::get_attributes( attr_sItem *attrinfo, int *item_count)
{
  int i = *item_count;

  strcpy( attrinfo[i].name, "RadioButton.Attribute");
  attrinfo[i].value = attribute;
  attrinfo[i].type = glow_eType_String;
  attrinfo[i++].size = sizeof( attribute);

  dyn->display_access = true;
  *item_count = i;
}

void GeRadioButton::set_attribute( grow_tObject object, char *attr_name, int *cnt)
{
  (*cnt)--;
  if ( *cnt == 0) {
    char msg[200];

    strncpy( attribute, attr_name, sizeof( attribute));
    sprintf( msg, "RadioButton.Attribute = %s", attr_name);
    dyn->graph->message( 'I', msg);
  }
}

void GeRadioButton::replace_attribute( char *from, char *to, int *cnt, int strict)
{
  GeDyn::replace_attribute( attribute, sizeof(attribute), from, to, cnt, strict);
}

void GeRadioButton::save( ofstream& fp)
{
  fp << int(ge_eSave_RadioButton) << endl;
  fp << int(ge_eSave_RadioButton_attribute) << FSPACE << attribute << endl;
  fp << int(ge_eSave_End) << endl;
}

void GeRadioButton::open( ifstream& fp)
{
  int		type;
  int 		end_found = 0;
  char		dummy[40];

  for (;;)
  {
    fp >> type;
    switch( type) {
      case ge_eSave_RadioButton: break;
      case ge_eSave_RadioButton_attribute:
        fp.get();
        fp.getline( attribute, sizeof(attribute));
        break;
      case ge_eSave_End: end_found = 1; break;
      default:
        cout << "GeRadioButton:open syntax error" << endl;
        fp.getline( dummy, sizeof(dummy));
    }
    if ( end_found)
      break;
  }  
}

int GeRadioButton::connect( grow_tObject object, glow_sTraceData *trace_data)
{
  int		attr_type, attr_size;
  char		parsed_name[120];
  int		sts;

  size = 4;
  p = 0;
  db = dyn->graph->parse_attr_name( attribute, parsed_name,
				    &inverted, &attr_type, &attr_size);
  if ( strcmp( parsed_name,"") == 0)
    return 1;

  sts = dyn->graph->ref_object_info( dyn->cycle, parsed_name, (void **)&p, &subid, size);
  if ( EVEN(sts)) return sts;

  if ( p)
    trace_data->p = p;
  first_scan = true;
  return 1;
}

int GeRadioButton::disconnect( grow_tObject object)
{
  if ( p && db == graph_eDatabase_Gdh)
    gdh_UnrefObjectInfo( subid);
  p = 0;
  return 1;
}

int GeRadioButton::scan( grow_tObject object)
{
  if ( !p)
    return 1;

  if ( !first_scan) {
    if ( old_value == *p) {
      // No change since last time
      return 1;
    }
  }
  else
    first_scan = false;

  if ( (!inverted && *p) || (inverted && !*p)) {
    grow_SetObjectLastNodeClass( object);
  }
  else {
    grow_SetObjectFirstNodeClass( object);
  }
  old_value = *p;
  return 1;
}

int GeRadioButton::action( grow_tObject object, glow_tEvent event)
{
  if ( !dyn->graph->is_authorized( dyn->access))
    return 1;

  switch ( event->event) {
  case glow_eEvent_MB1Down:
    grow_SetClickSensitivity( dyn->graph->grow->ctx, glow_mSensitivity_MB1Click);
    grow_SetObjectColorInverse( object, 1);
    break;
  case glow_eEvent_MB1Up:
    grow_SetObjectColorInverse( object, 0);
    break;
  case glow_eEvent_MB1Click: {
    grow_tObject 	group;
    grow_tObject 	*objectlist, *object_p;
    int 		object_cnt;
    int			sts;
    char       		parsed_name[120];
    int			inverted;
    int			attr_type, attr_size;
    pwr_tBoolean 	value;

    if ( dyn->total_action_type & ge_mActionType_Confirm)
      break;

    sts = grow_GetObjectGroup( dyn->graph->grow->ctx, object, &group);
    if ( EVEN(sts)) break;

    grow_GetGroupObjectList( group, &objectlist, &object_cnt);
    object_p = objectlist;
    for ( int i = 0; i < object_cnt; i++) {
      if ( *object_p != event->object.object &&
	   grow_GetObjectType( *object_p) == glow_eObjectType_GrowNode) {
	value = 0;

	GeDyn *gm_dyn;

        grow_GetUserData( *object_p, (void **)&gm_dyn);
	if ( gm_dyn->total_action_type & ge_mActionType_RadioButton ) {
	  for ( GeDynElem *elem = gm_dyn->elements; elem; elem = elem->next) {
	    if ( elem->action_type == ge_mActionType_RadioButton) {
	      dyn->graph->parse_attr_name( ((GeRadioButton *)elem)->attribute, parsed_name,
					   &inverted, &attr_type, &attr_size);
	      sts = gdh_SetObjectInfo( parsed_name, &value, sizeof(value));
	    }
	  }
	}
      }
      object_p++;
    }

    value = 1;
    
    dyn->graph->parse_attr_name( attribute, parsed_name,
			    &inverted, &attr_type, &attr_size);
    sts = gdh_SetObjectInfo( parsed_name, &value, sizeof(value));
    if ( EVEN(sts)) printf("RadioButton error: %s\n", attribute);
    break;
  }
  default: ;    
  }
  return 1;
}

int GeRadioButton::export_java( grow_tObject object, ofstream& fp, bool first, char *var_name)
{
  if ( first)
    fp << "      ";
  else
    fp << "      ,";
  fp << "new GeDynRadioButton(" << var_name << ".dd, \"" << attribute << "\")" << endl;
  return 1;
}

void GeTipText::get_attributes( attr_sItem *attrinfo, int *item_count)
{
  int i = *item_count;

  strcpy( attrinfo[i].name, "ToolTip.Text");
  attrinfo[i].value = text;
  attrinfo[i].type = glow_eType_String;
  attrinfo[i++].size = sizeof( text);
  *item_count = i;
}

void GeTipText::save( ofstream& fp)
{
  fp << int(ge_eSave_TipText) << endl;
  fp << int(ge_eSave_TipText_text) << FSPACE << text << endl;
  fp << int(ge_eSave_End) << endl;
}

void GeTipText::open( ifstream& fp)
{
  int		type;
  int 		end_found = 0;
  char		dummy[40];

  for (;;)
  {
    fp >> type;
    switch( type) {
      case ge_eSave_TipText: break;
      case ge_eSave_TipText_text:
        fp.get();
        fp.getline( text, sizeof(text));
        break;
      case ge_eSave_End: end_found = 1; break;
      default:
        cout << "GeTipText:open syntax error" << endl;
        fp.getline( dummy, sizeof(dummy));
    }
    if ( end_found)
      break;
  }  
}

int GeTipText::action( grow_tObject object, glow_tEvent event)
{
  switch ( event->event) {
  case glow_eEvent_TipText:
    grow_SetTipText( dyn->graph->grow->ctx, event->object.object, 
		     text, event->any.x_pixel, event->any.y_pixel);
    break;
  default: ;    
  }
  return 1;
}

int GeTipText::export_java( grow_tObject object, ofstream& fp, bool first, char *var_name)
{
  if ( first)
    fp << "      ";
  else
    fp << "      ,";
  fp << "new GeDynTipText(" << var_name << ".dd, \"" << text << "\")" << endl;
  return 1;
}

void GeHelp::get_attributes( attr_sItem *attrinfo, int *item_count)
{
  int i = *item_count;

  strcpy( attrinfo[i].name, "Help.Topic");
  attrinfo[i].value = topic;
  attrinfo[i].type = glow_eType_String;
  attrinfo[i++].size = sizeof( topic);

  strcpy( attrinfo[i].name, "Help.Bookmark");
  attrinfo[i].value = bookmark;
  attrinfo[i].type = glow_eType_String;
  attrinfo[i++].size = sizeof( bookmark);

  dyn->display_access = true;
  *item_count = i;
}

int GeHelp::get_transtab( char **tt)
{
  static char transtab[][32] = {	"SubGraph",		"SubGraph",
					"A1",			"Text",
					"Dynamic",		"",
					""};

  *tt = (char *) transtab;
  return 0;
}

void GeHelp::save( ofstream& fp)
{
  fp << int(ge_eSave_Help) << endl;  
  fp << int(ge_eSave_Help_topic) << FSPACE << topic << endl;
  fp << int(ge_eSave_Help_bookmark) << FSPACE << bookmark << endl;
  fp << int(ge_eSave_End) << endl;
}

void GeHelp::open( ifstream& fp)
{
  int		type;
  int 		end_found = 0;
  char		dummy[40];

  for (;;)
  {
    fp >> type;
    switch( type) {
      case ge_eSave_Help: break;
      case ge_eSave_Help_topic:
        fp.get();
        fp.getline( topic, sizeof(topic));
        break;
      case ge_eSave_Help_bookmark:
        fp.get();
        fp.getline( bookmark, sizeof(bookmark));
        break;
      case ge_eSave_End: end_found = 1; break;
      default:
        cout << "GeHelp:open syntax error" << endl;
        fp.getline( dummy, sizeof(dummy));
    }
    if ( end_found)
      break;
  }  
}

int GeHelp::action( grow_tObject object, glow_tEvent event)
{
  if ( !dyn->graph->is_authorized( dyn->access))
    return 1;

  switch ( event->event) {
  case glow_eEvent_MB1Down:
    grow_SetClickSensitivity( dyn->graph->grow->ctx, glow_mSensitivity_MB1Click);
    grow_SetObjectColorInverse( object, 1);
    break;
  case glow_eEvent_MB1Up:
    grow_SetObjectColorInverse( object, 0);
    break;
  case glow_eEvent_Key_Return:
  case glow_eEvent_MB1Click:
    if ( dyn->total_action_type & ge_mActionType_Confirm)
      break;

    if ( dyn->graph->command_cb) {
      char command[200];
      char cmd[200];

      if ( strcmp( bookmark, "") != 0)
	sprintf( command, "help %s /bookmark=%s", topic, bookmark);
      else
	sprintf( command, "help %s", topic);
      dyn->graph->get_command( command, cmd);
      (dyn->graph->command_cb)( dyn->graph->parent_ctx, cmd);
    }
    break;
  default: ;    
  }
  return 1;
}

int GeHelp::export_java( grow_tObject object, ofstream& fp, bool first, char *var_name)
{
  char command[200];
  
  if ( strcmp( bookmark, "") != 0)
    sprintf( command, "help %s /bookmark=%s", topic, bookmark);
  else
    sprintf( command, "help %s", topic);

  if ( first)
    fp << "      ";
  else
    fp << "      ,";
  fp << "new GeDynCommand(" << var_name << ".dd, \"" << GeDyn::cmd_cnv( command) << "\")" << endl;
  return 1;
}

void GeOpenGraph::get_attributes( attr_sItem *attrinfo, int *item_count)
{
  int i = *item_count;

  strcpy( attrinfo[i].name, "OpenGraph.GraphObject");
  attrinfo[i].value = graph_object;
  attrinfo[i].type = glow_eType_String;
  attrinfo[i++].size = sizeof( graph_object);

  dyn->display_access = true;
  *item_count = i;
}

int GeOpenGraph::get_transtab( char **tt)
{
  static char transtab[][32] = {	"SubGraph",		"SubGraph",
					"A1",			"Text",
					"Dynamic",		"",
					""};

  *tt = (char *) transtab;
  return 0;
}

void GeOpenGraph::set_attribute( grow_tObject object, char *attr_name, int *cnt)
{
  (*cnt)--;
  if ( *cnt == 0) {
    char msg[200];
    char *s;

    strncpy( graph_object, attr_name, sizeof( graph_object));
    if ( (s = strrchr( graph_object, '.')))
      *s = 0;
    sprintf( msg, "OpenGraph.GraphObject = %s", graph_object);
    dyn->graph->message( 'I', msg);
  }
}

void GeOpenGraph::replace_attribute( char *from, char *to, int *cnt, int strict)
{
  GeDyn::replace_attribute( graph_object, sizeof(graph_object), from, to, cnt, strict);
}

void GeOpenGraph::save( ofstream& fp)
{
  fp << int(ge_eSave_OpenGraph) << endl;  
  fp << int(ge_eSave_OpenGraph_graph_object) << FSPACE << graph_object << endl;
  fp << int(ge_eSave_End) << endl;
}

void GeOpenGraph::open( ifstream& fp)
{
  int		type;
  int 		end_found = 0;
  char		dummy[40];

  for (;;)
  {
    fp >> type;
    switch( type) {
      case ge_eSave_OpenGraph: break;
      case ge_eSave_OpenGraph_graph_object:
        fp.get();
        fp.getline( graph_object, sizeof(graph_object));
        break;
      case ge_eSave_End: end_found = 1; break;
      default:
        cout << "GeOpenGraph:open syntax error" << endl;
        fp.getline( dummy, sizeof(dummy));
    }
    if ( end_found)
      break;
  }  
}

int GeOpenGraph::action( grow_tObject object, glow_tEvent event)
{
  if ( !dyn->graph->is_authorized( dyn->access))
    return 1;

  switch ( event->event) {
  case glow_eEvent_MB1Down:
    grow_SetClickSensitivity( dyn->graph->grow->ctx, glow_mSensitivity_MB1Click);
    grow_SetObjectColorInverse( object, 1);
    break;
  case glow_eEvent_MB1Up:
    grow_SetObjectColorInverse( object, 0);
    break;
  case glow_eEvent_Key_Return:
  case glow_eEvent_MB1Click:
    if ( dyn->total_action_type & ge_mActionType_Confirm)
      break;

    if ( dyn->graph->command_cb) {
      char command[200];
      char cmd[200];

      sprintf( command, "open graph/object=%s", graph_object);
      dyn->graph->get_command( command, cmd);
      (dyn->graph->command_cb)( dyn->graph->parent_ctx, cmd);
    }
    break;
  default: ;    
  }
  return 1;
}

int GeOpenGraph::export_java( grow_tObject object, ofstream& fp, bool first, char *var_name)
{
  char command[200];
  
  sprintf( command, "open graph /object=%s", graph_object);

  if ( first)
    fp << "      ";
  else
    fp << "      ,";
  fp << "new GeDynCommand(" << var_name << ".dd, \"" << GeDyn::cmd_cnv( command) << "\")" << endl;
  return 1;
}

void GeOpenURL::get_attributes( attr_sItem *attrinfo, int *item_count)
{
  int i = *item_count;

  strcpy( attrinfo[i].name, "OpenURL.URL");
  attrinfo[i].value = url;
  attrinfo[i].type = glow_eType_String;
  attrinfo[i++].size = sizeof( url);

  dyn->display_access = true;
  *item_count = i;
}

int GeOpenURL::get_transtab( char **tt)
{
  static char transtab[][32] = {	"SubGraph",		"SubGraph",
					"A1",			"Text",
					"Dynamic",		"",
					""};

  *tt = (char *) transtab;
  return 0;
}

void GeOpenURL::save( ofstream& fp)
{
  fp << int(ge_eSave_OpenURL) << endl;  
  fp << int(ge_eSave_OpenURL_url) << FSPACE << url << endl;
  fp << int(ge_eSave_End) << endl;
}

void GeOpenURL::open( ifstream& fp)
{
  int		type;
  int 		end_found = 0;
  char		dummy[40];

  for (;;)
  {
    fp >> type;
    switch( type) {
      case ge_eSave_OpenURL: break;
      case ge_eSave_OpenURL_url:
        fp.get();
        fp.getline( url, sizeof(url));
        break;
      case ge_eSave_End: end_found = 1; break;
      default:
        cout << "GeOpenURL:open syntax error" << endl;
        fp.getline( dummy, sizeof(dummy));
    }
    if ( end_found)
      break;
  }  
}

int GeOpenURL::action( grow_tObject object, glow_tEvent event)
{
  if ( !dyn->graph->is_authorized( dyn->access))
    return 1;

  switch ( event->event) {
  case glow_eEvent_MB1Down:
    grow_SetClickSensitivity( dyn->graph->grow->ctx, glow_mSensitivity_MB1Click);
    grow_SetObjectColorInverse( object, 1);
    break;
  case glow_eEvent_MB1Up:
    grow_SetObjectColorInverse( object, 0);
    break;
  case glow_eEvent_Key_Return:
  case glow_eEvent_MB1Click:
    if ( dyn->total_action_type & ge_mActionType_Confirm)
      break;

    if ( dyn->graph->command_cb) {
      char command[200];
      char cmd[200];

      sprintf( command, "open url \"%s\"", url);
      dyn->graph->get_command( command, cmd);
      (dyn->graph->command_cb)( dyn->graph->parent_ctx, cmd);
    }
    break;
  default: ;    
  }
  return 1;
}

int GeOpenURL::export_java( grow_tObject object, ofstream& fp, bool first, char *var_name)
{
  char command[200];

  sprintf( command, "open url \"%s\"", url);

  if ( first)
    fp << "      ";
  else
    fp << "      ,";
  fp << "new GeDynCommand(" << var_name << ".dd, \"" << GeDyn::cmd_cnv( command) << "\")" << endl;
  return 1;
}

void GeInputFocus::get_attributes( attr_sItem *attrinfo, int *item_count)
{
  int i = *item_count;

  strcpy( attrinfo[i].name, "InputFocus.InitialFocus");
  attrinfo[i].value = &initial_focus;
  attrinfo[i].type = ge_eAttrType_InputFocus;
  attrinfo[i++].size = sizeof( initial_focus);

  strcpy( attrinfo[i].name, "InputFocus.NextHorizontal");
  attrinfo[i].value = next_horizontal;
  attrinfo[i].type = glow_eType_String;
  attrinfo[i++].size = sizeof( next_horizontal);

  strcpy( attrinfo[i].name, "InputFocus.NextVertical");
  attrinfo[i].value = next_vertical;
  attrinfo[i].type = glow_eType_String;
  attrinfo[i++].size = sizeof( next_vertical);

  strcpy( attrinfo[i].name, "InputFocus.NextTab");
  attrinfo[i].value = next_tab;
  attrinfo[i].type = glow_eType_String;
  attrinfo[i++].size = sizeof( next_tab);

  *item_count = i;
}

void GeInputFocus::save( ofstream& fp)
{
  fp << int(ge_eSave_InputFocus) << endl;  
  fp << int(ge_eSave_InputFocus_initial_focus) << FSPACE << initial_focus << endl;
  fp << int(ge_eSave_InputFocus_next_horizontal) << FSPACE << next_horizontal << endl;
  fp << int(ge_eSave_InputFocus_next_vertical) << FSPACE << next_vertical << endl;
  fp << int(ge_eSave_InputFocus_next_tab) << FSPACE << next_tab << endl;
  fp << int(ge_eSave_End) << endl;
}

void GeInputFocus::open( ifstream& fp)
{
  int		type;
  int 		end_found = 0;
  char		dummy[40];

  for (;;)
  {
    fp >> type;
    switch( type) {
      case ge_eSave_InputFocus: break;
      case ge_eSave_InputFocus_initial_focus: fp >> initial_focus; break;
      case ge_eSave_InputFocus_next_horizontal:
        fp.get();
        fp.getline( next_horizontal, sizeof(next_horizontal));
        break;
      case ge_eSave_InputFocus_next_vertical:
        fp.get();
        fp.getline( next_vertical, sizeof(next_vertical));
        break;
      case ge_eSave_InputFocus_next_tab:
        fp.get();
        fp.getline( next_tab, sizeof(next_tab));
        break;
      case ge_eSave_End: end_found = 1; break;
      default:
        cout << "GeInputFocus:open syntax error" << endl;
        fp.getline( dummy, sizeof(dummy));
    }
    if ( end_found)
      break;
  }  
}

int GeInputFocus::action( grow_tObject object, glow_tEvent event)
{
  int sts;
  grow_tObject next;
  int found;

  switch ( event->event) {
  case glow_eEvent_MB1Click:
    grow_SetObjectInputFocus( object, 1);
    dyn->graph->set_inputfocus( 1);
    break;
  case glow_eEvent_InputFocusInit:
    if ( initial_focus & ge_mInputFocus_InitialFocus)
      grow_SetObjectInputFocus( object, 1);
    break;
  case glow_eEvent_Key_Right:
    if ( event->object.object_type != glow_eObjectType_NoObject) {
      found = 0;
      if ( strcmp( next_horizontal, "") != 0) {
	sts = grow_FindObjectByName( dyn->graph->grow->ctx, next_horizontal, &next);
	if ( ODD(sts)) {
	  // Check that this object can handle input focus
	  GeDyn 	*next_dyn;
	  grow_GetUserData( next, (void **)&next_dyn);
	  if ( next_dyn->total_action_type & ge_mActionType_InputFocus)
	    found = 1;
	}
      }
      else if ( strcmp( next_tab, "") != 0) {
	sts = grow_FindObjectByName( dyn->graph->grow->ctx, next_tab, &next);
	if ( ODD(sts)) {
	  // Check that this object can handle input focus
	  GeDyn 	*next_dyn;
	  grow_GetUserData( next, (void **)&next_dyn);
	  if ( next_dyn->total_action_type & ge_mActionType_InputFocus)		
	    found = 1;
	}
      }

      if ( found)
	grow_SetObjectInputFocus( next, 1);
      else
 	grow_SetObjectInputFocus( object, 0);
    }
    else {
      // No current object with input focus, check initial mask
      if ( initial_focus & ge_mInputFocus_FirstHorizontal)
 	grow_SetObjectInputFocus( object, 1);
    }
    break;
  case glow_eEvent_Key_Left:
    if ( event->object.object_type != glow_eObjectType_NoObject) {
      char 	name[40];
      grow_tObject *objectlist, *object_p;
      int 	object_cnt;
      int	i;
      GeDyn 	*gm_dyn;
      grow_tObject prev;

      grow_GetObjectName( object, name);
    
      // Find object that has this object as next_horizontal
      grow_GetObjectList( dyn->graph->grow->ctx, &objectlist, &object_cnt);
      object_p = objectlist;
      found = 0;
      for ( i = 0; i < object_cnt; i++) {
	if ( grow_GetObjectType( *object_p) == glow_eObjectType_GrowNode ||
	     grow_GetObjectType( *object_p) == glow_eObjectType_GrowSlider ||
	     grow_GetObjectType( *object_p) == glow_eObjectType_GrowGroup) {
	  grow_GetUserData( *object_p, (void **)&gm_dyn);
	  if ( gm_dyn->total_action_type & ge_mActionType_InputFocus ) {
	    for ( GeDynElem *elem = gm_dyn->elements; elem; elem = elem->next) {
	      if ( elem->action_type == ge_mActionType_InputFocus) {
		if ( strcmp( ((GeInputFocus *)elem)->next_horizontal, name) == 0) {
		  found = 1;
		  prev = *object_p;
		  break;
		}
	      }
	    }
	  }
	}
	if ( found)
	  break;
	object_p++;
      }
      
      if ( found) {
	// Check that this object can handle input focus
	GeDyn 	*prev_dyn;
	grow_GetUserData( prev, (void **)&prev_dyn);
	if ( prev_dyn->total_action_type & ge_mActionType_InputFocus)		
	  grow_SetObjectInputFocus( prev, 1);
	else
	  grow_SetObjectInputFocus( object, 0);
      }
    }
    else {
      // No current object with input focus, check initial mask
      if ( initial_focus & ge_mInputFocus_LastHorizontal)
 	grow_SetObjectInputFocus( object, 1);
    }
    break;

  case glow_eEvent_Key_Down:
    if ( event->object.object_type != glow_eObjectType_NoObject) {
      found = 0;
      if ( strcmp( next_vertical, "") != 0) {
	sts = grow_FindObjectByName( dyn->graph->grow->ctx, next_vertical, &next);
	if ( ODD(sts)) {
	  // Check that this object can handle input focus
	  GeDyn 	*next_dyn;
	  grow_GetUserData( next, (void **)&next_dyn);
	  if ( next_dyn->total_action_type & ge_mActionType_InputFocus)
	    found = 1;
	}
      }
      else if ( strcmp( next_tab, "") != 0) {
	sts = grow_FindObjectByName( dyn->graph->grow->ctx, next_tab, &next);
	if ( ODD(sts)) {
	  // Check that this object can handle input focus
	  GeDyn 	*next_dyn;
	  grow_GetUserData( next, (void **)&next_dyn);
	  if ( next_dyn->total_action_type & ge_mActionType_InputFocus)		
	    found = 1;
	}
      }
      
      if ( found)
	grow_SetObjectInputFocus( next, 1);
      else
	grow_SetObjectInputFocus( object, 0);
    }
    else {
      // No current object with input focus, check initial mask
      if ( initial_focus & ge_mInputFocus_FirstVertical)
 	grow_SetObjectInputFocus( object, 1);
    }
    break;
  case glow_eEvent_Key_Up:
    if ( event->object.object_type != glow_eObjectType_NoObject) {
      char 	name[40];
      grow_tObject *objectlist, *object_p;
      int 	object_cnt;
      int		i;
      GeDyn 	*gm_dyn;
      grow_tObject prev;
      int found;
      
      grow_GetObjectName( object, name);
      
      // Find object that has this object as next_vertical
      grow_GetObjectList( dyn->graph->grow->ctx, &objectlist, &object_cnt);
      object_p = objectlist;
      found = 0;
      for ( i = 0; i < object_cnt; i++) {
	if ( grow_GetObjectType( *object_p) == glow_eObjectType_GrowNode ||
	     grow_GetObjectType( *object_p) == glow_eObjectType_GrowSlider ||
	     grow_GetObjectType( *object_p) == glow_eObjectType_GrowGroup) {
	  grow_GetUserData( *object_p, (void **)&gm_dyn);
	  if ( gm_dyn->total_action_type & ge_mActionType_InputFocus ) {
	    for ( GeDynElem *elem = gm_dyn->elements; elem; elem = elem->next) {
	      if ( elem->action_type == ge_mActionType_InputFocus) {
		if ( strcmp( ((GeInputFocus *)elem)->next_vertical, name) == 0) {
		  found = 1;
		  prev = *object_p;
		  break;
		}
	      }
	    }
	  }
	}
	if ( found)
	  break;
	object_p++;
      }

      if ( found) {
	// Check that this object can handle input focus
	GeDyn 	*prev_dyn;
	grow_GetUserData( prev, (void **)&prev_dyn);
	if ( prev_dyn->total_action_type & ge_mActionType_InputFocus)		
	  grow_SetObjectInputFocus( prev, 1);
	else
	  grow_SetObjectInputFocus( object, 0);
      }    
    }
    else {
      // No current object with input focus, check initial mask
      if ( initial_focus & ge_mInputFocus_LastVertical)
 	grow_SetObjectInputFocus( object, 1);
    }
    break;

  case glow_eEvent_Key_Tab:
    if ( event->object.object_type != glow_eObjectType_NoObject) {
      found = 0;
      if ( strcmp( next_tab, "") != 0) {
	sts = grow_FindObjectByName( dyn->graph->grow->ctx, next_tab, &next);
	if ( ODD(sts)) {
	  // Check that this object can handle input focus
	  GeDyn 	*next_dyn;
	  grow_GetUserData( next, (void **)&next_dyn);
	  if ( next_dyn->total_action_type & ge_mActionType_InputFocus)
	    found = 1;
	}
      }
      if ( found) {
	grow_SetObjectInputFocus( next, 1);

	// Mark this object as previous tab
	if ( grow_GetObjectType( next) == glow_eObjectType_GrowNode ||
	     grow_GetObjectType( next) == glow_eObjectType_GrowSlider ||
	     grow_GetObjectType( next) == glow_eObjectType_GrowGroup) {
	  GeDyn *next_dyn;
	  grow_GetUserData( next, (void **)&next_dyn);
	  for ( GeDynElem *elem = next_dyn->elements; elem; elem = elem->next) {
	    if ( elem->action_type == ge_mActionType_InputFocus) {
	      ((GeInputFocus *)elem)->prev_tab = object;
	      break;
	    }
	  }
	}
      }
      else
	grow_SetObjectInputFocus( object, 0);
    }
    else {
      // No current object with input focus, check initial mask
      if ( initial_focus & ge_mInputFocus_FirstTab)
 	grow_SetObjectInputFocus( object, 1);
    }
    break;

  case glow_eEvent_Key_ShiftTab:
    if ( event->object.object_type != glow_eObjectType_NoObject) {
      if ( prev_tab) {
	GeDyn 	*prev_dyn;
	grow_GetUserData( prev_tab, (void **)&prev_dyn);
	if ( prev_dyn->total_action_type & ge_mActionType_InputFocus)
	  grow_SetObjectInputFocus( prev_tab, 1);
	else
	  grow_SetObjectInputFocus( object, 0);
      }
    }
    break;

  case glow_eEvent_Key_Escape:
    if ( event->object.object_type != glow_eObjectType_NoObject) {
      grow_SetObjectInputFocus( object, 0);
    }
    break;

  default: ;    
  }
  return 1;
}

void GeCloseGraph::get_attributes( attr_sItem *attrinfo, int *item_count)
{
  int i = *item_count;

  dyn->display_access = true;
  *item_count = i;
}

void GeCloseGraph::save( ofstream& fp)
{
  fp << int(ge_eSave_CloseGraph) << endl;  
  fp << int(ge_eSave_End) << endl;
}

void GeCloseGraph::open( ifstream& fp)
{
  int		type;
  int 		end_found = 0;
  char		dummy[40];

  for (;;)
  {
    fp >> type;
    switch( type) {
      case ge_eSave_CloseGraph: break;
      case ge_eSave_End: end_found = 1; break;
      default:
        cout << "GeCloseGraph:open syntax error" << endl;
        fp.getline( dummy, sizeof(dummy));
    }
    if ( end_found)
      break;
  }  
}

int GeCloseGraph::action( grow_tObject object, glow_tEvent event)
{
  if ( !dyn->graph->is_authorized( dyn->access))
    return 1;

  switch ( event->event) {
  case glow_eEvent_MB1Down:
    grow_SetClickSensitivity( dyn->graph->grow->ctx, glow_mSensitivity_MB1Click);
    grow_SetObjectColorInverse( object, 1);
    break;
  case glow_eEvent_MB1Up:
    grow_SetObjectColorInverse( object, 0);
    break;
  case glow_eEvent_MB1Click:
  case glow_eEvent_Key_Return:
    if ( dyn->total_action_type & ge_mActionType_Confirm)
      break;

    if ( dyn->graph->close_cb)
      (dyn->graph->close_cb)( dyn->graph->parent_ctx);
    break;
  default: ;    
  }
  return 1;
}

int GeCloseGraph::export_java( grow_tObject object, ofstream& fp, bool first, char *var_name)
{
  if ( first)
    fp << "      ";
  else
    fp << "      ,";
  fp << "new GeDynCloseGraph(" << var_name << ".dd)" << endl;
  return 1;
}

void GeSlider::get_attributes( attr_sItem *attrinfo, int *item_count)
{
  int i = *item_count;

  strcpy( attrinfo[i].name, "Slider.Attribute");
  attrinfo[i].value = attribute;
  attrinfo[i].type = glow_eType_String;
  attrinfo[i++].size = sizeof( attribute);

  dyn->display_access = true;
  *item_count = i;
}

void GeSlider::set_attribute( grow_tObject object, char *attr_name, int *cnt)
{
  (*cnt)--;
  if ( *cnt == 0) {
    char msg[200];

    strncpy( attribute, attr_name, sizeof( attribute));
    sprintf( msg, "Slider.Attribute = %s", attr_name);
    dyn->graph->message( 'I', msg);
  }
}

void GeSlider::replace_attribute( char *from, char *to, int *cnt, int strict)
{
  GeDyn::replace_attribute( attribute, sizeof(attribute), from, to, cnt, strict);
}

void GeSlider::save( ofstream& fp)
{
  fp << int(ge_eSave_Slider) << endl;
  fp << int(ge_eSave_Slider_attribute) << FSPACE << attribute << endl;
  fp << int(ge_eSave_End) << endl;
}

void GeSlider::open( ifstream& fp)
{
  int		type;
  int 		end_found = 0;
  char		dummy[40];

  for (;;)
  {
    fp >> type;
    switch( type) {
      case ge_eSave_Slider: break;
      case ge_eSave_Slider_attribute:
        fp.get();
        fp.getline( attribute, sizeof(attribute));
        break;
      case ge_eSave_End: end_found = 1; break;
      default:
        cout << "GeSlider:open syntax error" << endl;
        fp.getline( dummy, sizeof(dummy));
    }
    if ( end_found)
      break;
  }  
}

int GeSlider::connect( grow_tObject object, glow_sTraceData *trace_data)
{
  int		inverted;
  char		parsed_name[120];
  int		sts;

  size = 4;
  p = 0;
  db = dyn->graph->parse_attr_name( attribute, parsed_name,
				    &inverted, &attr_type, &size);
  if ( strcmp( parsed_name,"") == 0)
    return 1;

  switch ( attr_type) {
  case pwr_eType_Float32:
  case pwr_eType_Int32:
    break;
  default:
    return 1;
  }
  sts = dyn->graph->ref_object_info( dyn->cycle, parsed_name, (void **)&p, &subid, size);
  if ( EVEN(sts)) return sts;

  if ( p)
    trace_data->p = p;
  first_scan = true;
  slider_disabled = 0;

  // Get min and max position from sider background
  double max_value, min_value, max_pos, min_pos;
  glow_eDirection direction;
  double ll_x, ll_y, ur_x, ur_y;
  grow_tObject background;
  double origo;

  if ( !grow_TransformIsStored( object)) {
    grow_StoreTransform( object);
    grow_MeasureNode( object, &ll_x, &ll_y, &ur_x, &ur_y);
    grow_GetSliderInfo( object, &direction, 
		&max_value, &min_value, &max_pos, &min_pos);
    sts = grow_GetBackgroundObjectLimits( dyn->graph->grow->ctx, 
					(glow_eTraceType)ge_mDynType_SliderBackground,
					(ll_x + ur_x) / 2, (ll_y + ur_y) / 2, &background,
					&min_pos, &max_pos, &direction);
    if ( ODD(sts)) {
      grow_GetSliderOrigo( object, direction, &origo);

      switch( direction) {
      case glow_eDirection_Down:
	grow_SetSliderInfo( object, direction,
			  max_value, min_value, max_pos - origo, min_pos - origo);

	grow_MoveNode( object, ll_x, min_pos - origo);
	break;
      case glow_eDirection_Up:
	grow_SetSliderInfo( object, direction,
			  max_value, min_value, max_pos - (ur_y - ll_y - origo), 
			  min_pos - (ur_y - ll_y - origo));
	grow_MoveNode( object, ll_x, min_pos - (ur_y - ll_y - origo));
	break;
      case glow_eDirection_Left:
	grow_SetSliderInfo( object, direction,
			  max_value, min_value, max_pos - (ur_x - ll_x - origo), 
			  min_pos - (ur_x - ll_x - origo));
	grow_MoveNode( object, min_pos - (ur_x - ll_x - origo), ll_y);
	break;
      case glow_eDirection_Right:
	grow_SetSliderInfo( object, direction,
			  max_value, min_value, max_pos - origo, min_pos - origo);
      
	grow_MoveNode( object, min_pos - origo, ll_y);
	break;
      default:
	;
      }
    }
    grow_StoreTransform( object);
  }
  return 1;
}

int GeSlider::disconnect( grow_tObject object)
{
  if ( p && db == graph_eDatabase_Gdh)
    gdh_UnrefObjectInfo( subid);
  p = 0;
  return 1;
}

int GeSlider::scan( grow_tObject object)
{
  double max_value, min_value, max_pos, min_pos;
  glow_eDirection direction;

  if ( !first_scan) {
    switch ( attr_type) {
    case pwr_eType_Float32:
      if ( fabs( old_value - *p) < FLT_EPSILON)
	// No change since last time
	return 1;
    case pwr_eType_Int32:
      if ( *(pwr_tInt32 *)p == (pwr_tInt32) old_value)
	return 1;
    default: ;
    }
  }
  else
    first_scan = 0;

  grow_GetSliderInfo( object, &direction, 
		      &max_value, &min_value, &max_pos, &min_pos);
  if ( min_pos != max_pos) {
    if ( dyn->graph->current_slider != object &&
	 max_value != min_value) {
      float value;
      double pos_x, pos_y;

      switch ( attr_type) {
      case pwr_eType_Float32:
	value = *p;
	break;
      default:
	value = (float) (*(pwr_tInt32 *)p);
	break;
      }
          
      switch ( direction) {
      case glow_eDirection_Down:
	pos_y = (max_value - value) / (max_value - min_value) *
	  (max_pos - min_pos);
	if ( pos_y < 0)
	  pos_y = 0;
	else if ( pos_y > max_pos - min_pos)
	  pos_y = max_pos - min_pos;
	pos_x = 0;
	break;
      case glow_eDirection_Right:
	pos_x = max_pos - min_pos - 
	  (value - min_value) / (max_value - min_value) *
	  (max_pos - min_pos);
	if ( pos_x < 0)
	  pos_x = 0;
	else if ( pos_x > max_pos - min_pos)
	  pos_x = max_pos - min_pos;
	pos_y = 0;
	break;
      case glow_eDirection_Left:
	pos_x = max_pos - min_pos - 
	  (max_value - value) / (max_value - min_value) *
	  (max_pos - min_pos);
	if ( pos_x < 0)
	  pos_x = 0;
	else if ( pos_x > max_pos - min_pos)
	  pos_x = max_pos - min_pos;
	pos_y = 0;
	break;
      default:   // Up
	pos_y = (value - min_value) / (max_value - min_value) *
	  (max_pos - min_pos);
	if ( pos_y < 0)
	  pos_y = 0;
	else if ( pos_y > max_pos - min_pos)
	  pos_y = max_pos - min_pos;
	pos_x = 0;
      }
      grow_SetObjectPosition( object, pos_x, pos_y);
    }
  }
  old_value = *p;
  return 1;
}

int GeSlider::action( grow_tObject object, glow_tEvent event)
{
  switch ( event->event) {
  case glow_eEvent_MB1Down:
    grow_SetClickSensitivity( dyn->graph->grow->ctx, glow_mSensitivity_MB1Press);
    break;
  case glow_eEvent_SliderMoveStart: {
    double max_value, min_value, max_pos, min_pos;
    glow_eDirection direction;

    if ( !dyn->graph->is_authorized( dyn->access) ||
	 slider_disabled) {
      grow_SetMoveRestrictions( dyn->graph->grow->ctx, 
				glow_eMoveRestriction_Disable, 0, 0, NULL);
      dyn->graph->current_slider = NULL;
      break;
    }
    grow_GetSliderInfo( object, &direction, 
			&max_value, &min_value, &max_pos, &min_pos);
    if ( direction == glow_eDirection_Right || 
	 direction == glow_eDirection_Left)
      grow_SetMoveRestrictions( dyn->graph->grow->ctx, 
				glow_eMoveRestriction_HorizontalSlider,
				max_pos, min_pos, event->object.object);
    else
      grow_SetMoveRestrictions( dyn->graph->grow->ctx, 
				glow_eMoveRestriction_VerticalSlider,
				max_pos, min_pos, event->object.object);

    dyn->graph->current_slider = object;
    break;
  }
  case glow_eEvent_SliderMoved: {
    double 		max_value, min_value, max_pos, min_pos;
    glow_eDirection 	direction;
    float 		value;
    int			sts;
    double		ll_x, ll_y, ur_x, ur_y;
    char		parsed_name[120];
    int			inverted;
    int			attr_type, attr_size;
    
    grow_GetSliderInfo( object, &direction, 
			&max_value, &min_value, &max_pos, &min_pos);
    if ( min_pos != max_pos) {
      grow_MeasureNode( object, &ll_x, &ll_y, &ur_x, &ur_y);
        
      switch ( direction) {
      case glow_eDirection_Down:
	value = float( (max_pos - ll_y) / (max_pos - min_pos) *
		       (max_value - min_value) + min_value);
	break;
      case glow_eDirection_Right:
	value = float( (max_pos - ll_x) / (max_pos - min_pos) *
		       (max_value - min_value) + min_value);
	break;
      case glow_eDirection_Left:
	value = float( (ll_x - min_pos) / (max_pos - min_pos) *
		       (max_value - min_value) + min_value);
	break;
      default:
	value = float( (ll_y - min_pos) / (max_pos - min_pos) *
		       (max_value - min_value) + min_value);
      }
      if ( value > max_value)
	value = max_value;
      if ( value < min_value)
	value = min_value;
      
      dyn->graph->parse_attr_name( attribute, parsed_name, &inverted, &attr_type, &attr_size);
      switch ( attr_type) {
      case pwr_eType_Float32:
	sts = gdh_SetObjectInfo( parsed_name, &value, sizeof(value));
	break;
      default: {
	pwr_tInt32 ivalue = (pwr_tInt32) (value > 0 ? value + 0.5 : value - 0.5);
	sts = gdh_SetObjectInfo( parsed_name, &ivalue, sizeof(ivalue));
	}
      }
      if ( EVEN(sts)) printf("Slider error: %s\n", attribute);
    }
    break;
  }
  default: ;    
  }
  return 1;
}

int GeSlider::export_java( grow_tObject object, ofstream& fp, bool first, char *var_name)
{
  double dim_x0, dim_x1, dim_y0, dim_y1;
  double min_pos, max_pos, min_value, max_value;
  glow_eDirection direction;
  grow_MeasureJavaBean( dyn->graph->grow->ctx, &dim_x1, &dim_x0, &dim_y1, &dim_y0);
  grow_GetSliderInfo( object, &direction, &max_value, &min_value, &max_pos, &min_pos);
  grow_GetSliderInfoPixel( object, &direction, &max_pos, &min_pos, 
			   ge_mDynType_SliderBackground);

  switch ( direction) {
  case glow_eDirection_Left:
  case glow_eDirection_Right:
    min_pos -= dim_x0;
    max_pos -= dim_x0;
  default:
    min_pos -= dim_y0;
    max_pos -= dim_y0;
  }

  if ( first)
    fp << "      ";
  else
    fp << "      ,";
  fp << "new GeDynSlider(" << var_name << ".dd, \"" << attribute << "\"," 
     << min_value << "," << max_value << "," << direction << ","
     << min_pos << "," << max_pos << ")" << endl;
  return 1;
}

void GeFastCurve::get_attributes( attr_sItem *attrinfo, int *item_count)
{
  int i = *item_count;

  strcpy( attrinfo[i].name, "FastCurve.FastObject");
  attrinfo[i].value = fast_object;
  attrinfo[i].type = glow_eType_String;
  attrinfo[i++].size = sizeof( fast_object);

  *item_count = i;
}

void GeFastCurve::set_attribute( grow_tObject object, char *attr_name, int *cnt)
{
  (*cnt)--;
  if ( *cnt == 0) {
    char msg[200];
    char *s;

    strncpy( fast_object, attr_name, sizeof( fast_object));
    if ( (s = strchr( fast_object, '.')))
      *s = 0;
    sprintf( msg, "FastCurve.FastObject = %s", fast_object);
    dyn->graph->message( 'I', msg);
  }
}

void GeFastCurve::replace_attribute( char *from, char *to, int *cnt, int strict)
{
  GeDyn::replace_attribute( fast_object, sizeof(fast_object), from, to, cnt, strict);
}

void GeFastCurve::save( ofstream& fp)
{
  fp << int(ge_eSave_FastCurve) << endl;
  fp << int(ge_eSave_FastCurve_fast_object) << FSPACE << fast_object << endl;
  fp << int(ge_eSave_End) << endl;
}

void GeFastCurve::open( ifstream& fp)
{
  int		type;
  int 		end_found = 0;
  char		dummy[40];

  for (;;)
  {
    fp >> type;
    switch( type) {
      case ge_eSave_FastCurve: break;
      case ge_eSave_FastCurve_fast_object:
        fp.get();
        fp.getline( fast_object, sizeof(fast_object));
        break;
      case ge_eSave_End: end_found = 1; break;
      default:
        cout << "GeFastCurve:open syntax error" << endl;
        fp.getline( dummy, sizeof(dummy));
    }
    if ( end_found)
      break;
  }
}

int GeFastCurve::connect( grow_tObject object, glow_sTraceData *trace_data)
{
  int		attr_type, attr_size;
  char		parsed_name[120];
  int		sts;
  int		inverted;
  char 		attr_name[120];
  pwr_sClass_DsFastCurve fp;
  int		i;

  dyn->graph->parse_attr_name( fast_object, parsed_name,
				    &inverted, &attr_type, &attr_size);
  sts = gdh_GetObjectInfo( parsed_name, &fp, sizeof(fp));
  if ( EVEN(sts)) return 1;

  max_points = fp.NoOfPoints;
  fast_function = fp.Function;

  if ( fast_function & fast_mFunction_BeforeTrigg) {
    strcpy( attr_name, parsed_name);
    strcat( attr_name, ".TriggIndex");
    gdh_NameToAttrref( pwr_cNObjid, attr_name, &trigg_index_attr);

    strcpy( attr_name, parsed_name);
    strcat( attr_name, ".FirstIndex");
    gdh_NameToAttrref( pwr_cNObjid, attr_name, &first_index_attr);

    strcpy( attr_name, parsed_name);
    strcat( attr_name, ".LastIndex");
    gdh_NameToAttrref( pwr_cNObjid, attr_name, &last_index_attr);
  }


  // Subscribe to object
  strcpy( attr_name, parsed_name);
  strcat( attr_name, ".New");
  sts = dyn->graph->ref_object_info( dyn->cycle, attr_name, (void **)&new_p, &subid, sizeof(pwr_tBoolean));
  if ( EVEN(sts)) return sts;

  memcpy( &time_buff, &fp.TimeBuffer, sizeof(time_buff));

  fast_cnt = 0;
  for ( i = 0; i < FAST_CURVES; i++) {
    if ( fp.CurveValid[i]) {
      memcpy( &buff[fast_cnt], &fp.Buffers[i], sizeof(buff[0]));
      type[fast_cnt] = (pwr_eType) fp.AttributeType[i];
      fast_idx[i] = fast_cnt + 1;
      curve_idx[fast_cnt + 1] = i;
      fast_cnt++;
    }
  }
  
  for ( i = 0; i < fast_cnt; i++) {
    switch( type[i]) {
      case pwr_eType_Float32:
      case pwr_eType_Int32:
      case pwr_eType_UInt32:
        element_size[i] = 4;
        break;
      case pwr_eType_Float64:
        element_size[i] = 8;
        break;
      case pwr_eType_Int16:
      case pwr_eType_UInt16:
        element_size[i] = 2;
        break;
      case pwr_eType_Int8:
      case pwr_eType_UInt8:
        element_size[i] = 1;
        break;
      default:
        element_size[i] = 4;
    }
  }

  if ( new_p)
    trace_data->p = new_p;
  first_scan = true;
  return 1;
}

int GeFastCurve::disconnect( grow_tObject object)
{
  if ( new_p)
    gdh_UnrefObjectInfo( subid);
  new_p = 0;
  return 1;
}

int GeFastCurve::scan( grow_tObject object)
{
  if ( !new_p)
    return 1;

  int i, j, k;
  pwr_tStatus sts;
  int trigg_index, first_index, last_index;
  double *data[DYN_FAST_MAX];

  // Check if any new value
  if ( (*new_p && !old_new) || first_scan) {

    // Update curves
    if ( fast_function & fast_mFunction_BeforeTrigg) {

      // Get first, last and trigg index
      sts = gdh_GetObjectInfoAttrref( &trigg_index_attr, &trigg_index,
				      sizeof(trigg_index));
      if ( EVEN(sts)) return sts;
      sts = gdh_GetObjectInfoAttrref( &first_index_attr, &first_index,
				      sizeof(first_index));
      if ( EVEN(sts)) return sts;
      sts = gdh_GetObjectInfoAttrref( &last_index_attr, &last_index,
				      sizeof(last_index));
      if ( EVEN(sts)) return sts;

       // Read into temporary buffer
      pwr_tFloat32 *tmp = (pwr_tFloat32 *)calloc( max_points, 4);
      data[0] = (double *) calloc( max_points, 8);
      sts = gdh_GetObjectInfoAttrref( &time_buff, tmp,
				      max_points * 4);
      if ( EVEN(sts)) return sts;

      k = first_index;
      for ( j = 0; j < max_points; j++) {
	if ( k >= max_points)
	  k = 0;
	data[0][j] = tmp[k] - tmp[trigg_index];
	if ( k == last_index)
	  break;
	k++;
      }
      // If to few points, fill with dummy data
      for ( ; j < max_points; j++) {
	data[0][j] = tmp[k] - tmp[trigg_index];
      }
      free( tmp);
    }
    else {
      pwr_tFloat32 *tmp = (pwr_tFloat32 *)calloc( max_points, 4);
      data[0] = (double *) calloc( max_points, 8);
      sts = gdh_GetObjectInfoAttrref( &time_buff, tmp,
				      max_points * 4);
      if ( EVEN(sts)) return sts;

      for ( j = 0; j < max_points; j++)
	data[0][j] = tmp[j];
      free( tmp);
    }
    for ( i = 0; i < fast_cnt; i++) {
      if ( fast_function & fast_mFunction_BeforeTrigg) {
	// Read into temporary buffer
	void *tmp = calloc( max_points, element_size[i]);
	data[i + 1] = (double *) calloc( max_points, 8);
	sts = gdh_GetObjectInfoAttrref( &buff[i], tmp, 
		     max_points * element_size[i]);
	if ( EVEN(sts)) return sts;

	k = first_index;
	for ( j = 0; j < max_points; j++) {
	  if ( k >= max_points)
	    k = 0;
	  switch( type[i]) {
	  case pwr_eType_Float32:
	    data[i + 1][j] = ((pwr_tFloat32 *)tmp)[k];
	    break;
	  case pwr_eType_Float64:
	    data[i + 1][j] = ((pwr_tFloat64 *)tmp)[k];
	    break;
	  case pwr_eType_Int32:
	    data[i + 1][j] = ((pwr_tInt32 *)tmp)[k];
	    break;
	  case pwr_eType_UInt32:
	    data[i + 1][j] = ((pwr_tUInt32 *)tmp)[k];
	    break;
	  case pwr_eType_Int16:
	    data[i + 1][j] = ((pwr_tUInt16 *)tmp)[k];
	    break;
	  case pwr_eType_UInt16:
	    data[i + 1][j] = ((pwr_tUInt16 *)tmp)[k];
	    break;
	  case pwr_eType_Int8:
	    data[i + 1][j] = ((pwr_tInt8 *)tmp)[k];
	    break;
	  case pwr_eType_UInt8:
	    data[i + 1][j] = ((pwr_tUInt8 *)tmp)[k];
	    break;
	  default:
	    ;
	  }
	  if ( k == last_index)
	    break;
	  k++;
	}
	// If to few points, fill with 0
	for ( ; j < max_points; j++) {
	  switch( type[i]) {
	  case pwr_eType_Float32:
	    data[i + 1][j] = ((pwr_tFloat32 *)tmp)[k];
	    break;
	  case pwr_eType_Float64:
	    data[i + 1][j] = ((pwr_tFloat64 *)tmp)[k];
	    break;
	  case pwr_eType_Int32:
	    data[i + 1][j] = ((pwr_tInt32 *)tmp)[k];
	    break;
	  case pwr_eType_UInt32:
	    data[i + 1][j] = ((pwr_tUInt32 *)tmp)[k];
	    break;
	  case pwr_eType_Int16:
	    data[i + 1][j] = ((pwr_tInt16 *)tmp)[k];
	    break;
	  case pwr_eType_UInt16:
	    data[i + 1][j] = ((pwr_tInt16 *)tmp)[k];
	    break;
	  case pwr_eType_Int8:
	    data[i + 1][j] = ((pwr_tInt8 *)tmp)[k];
	    break;
	  case pwr_eType_UInt8:
	    data[i + 1][j] = ((pwr_tUInt8 *)tmp)[k];
	    break;
	  default: ;
	  }
	}
	free(tmp);
      }
      else {
	void *tmp = calloc( max_points, element_size[i]);
	data[i + 1] = (double *) calloc( max_points, 8);
	sts = gdh_GetObjectInfoAttrref( &buff[i], tmp, 
	      max_points * element_size[i]);
	if ( EVEN(sts)) return sts;

	for ( j = 0; j < max_points; j++) {
	  switch( type[i]) {
	  case pwr_eType_Float32:
	    data[i + 1][j] = ((pwr_tFloat32 *)tmp)[j];
	    break;
	  case pwr_eType_Float64:
	    data[i + 1][j] = ((pwr_tFloat64 *)tmp)[j];
	    break;
	  case pwr_eType_Int32:
	    data[i + 1][j] = ((pwr_tInt32 *)tmp)[j];
	    break;
	  case pwr_eType_UInt32:
	    data[i + 1][j] = ((pwr_tUInt32 *)tmp)[j];
	    break;
	  case pwr_eType_Int16:
	    data[i + 1][j] = ((pwr_tInt16 *)tmp)[j];
	    break;
	  case pwr_eType_UInt16:
	    data[i + 1][j] = ((pwr_tInt16 *)tmp)[j];
	    break;
	  case pwr_eType_Int8:
	    data[i + 1][j] = ((pwr_tInt8 *)tmp)[j];
	    break;
	  case pwr_eType_UInt8:
	    data[i + 1][j] = ((pwr_tUInt8 *)tmp)[j];
	    break;
	  default: ;
	  }
	}
	free(tmp);
      }
    }
    grow_SetTrendData( object, data, MIN( fast_cnt + 1, 3), max_points);
    for ( i = 0; i < fast_cnt; i++)
      free( data[i]);
    first_scan = 0;
  }
  old_new = *new_p;

  return 1;
}

void GePulldownMenu::get_attributes( attr_sItem *attrinfo, int *item_count)
{
  int i = *item_count;
  
  int b_mask = ge_mInstance_1;
  for ( int j = 0; j < 32; j++) {
    if ( b_mask & button_mask && !items_dyn[j]) {
      items_dyn[j] = new GeDyn( dyn->graph, ge_eDynAttr_Menu);
    }
    b_mask = b_mask << 1;
  }

  strcpy( attrinfo[i].name, "PulldownMenu.ItemMask");
  attrinfo[i].value = &button_mask;
  attrinfo[i].type = ge_eAttrType_InstanceMask;
  attrinfo[i++].size = sizeof( button_mask);

  b_mask = ge_mInstance_1;
  for ( int j = 0; j < 32; j++) {
    if ( b_mask & button_mask) {
      sprintf( attrinfo[i].name, "PulldownMenu.ItemText%d", j+1);
      attrinfo[i].value = items_text[j];
      attrinfo[i].type = glow_eType_String;
      attrinfo[i++].size = sizeof( items_text[0]);

      sprintf( attrinfo[i].name, "PulldownMenu.ItemDyn%d", j+1);
      attrinfo[i].value = items_dyn[j];
      attrinfo[i].type = ge_eAttrType_Dyn;
      attrinfo[i++].size = 0;
    }
    b_mask = b_mask << 1;
  }
  dyn->display_access = true;
  *item_count = i;
}

int GePulldownMenu::get_transtab( char **tt)
{
  static char transtab[][32] = {	"SubGraph",		"SubGraph",
					"A1",			"Text",
					"Dynamic",		"",
					""};

  *tt = (char *) transtab;
  return 0;
}

void GePulldownMenu::save( ofstream& fp)
{
  fp << int(ge_eSave_PulldownMenu) << endl;  
  fp << int(ge_eSave_PulldownMenu_button_mask) << FSPACE << int(button_mask) << endl;
  int b_mask = ge_mInstance_1;
  for ( int j = 0; j < 32; j++) {
    if ( b_mask & button_mask) {
      fp << int(ge_eSave_PulldownMenu_items_text0+j) << FSPACE << items_text[j] << endl;
      fp << int(ge_eSave_PulldownMenu_items_dyn0+j) << endl;
      items_dyn[j]->save( fp);
    }
    b_mask = b_mask << 1;
  }
  fp << int(ge_eSave_End) << endl;
}

void GePulldownMenu::open( ifstream& fp)
{
  int		type;
  int 		end_found = 0;
  char		dummy[40];
  int		tmp;

  for (;;)
  {
    fp >> type;
    switch( type) {
      case ge_eSave_PulldownMenu: break;
      case ge_eSave_PulldownMenu_button_mask: fp >> tmp; button_mask = (ge_mInstance)tmp; break;
      case ge_eSave_PulldownMenu_items_text0:
        fp.get();
        fp.getline( items_text[0], sizeof(items_text[0]));
        break;
      case ge_eSave_PulldownMenu_items_text1:
        fp.get();
        fp.getline( items_text[1], sizeof(items_text[0]));
        break;
      case ge_eSave_PulldownMenu_items_text2:
        fp.get();
        fp.getline( items_text[2], sizeof(items_text[0]));
        break;
      case ge_eSave_PulldownMenu_items_text3:
        fp.get();
        fp.getline( items_text[3], sizeof(items_text[0]));
        break;
      case ge_eSave_PulldownMenu_items_text4:
        fp.get();
        fp.getline( items_text[4], sizeof(items_text[0]));
        break;
      case ge_eSave_PulldownMenu_items_text5:
        fp.get();
        fp.getline( items_text[5], sizeof(items_text[0]));
        break;
      case ge_eSave_PulldownMenu_items_text6:
        fp.get();
        fp.getline( items_text[6], sizeof(items_text[0]));
        break;
      case ge_eSave_PulldownMenu_items_text7:
        fp.get();
        fp.getline( items_text[7], sizeof(items_text[0]));
        break;
      case ge_eSave_PulldownMenu_items_text8:
        fp.get();
        fp.getline( items_text[8], sizeof(items_text[0]));
        break;
      case ge_eSave_PulldownMenu_items_text9:
        fp.get();
        fp.getline( items_text[9], sizeof(items_text[0]));
        break;
      case ge_eSave_PulldownMenu_items_text10:
        fp.get();
        fp.getline( items_text[10], sizeof(items_text[0]));
        break;
      case ge_eSave_PulldownMenu_items_text11:
        fp.get();
        fp.getline( items_text[11], sizeof(items_text[0]));
        break;
      case ge_eSave_PulldownMenu_items_text12:
        fp.get();
        fp.getline( items_text[12], sizeof(items_text[0]));
        break;
      case ge_eSave_PulldownMenu_items_text13:
        fp.get();
        fp.getline( items_text[13], sizeof(items_text[0]));
        break;
      case ge_eSave_PulldownMenu_items_text14:
        fp.get();
        fp.getline( items_text[14], sizeof(items_text[0]));
        break;
      case ge_eSave_PulldownMenu_items_text15:
        fp.get();
        fp.getline( items_text[15], sizeof(items_text[0]));
        break;
      case ge_eSave_PulldownMenu_items_text16:
        fp.get();
        fp.getline( items_text[16], sizeof(items_text[0]));
        break;
      case ge_eSave_PulldownMenu_items_text17:
        fp.get();
        fp.getline( items_text[17], sizeof(items_text[0]));
        break;
      case ge_eSave_PulldownMenu_items_text18:
        fp.get();
        fp.getline( items_text[18], sizeof(items_text[0]));
        break;
      case ge_eSave_PulldownMenu_items_text19:
        fp.get();
        fp.getline( items_text[19], sizeof(items_text[0]));
        break;
      case ge_eSave_PulldownMenu_items_text20:
        fp.get();
        fp.getline( items_text[20], sizeof(items_text[0]));
        break;
      case ge_eSave_PulldownMenu_items_text21:
        fp.get();
        fp.getline( items_text[21], sizeof(items_text[0]));
        break;
      case ge_eSave_PulldownMenu_items_text22:
        fp.get();
        fp.getline( items_text[22], sizeof(items_text[0]));
        break;
      case ge_eSave_PulldownMenu_items_text23:
        fp.get();
        fp.getline( items_text[23], sizeof(items_text[0]));
        break;
      case ge_eSave_PulldownMenu_items_text24:
        fp.get();
        fp.getline( items_text[24], sizeof(items_text[0]));
        break;
      case ge_eSave_PulldownMenu_items_text25:
        fp.get();
        fp.getline( items_text[25], sizeof(items_text[0]));
        break;
      case ge_eSave_PulldownMenu_items_text26:
        fp.get();
        fp.getline( items_text[26], sizeof(items_text[0]));
        break;
      case ge_eSave_PulldownMenu_items_text27:
        fp.get();
        fp.getline( items_text[27], sizeof(items_text[0]));
        break;
      case ge_eSave_PulldownMenu_items_text28:
        fp.get();
        fp.getline( items_text[28], sizeof(items_text[0]));
        break;
      case ge_eSave_PulldownMenu_items_text29:
        fp.get();
        fp.getline( items_text[29], sizeof(items_text[0]));
        break;
      case ge_eSave_PulldownMenu_items_text30:
        fp.get();
        fp.getline( items_text[30], sizeof(items_text[0]));
        break;
      case ge_eSave_PulldownMenu_items_text31:
        fp.get();
        fp.getline( items_text[31], sizeof(items_text[0]));
        break;
      case ge_eSave_PulldownMenu_items_dyn0:
	items_dyn[0]->open( fp);
	break;
      case ge_eSave_PulldownMenu_items_dyn1:
	if ( !items_dyn[1])
	  items_dyn[1] = new GeDyn( dyn->graph, ge_eDynAttr_Menu);
	items_dyn[1]->open( fp);
	break;
      case ge_eSave_PulldownMenu_items_dyn2:
	if ( !items_dyn[2])
	  items_dyn[2] = new GeDyn( dyn->graph, ge_eDynAttr_Menu);
	items_dyn[2]->open( fp);
	break;
      case ge_eSave_PulldownMenu_items_dyn3:
	if ( !items_dyn[3])
	  items_dyn[3] = new GeDyn( dyn->graph, ge_eDynAttr_Menu);
	items_dyn[3]->open( fp);
	break;
      case ge_eSave_PulldownMenu_items_dyn4:
	if ( !items_dyn[4])
	  items_dyn[4] = new GeDyn( dyn->graph, ge_eDynAttr_Menu);
	items_dyn[4]->open( fp);
	break;
      case ge_eSave_PulldownMenu_items_dyn5:
	if ( !items_dyn[5])
	  items_dyn[5] = new GeDyn( dyn->graph, ge_eDynAttr_Menu);
	items_dyn[5]->open( fp);
	break;
      case ge_eSave_PulldownMenu_items_dyn6:
	if ( !items_dyn[6])
	  items_dyn[6] = new GeDyn( dyn->graph, ge_eDynAttr_Menu);
	items_dyn[6]->open( fp);
	break;
      case ge_eSave_PulldownMenu_items_dyn7:
	if ( !items_dyn[7])
	  items_dyn[7] = new GeDyn( dyn->graph, ge_eDynAttr_Menu);
	items_dyn[7]->open( fp);
	break;
      case ge_eSave_PulldownMenu_items_dyn8:
	if ( !items_dyn[8])
	  items_dyn[8] = new GeDyn( dyn->graph, ge_eDynAttr_Menu);
	items_dyn[8]->open( fp);
	break;
      case ge_eSave_PulldownMenu_items_dyn9:
	if ( !items_dyn[9])
	  items_dyn[9] = new GeDyn( dyn->graph, ge_eDynAttr_Menu);
	items_dyn[9]->open( fp);
	break;
      case ge_eSave_PulldownMenu_items_dyn10:
	if ( !items_dyn[10])
	  items_dyn[10] = new GeDyn( dyn->graph, ge_eDynAttr_Menu);
	items_dyn[10]->open( fp);
	break;
      case ge_eSave_PulldownMenu_items_dyn11:
	if ( !items_dyn[11])
	  items_dyn[11] = new GeDyn( dyn->graph, ge_eDynAttr_Menu);
	items_dyn[11]->open( fp);
	break;
      case ge_eSave_PulldownMenu_items_dyn12:
	if ( !items_dyn[12])
	  items_dyn[12] = new GeDyn( dyn->graph, ge_eDynAttr_Menu);
	items_dyn[12]->open( fp);
	break;
      case ge_eSave_PulldownMenu_items_dyn13:
	if ( !items_dyn[13])
	  items_dyn[13] = new GeDyn( dyn->graph, ge_eDynAttr_Menu);
	items_dyn[13]->open( fp);
	break;
      case ge_eSave_PulldownMenu_items_dyn14:
	if ( !items_dyn[14])
	  items_dyn[14] = new GeDyn( dyn->graph, ge_eDynAttr_Menu);
	items_dyn[14]->open( fp);
	break;
      case ge_eSave_PulldownMenu_items_dyn15:
	if ( !items_dyn[15])
	  items_dyn[15] = new GeDyn( dyn->graph, ge_eDynAttr_Menu);
	items_dyn[15]->open( fp);
	break;
      case ge_eSave_PulldownMenu_items_dyn16:
	if ( !items_dyn[16])
	  items_dyn[16] = new GeDyn( dyn->graph, ge_eDynAttr_Menu);
	items_dyn[16]->open( fp);
	break;
      case ge_eSave_PulldownMenu_items_dyn17:
	if ( !items_dyn[17])
	  items_dyn[17] = new GeDyn( dyn->graph, ge_eDynAttr_Menu);
	items_dyn[17]->open( fp);
	break;
      case ge_eSave_PulldownMenu_items_dyn18:
	if ( !items_dyn[18])
	  items_dyn[18] = new GeDyn( dyn->graph, ge_eDynAttr_Menu);
	items_dyn[18]->open( fp);
	break;
      case ge_eSave_PulldownMenu_items_dyn19:
	if ( !items_dyn[19])
	  items_dyn[19] = new GeDyn( dyn->graph, ge_eDynAttr_Menu);
	items_dyn[19]->open( fp);
	break;
      case ge_eSave_PulldownMenu_items_dyn20:
	if ( !items_dyn[20])
	  items_dyn[20] = new GeDyn( dyn->graph, ge_eDynAttr_Menu);
	items_dyn[20]->open( fp);
	break;
      case ge_eSave_PulldownMenu_items_dyn21:
	if ( !items_dyn[21])
	  items_dyn[21] = new GeDyn( dyn->graph, ge_eDynAttr_Menu);
	items_dyn[21]->open( fp);
	break;
      case ge_eSave_PulldownMenu_items_dyn22:
	if ( !items_dyn[22])
	  items_dyn[22] = new GeDyn( dyn->graph, ge_eDynAttr_Menu);
	items_dyn[22]->open( fp);
	break;
      case ge_eSave_PulldownMenu_items_dyn23:
	if ( !items_dyn[23])
	  items_dyn[23] = new GeDyn( dyn->graph, ge_eDynAttr_Menu);
	items_dyn[23]->open( fp);
	break;
      case ge_eSave_PulldownMenu_items_dyn24:
	if ( !items_dyn[24])
	  items_dyn[24] = new GeDyn( dyn->graph, ge_eDynAttr_Menu);
	items_dyn[24]->open( fp);
	break;
      case ge_eSave_PulldownMenu_items_dyn25:
	if ( !items_dyn[25])
	  items_dyn[25] = new GeDyn( dyn->graph, ge_eDynAttr_Menu);
	items_dyn[25]->open( fp);
	break;
      case ge_eSave_PulldownMenu_items_dyn26:
	if ( !items_dyn[26])
	  items_dyn[26] = new GeDyn( dyn->graph, ge_eDynAttr_Menu);
	items_dyn[26]->open( fp);
	break;
      case ge_eSave_PulldownMenu_items_dyn27:
	if ( !items_dyn[27])
	  items_dyn[27] = new GeDyn( dyn->graph, ge_eDynAttr_Menu);
	items_dyn[27]->open( fp);
	break;
      case ge_eSave_PulldownMenu_items_dyn28:
	if ( !items_dyn[28])
	  items_dyn[28] = new GeDyn( dyn->graph, ge_eDynAttr_Menu);
	items_dyn[28]->open( fp);
	break;
      case ge_eSave_PulldownMenu_items_dyn29:
	if ( !items_dyn[29])
	  items_dyn[29] = new GeDyn( dyn->graph, ge_eDynAttr_Menu);
	items_dyn[29]->open( fp);
	break;
      case ge_eSave_PulldownMenu_items_dyn30:
	if ( !items_dyn[30])
	  items_dyn[30] = new GeDyn( dyn->graph, ge_eDynAttr_Menu);
	items_dyn[30]->open( fp);
	break;
      case ge_eSave_PulldownMenu_items_dyn31:
	if ( !items_dyn[31])
	  items_dyn[31] = new GeDyn( dyn->graph, ge_eDynAttr_Menu);
	items_dyn[31]->open( fp);
	break;
      case ge_eSave_End: end_found = 1; break;
      default:
        cout << "GePulldownMenu:open syntax error" << endl;
        fp.getline( dummy, sizeof(dummy));
    }
    if ( end_found)
      break;
  }  
}

int GePulldownMenu::action( grow_tObject object, glow_tEvent event)
{
  if ( !dyn->graph->is_authorized( dyn->access))
    return 1;

  switch ( event->event) {
  case glow_eEvent_MB1Down:
    break;
  case glow_eEvent_MB1Up:
    break;
  case glow_eEvent_Key_Up:
    if ( !menu_object)
      break;
    if ( grow_GetMenuInputFocus( menu_object)) {
      grow_MenuShiftCurrentItem( menu_object, -1);
      return GE__NO_PROPAGATE;
    }
    else {
      int sts;
      for ( int j = 0; j < 32; j++) {
	if ( items_dyn[j] && 
	     items_dyn[j]->action_type & ge_mActionType_PulldownMenu) {
	  sts = items_dyn[j]->action( 0, event);
	  if ( sts == GE__NO_PROPAGATE) return sts;
	}
      }
    }
    break;
  case glow_eEvent_Key_Down:
    if ( !menu_object)
      break;
    if ( grow_GetMenuInputFocus( menu_object)) {
      grow_MenuShiftCurrentItem( menu_object, 1);
      return GE__NO_PROPAGATE;
    }
    else {
      int sts;
      for ( int j = 0; j < 32; j++) {
	if ( items_dyn[j] && 
	     items_dyn[j]->action_type & ge_mActionType_PulldownMenu) {
	  sts = items_dyn[j]->action( 0, event);
	  if ( sts == GE__NO_PROPAGATE) return sts;
	}
      }
    }
    break;
  case glow_eEvent_Key_Right:
    if ( !menu_object)
      break;

    if ( grow_GetMenuInputFocus( menu_object)) {
      int item, sts;
      sts = grow_MenuGetCurrentItem( menu_object, &item);
      if ( EVEN(sts))
	break;
      
      if ( items_dyn[item] && 
	   items_dyn[item]->action_type & ge_mActionType_PulldownMenu) {
	// Send create menu event
	glow_sEvent e;
	e.event = glow_eEvent_MenuCreate;
	e.menu.item = item;
	e.menu.object = menu_object;
	grow_GetSubmenuPosition( menu_object, item, &e.menu.x, &e.menu.y);
	action( object, &e);
	return GE__NO_PROPAGATE;
      }
    }
    else {
      int sts;
      for ( int j = 0; j < 32; j++) {
	if ( items_dyn[j] &&
	     items_dyn[j]->action_type & ge_mActionType_PulldownMenu) {
	  sts = items_dyn[j]->action( 0, event);
	  if ( sts == GE__NO_PROPAGATE) return sts;
	}
      }
    }
    break;
  case glow_eEvent_Key_Left:
    if ( !menu_object)
      break;

    if ( !object && grow_GetMenuInputFocus( menu_object)) {
      // Delete this and all child menues
      glow_sEvent e;
      e.event = glow_eEvent_MenuDelete;
      e.menu.object = 0;
      for ( int j = 0; j < 32; j++) {
	if ( items_dyn[j] && 
	     items_dyn[j]->action_type & ge_mActionType_PulldownMenu)
	  items_dyn[j]->action( 0, &e);
      }

      grow_DeleteObject( dyn->graph->grow->ctx, menu_object);
      menu_object = 0;
      return GE__NO_PROPAGATE;
    }
    else {
      int sts;
      for ( int j = 0; j < 32; j++) {
	if ( items_dyn[j] &&
	     items_dyn[j]->action_type & ge_mActionType_PulldownMenu) {
	  sts = items_dyn[j]->action( 0, event);
	  if ( sts == GE__NO_PROPAGATE) return sts;
	}
      }
    }
    break;
  case glow_eEvent_Key_Return:
    if ( !menu_object)
      break;

    if ( grow_GetMenuInputFocus( menu_object)) {
      // Send menu activated event
      int sts;
      glow_sEvent e;
      e.event = glow_eEvent_MenuActivated;
      e.menu.object = menu_object;
      sts = grow_MenuGetCurrentItem( menu_object, &e.menu.item);
      if ( EVEN(sts))
	break;

      int close_graph = 0;
      if ( items_dyn[e.menu.item] && items_dyn[e.menu.item]->action_type & ge_mActionType_CloseGraph)
	close_graph = 1;

      action( object, &e);

      if ( !close_graph) {
	// Close any open menu
	Graph *graph = dyn->graph;
	glow_sEvent e;
	grow_tObject 	*objectlist;
	int 		object_cnt, cnt;
	int		i;

	e.event = glow_eEvent_MenuDelete;
	e.any.type = glow_eEventType_Menu;
	e.menu.object = 0;
	
	grow_GetObjectList( graph->grow->ctx, &objectlist, &object_cnt);
	for ( i = 0; i < object_cnt; i++) {
	  if ( grow_GetObjectType( objectlist[i]) == glow_eObjectType_GrowNode ||
	       grow_GetObjectType( objectlist[i]) == glow_eObjectType_GrowGroup) {
	    grow_GetUserData( objectlist[i], (void **)&dyn);
	    dyn->action( objectlist[i], &e);
	    grow_GetObjectList( graph->grow->ctx, &objectlist, &cnt);
	    if ( cnt != object_cnt)
	      // Something is deleted
	      break;
	  }
	}
	return GE__NO_PROPAGATE;
      }
    }
    else {
      for ( int j = 0; j < 32; j++) {
	if ( items_dyn[j] &&
	     items_dyn[j]->action_type & ge_mActionType_PulldownMenu)
	  items_dyn[j]->action( object, event);
      }
    }
    break;

  case glow_eEvent_InputFocusGained:
    if ( menu_object)
      break;
    clock_gettime( CLOCK_REALTIME, &focus_gained_time);
  case glow_eEvent_MB1Click:
    if ( event->event == glow_eEvent_MB1Click) {
      pwr_tTime now;
      clock_gettime( CLOCK_REALTIME, &now);
      if ( abs(now.tv_sec - focus_gained_time.tv_sec) < 2)
	break;
    }
    if ( menu_object) {
      // Close, delete this menu and all childmenues
      for ( int j = 0; j < 32; j++) {
	if ( items_dyn[j] && 
	     items_dyn[j]->action_type & ge_mActionType_PulldownMenu)
	  items_dyn[j]->action( 0, event);
      }

      grow_DeleteObject( dyn->graph->grow->ctx, menu_object);
      menu_object = 0;
    }
    else if ( object) {
      double	ll_x, ll_y, ur_x, ur_y;
      glow_sMenuInfo info;

      int b_mask = 1;
      for ( int i = 0; i < 32; i++) {
	if ( b_mask & button_mask) {
	  info.item[i].occupied = true;
	  strcpy( info.item[i].text, items_text[i]);

	  // Check access
	  if ( items_dyn[i]->action_type & ge_mActionType_PulldownMenu)
	    info.item[i].type = glow_eMenuItem_PulldownMenu;
	  else {
	    // Check access
	    if ( dyn->graph->is_authorized( items_dyn[i]->access))
	      info.item[i].type = glow_eMenuItem_Button;
	    else
	      info.item[i].type = glow_eMenuItem_ButtonDisabled;
	  }
	}
	else
	  info.item[i].occupied = false;
	b_mask = b_mask << 1;
      }

      // Get fillcolor, and textattributes from object
      glow_eDrawType text_drawtype, text_color, bg_color;
      int text_size;
      int sts;

      sts = grow_GetObjectAnnotInfo( object, 1, &text_size, &text_drawtype, &text_color, &bg_color);
      if ( EVEN(sts)) {
	text_size = 2;
	text_drawtype = glow_eDrawType_TextHelveticaBold;
	text_color = glow_eDrawType_Line;
	bg_color = glow_eDrawType_LightGray;
      }
      else if ( bg_color == glow_eDrawType_No || bg_color == glow_eDrawType_Inherit)
	bg_color = glow_eDrawType_LightGray;
	
      grow_MeasureNode( object, &ll_x, &ll_y, &ur_x, &ur_y);
      grow_CreateGrowMenu( dyn->graph->grow->ctx, "__Menu", &info, ll_x, ur_y, ur_x - ll_x,
			   glow_eDrawType_Line, 0, 1, 1, bg_color, text_size,
			   text_drawtype, text_color,
			   glow_eDrawType_MediumGray, 0,
			   &menu_object);
      grow_SetMenuInputFocus( menu_object, 1);
    }
    break;
  case glow_eEvent_MenuActivated:
    if ( !menu_object)
      break;
    if ( event->menu.object == menu_object) {
      if ( items_dyn[event->menu.item]) {
 	glow_sEvent e;
	e.event = glow_eEvent_MB1Click;
	items_dyn[event->menu.item]->action( event->menu.object, &e);
      }
    }
    else {
      for ( int j = 0; j < 32; j++) {
	if ( items_dyn[j] && 
	     items_dyn[j]->action_type & ge_mActionType_PulldownMenu)
	  items_dyn[j]->action( 0, event);
      }
    }
    break;
  case glow_eEvent_MenuCreate:
    if ( object && !(grow_GetObjectType( object) == glow_eObjectType_GrowMenu)) {
      // Parent menu, call submenues
      if ( event->menu.object == menu_object) {
	// Call specified item to create the menu
	// send menu_object as object arg to pass parent menu
	if ( items_dyn[event->menu.item])
	  items_dyn[event->menu.item]->action( menu_object, event);
      }
      else {
	// Send event to all child menu items
	for ( int j = 0; j < 32; j++) {
	  if ( items_dyn[j] && items_dyn[j]->action_type & ge_mActionType_PulldownMenu)
	    items_dyn[j]->action( 0, event);
	}
      }
    }
    else {
      // A submenu
      if ( object) {
	// Create this menu
	glow_sMenuInfo info;

	int b_mask = 1;
	for ( int i = 0; i < 32; i++) {
	  if ( b_mask & button_mask) {
	    info.item[i].occupied = true;
	    strcpy( info.item[i].text, items_text[i]);
	    if ( items_dyn[i]->action_type & ge_mActionType_PulldownMenu)
	      info.item[i].type = glow_eMenuItem_PulldownMenu;
	    else {
	      // Check access
	      if ( dyn->graph->is_authorized( items_dyn[i]->access))
		info.item[i].type = glow_eMenuItem_Button;
	      else
		info.item[i].type = glow_eMenuItem_ButtonDisabled;
	    }
	  }
	  else
	    info.item[i].occupied = false;
	  b_mask = b_mask << 1;
	}

	glow_eDrawType text_drawtype, text_color, bg_color, text_color_disabled;
	int text_size;

	grow_GetMenuChar( object, &text_size, &bg_color, &text_drawtype, &text_color, 
			  &text_color_disabled);

	grow_CreateGrowMenu( dyn->graph->grow->ctx, "__Menu", &info, event->menu.x, event->menu.y, 0,
			     glow_eDrawType_Line, 0, 1, 1, bg_color, text_size,
			     text_drawtype, text_color,
			     text_color_disabled, object,
			     &menu_object);
	grow_SetMenuInputFocus( object, 0);
	grow_SetMenuInputFocus( menu_object, 1);
      }
      else {
	if ( menu_object == event->menu.object) {
	  // Call specified item to create the menu
	  // send menu_object as object arg to pass parent menu
	  if ( items_dyn[event->menu.item])
	    items_dyn[event->menu.item]->action( menu_object, event);
	}
	else {
	  // Send event to child menu items
	  for ( int j = 0; j < 32; j++) {
	    if ( items_dyn[j] && items_dyn[j]->action_type & ge_mActionType_PulldownMenu)
	      items_dyn[j]->action( 0, event);
	  }
	}
      }
    }
    break;
  case glow_eEvent_InputFocusLost: {
    // Delete this and all child menues
    glow_sEvent e;
    e.event = glow_eEvent_MenuDelete;
    e.menu.object = 0;

    for ( int j = 0; j < 32; j++) {
      if ( items_dyn[j] && 
	   items_dyn[j]->action_type & ge_mActionType_PulldownMenu)
	items_dyn[j]->action( 0, &e);
    }
    grow_DeleteObject( dyn->graph->grow->ctx, menu_object);
    menu_object = 0;
    break;
  }
  case glow_eEvent_MenuDelete:
    if ( menu_object == 0)
      break;
    if ( event->menu.object == 0) {
      // Delete this and all child menues
      for ( int j = 0; j < 32; j++) {
	if ( items_dyn[j] && 
	     items_dyn[j]->action_type & ge_mActionType_PulldownMenu)
	  items_dyn[j]->action( 0, event);
      }
      grow_DeleteObject( dyn->graph->grow->ctx, menu_object);
      menu_object = 0;
    }
    else {
      if ( event->menu.object == menu_object) {
	// Delete this and all child menues
	event->menu.object = 0;
	for ( int j = 0; j < 32; j++) {
	  if ( items_dyn[j] && 
	       items_dyn[j]->action_type & ge_mActionType_PulldownMenu)
	    items_dyn[j]->action( 0, event);
	}
	event->menu.object = menu_object;

	grow_DeleteObject( dyn->graph->grow->ctx, menu_object);
	menu_object = 0;
      }
      else {
	for ( int j = 0; j < 32; j++) {
	  if ( items_dyn[j] && 
	       items_dyn[j]->action_type & ge_mActionType_PulldownMenu)
	    items_dyn[j]->action( 0, event);
	}
      }
    }
    break;
  default: ;
  }
  return 1;
}

int GePulldownMenu::export_java( grow_tObject object, ofstream& fp, bool first, char *var_name)
{
  if ( first)
    fp << "      ";
  else
    fp << "      ,";
  fp << "new GeDynPulldownMenu(" << var_name << ".dd, new String[] {" << endl << "        ";
  int b_mask = 1;
  for ( int i = 0; i < 32; i++) {
    if ( b_mask & button_mask)
      fp << "\"" << items_text[i] << "\"";
    else
      fp << "null";
    if ( i != 31)
      fp << ",";
    b_mask = b_mask << 1;
  }
  fp << "}," << endl << "        new GeDyn[] {" << endl << "        ";
  b_mask = 1;
  for ( int i = 0; i < 32; i++) {
    if ( b_mask & button_mask)
      items_dyn[i]->export_java_object( object, fp, var_name);
    else {
      fp << "null";
      if ( i != 31)
	fp << ",";
    }
    b_mask = b_mask << 1;
  }
  fp << endl << "      })" << endl;
  return 1;
}


void GeOptionMenu::get_attributes( attr_sItem *attrinfo, int *item_count)
{
  int i = *item_count;
  
  strcpy( attrinfo[i].name, "OptionMenu.Attribute");
  attrinfo[i].value = attribute;
  attrinfo[i].type = glow_eType_String;
  attrinfo[i++].size = sizeof( attribute);

  strcpy( attrinfo[i].name, "OptionMenu.ItemMask");
  attrinfo[i].value = &button_mask;
  attrinfo[i].type = ge_eAttrType_InstanceMask;
  attrinfo[i++].size = sizeof( button_mask);

  int b_mask = ge_mInstance_1;
  for ( int j = 0; j < 32; j++) {
    if ( b_mask & button_mask) {
      sprintf( attrinfo[i].name, "OptionMenu.ItemText%d", j+1);
      attrinfo[i].value = items_text[j];
      attrinfo[i].type = glow_eType_String;
      attrinfo[i++].size = sizeof( items_text[0]);

      sprintf( attrinfo[i].name, "OptionMenu.ItemEnum%d", j+1);
      attrinfo[i].value = &items_enum[j];
      attrinfo[i].type = glow_eType_Int;
      attrinfo[i++].size = sizeof(items_enum[0]);
    }
    b_mask = b_mask << 1;
  }
  dyn->display_access = true;
  *item_count = i;
}

int GeOptionMenu::get_transtab( char **tt)
{
  static char transtab[][32] = {	"SubGraph",		"SubGraph",
					"A1",			"",
					"Dynamic",		"",
					""};

  *tt = (char *) transtab;
  return 0;
}

void GeOptionMenu::set_attribute( grow_tObject object, char *attr_name, int *cnt)
{
  (*cnt)--;
  if ( *cnt == 0) {
    char msg[200];

    strncpy( attribute, attr_name, sizeof( attribute));
    sprintf( msg, "OptionMenu.Attribute = %s", attr_name);
    dyn->graph->message( 'I', msg);
  }
}

void GeOptionMenu::replace_attribute( char *from, char *to, int *cnt, int strict)
{
  GeDyn::replace_attribute( attribute, sizeof(attribute), from, to, cnt, strict);
}

void GeOptionMenu::save( ofstream& fp)
{
  fp << int(ge_eSave_OptionMenu) << endl;  
  fp << int(ge_eSave_OptionMenu_attribute) << FSPACE << attribute << endl;
  fp << int(ge_eSave_OptionMenu_button_mask) << FSPACE << int(button_mask) << endl;
  int b_mask = ge_mInstance_1;
  for ( int j = 0; j < 32; j++) {
    if ( b_mask & button_mask) {
      fp << int(ge_eSave_OptionMenu_items_text0+j) << FSPACE << items_text[j] << endl;
      fp << int(ge_eSave_OptionMenu_items_enum0+j) << FSPACE << items_enum[j] << endl;
    }
    b_mask = b_mask << 1;
  }
  fp << int(ge_eSave_End) << endl;
}

void GeOptionMenu::open( ifstream& fp)
{
  int		type;
  int 		end_found = 0;
  char		dummy[40];
  int		tmp;

  for (;;)
  {
    fp >> type;
    switch( type) {
      case ge_eSave_OptionMenu: break;
      case ge_eSave_OptionMenu_attribute:
        fp.get();
        fp.getline( attribute, sizeof(attribute));
        break;
      case ge_eSave_OptionMenu_button_mask: fp >> tmp; button_mask = (ge_mInstance)tmp; break;
      case ge_eSave_OptionMenu_items_text0:
        fp.get();
        fp.getline( items_text[0], sizeof(items_text[0]));
        break;
      case ge_eSave_OptionMenu_items_text1:
        fp.get();
        fp.getline( items_text[1], sizeof(items_text[0]));
        break;
      case ge_eSave_OptionMenu_items_text2:
        fp.get();
        fp.getline( items_text[2], sizeof(items_text[0]));
        break;
      case ge_eSave_OptionMenu_items_text3:
        fp.get();
        fp.getline( items_text[3], sizeof(items_text[0]));
        break;
      case ge_eSave_OptionMenu_items_text4:
        fp.get();
        fp.getline( items_text[4], sizeof(items_text[0]));
        break;
      case ge_eSave_OptionMenu_items_text5:
        fp.get();
        fp.getline( items_text[5], sizeof(items_text[0]));
        break;
      case ge_eSave_OptionMenu_items_text6:
        fp.get();
        fp.getline( items_text[6], sizeof(items_text[0]));
        break;
      case ge_eSave_OptionMenu_items_text7:
        fp.get();
        fp.getline( items_text[7], sizeof(items_text[0]));
        break;
      case ge_eSave_OptionMenu_items_text8:
        fp.get();
        fp.getline( items_text[8], sizeof(items_text[0]));
        break;
      case ge_eSave_OptionMenu_items_text9:
        fp.get();
        fp.getline( items_text[9], sizeof(items_text[0]));
        break;
      case ge_eSave_OptionMenu_items_text10:
        fp.get();
        fp.getline( items_text[10], sizeof(items_text[0]));
        break;
      case ge_eSave_OptionMenu_items_text11:
        fp.get();
        fp.getline( items_text[11], sizeof(items_text[0]));
        break;
      case ge_eSave_OptionMenu_items_text12:
        fp.get();
        fp.getline( items_text[12], sizeof(items_text[0]));
        break;
      case ge_eSave_OptionMenu_items_text13:
        fp.get();
        fp.getline( items_text[13], sizeof(items_text[0]));
        break;
      case ge_eSave_OptionMenu_items_text14:
        fp.get();
        fp.getline( items_text[14], sizeof(items_text[0]));
        break;
      case ge_eSave_OptionMenu_items_text15:
        fp.get();
        fp.getline( items_text[15], sizeof(items_text[0]));
        break;
      case ge_eSave_OptionMenu_items_text16:
        fp.get();
        fp.getline( items_text[16], sizeof(items_text[0]));
        break;
      case ge_eSave_OptionMenu_items_text17:
        fp.get();
        fp.getline( items_text[17], sizeof(items_text[0]));
        break;
      case ge_eSave_OptionMenu_items_text18:
        fp.get();
        fp.getline( items_text[18], sizeof(items_text[0]));
        break;
      case ge_eSave_OptionMenu_items_text19:
        fp.get();
        fp.getline( items_text[19], sizeof(items_text[0]));
        break;
      case ge_eSave_OptionMenu_items_text20:
        fp.get();
        fp.getline( items_text[20], sizeof(items_text[0]));
        break;
      case ge_eSave_OptionMenu_items_text21:
        fp.get();
        fp.getline( items_text[21], sizeof(items_text[0]));
        break;
      case ge_eSave_OptionMenu_items_text22:
        fp.get();
        fp.getline( items_text[22], sizeof(items_text[0]));
        break;
      case ge_eSave_OptionMenu_items_text23:
        fp.get();
        fp.getline( items_text[23], sizeof(items_text[0]));
        break;
      case ge_eSave_OptionMenu_items_text24:
        fp.get();
        fp.getline( items_text[24], sizeof(items_text[0]));
        break;
      case ge_eSave_OptionMenu_items_text25:
        fp.get();
        fp.getline( items_text[25], sizeof(items_text[0]));
        break;
      case ge_eSave_OptionMenu_items_text26:
        fp.get();
        fp.getline( items_text[26], sizeof(items_text[0]));
        break;
      case ge_eSave_OptionMenu_items_text27:
        fp.get();
        fp.getline( items_text[27], sizeof(items_text[0]));
        break;
      case ge_eSave_OptionMenu_items_text28:
        fp.get();
        fp.getline( items_text[28], sizeof(items_text[0]));
        break;
      case ge_eSave_OptionMenu_items_text29:
        fp.get();
        fp.getline( items_text[29], sizeof(items_text[0]));
        break;
      case ge_eSave_OptionMenu_items_text30:
        fp.get();
        fp.getline( items_text[30], sizeof(items_text[0]));
        break;
      case ge_eSave_OptionMenu_items_text31:
        fp.get();
        fp.getline( items_text[31], sizeof(items_text[0]));
        break;
      case ge_eSave_OptionMenu_items_enum0: fp >> items_enum[0]; break;
      case ge_eSave_OptionMenu_items_enum1: fp >> items_enum[1]; break;
      case ge_eSave_OptionMenu_items_enum2: fp >> items_enum[2]; break;
      case ge_eSave_OptionMenu_items_enum3: fp >> items_enum[3]; break;
      case ge_eSave_OptionMenu_items_enum4: fp >> items_enum[4]; break;
      case ge_eSave_OptionMenu_items_enum5: fp >> items_enum[5]; break;
      case ge_eSave_OptionMenu_items_enum6: fp >> items_enum[6]; break;
      case ge_eSave_OptionMenu_items_enum7: fp >> items_enum[7]; break;
      case ge_eSave_OptionMenu_items_enum8: fp >> items_enum[8]; break;
      case ge_eSave_OptionMenu_items_enum9: fp >> items_enum[9]; break;
      case ge_eSave_OptionMenu_items_enum10: fp >> items_enum[10]; break;
      case ge_eSave_OptionMenu_items_enum11: fp >> items_enum[11]; break;
      case ge_eSave_OptionMenu_items_enum12: fp >> items_enum[12]; break;
      case ge_eSave_OptionMenu_items_enum13: fp >> items_enum[13]; break;
      case ge_eSave_OptionMenu_items_enum14: fp >> items_enum[14]; break;
      case ge_eSave_OptionMenu_items_enum15: fp >> items_enum[15]; break;
      case ge_eSave_OptionMenu_items_enum16: fp >> items_enum[16]; break;
      case ge_eSave_OptionMenu_items_enum17: fp >> items_enum[17]; break;
      case ge_eSave_OptionMenu_items_enum18: fp >> items_enum[18]; break;
      case ge_eSave_OptionMenu_items_enum19: fp >> items_enum[19]; break;
      case ge_eSave_OptionMenu_items_enum20: fp >> items_enum[20]; break;
      case ge_eSave_OptionMenu_items_enum21: fp >> items_enum[21]; break;
      case ge_eSave_OptionMenu_items_enum22: fp >> items_enum[22]; break;
      case ge_eSave_OptionMenu_items_enum23: fp >> items_enum[23]; break;
      case ge_eSave_OptionMenu_items_enum24: fp >> items_enum[24]; break;
      case ge_eSave_OptionMenu_items_enum25: fp >> items_enum[25]; break;
      case ge_eSave_OptionMenu_items_enum26: fp >> items_enum[26]; break;
      case ge_eSave_OptionMenu_items_enum27: fp >> items_enum[27]; break;
      case ge_eSave_OptionMenu_items_enum28: fp >> items_enum[28]; break;
      case ge_eSave_OptionMenu_items_enum29: fp >> items_enum[29]; break;
      case ge_eSave_OptionMenu_items_enum30: fp >> items_enum[30]; break;
      case ge_eSave_OptionMenu_items_enum31: fp >> items_enum[31]; break;
      case ge_eSave_End: end_found = 1; break;
      default:
        cout << "GeOptionMenu:open syntax error" << endl;
        fp.getline( dummy, sizeof(dummy));
    }
    if ( end_found)
      break;
  }  
}

int GeOptionMenu::connect( grow_tObject object, glow_sTraceData *trace_data)
{
  int		attr_type, attr_size;
  char		parsed_name[120];
  int		sts;
  int		inverted;

  p = 0;
  db = dyn->graph->parse_attr_name( attribute, parsed_name,
				    &inverted, &attr_type, &attr_size);
  if ( strcmp( parsed_name,"") == 0)
    return 1;

  size = attr_size;
  switch ( db) {
  case graph_eDatabase_Gdh:
    sts = dyn->graph->ref_object_info( dyn->cycle, parsed_name, &p, &subid, attr_size);
    if ( EVEN(sts)) return sts;
    if ( attr_type != 0)
      type_id = attr_type;
    else
      type_id = pwr_eType_Int32;
    break;
  case graph_eDatabase_Local:
    p = dyn->graph->localdb_ref_or_create( parsed_name, attr_type);
    type_id = attr_type;
  case graph_eDatabase_User:
    type_id = attr_type;
    break;
  default:
    ;
  }

  switch ( type_id) {
  case pwr_eType_Float32: size = sizeof( pwr_tFloat32); break;
  case pwr_eType_Int32: size = sizeof( pwr_tInt32); break;
  case pwr_eType_UInt32: size = sizeof( pwr_tUInt32); break;
  case pwr_eType_Int16: size = sizeof( pwr_tInt16); break;
  case pwr_eType_UInt16: size = sizeof( pwr_tUInt16); break;
  case pwr_eType_Int8: size = sizeof( pwr_tInt8); break;
  case pwr_eType_UInt8: size = sizeof( pwr_tUInt8); break;
  case pwr_eType_Float64: size = sizeof( pwr_tFloat64); break;
  default:
    return 1;
  }

  if ( p)
    trace_data->p = p;
  first_scan = true;
  return 1;
}

int GeOptionMenu::disconnect( grow_tObject object)
{
  if ( p && db == graph_eDatabase_Gdh)
    gdh_UnrefObjectInfo( subid);
  p = 0;
  return 1;
}

int GeOptionMenu::scan( grow_tObject object)
{
  if ( !p)
    return 1;

  if ( !first_scan) {
    if ( memcmp( &old_value, p, size) == 0 )
      // No change since last time
      return 1;
  }
  else
    first_scan = false;

  unsigned int enum_value;

  switch( type_id) {
  case pwr_eType_Float32: enum_value = (unsigned int) (*(pwr_tFloat32 *) p + 0.5); break;
  case pwr_eType_Int32: enum_value = (unsigned int) *(pwr_tInt32 *) p; break;
  case pwr_eType_UInt32: enum_value = (unsigned int) *(pwr_tUInt32 *) p; break;
  case pwr_eType_Int16: enum_value = (unsigned int) *(pwr_tInt16 *) p; break;
  case pwr_eType_UInt16: enum_value = (unsigned int) *(pwr_tUInt16 *) p; break;
  case pwr_eType_Int8: enum_value = (unsigned int) *(pwr_tInt8 *) p; break;
  case pwr_eType_UInt8: enum_value = (unsigned int) *(pwr_tUInt8 *) p; break;
  case pwr_eType_Float64: enum_value = (unsigned int) *(pwr_tFloat64 *) p; break;
  default:
    return 1;
  }

  int found = 0;
  for ( int i = 0; i < 32; i++) {
    if ( items_enum[i] == enum_value) {
      grow_SetAnnotation( object, 1, items_text[i], strlen(items_text[i]));
      found = 1;
      break;
    }
  }
  if ( !found)
    grow_SetAnnotation( object, 1, "", 0);

  memcpy( &old_value, p, MIN(size, (int) sizeof(old_value)));
  return 1;
}

int GeOptionMenu::action( grow_tObject object, glow_tEvent event)
{
  if ( !dyn->graph->is_authorized( dyn->access))
    return 1;

  switch ( event->event) {
  case glow_eEvent_MB1Down:
    break;
  case glow_eEvent_MB1Up:
    break;
  case glow_eEvent_Key_Up:
    if ( !menu_object)
      break;
    grow_MenuShiftCurrentItem( menu_object, -1);
    return GE__NO_PROPAGATE;
    break;
  case glow_eEvent_Key_Down:
    if ( !menu_object) {
      // Create a menu by triggering a click event
      glow_sEvent e;

      e.event = glow_eEvent_MB1Click;
      e.object.object = object,
      dyn->action( object, &e);
      return GE__NO_PROPAGATE;
    }
    grow_MenuShiftCurrentItem( menu_object, 1);
    return GE__NO_PROPAGATE;
    break;
  case glow_eEvent_Key_Return:
    if ( !menu_object) {
      // Open menu, simulate a MB1 click event
      glow_sEvent e;
      e.event = glow_eEvent_MB1Click;
      e.object.object = object;
      action( object, &e);
    }
    else {
      // Activate current item, simulate an activated event
      int sts;
      glow_sEvent e;

      e.event = glow_eEvent_MenuActivated;
      e.menu.object = menu_object;

      sts = grow_MenuGetCurrentItem( menu_object, &e.menu.item);
      if ( EVEN(sts))
	break;
      
      action( object, &e);

      //Close
      grow_DeleteObject( dyn->graph->grow->ctx, menu_object);
      menu_object = 0;

      // Trigger a tab event
      e.event = glow_eEvent_Key_Tab;
      e.object.object = object,
      dyn->action( object, &e);
    }
    break;
  case glow_eEvent_InputFocusGained:
    if ( menu_object)
      break;
    clock_gettime( CLOCK_REALTIME, &focus_gained_time);
    break;
  case glow_eEvent_MB1Click:
    // if ( event->event == glow_eEvent_MB1Click) {
    //  pwr_tTime now;
    //  clock_gettime( CLOCK_REALTIME, &now);
    //  if ( abs(now.tv_sec - focus_gained_time.tv_sec) < 2)
    //	break;
    //}
    if ( menu_object) {
      // Close, delete this menu
      grow_DeleteObject( dyn->graph->grow->ctx, menu_object);
      menu_object = 0;
    }
    else {
      double	ll_x, ll_y, ur_x, ur_y;
      glow_sMenuInfo info;

      int b_mask = 1;
      for ( int i = 0; i < 32; i++) {
	if ( b_mask & button_mask) {
	  info.item[i].occupied = true;
	  strcpy( info.item[i].text, items_text[i]);

	  // Check access
	  info.item[i].type = glow_eMenuItem_Button;
	}
	else
	  info.item[i].occupied = false;
	b_mask = b_mask << 1;
      }

      // Get fillcolor, and textattributes from object
      glow_eDrawType text_drawtype, text_color, bg_color;
      int text_size;
      int sts;

      sts = grow_GetObjectAnnotInfo( object, 1, &text_size, &text_drawtype, &text_color, &bg_color);
      if ( EVEN(sts)) {
	text_size = 2;
	text_drawtype = glow_eDrawType_TextHelveticaBold;
	text_color = glow_eDrawType_Line;
	bg_color = glow_eDrawType_LightGray;
      }
      else if ( bg_color == glow_eDrawType_No || bg_color == glow_eDrawType_Inherit)
	bg_color = glow_eDrawType_LightGray;
	
      grow_MeasureNode( object, &ll_x, &ll_y, &ur_x, &ur_y);
      grow_CreateGrowMenu( dyn->graph->grow->ctx, "__Menu", &info, ll_x, ur_y, ur_x - ll_x,
			   glow_eDrawType_Line, 0, 1, 1, bg_color, text_size,
			   text_drawtype, text_color,
			   glow_eDrawType_MediumGray, 0,
			   &menu_object);
    }
    break;
  case glow_eEvent_MenuActivated:
    if ( menu_object == 0)
      break;
    if ( event->menu.object == menu_object) {
      // Set enum value to attribute
      int 		sts;
      char		parsed_name[120];
      int      		inverted;
      int      		attr_type, attr_size;
      
      dyn->graph->parse_attr_name( attribute, parsed_name, &inverted, &attr_type, &attr_size);
      switch ( type_id) {
      case pwr_eType_Float32: {
	pwr_tFloat32 value = items_enum[event->menu.item];
	sts = gdh_SetObjectInfo( parsed_name, &value, sizeof(value));
	break;
      }
      case pwr_eType_Int32: {
	pwr_tInt32 value = items_enum[event->menu.item];
	sts = gdh_SetObjectInfo( parsed_name, &value, sizeof(value));
	break;
      }
      case pwr_eType_UInt32: {
	pwr_tUInt32 value = items_enum[event->menu.item];
	sts = gdh_SetObjectInfo( parsed_name, &value, sizeof(value));
	break;
      }
      case pwr_eType_Int16: {
	pwr_tInt32 value = items_enum[event->menu.item];
	sts = gdh_SetObjectInfo( parsed_name, &value, sizeof(value));
	break;
      }
      case pwr_eType_UInt16: {
	pwr_tUInt32 value = items_enum[event->menu.item];
	sts = gdh_SetObjectInfo( parsed_name, &value, sizeof(value));
	break;
      }
      case pwr_eType_Int8: {
	pwr_tInt8 value = items_enum[event->menu.item];
	sts = gdh_SetObjectInfo( parsed_name, &value, sizeof(value));
	break;
      }
      case pwr_eType_UInt8: {
	pwr_tUInt8 value = items_enum[event->menu.item];
	sts = gdh_SetObjectInfo( parsed_name, &value, sizeof(value));
	break;
      }
      default:
	sts = 0;
      }
      if ( EVEN(sts)) printf("Option menu error: %s\n", attribute);
    }
    break;
  case glow_eEvent_MenuCreate:
    break;
  case glow_eEvent_InputFocusLost: {
    // Delete this menu
    grow_DeleteObject( dyn->graph->grow->ctx, menu_object);
    menu_object = 0;
    break;
  }
  case glow_eEvent_MenuDelete:
    if ( menu_object == 0)
      break;
    if ( event->menu.object == 0 || event->menu.object == menu_object) {
      // Delete this menu
      grow_DeleteObject( dyn->graph->grow->ctx, menu_object);
      menu_object = 0;
    }
    break;
  default: ;
  }
  return 1;
}

int GeOptionMenu::export_java( grow_tObject object, ofstream& fp, bool first, char *var_name)
{
  if ( first)
    fp << "      ";
  else
    fp << "      ,";
  fp << "new GeDynOptionMenu(" << var_name << ".dd, \"" << attribute << "\", new String[] {" << endl << "        ";
  int b_mask = 1;
  for ( int i = 0; i < 32; i++) {
    if ( b_mask & button_mask)
      fp << "\"" << items_text[i] << "\"";
    else
      fp << "null";
    if ( i != 31)
      fp << ",";
    b_mask = b_mask << 1;
  }
  fp << "}," << endl << "        new int[] {" << endl << "        ";
  b_mask = 1;
  for ( int i = 0; i < 32; i++) {
    if ( b_mask & button_mask)
      fp << items_enum[i];
    else
      fp << "0";
    if ( i != 31)
      fp << ",";
    b_mask = b_mask << 1;
  }
  fp << endl << "      })" << endl;
  return 1;
}

void GeAnalogText::get_attributes( attr_sItem *attrinfo, int *item_count)
{
  int i = *item_count;
  
  strcpy( attrinfo[i].name, "AnalogText.Attribute");
  attrinfo[i].value = attribute;
  attrinfo[i].type = glow_eType_String;
  attrinfo[i++].size = sizeof( attribute);

  strcpy( attrinfo[i].name, "AnalogText.TextMask");
  attrinfo[i].value = &button_mask;
  attrinfo[i].type = ge_eAttrType_InstanceMask;
  attrinfo[i++].size = sizeof( button_mask);

  int b_mask = ge_mInstance_1;
  for ( int j = 0; j < 32; j++) {
    if ( b_mask & button_mask) {
      sprintf( attrinfo[i].name, "AnalogText.Text%d", j+1);
      attrinfo[i].value = items_text[j];
      attrinfo[i].type = glow_eType_String;
      attrinfo[i++].size = sizeof( items_text[0]);

      sprintf( attrinfo[i].name, "AnalogText.Enum%d", j+1);
      attrinfo[i].value = &items_enum[j];
      attrinfo[i].type = glow_eType_Int;
      attrinfo[i++].size = sizeof(items_enum[0]);
    }
    b_mask = b_mask << 1;
  }
  dyn->display_access = true;
  *item_count = i;
}

void GeAnalogText::set_attribute( grow_tObject object, char *attr_name, int *cnt)
{
  (*cnt)--;
  if ( *cnt == 0) {
    char msg[200];

    strncpy( attribute, attr_name, sizeof( attribute));
    sprintf( msg, "AnalogText.Attribute = %s", attr_name);
    dyn->graph->message( 'I', msg);
  }
}

void GeAnalogText::replace_attribute( char *from, char *to, int *cnt, int strict)
{
  GeDyn::replace_attribute( attribute, sizeof(attribute), from, to, cnt, strict);
}

void GeAnalogText::save( ofstream& fp)
{
  fp << int(ge_eSave_AnalogText) << endl;  
  fp << int(ge_eSave_AnalogText_super) << endl;  
  GeOptionMenu::save( fp);
  fp << int(ge_eSave_End) << endl;
}

void GeAnalogText::open( ifstream& fp)
{
  int		type;
  int 		end_found = 0;
  char		dummy[40];

  for (;;)
  {
    fp >> type;
    switch( type) {
      case ge_eSave_AnalogText: break;
      case ge_eSave_AnalogText_super:
	GeOptionMenu::open( fp);
        break;
      case ge_eSave_End: end_found = 1; break;
      default:
        cout << "GeAnalogText:open syntax error" << endl;
        fp.getline( dummy, sizeof(dummy));
    }
    if ( end_found)
      break;
  }  
}


int GeAnalogText::export_java( grow_tObject object, ofstream& fp, bool first, char *var_name)
{
  if ( first)
    fp << "      ";
  else
    fp << "      ,";
  fp << "new GeDynAnalogText(" << var_name << ".dd, \"" << attribute << "\", new String[] {" << endl << "        ";
  int b_mask = 1;
  for ( int i = 0; i < 32; i++) {
    if ( b_mask & button_mask)
      fp << "\"" << items_text[i] << "\"";
    else
      fp << "null";
    if ( i != 31)
      fp << ",";
    b_mask = b_mask << 1;
  }
  fp << "}," << endl << "        new int[] {" << endl << "        ";
  b_mask = 1;
  for ( int i = 0; i < 32; i++) {
    if ( b_mask & button_mask)
      fp << items_enum[i];
    else
      fp << "0";
    if ( i != 31)
      fp << ",";
    b_mask = b_mask << 1;
  }
  fp << endl << "      })" << endl;
  return 1;
}









