#include "flow_std.h"


#include <iostream.h>
#include <fstream.h>
#include <stdlib.h>
#include "flow_ctx.h"
#include "flow_point.h"
#include "flow_rect.h"
#include "flow_node.h"
#include "flow_con.h"
#include "flow_nodeclass.h"
#include "flow_conclass.h"
#include "flow_conpoint.h"
#include "flow_line.h"
#include "flow_arc.h"
#include "flow_text.h"
#include "flow_annot.h"
#include "flow_arrow.h"
#include "flow_array.h"
#include "flow_pixmap.h"
#include "flow_annotpixmap.h"
#include "flow_radiobutton.h"
#include "flow_msg.h"

#define r_toupper(c) (((c) >= 'a' && (c) <= 'z') ? (c) & 0xDF : (c))
#define r_tolower(c) (((c) >= 'A' && (c) <= 'Z') ? (c) | 0x20 : (c))


FlowArray::FlowArray( int allocate, int incr) : allocated( allocate),
	alloc_incr(incr), a_size(0)
{
  a = (FlowArrayElem **) calloc( allocated, sizeof( FlowArrayElem *));
}

void FlowArray::new_array( const FlowArray& array)
{
  a_size = array.a_size;
  allocated = array.allocated;
  alloc_incr = array.alloc_incr;
  a = (FlowArrayElem **) calloc( allocated, sizeof( FlowArrayElem *));
  memcpy( a, array.a, a_size);
}

void FlowArray::move_from( FlowArray& array)
{
  int i;

  free( a);
  a_size = 0;
  allocated = alloc_incr;
  a = (FlowArrayElem **) calloc( allocated, sizeof( FlowArrayElem *));
  for ( i = 0; i < array.a_size; i++)
  {
    insert( array.a[i]);
    array.remove( array.a[i]);
    i--;
  }
}

void FlowArray::copy_from_common_objects( FlowArray& array)
{
  int i;

  a_size = 0;
  free( a);
  allocated = alloc_incr;
  a = (FlowArrayElem **) calloc( allocated, sizeof( FlowArrayElem *));
  for ( i = 0; i < array.a_size; i++)
  {
    insert( array.a[i]);
  }
}

void FlowArray::copy_from( const FlowArray& array)
{
  int i;

  a_size = 0;
  for ( i = 0; i < array.a_size; i++)
  {
    switch( array.a[i]->type())
    {
      case flow_eObjectType_Node:
      {
        FlowNode *n = new FlowNode();
	n->copy_from(*(FlowNode *)array.a[i]);
        insert( n);        
        break;
      }
      default:
        ;
    }
  }
  for ( i = 0; i < array.a_size; i++)
  {
    switch( array.a[i]->type())
    {
      case flow_eObjectType_Con:
      {
	/* Both source and destination has to be members */
	FlowNode *dest_node = 0;
	FlowNode *source_node = 0;
        for ( int j = 0, k = 0; j < array.a_size; j++)
        {
          switch( array.a[j]->type())
          {
            case flow_eObjectType_Node:
            {
	      if ( array.a[j] == ((FlowCon *)array.a[i])->destination())
                dest_node = (FlowNode *)a[k];
	      if ( array.a[j] == ((FlowCon *)array.a[i])->source())
                source_node = (FlowNode *)a[k];
              k++;
              break;
            }
            default:
              ;
          }
        }
        if ( dest_node && source_node)
        {
          FlowCon *n = new FlowCon(*(FlowCon *)array.a[i], source_node, 
		dest_node);
          insert( n);
        }
        break;
      }
      case flow_eObjectType_Line:
      {
        FlowLine *n = new FlowLine(*(FlowLine *)array.a[i]);
        insert( n);
        break;
      }
      case flow_eObjectType_Arc:
      {
        FlowArc *n = new FlowArc(*(FlowArc *)array.a[i]);
        insert( n);
        break;
      }
      case flow_eObjectType_Rect:
      {
        FlowRect *n = new FlowRect(*(FlowRect *)array.a[i]);
        insert( n);
        break;
      }
      case flow_eObjectType_Text:
      {
        FlowText *n = new FlowText(*(FlowText *)array.a[i]);
        insert( n);
        break;
      }
      case flow_eObjectType_Pixmap:
      {
        FlowPixmap *n = new FlowPixmap(*(FlowPixmap *)array.a[i]);
        insert( n);
        break;
      }
      case flow_eObjectType_AnnotPixmap:
      {
        FlowAnnotPixmap *n = new FlowAnnotPixmap(*(FlowAnnotPixmap *)array.a[i]);
        insert( n);
        break;
      }
      case flow_eObjectType_Radiobutton:
      {
        FlowRadiobutton *n = new FlowRadiobutton(*(FlowRadiobutton *)array.a[i]);
        insert( n);
        break;
      }
      default:
        ;
    }
  }
}

FlowArray::~FlowArray()
{
  int i;

  for ( i = 0; i < a_size; i++)
  {
    delete a[i];
  }
  free( a);
}

FlowArrayElem *FlowArray::operator[] ( int idx)
{
  return *(a + idx);
}

int FlowArray::insert( FlowArrayElem *element)
{
  FlowArrayElem **a_tmp;

  if ( find( element))
    return 0;

  if ( allocated <= a_size)
  {
    allocated += alloc_incr;
    a_tmp = (FlowArrayElem **) calloc( allocated, sizeof( FlowArrayElem *));
    memcpy( a_tmp, a, a_size * sizeof( FlowArrayElem *));
    free( (char *) a);
    a = a_tmp;
  }
  *(a + a_size) = element;
  a_size++;
  return 1;
}

int FlowArray::brow_insert( FlowArrayElem *element, FlowArrayElem *destination, 
	flow_eDest code)
{
  FlowArrayElem **a_tmp;
  int idx, i, j, found;
  int destination_level;

  if ( find( element))
    return 0;

  if ( !destination )
  {
    switch( code)
    {
      case flow_eDest_IntoLast:
        idx = a_size;
        break;
      default:
        idx = 0;
    }
    destination_level = 0;
  }
  else
  {
    found = 0;
    for ( idx = 0; idx < a_size; idx++)
    {
      if ( a[idx] == destination)
      {
        found = 1;
        destination_level = ((FlowNode *)a[idx])->get_level();
        idx++;
        break;
      }
    }
    if ( !found)
      return 0;
  }

  if ( allocated <= a_size)
  {
    allocated += alloc_incr;
    a_tmp = (FlowArrayElem **) calloc( allocated, sizeof( FlowArrayElem *));
    memcpy( a_tmp, a, a_size * sizeof( FlowArrayElem *));
    free( (char *) a);
    a = a_tmp;
  }

  switch( code)
  {
    case flow_eDest_IntoFirst:
      for ( j = a_size - 1; j >= idx; j--)
        a[j + 1] = a[j];        
      a[idx] = element;
      if ( destination)
        ((FlowNode *)element)->set_level( destination_level + 1);
      else
        ((FlowNode *)element)->set_level( destination_level);
      a_size++;
      break;
    case flow_eDest_IntoLast:
      for ( i = idx; i < a_size; i++)
      {
        if ( ((FlowNode *)a[i])->get_level() <= destination_level)
          break;
      }
      idx = i;
      for ( j = a_size - 1; j >= idx; j--)
        a[j + 1] = a[j];        
      a[idx] = element;
      if ( destination)
        ((FlowNode *)element)->set_level( destination_level + 1);
      else
        ((FlowNode *)element)->set_level( destination_level);
      a_size++;
      break;
    case flow_eDest_After:
      for ( i = idx; i < a_size; i++)
      {
        if ( ((FlowNode *)a[i])->get_level() >= destination_level)
          break;
      }
      idx = i;
      for ( j = a_size - 1; j >= idx; j--)
        a[j + 1] = a[j];        
      a[idx] = element;
      ((FlowNode *)element)->set_level( destination_level);
      a_size++;
      break;
    case flow_eDest_Before:
      if ( idx > 0)
        idx--;
      for ( j = a_size - 1; j >= idx; j--)
        a[j + 1] = a[j];        
      a[idx] = element;
      ((FlowNode *)element)->set_level( destination_level);
      a_size++;
      break;
    default:
      ;
  }
  return 1;
}

void FlowArray::remove( FlowArrayElem *element)
{
  int		i;

  for ( i = 0; i < a_size; i++)
  {
    if ( *(a + i) == element)
    {
      memcpy( a+i, a+i+1, (a_size-i-1)*sizeof(*a));
      a_size--;
      return;
    }
  }
}

void FlowArray::brow_remove( void *ctx, FlowArrayElem *element)
{
  brow_close( ctx, element);
  ((FlowCtx *)ctx)->delete_object( element);
}

void FlowArray::brow_close( void *ctx, FlowArrayElem *element)
{
  int		i;
  int		idx, next_idx;
  int		found;
  int		level;
  FlowArrayElem *e;

  found = 0;
  for ( i = 0; i < a_size; i++)
  {
    if ( *(a + i) == element)
    {
      idx = i;
      found = 1;
    }
  }
  if ( !found)
    return;

  // Find next element with the same level
  level = ((FlowNode *)a[idx])->get_level();
  for ( i = idx + 1; i < a_size; i++)
  {
    if (((FlowNode *)a[i])->get_level() <= level)
      break;
  }
  next_idx = i;
  if ( next_idx == idx + 1)
    return;

  for ( i = idx + 1; i < next_idx; i++)
  {
    e = a[i];
    ((FlowCtx *)ctx)->delete_object( a[i]);
    i--;
    next_idx--;
  }
}

int FlowArray::brow_get_parent( FlowArrayElem *element, FlowArrayElem **parent)
{
  int		i;
  int		idx;
  int		found;
  int		level;

  found = 0;
  for ( i = 0; i < a_size; i++)
  {
    if ( *(a + i) == element)
    {
      idx = i;
      found = 1;
      break;
    }
  }
  if ( !found)
    return FLOW__NOELEM;

  // Find previous element with lower level
  found = 0;
  level = ((FlowNode *)a[idx])->get_level();
  for ( i = idx - 1; i >= 0; i--)
  {
    if (((FlowNode *)a[i])->get_level() < level)
    {
      found = 1;
      break;
    }
  }
  if ( !found)
    return FLOW__NOPARENT;
  *parent = a[i];
  return 1;
}

int FlowArray::brow_get_child( FlowArrayElem *element, FlowArrayElem **child)
{
  int		i;
  int		idx;
  int		found;

  found = 0;
  for ( i = 0; i < a_size; i++)
  {
    if ( *(a + i) == element)
    {
      idx = i;
      found = 1;
      break;
    }
  }
  if ( !found)
    return FLOW__NOELEM;

  if ( idx == a_size - 1)
    return FLOW__NOCHILD;

  // Return next element if higher level
  if (((FlowNode *)a[idx+1])->get_level() > ((FlowNode *)a[idx])->get_level())
    *child = a[idx+1];
  else
    return FLOW__NOCHILD;
  return 1;
}

int FlowArray::brow_get_next_sibling( FlowArrayElem *element, 
		FlowArrayElem **sibling)
{
  int		i;
  int		idx;
  int		found;
  int		level;

  found = 0;
  for ( i = 0; i < a_size; i++)
  {
    if ( *(a + i) == element)
    {
      idx = i;
      found = 1;
      break;
    }
  }
  if ( !found)
    return FLOW__NOELEM;

  if ( idx == a_size - 1)
    return FLOW__NONEXTSIBLING;

  // Return next element of higher level
  level = ((FlowNode *)a[idx])->get_level();
  for ( i = idx + 1; i < a_size; i++)
  { 
    if (((FlowNode *)a[i])->get_level() == level)
    {
      *sibling = a[i];
      return 1;
    }
    if (((FlowNode *)a[i])->get_level() < level)
      return FLOW__NONEXTSIBLING;
   }
   return FLOW__NONEXTSIBLING;
}

void FlowArray::zoom()
{
  int		i;

  for ( i = 0; i < a_size; i++)
  {
    a[i]->zoom();
  }
}

void FlowArray::nav_zoom()
{
  int		i;

  for ( i = 0; i < a_size; i++)
  {
    a[i]->nav_zoom();
  }
}

void FlowArray::print_zoom()
{
  int		i;

  for ( i = 0; i < a_size; i++)
  {
    a[i]->print_zoom();
  }
}

void FlowArray::traverse( int x, int y)
{
  int		i;

  for ( i = 0; i < a_size; i++)
  {
    a[i]->traverse( x, y);
  }
}

void FlowArray::conpoint_select( void *pos, int x, int y, double *distance, 
		void **cp)
{
  int		i;

  for ( i = 0; i < a_size; i++)
  {
    a[i]->conpoint_select( pos, x, y, distance, cp);
  }
}

void FlowArray::print( void *pos, void *node) 
{
  int i;

  for ( i = 0; i < a_size; i++)
  {
    a[i]->print( pos, node);
  }
}

void FlowArray::save( ofstream& fp, flow_eSaveMode mode) 
{
  int i;

  fp <<	int(flow_eSave_Array) << endl;
  for ( i = 0; i < a_size; i++)
  {
    if ( a[i]->type() != flow_eObjectType_Con)
      a[i]->save( fp, mode);
  }
  for ( i = 0; i < a_size; i++)
  {
    if ( a[i]->type() == flow_eObjectType_Con)
      a[i]->save( fp, mode);
  }
  fp <<	int(flow_eSave_End) << endl;
}

void FlowArray::open( void *ctx, ifstream& fp) 
{
  int		type;
  int 		end_found = 0;
  char		dummy[40];

  for (;;)
  {
    fp >> type;
    switch( type) {
      case flow_eSave_Array: break;
      case flow_eSave_Rect: 
      {
        FlowRect *n = new FlowRect( (FlowCtx *) ctx);
	n->open( fp);
        insert( n);
        break;
      }
      case flow_eSave_Line: 
      {
        FlowLine *n = new FlowLine( (FlowCtx *) ctx);
	n->open( fp);
        insert( n);
        break;
      }
      case flow_eSave_Arc: 
      {
        FlowArc *n = new FlowArc( (FlowCtx *) ctx);
	n->open( fp);
        insert( n);
        break;
      }
      case flow_eSave_Text: 
      {
        FlowText *n = new FlowText( (FlowCtx *) ctx, "");
	n->open( fp);
        insert( n);
        break;
      }
      case flow_eSave_Pixmap: 
      {
        FlowPixmap *n = new FlowPixmap( (FlowCtx *) ctx, 
		(flow_sPixmapData *) NULL);
	n->open( fp);
        insert( n);
        break;
      }
      case flow_eSave_AnnotPixmap: 
      {
        FlowAnnotPixmap *n = new FlowAnnotPixmap( (FlowCtx *) ctx, 0);
	n->open( fp);
        insert( n);
        break;
      }
      case flow_eSave_Radiobutton: 
      {
        FlowRadiobutton *n = new FlowRadiobutton( (FlowCtx *) ctx);
	n->open( fp);
        insert( n);
        break;
      }
      case flow_eSave_NodeClass: 
      {
        FlowNodeClass *n = new FlowNodeClass( (FlowCtx *) ctx, "");
	n->open( fp);
        insert( n);
        break;
      }
      case flow_eSave_ConClass: 
      {
        FlowConClass *n = new FlowConClass( (FlowCtx *) ctx, "", 
		flow_eConType_Straight, flow_eCorner_Right, flow_eDrawType_Line, 
		1);
	n->open( fp);
        insert( n);
        break;
      }
      case flow_eSave_ConPoint: 
      {
        FlowConPoint *n = new FlowConPoint( (FlowCtx *) ctx);
	n->open( fp);
        insert( n);
        break;
      }
      case flow_eSave_Annot: 
      {
        FlowAnnot *n = new FlowAnnot( (FlowCtx *) ctx);
	n->open( fp);
        insert( n);
        break;
      }
      case flow_eSave_Arrow: 
      {
        FlowArrow *n = new FlowArrow( (FlowCtx *) ctx,0,0,0,0,0,0,flow_eDrawType_Line);
	n->open( fp);
        insert( n);
        break;
      }
      case flow_eSave_Node: 
      {
        FlowNode *n = new FlowNode( (FlowCtx *) ctx, "", 0, 0, 0);
	n->open( fp);
        insert( n);
        break;
      }
      case flow_eSave_Con: 
      {
        FlowCon *n = new FlowCon( (FlowCtx *) ctx, "", (FlowConClass *)0, 
 		(FlowNode *)0, (FlowNode *)0, 0, 0);
	n->open( fp);
        insert( n);
        break;
      }
      case flow_eSave_Point: 
      {
        FlowPoint *n = new FlowPoint( (FlowCtx *) ctx);
	n->open( fp);
        insert( n);
        break;
      }
      case flow_eSave_End: end_found = 1; break;
      default:
        cout << "FlowArray:open syntax error" << endl;
        fp.getline( dummy, sizeof(dummy));
    }
    if ( end_found)
      break;
  }
}

void FlowArray::draw( void *pos, int highlight, int hot, void *node) 
{
  int i;

  for ( i = 0; i < a_size; i++)
  {
    a[i]->draw( pos, highlight, hot, node);
  }
}

void FlowArray::draw_inverse( void *pos, int hot, void *node) 
{
  int i;

  for ( i = 0; i < a_size; i++)
  {
    a[i]->draw_inverse( pos, hot, node);
  }
}

void FlowArray::erase( void *pos, int hot, void *node) 
{
  int i;

  for ( i = 0; i < a_size; i++)
  {
    a[i]->erase( pos, hot, node);
  }
}

void FlowArray::nav_draw( void *pos, int highlight, void *node) 
{
  int i;

  for ( i = 0; i < a_size; i++)
  {
    a[i]->nav_draw( pos, highlight, node);
  }
}

void FlowArray::nav_erase( void *pos, void *node) 
{
  int i;

  for ( i = 0; i < a_size; i++)
  {
    a[i]->nav_erase( pos, node);
  }
}

int FlowArray::find( FlowArrayElem *element) 
{
  int i;

  for ( i = 0; i < a_size; i++)
  {
    if ( a[i] == element)
      return 1;
  }
  return 0;
}

int FlowArray::find_by_name( char *name, FlowArrayElem **element) 
{
  int i;
  char object_name[32];

  for ( i = 0; i < a_size; i++)
  {
    a[i]->get_object_name( object_name);
    if ( strcmp( name, object_name) == 0)
    {
      *element = a[i];
      return 1;
    }
  }
  return 0;
}

int FlowArray::find_by_name_no_case( char *name, FlowArrayElem **element) 
{
  int i;
  char object_name[32];
  char lname[32];
  char *s;

  strcpy( lname, name);
  for ( s = lname; *s; s++)
    *s = r_toupper( *s);

  for ( i = 0; i < a_size; i++)
  {
    a[i]->get_object_name( object_name);
    for ( s = object_name; *s; s++)
      *s = r_toupper( *s);
    if ( strcmp( lname, object_name) == 0)
    {
      *element = a[i];
      return 1;
    }
  }
  return 0;
}

void FlowArray::set_highlight( int on) 
{
  int i;

  for ( i = 0; i < a_size; i++)
  {
    a[i]->set_highlight( on);
  }
}

void FlowArray::set_hot( int on) 
{
  int i;

  for ( i = 0; i < a_size; i++)
  {
    a[i]->set_hot( on);
  }
}

void FlowArray::select_region_insert( double ll_x, double ll_y, double ur_x, double ur_y)
{
  int i;

  for ( i = 0; i < a_size; i++)
  {
    a[i]->select_region_insert( ll_x, ll_y, ur_x, ur_y);
  }
}

void FlowArray::get_borders(
	double *x_right, double *x_left, double *y_high, double *y_low)
{
  int i;

  for ( i = 0; i < a_size; i++)
  {
    a[i]->get_borders( x_right, x_left, y_high, y_low);
  }
}

void FlowArray::get_borders()
{
  int i;

  for ( i = 0; i < a_size; i++)
  {
    a[i]->get_borders();
  }
}

void FlowArray::get_borders( double pos_x, double pos_y, double *x_right, 
	double *x_left, double *y_high, double *y_low, void *node)
{
  int i;

  for ( i = 0; i < a_size; i++)
  {
    a[i]->get_borders( pos_x, pos_y, x_right, x_left, y_high, y_low, node);
  }
}

void FlowArray::move( int delta_x, int delta_y, int grid)
{
  int i;

  for ( i = 0; i < a_size; i++)
  {
    a[i]->move( delta_x, delta_y, grid);
  }
}

void FlowArray::shift( void *pos, double delta_x, double delta_y, int highlight, int hot)
{
  int i;

  for ( i = 0; i < a_size; i++)
  {
    a[i]->shift( pos, delta_x, delta_y, highlight, hot);
  }
}

void FlowArray::move_noerase( int delta_x, int delta_y, int grid)
{
  int i;

  for ( i = 0; i < a_size; i++)
  {
    a[i]->move_noerase( delta_x, delta_y, grid);
  }
}

void FlowArray::conpoint_refcon_redraw( void *node, int conpoint)
{
  int i;

  for ( i = 0; i < a_size; i++)
  {
    a[i]->conpoint_refcon_redraw( node, conpoint);
  }
}

void FlowArray::conpoint_refcon_erase( void *node, int conpoint)
{
  int i;

  for ( i = 0; i < a_size; i++)
  {
    a[i]->conpoint_refcon_redraw( node, conpoint);
  }
}

void FlowArray::set_inverse( int on)
{
  int i;

  for ( i = 0; i < a_size; i++)
  {
    a[i]->set_inverse( on);
  }
}

int FlowArray::event_handler( flow_eEvent event, int x, int y)
{
  int		i;
  int		sts;

  for ( i = 0; i < a_size; i++)
  {
    sts = a[i]->event_handler( event, x, y);
    if ( sts) return sts;
  }
  return 0;
}

int FlowArray::event_handler( void *pos, flow_eEvent event, int x, int y, 
	void *node)
{
  int		i;
  int		sts;

  for ( i = 0; i < a_size; i++)
  {
    sts = a[i]->event_handler( pos, event, x, y, node);
    if ( sts) return sts;
  }
  return 0;
}

// Special eventhandler for connection lines...

int FlowArray::event_handler( void *pos, flow_eEvent event, int x, int y, 
		int num)
{
  int		i;
  int		sts;

  for ( i = 0; i < num; i++)
  {
    sts = a[i]->event_handler( pos, event, x, y, NULL);
    if ( sts) return sts;
  }
  return 0;
}

void FlowArray::configure()
{
  int		i;

  for ( i = 0; i < a_size; i++)
  {
    if ( i == 0)
      a[i]->configure( NULL);
    else
      a[i]->configure( a[i-1]);
  }
}

void FlowArray::move_widgets( int x, int y)
{
  int		i;

  for ( i = 0; i < a_size; i++)
  {
      a[i]->move_widgets( x, y);
  }
}

int FlowArray::get_next( FlowArrayElem *element, FlowArrayElem **next) 
{
  int i;

  for ( i = 0; i < a_size; i++)
  {
    if ( a[i] == element)
    {
      if ( i == a_size - 1)
        return FLOW__NONEXT;
      *next = a[i+1];
      return 1;
    }
  }
  return FLOW__NOELEM;
}

int FlowArray::get_previous( FlowArrayElem *element, FlowArrayElem **prev) 
{
  int i;

  for ( i = 0; i < a_size; i++)
  {
    if ( a[i] == element)
    {
      if ( i == 0)
        return FLOW__NOPREVIOUS;
      *prev = a[i-1];
      return 1;
    }
  }
  return FLOW__NOELEM;
}

int FlowArray::get_first( FlowArrayElem **first) 
{
  if ( a_size == 0)
    return FLOW__NOELEM;
  *first = a[0];
  return 1;
}

int FlowArray::get_last( FlowArrayElem **last) 
{
  if ( a_size == 0)
    return FLOW__NOELEM;
  *last = a[a_size - 1];
  return 1;
}

