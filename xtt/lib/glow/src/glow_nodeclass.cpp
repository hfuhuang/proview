#include "glow_std.h"

#include <iostream.h>
#include <fstream.h>
#include <stdlib.h>
#include <string.h>
#include <float.h>
#include <math.h>
#include "glow_nodeclass.h"
#include "glow_annot.h"
#include "glow_growannot.h"
#include "glow_annotpixmap.h"
#include "glow_growrect.h"
#include "glow_growctx.h"
#include "glow_msg.h"

GlowNodeClass::GlowNodeClass( GlowCtx *glow_ctx, char *name, 
	glow_eNodeGroup grp)
  : ctx(glow_ctx), a(10,10), group(grp), dynamic(0), dynamicsize(0),
    arg_cnt(0), nc_extern(0), dyn_type(0), dyn_action_type(0),
    no_con_obstacle(0), slider(0), animation_count(1),
    y0(0), y1(0), x0(0), x1(0),
    next_nc(0), prev_nc(0), cycle(glow_eCycle_Slow), user_data(0)
{
  memset( dyn_color, 0, sizeof( dyn_color));
  memset( dyn_attr, 0, sizeof( dyn_attr));
  strcpy( nc_name, name);
  strcpy( java_name, "");
  strcpy( next_nodeclass, "");
  memset( argname, 0, sizeof(argname));
  memset( argtype, 0, sizeof(argtype));
}

GlowNodeClass::GlowNodeClass( const GlowNodeClass& nc)
{
  memcpy( this, &nc, sizeof(nc));

  a.new_array( nc.a);
  a.copy_from( nc.a);
  if ( dynamicsize)
  {
    dynamic = (char *) calloc( 1, dynamicsize);
    memcpy( dynamic, nc.dynamic, dynamicsize);
  }
  if ( user_data && ctx->userdata_copy_callback)
    (ctx->userdata_copy_callback)( this, user_data, &user_data, glow_eUserdataCbType_NodeClass);
}

GlowNodeClass::~GlowNodeClass()
{
  ctx->object_deleted( this);
}

void GlowNodeClass::print( GlowPoint *pos, void *node)
{
  int		i;

  for ( i = 0; i < a.a_size; i++)
  {
    a.a[i]->print( pos, node);
  }
}

void GlowNodeClass::save( ofstream& fp, glow_eSaveMode mode)
{
  char *s;
  int	i;

  if ( saved)
    return;
  set_saved(1);

  if ( slider)
    x0 = x1 = y1 = 0;


  if ( next_nc)
    ((GlowNodeClass *)next_nc)->save( fp, mode);

  if ( nc_extern)
    return;
  if ( (mode == glow_eSaveMode_Trace && group != glow_eNodeGroup_Trace) ||
       (mode == glow_eSaveMode_Edit && group == glow_eNodeGroup_Trace) )
    return;
  fp <<	int(glow_eSave_NodeClass) << endl;
  fp <<	int(glow_eSave_NodeClass_nc_name) << FSPACE << nc_name << endl;
  fp <<	int(glow_eSave_NodeClass_a) << endl;
  a.save( fp, mode);
  fp <<	int(glow_eSave_NodeClass_group) << FSPACE << int(group) << endl;
  fp << int(glow_eSave_NodeClass_dynamicsize) << FSPACE << dynamicsize << endl;
  fp << int(glow_eSave_NodeClass_dynamic) << endl;
  if( dynamic)
  {
    fp << "\"";
    for ( s  = dynamic; *s; s++)
    {
      if ( *s == '"')
        fp << "\\";
      fp << *s;
    }
    fp << "\"" << endl;
  }
  fp << int(glow_eSave_NodeClass_arg_cnt) << FSPACE << arg_cnt << endl;
  fp << int(glow_eSave_NodeClass_argname) << endl;  
  for ( i = 0; i < arg_cnt; i++)
    fp << argname[i] << endl;
  fp << int(glow_eSave_NodeClass_argtype) << endl;  
  for ( i = 0; i < arg_cnt; i++)
    fp << argtype[i] << endl;
  fp << int(glow_eSave_NodeClass_dyn_type) << FSPACE << dyn_type << endl;
  fp << int(glow_eSave_NodeClass_dyn_action_type) << FSPACE << dyn_action_type << endl;
  fp << int(glow_eSave_NodeClass_dyn_color1) << FSPACE << int(dyn_color[0]) << endl;
  fp << int(glow_eSave_NodeClass_dyn_color2) << FSPACE << int(dyn_color[1]) << endl;
  fp << int(glow_eSave_NodeClass_dyn_color3) << FSPACE << int(dyn_color[2]) << endl;
  fp << int(glow_eSave_NodeClass_dyn_color4) << FSPACE << int(dyn_color[3]) << endl;
  fp << int(glow_eSave_NodeClass_dyn_attr1) << FSPACE << dyn_attr[0] << endl;
  fp << int(glow_eSave_NodeClass_dyn_attr2) << FSPACE << dyn_attr[1] << endl;
  fp << int(glow_eSave_NodeClass_dyn_attr3) << FSPACE << dyn_attr[2] << endl;
  fp << int(glow_eSave_NodeClass_dyn_attr4) << FSPACE << dyn_attr[3] << endl;
  fp << int(glow_eSave_NodeClass_no_con_obstacle) << FSPACE << no_con_obstacle << endl;
  fp << int(glow_eSave_NodeClass_slider) << FSPACE << slider << endl;
  fp <<	int(glow_eSave_NodeClass_java_name) << FSPACE << java_name << endl;
  fp <<	int(glow_eSave_NodeClass_next_nodeclass) << FSPACE << next_nodeclass << endl;
  fp << int(glow_eSave_NodeClass_animation_count) << FSPACE << animation_count << endl;
  fp << int(glow_eSave_NodeClass_cycle) << FSPACE << int(cycle) << endl;
  fp << int(glow_eSave_NodeClass_y0) << FSPACE << y0 << endl;
  fp << int(glow_eSave_NodeClass_y1) << FSPACE << y1 << endl;
  fp << int(glow_eSave_NodeClass_x0) << FSPACE << x0 << endl;
  fp << int(glow_eSave_NodeClass_x1) << FSPACE << x1 << endl;
  fp << int(glow_eSave_NodeClass_input_focus_mark) << FSPACE << int(input_focus_mark) << endl;
  if ( user_data && ctx->userdata_save_callback) {
    fp << int(glow_eSave_NodeClass_userdata_cb) << endl;
    (ctx->userdata_save_callback)(&fp, this, glow_eUserdataCbType_NodeClass);
  }
  fp <<	int(glow_eSave_End) << endl;
}

void GlowNodeClass::open( ifstream& fp)
{
  int		type;
  int 		end_found = 0;
  char		dummy[40];
  int		tmp;
  int		i, j;
  char		c;
  int		sts;

  for (;;)
  {
    fp >> type;
    switch( type) {
      case glow_eSave_NodeClass: break;
      case glow_eSave_NodeClass_nc_name:
        fp.get();
        fp.getline( nc_name, sizeof(nc_name));
        break;
      case glow_eSave_NodeClass_a: a.open( ctx, fp); break;
      case glow_eSave_NodeClass_group: fp >> tmp; group = (glow_eNodeGroup)tmp; break;
      case glow_eSave_NodeClass_dynamicsize: fp >> dynamicsize; break;
      case glow_eSave_NodeClass_dynamic:
        fp.getline( dummy, sizeof(dummy));
        if ( dynamicsize)
        {
          dynamic = (char *) calloc( 1, dynamicsize);
	  fp.get();
          for ( j = 0; j < dynamicsize; j++)
	  {
	    if ((c = fp.get()) == '"')
	    {
	      if ( dynamic[j-1] == '\\')
	        j--;
	      else
              {
	        dynamic[j] = 0;
                break;
              }
	    }
            dynamic[j] = c;
	  }
          fp.getline( dummy, sizeof(dummy));
        }
        break;
      case glow_eSave_NodeClass_arg_cnt: fp >> arg_cnt; break;
      case glow_eSave_NodeClass_argname:
        fp.get();
        for ( i = 0; i < arg_cnt; i++)
        {
          fp.getline( argname[i], sizeof(argname[0]));
        }
        break;
      case glow_eSave_NodeClass_argtype:
        for ( i = 0; i < arg_cnt; i++)
          fp >> argtype[i];
        break;
      case glow_eSave_NodeClass_dyn_type: fp >> dyn_type; break; 
      case glow_eSave_NodeClass_dyn_action_type: fp >> dyn_action_type; break; 
      case glow_eSave_NodeClass_dyn_color1:
        fp >> tmp;
	dyn_color[0] = (glow_eDrawType)tmp;
        break;
      case glow_eSave_NodeClass_dyn_color2:
        fp >> tmp;
	dyn_color[1] = (glow_eDrawType)tmp;
        break;
      case glow_eSave_NodeClass_dyn_color3:
        fp >> tmp;
	dyn_color[2] = (glow_eDrawType)tmp;
        break;
      case glow_eSave_NodeClass_dyn_color4:
        fp >> tmp;
	dyn_color[3] = (glow_eDrawType)tmp;
        break;
      case glow_eSave_NodeClass_dyn_attr1: fp >> dyn_attr[0]; break;
      case glow_eSave_NodeClass_dyn_attr2: fp >> dyn_attr[1]; break;
      case glow_eSave_NodeClass_dyn_attr3: fp >> dyn_attr[2]; break;
      case glow_eSave_NodeClass_dyn_attr4: fp >> dyn_attr[3]; break;
      case glow_eSave_NodeClass_no_con_obstacle: fp >> no_con_obstacle; break;
      case glow_eSave_NodeClass_slider: fp >> slider; break;
      case glow_eSave_NodeClass_java_name:
        fp.get();
        fp.getline( java_name, sizeof(java_name));
        break;
      case glow_eSave_NodeClass_next_nodeclass:
        fp.get();
        fp.getline( next_nodeclass, sizeof(next_nodeclass));
        break;
      case glow_eSave_NodeClass_animation_count: fp >> animation_count; break;
      case glow_eSave_NodeClass_cycle:
        fp >> tmp;
	cycle = (glow_eCycle)tmp;
        break;
      case glow_eSave_NodeClass_y0: fp >> y0; break;
      case glow_eSave_NodeClass_y1: fp >> y1; break;
      case glow_eSave_NodeClass_x0: fp >> x0; break;
      case glow_eSave_NodeClass_x1: fp >> x1; break;
      case glow_eSave_NodeClass_input_focus_mark:
        fp >> tmp;
	input_focus_mark = (glow_eInputFocusMark)tmp;
        break;
      case glow_eSave_NodeClass_userdata_cb:
	if ( ctx->userdata_open_callback)
	  (ctx->userdata_open_callback)(&fp, this, glow_eUserdataCbType_NodeClass);
	break;
      case glow_eSave_End: end_found = 1; break;
      default:
        cout << "GlowNodeClass:open syntax error" << endl;
        fp.getline( dummy, sizeof(dummy));
    }
    if ( end_found)
      break;
  }

  if ( strcmp( next_nodeclass, "") != 0)
  {
    next_nc = ctx->get_nodeclass_from_name( next_nodeclass);
    if ( !next_nc)
    {
      sts = ((GrowCtx *)ctx)->open_subgraph_from_name( next_nodeclass, 
			glow_eSaveMode_SubGraph);
      if ( ODD(sts))
      {
        next_nc = ctx->get_nodeclass_from_name( next_nodeclass);
        if ( next_nc) 
          ((GlowNodeClass *)next_nc)->nc_extern = nc_extern;
      }
    }
    if ( !next_nc) 
      cout << "GlowNode:next_nodeclass not found: " << nc_name << " " <<
		next_nodeclass << endl;
    else if ( ((GlowNodeClass *)next_nc)->prev_nc)
    {
      next_nc = 0;
      cout << "GlowNode:next_nodeclass already chained: " << nc_name << " " <<
		next_nodeclass << endl;
    }
    else
      ((GlowNodeClass *)next_nc)->prev_nc = (GlowArrayElem *) this;
  }
}

void GlowNodeClass::draw( GlowPoint *pos, int highlight, int hot, void *node)
{
  int		i;

  for ( i = 0; i < a.a_size; i++)
  {
    a.a[i]->draw( pos, highlight, hot, node);
  }
}

void GlowNodeClass::nav_draw( GlowPoint *pos, int highlight, void *node)
{
  int		i;

  for ( i = 0; i < a.a_size; i++)
  {
    a.a[i]->nav_draw( pos, highlight, node);
  }
}

void GlowNodeClass::draw_inverse( GlowPoint *pos, int hot, void *node)
{
  int		i;

  for ( i = 0; i < a.a_size; i++)
  {
    switch ( a.a[i]->type())
    { 
      case glow_eObjectType_Radiobutton:
        a.a[i]->draw( pos, 0, hot, node);
        break;
      default:
        a.a[i]->draw_inverse( pos, hot, node);
    }
  }
}

void GlowNodeClass::erase( GlowPoint *pos, int hot, void *node)
{
  int		i;

  for ( i = 0; i < a.a_size; i++)
  {
    a.a[i]->erase( pos, hot, node);
  }
}

void GlowNodeClass::nav_erase( GlowPoint *pos, void *node)
{
  int		i;

  for ( i = 0; i < a.a_size; i++)
  {
    a.a[i]->nav_erase( pos, node);
  }
}

void GlowNodeClass::draw( GlowTransform *t, int highlight, int hot, void *node,
			  void *colornode)
{
  int		i;

  for ( i = 0; i < a.a_size; i++)
  {
    a.a[i]->draw( t, highlight, hot, node, colornode);
  }
}

void GlowNodeClass::nav_draw( GlowTransform *t, int highlight, void *node, 
			      void *colornode)
{
  int		i;

  for ( i = 0; i < a.a_size; i++)
  {
    a.a[i]->nav_draw( t, highlight, node, colornode);
  }
}

void GlowNodeClass::erase( GlowTransform *t, int hot, void *node)
{
  int		i;

  for ( i = 0; i < a.a_size; i++)
  {
    a.a[i]->erase( t, hot, node);
  }
}

void GlowNodeClass::nav_erase( GlowTransform *t, void *node)
{
  int		i;

  for ( i = 0; i < a.a_size; i++)
  {
    a.a[i]->nav_erase( t, node);
  }
}

int GlowNodeClass::get_conpoint( int num, double *x, double *y, 
	glow_eDirection *dir)
{
  int		i, sts;

  for ( i = 0; i < a.a_size; i++)
  {
    sts = a.a[i]->get_conpoint( num, x, y, dir);
    if ( sts)
      return sts;
  }
  return GLOW__NOCONPOINT;
}

int GlowNodeClass::get_conpoint( GlowTransform *t, int num, bool flip_horizontal,
				 bool flip_vertical, double *x, double *y, 
	glow_eDirection *dir)
{
  int		i, sts;

  for ( i = 0; i < a.a_size; i++)
  {
    sts = a.a[i]->get_conpoint( t, num, flip_horizontal, flip_vertical, x, y, dir);
    if ( sts)
      return sts;
  }
  return GLOW__NOCONPOINT;
}

int GlowNodeClass::event_handler( void *pos, glow_eEvent event, int x, int y,
	void *node)
{
  return a.event_handler( pos, event, x, y, node);
}

int GlowNodeClass::event_handler( glow_eEvent event, double fx, double fy)
{
  return a.event_handler( event, fx, fy);
}

void GlowNodeClass::erase_annotation( void *pos, int highlight, int hot, 
	void *node, int num)
{
  int		i;

  for ( i = 0; i < a.a_size; i++)
  {
    if ( (a.a[i]->type() == glow_eObjectType_Annot ||
	  a.a[i]->type() == glow_eObjectType_GrowAnnot) &&
         ((GlowAnnot *)a.a[i])->number == num)
    {
      a.a[i]->erase( pos, hot, node);
      a.a[i]->nav_erase( pos, node);
      break;
    }
  }
}

void GlowNodeClass::draw_annotation( void *pos, int highlight, int hot, 
	void *node, int num)
{
  int		i;

  for ( i = 0; i < a.a_size; i++)
  {
    if ( (a.a[i]->type() == glow_eObjectType_Annot ||
	  a.a[i]->type() == glow_eObjectType_GrowAnnot) &&
         ((GlowAnnot *)a.a[i])->number == num)
    {
      a.a[i]->draw( pos, highlight, hot, node);
      a.a[i]->nav_draw( pos, highlight, node);
      break;
    }
  }
}

void GlowNodeClass::erase_annotation( GlowTransform *t, int highlight, int hot, 
	void *node, int num)
{
  int		i;

  for ( i = 0; i < a.a_size; i++)
  {
    if ( a.a[i]->type() == glow_eObjectType_GrowAnnot &&
         ((GlowAnnot *)a.a[i])->number == num)
    {
      ((GrowAnnot *)a.a[i])->erase_background( t, hot, node);
      a.a[i]->nav_erase( t, node);
      break;
    }
  }
}

void GlowNodeClass::draw_annotation( GlowTransform *t, int highlight, int hot, 
	void *node, int num)
{
  int		i;

  for ( i = 0; i < a.a_size; i++)
  {
    if ( a.a[i]->type() == glow_eObjectType_GrowAnnot &&
         ((GlowAnnot *)a.a[i])->number == num)
    {
      a.a[i]->draw( t, highlight, hot, node, NULL);
      a.a[i]->nav_draw( t, highlight, node, NULL);
      break;
    }
  }
}

int GlowNodeClass::check_annotation( int num)
{
  int		i;

  for ( i = 0; i < a.a_size; i++)
  {
    if ( (a.a[i]->type() == glow_eObjectType_Annot || 
	  a.a[i]->type() == glow_eObjectType_GrowAnnot) &&
         ((GlowAnnot *)a.a[i])->number == num)
      return 1;
  }
  return 0;
}

void GlowNodeClass::open_annotation_input( void *pos, void *node, int num)
{
  int		i;

  for ( i = 0; i < a.a_size; i++)
  {
    if ( (a.a[i]->type() == glow_eObjectType_Annot &&
         ((GlowAnnot *)a.a[i])->number == num) ||
         (a.a[i]->type() == glow_eObjectType_GrowAnnot &&
         ((GrowAnnot *)a.a[i])->number == num))
    {
      ((GlowAnnot *)a.a[i])->open_annotation_input( pos, node);
      break;
    }
  }
}

void GlowNodeClass::close_annotation_input( void *node, int num)
{
  int		i;

  for ( i = 0; i < a.a_size; i++)
  {
    if ( (a.a[i]->type() == glow_eObjectType_Annot || 
	  a.a[i]->type() == glow_eObjectType_GrowAnnot) &&
         ((GlowAnnot *)a.a[i])->number == num)
    {
      ((GlowAnnot *)a.a[i])->close_annotation_input( node);
      break;
    }
  }
}

int GlowNodeClass::get_annotation_input( void *node, int num, char **text)
{
  int		i, sts;

  for ( i = 0; i < a.a_size; i++)
  {
    if ( (a.a[i]->type() == glow_eObjectType_Annot || 
	  a.a[i]->type() == glow_eObjectType_GrowAnnot) &&
         ((GlowAnnot *)a.a[i])->number == num)
    {
      sts = ((GlowAnnot *)a.a[i])->get_annotation_input( node, text);
      break;
    }
  }
  return sts;
}

void GlowNodeClass::move_widgets( void *node, int x, int y)
{
  for ( int i = 0; i < a.a_size; i++)
  {
    if ( a.a[i]->type() == glow_eObjectType_Annot || 
	 a.a[i]->type() == glow_eObjectType_GrowAnnot)
      ((GlowAnnot *)a.a[i])->move_widgets( node, x, y);
  }
}

void GlowNodeClass::configure_annotations( void *pos, void *node)
{
  int		i;

  for ( i = 0; i < a.a_size; i++)
  {
    if ( a.a[i]->type() == glow_eObjectType_Annot)
    {
      ((GlowAnnot *)a.a[i])->configure_annotations( pos, node);
    }
    else if ( a.a[i]->type() == glow_eObjectType_AnnotPixmap)
    {
      ((GlowAnnotPixmap *)a.a[i])->configure_annotations( pos, node);
    }
  }
}

void GlowNodeClass::measure_annotation( int num, char *text, double *width,
	double *height)
{
  int		i;

  for ( i = 0; i < a.a_size; i++)
  {
    if ( (a.a[i]->type() == glow_eObjectType_Annot || 
	  a.a[i]->type() == glow_eObjectType_GrowAnnot) &&
         ((GlowAnnot *)a.a[i])->number == num)
    {
      ((GlowAnnot *)a.a[i])->measure_annot( text, width, height);
      break;
    }
  }
}

void GlowNodeClass::get_obstacle_borders( double pos_x, double pos_y, double *x_right, 
		double *x_left, double *y_high, double *y_low, void *node)
{ 
  int i;

  switch ( group)
  {
    case glow_eNodeGroup_Document:
      for ( i = 0; i < a.a_size; i++)
      {
        if ( a.a[i]->type() == glow_eObjectType_Rect)
          a.a[i]->get_borders(pos_x, pos_y, x_right, x_left, y_high, y_low, node);
      }
      break;
    default:
      a.get_borders(pos_x, pos_y, x_right, x_left, y_high, y_low, node);
  }
}

void GlowNodeClass::get_object_name( char *name)
{
  strcpy( name, nc_name);
}

void GlowNodeClass::set_fill_color( glow_eDrawType drawtype)
{
  for ( int i = 0; i < a.a_size; i++)
    a.a[i]->set_fill_color( drawtype);
}

void GlowNodeClass::set_original_fill_color( glow_eDrawType drawtype)
{
  for ( int i = 0; i < a.a_size; i++)
    a.a[i]->set_original_fill_color( drawtype);
}

void GlowNodeClass::reset_fill_color()
{
  for ( int i = 0; i < a.a_size; i++)
    a.a[i]->reset_fill_color();
}

void GlowNodeClass::set_border_color( glow_eDrawType drawtype)
{
  for ( int i = 0; i < a.a_size; i++)
    a.a[i]->set_border_color( drawtype);
}

void GlowNodeClass::set_original_border_color( glow_eDrawType drawtype)
{
  for ( int i = 0; i < a.a_size; i++)
    a.a[i]->set_original_border_color( drawtype);
}

void GlowNodeClass::reset_border_color()
{
  for ( int i = 0; i < a.a_size; i++)
    a.a[i]->reset_border_color();
}

void GlowNodeClass::set_linewidth( int linewidth)
{
  for ( int i = 0; i < a.a_size; i++)
    a.a[i]->set_linewidth( linewidth);
}

int GlowNodeClass::draw_annot_background(  GlowTransform *t, void *node, 
		double x, double y)
{
  int sts;

  for ( int i = a.a_size - 1; i >= 0; i--)
  {
    if ( a.a[i]->type() == glow_eObjectType_GrowRect)
    {
      sts = ((GrowRect *)a.a[i])->draw_annot_background( t, node, x, y);
      if ( ODD(sts)) return sts;
    }
  }
  return 0;
}

int GlowNodeClass::get_annot_background(  GlowTransform *t, void *node,
		glow_eDrawType *background)
{
  int sts;

  for ( int i = a.a_size - 1; i >= 0; i--)
  {
    if ( a.a[i]->type() == glow_eObjectType_GrowRect)
    {
      sts = ((GrowRect *)a.a[i])->get_annot_background( t, node, background);
      if ( ODD(sts)) return sts;
    }
  }
  return 0;
}

void GlowNodeClass::get_annotation_numbers( int **numbers, int *cnt)
{
  int		i;
  int		*p;

  *cnt = 0;  
  p = (int *) calloc( 10, sizeof(int));
  for ( i = 0; i < a.a_size; i++)
  {
    if ( a.a[i]->type() == glow_eObjectType_Annot ||
	 a.a[i]->type() == glow_eObjectType_GrowAnnot)
    {
      if ( *cnt >= 10)
        break;
      p[*cnt] = ((GlowAnnot *)a.a[i])->number;
      (*cnt)++;
    }
  }
  *numbers = p;
}

void GlowNodeClass::measure_javabean( double *pix_x_right, double *pix_x_left, 
	double *pix_y_high, double *pix_y_low)
{ 
  double jb_x_right = -1e10;
  double jb_x_left = 1e10;
  double jb_y_high = -1e10;
  double jb_y_low = 1e10;
    
  GlowNodeClass *base = get_base_nc();

  if ( /* (next_nc || prev_nc) && */
       !(fabs( base->x0 - base->x1) < DBL_EPSILON ||
	 fabs( base->y0 - base->y1) < DBL_EPSILON)) {

    jb_x_right = base->x1;
    jb_x_left = base->x0;
    jb_y_high = base->y1;
    jb_y_low = base->y0;
  }
  else
    a.get_borders( (GlowTransform *) NULL, &jb_x_right, &jb_x_left, 
		   &jb_y_high, &jb_y_low);

  *pix_x_right = jb_x_right * ctx->zoom_factor_x - double(ctx->offset_x);
  *pix_x_left = jb_x_left * ctx->zoom_factor_x - double(ctx->offset_x);
  *pix_y_high = jb_y_high * ctx->zoom_factor_y - double(ctx->offset_y);
  *pix_y_low = jb_y_low * ctx->zoom_factor_y - double(ctx->offset_y);
}

int GlowNodeClass::get_java_name( char *jname) 
{
  if ( strcmp( java_name, "") != 0)
  {
    strcpy( jname, java_name);
    return 1;
  }
  strcpy( jname, nc_name);
  return 0;
}

int GlowNodeClass::get_pages()
{
  GlowNodeClass *next;
  int pages = 1;

  for ( next = (GlowNodeClass *) next_nc;
        next;
        next = (GlowNodeClass *) next->next_nc)
    pages++;
  return pages;
}

GlowNodeClass *GlowNodeClass::get_base_nc()
{
  GlowNodeClass *base;

  for ( base = this; base->prev_nc; base = (GlowNodeClass *)base->prev_nc)
    ;
  return base;
}

void GlowNodeClass::get_borders( GlowTransform *t, double *x_right, 
	double *x_left, double *y_high, double *y_low)
{
  GlowNodeClass *base = get_base_nc();

  if ( (!t || ( t && fabs( t->rotation/90 - int(t->rotation/90)) < DBL_EPSILON)) &&
       /* (next_nc || prev_nc) && */
       !(fabs( base->x0 - base->x1) < DBL_EPSILON || 
	 fabs( base->y0 - base->y1) < DBL_EPSILON)) {
    // Borders are given i x0, y0, x1, y1 
    // Will not work in rotated nodes
    double ll_x, ur_x, ll_y, ur_y, kx1, kx2, ky1, ky2;

    if ( t)
    {
      kx1 = t->x( base->x0, base->y0);
      kx2 = t->x( base->x1, base->y1);
      ky1 = t->y( base->x0, base->y0);
      ky2 = t->y( base->x1, base->y1);
    }
    else
    {
      kx1 = base->x0;
      kx2 = base->x1;
      ky1 = base->y0;
      ky2 = base->y1;
    }

    ll_x = min( kx1, kx2);
    ur_x = max( kx1, kx2);
    ll_y = min( ky1, ky2);
    ur_y = max( ky1, ky2);

    if ( ll_x < *x_left)
      *x_left = ll_x;
    if ( ur_x > *x_right)
      *x_right = ur_x;
    if ( ll_y < *y_low)
      *y_low = ll_y;
    if ( ur_y > *y_high)
      *y_high = ur_y;
  }
  else
    a.get_borders( t, x_right, x_left, y_high, y_low);
}

void GlowNodeClass::get_origo( GlowTransform *t, double *x, 
	double *y)
{
  GlowNodeClass *base = get_base_nc();

  if ( /* (next_nc || prev_nc) && */
       !(fabs( base->x0 - base->x1) < DBL_EPSILON || 
	 fabs( base->y0 - base->y1) < DBL_EPSILON)) {
    // Borders are given i x0, y0, x1, y1

    if ( t)
    {
      double ll_x, ll_y, ur_x, ur_y;
      ll_x = ll_y = 1e37; 
      ur_x = ur_y = -1e37;

      get_borders( t, &ur_x, &ll_x, &ur_y, &ll_y); 
      *x = t->x( 0, 0) - ll_x;
      *y = t->y( 0, 0) - ll_y;
    }
    else
    {
      *x = - base->x0;
      *y = - base->y0;
    }
  }
  else {
    double ll_x, ll_y, ur_x, ur_y;
    ll_x = ll_y = 1e37; 
    ur_x = ur_y = -1e37;

    a.get_borders( t, &ur_x, &ll_x, &ur_y, &ll_y);
    *x = -ll_x;
    *y = -ll_y;
  }
}

void GlowNodeClass::convert( glow_eConvert version) 
{
  a.convert( version);
  if ( dyn_type == 3 || dyn_type == 4 || dyn_type == 12) {
    if ( (glow_eDrawTone) dyn_color[0] == glow_eDrawTone_YellowGreen)
      dyn_color[0] = (glow_eDrawType) glow_eDrawTone_Yellow;
    if ( (glow_eDrawTone) dyn_color[1] == glow_eDrawTone_YellowGreen)
      dyn_color[1] = (glow_eDrawType) glow_eDrawTone_Yellow;
  }
  else {
    dyn_color[0] = GlowColor::convert( version, dyn_color[0]);
    dyn_color[1] = GlowColor::convert( version, dyn_color[1]);
  }
}

int GlowNodeClass::find_nc( GlowArrayElem *nodeclass)
{
  return a.find_nc( nodeclass);
}

int GlowNodeClass::get_annotation_info( void *node, int num, int *t_size, glow_eDrawType *t_drawtype,
					glow_eDrawType *t_color)
{
  int		i;

  for ( i = 0; i < a.a_size; i++)
  {
    if ( a.a[i]->type() == glow_eObjectType_GrowAnnot &&
         ((GrowAnnot *)a.a[i])->number == num)
    {
      ((GrowAnnot *)a.a[i])->get_annotation_info( node, t_size, t_drawtype, t_color);
      return 1;
    }
  }
  return 0;
}








