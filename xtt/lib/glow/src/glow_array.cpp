#include "glow_std.h"


#include <iostream.h>
#include <fstream.h>
#include <stdlib.h>
#include "glow_ctx.h"
#include "glow_point.h"
#include "glow_rect.h"
#include "glow_node.h"
#include "glow_con.h"
#include "glow_nodeclass.h"
#include "glow_conclass.h"
#include "glow_conpoint.h"
#include "glow_line.h"
#include "glow_polyline.h"
#include "glow_arc.h"
#include "glow_text.h"
#include "glow_annot.h"
#include "glow_arrow.h"
#include "glow_array.h"
#include "glow_pixmap.h"
#include "glow_annotpixmap.h"
#include "glow_radiobutton.h"
#include "glow_growrect.h"
#include "glow_growrectrounded.h"
#include "glow_growarc.h"
#include "glow_growline.h"
#include "glow_growpolyline.h"
#include "glow_growconpoint.h"
#include "glow_growsubannot.h"
#include "glow_growannot.h"
#include "glow_grownode.h"
#include "glow_growslider.h"
#include "glow_growtext.h"
#include "glow_growbar.h"
#include "glow_growtrend.h"
#include "glow_growslider.h"
#include "glow_growimage.h"
#include "glow_growgroup.h"
#include "glow_growaxis.h"
#include "glow_growconglue.h"
#include "glow_growwindow.h"
#include "glow_growfolder.h"
#include "glow_growtable.h"
#include "glow_msg.h"

GlowArray::GlowArray( int allocate, int incr) : allocated( allocate),
	alloc_incr(incr), a_size(0)
{
  a = (GlowArrayElem **) calloc( allocated, sizeof( GlowArrayElem *));
}

void GlowArray::new_array( const GlowArray& array)
{
  a_size = array.a_size;
  allocated = array.allocated;
  alloc_incr = array.alloc_incr;
  a = (GlowArrayElem **) calloc( allocated, sizeof( GlowArrayElem *));
  memcpy( a, array.a, a_size);
}

void GlowArray::move_from( GlowArray& array)
{
  int i;

  free( a);
  a_size = 0;
  allocated = alloc_incr;
  a = (GlowArrayElem **) calloc( allocated, sizeof( GlowArrayElem *));
  for ( i = 0; i < array.a_size; i++)
  {
    insert( array.a[i]);
    array.remove( array.a[i]);
    i--;
  }
}

void GlowArray::copy_from_common_objects( GlowArray& array)
{
  int i;

  a_size = 0;
  free( a);
  allocated = alloc_incr;
  a = (GlowArrayElem **) calloc( allocated, sizeof( GlowArrayElem *));
  for ( i = 0; i < array.a_size; i++)
  {
    insert( array.a[i]);
  }
}

void GlowArray::copy_from( const GlowArray& array)
{
  int i;

  a_size = 0;
  for ( i = 0; i < array.a_size; i++)
  {
    switch( array.a[i]->type())
    {
      case glow_eObjectType_Node:
      {
        GlowNode *n = new GlowNode();
	n->copy_from(*(GlowNode *)array.a[i]);
        n->highlight = 0;
        n->hot = 0;
        insert( n);        
        break;
      }
      case glow_eObjectType_GrowNode:
      {
        GrowNode *n = new GrowNode();
	n->copy_from(*(GrowNode *)array.a[i]);
        n->highlight = 0;
        n->hot = 0;
        insert( n);        
        break;
      }
      case glow_eObjectType_GrowGroup:
      {
        GrowGroup *n = new GrowGroup();
	n->copy_from(*(GrowGroup *)array.a[i]);
        n->highlight = 0;
        n->hot = 0;
        insert( n);        
        break;
      }
      case glow_eObjectType_GrowSlider:
      {
        GrowSlider *n = new GrowSlider();
	n->copy_from(*(GrowSlider *)array.a[i]);
        n->highlight = 0;
        n->hot = 0;
        insert( n);        
        break;
      }
      case glow_eObjectType_Point:
      {
        GlowPoint *n = new GlowPoint(*(GlowPoint *)array.a[i]);
        insert( n);
        break;
      }
      case glow_eObjectType_Line:
      {
        GlowLine *n = new GlowLine(*(GlowLine *)array.a[i]);
        insert( n);
        break;
      }
      case glow_eObjectType_PolyLine:
      {
        GlowPolyLine *n = new GlowPolyLine(*(GlowPolyLine *)array.a[i]);
        insert( n);
        break;
      }
      case glow_eObjectType_Arc:
      {
        GlowArc *n = new GlowArc(*(GlowArc *)array.a[i]);
        insert( n);
        break;
      }
      case glow_eObjectType_Arrow:
      {
        GlowArrow *n = new GlowArrow(*(GlowArrow *)array.a[i]);
        insert( n);
        break;
      }
      case glow_eObjectType_Rect:
      {
        GlowRect *n = new GlowRect(*(GlowRect *)array.a[i]);
        insert( n);
        break;
      }
      case glow_eObjectType_Text:
      {
        GlowText *n = new GlowText(*(GlowText *)array.a[i]);
        insert( n);
        break;
      }
      case glow_eObjectType_Pixmap:
      {
        GlowPixmap *n = new GlowPixmap(*(GlowPixmap *)array.a[i]);
        insert( n);
        break;
      }
      case glow_eObjectType_AnnotPixmap:
      {
        GlowAnnotPixmap *n = new GlowAnnotPixmap(*(GlowAnnotPixmap *)array.a[i]);
        insert( n);
        break;
      }
      case glow_eObjectType_Radiobutton:
      {
        GlowRadiobutton *n = new GlowRadiobutton(*(GlowRadiobutton *)array.a[i]);
        insert( n);
        break;
      }
      case glow_eObjectType_GrowRect:
      {
        GrowRect *n = new GrowRect(*(GrowRect *)array.a[i]);
        n->highlight = 0;
        n->hot = 0;
        insert( n);
        break;
      }
      case glow_eObjectType_GrowRectRounded:
      {
        GrowRectRounded *n = new GrowRectRounded(*(GrowRectRounded *)array.a[i]);
        n->highlight = 0;
        n->hot = 0;
        insert( n);
        break;
      }
      case glow_eObjectType_GrowImage:
      {
        GrowImage *n = new GrowImage();
	n->copy_from(*(GrowImage *)array.a[i]);
        n->highlight = 0;
        n->hot = 0;
        insert( n);
        break;
      }
      case glow_eObjectType_GrowAxis:
      {
        GrowAxis *n = new GrowAxis(*(GrowAxis *)array.a[i]);
        n->highlight = 0;
        n->hot = 0;
        insert( n);
        break;
      }
      case glow_eObjectType_GrowConGlue:
      {
        GrowConGlue *n = new GrowConGlue(*(GrowConGlue *)array.a[i]);
        n->highlight = 0;
        n->hot = 0;
	// Fix, This should be done in the copy constructor !!!
	sprintf(n->n_name, "O%d", ((GrowCtx *)(n->ctx))->get_next_objectname_num());
        insert( n);
        break;
      }
      case glow_eObjectType_GrowLine:
      {
        GrowLine *n = new GrowLine(*(GrowLine *)array.a[i]);
        n->highlight = 0;
        n->hot = 0;
        insert( n);
        break;
      }
      case glow_eObjectType_GrowPolyLine:
      {
        GrowPolyLine *n = new GrowPolyLine(*(GrowPolyLine *)array.a[i]);
        n->highlight = 0;
        n->hot = 0;
        insert( n);
        break;
      }
      case glow_eObjectType_GrowArc:
      {
        GrowArc *n = new GrowArc(*(GrowArc *)array.a[i]);
        n->highlight = 0;
        n->hot = 0;
        insert( n);
        break;
      }
      case glow_eObjectType_GrowConPoint:
      {
        GrowConPoint *n = new GrowConPoint(*(GrowConPoint *)array.a[i]);
        n->highlight = 0;
        n->hot = 0;
        insert( n);
        break;
      }
      case glow_eObjectType_GrowSubAnnot:
      {
        GrowSubAnnot *n = new GrowSubAnnot(*(GrowSubAnnot *)array.a[i]);
        n->highlight = 0;
        n->hot = 0;
        insert( n);
        break;
      }
      case glow_eObjectType_GrowText:
      {
        GrowText *n = new GrowText(*(GrowText *)array.a[i]);
        n->highlight = 0;
        n->hot = 0;
        insert( n);
        break;
      }
      case glow_eObjectType_GrowBar:
      {
        GrowBar *n = new GrowBar(*(GrowBar *)array.a[i]);
        n->highlight = 0;
        n->hot = 0;
	// Fix, This should be done in the copy constructor !!!
	if ( n->ctx->userdata_copy_callback)
	  (n->ctx->userdata_copy_callback)( n, 
	     ((GrowBar *)array.a[i])->user_data, &n->user_data);
        insert( n);
        break;
      }
      case glow_eObjectType_GrowTrend:
      {
        GrowTrend *n = new GrowTrend(*(GrowTrend *)array.a[i]);
        n->highlight = 0;
        n->hot = 0;
	// Fix, This should be done in the copy constructor !!!
	n->curve[0] = NULL;
	n->curve[1] = NULL;
        n->configure_curves();
	if ( n->ctx->userdata_copy_callback)
	  (n->ctx->userdata_copy_callback)( n, 
	     ((GrowTrend *)(array.a[i]))->user_data, &n->user_data);
        insert( n);
        break;
      }
      case glow_eObjectType_GrowWindow:
      {
        GrowWindow *n = new GrowWindow(*(GrowWindow *)array.a[i]);
        n->highlight = 0;
        n->hot = 0;
	// Fix, This should be done in the copy constructor !!!
	if ( n->ctx->userdata_copy_callback)
	  (n->ctx->userdata_copy_callback)( n, 
	     ((GrowWindow *)array.a[i])->user_data, &n->user_data);
	n->window_ctx = 0;
	n->v_scrollbar = 0;
	n->h_scrollbar = 0;
	n->new_ctx();
	n->configure_scrollbars();
        insert( n);
        break;
      }
      case glow_eObjectType_GrowFolder:
      {
        GrowFolder *n = new GrowFolder(*(GrowFolder *)array.a[i]);
        n->highlight = 0;
        n->hot = 0;
	// Fix, This should be done in the copy constructor !!!
	if ( n->ctx->userdata_copy_callback)
	  (n->ctx->userdata_copy_callback)( n, 
	     ((GrowFolder *)array.a[i])->user_data, &n->user_data);
	n->window_ctx = 0;
	n->v_scrollbar = 0;
	n->h_scrollbar = 0;
	n->new_ctx();
	n->configure_scrollbars();
        insert( n);
        break;
      }
      case glow_eObjectType_GrowTable:
      {
        GrowTable *n = new GrowTable(*(GrowTable *)array.a[i]);
        n->highlight = 0;
        n->hot = 0;
	// Fix, This should be done in the copy constructor !!!
	if ( n->ctx->userdata_copy_callback)
	  (n->ctx->userdata_copy_callback)( n, 
	     ((GrowTable *)array.a[i])->user_data, &n->user_data);
	n->v_scrollbar = 0;
	n->h_scrollbar = 0;
	n->cell_value = 0;
	n->configure();
	n->configure_scrollbars();
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
      case glow_eObjectType_Con:
      {
	/* Both source and destination has to be members */
	GlowNode *dest_node = 0;
	GlowNode *source_node = 0;
        for ( int j = 0, k = 0; j < array.a_size; j++)
        {
          switch( array.a[j]->type())
          {
            case glow_eObjectType_Node:
            case glow_eObjectType_GrowNode:
            case glow_eObjectType_GrowSlider:
            {
	      if ( array.a[j] == ((GlowCon *)array.a[i])->destination())
                dest_node = (GlowNode *)a[k];
	      if ( array.a[j] == ((GlowCon *)array.a[i])->source())
                source_node = (GlowNode *)a[k];
              k++;
              break;
            }
            default:
              ;
          }
        }
        if ( dest_node && source_node)
        {
          GlowCon *n = new GlowCon(*(GlowCon *)array.a[i], source_node, 
		dest_node);
          n->highlight = 0;
          n->hot = 0;
          insert( n);
        }
        break;
      }
      default:
        ;
    }
  }
}

GlowArray::~GlowArray()
{
  int i;

  for ( i = 0; i < a_size; i++)
  {
    delete a[i];
  }
  free( a);
}

GlowArrayElem *GlowArray::operator[] ( int idx)
{
  return *(a + idx);
}

int GlowArray::insert( GlowArrayElem *element)
{
  GlowArrayElem **a_tmp;

  if ( find( element))
    return 0;

  if ( allocated <= a_size)
  {
    allocated += alloc_incr;
    a_tmp = (GlowArrayElem **) calloc( allocated, sizeof( GlowArrayElem *));
    memcpy( a_tmp, a, a_size * sizeof( GlowArrayElem *));
    free( (char *) a);
    a = a_tmp;
  }
  *(a + a_size) = element;
  a_size++;
  return 1;
}

int GlowArray::brow_insert( GlowArrayElem *element, GlowArrayElem *destination, 
	glow_eDest code)
{
  GlowArrayElem **a_tmp;
  int idx, i, j, found;
  int destination_level;

  if ( find( element))
    return 0;

  if ( !destination )
  {
    switch( code)
    {
      case glow_eDest_IntoLast:
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
        destination_level = ((GlowNode *)a[idx])->get_level();
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
    a_tmp = (GlowArrayElem **) calloc( allocated, sizeof( GlowArrayElem *));
    memcpy( a_tmp, a, a_size * sizeof( GlowArrayElem *));
    free( (char *) a);
    a = a_tmp;
  }

  switch( code)
  {
    case glow_eDest_IntoFirst:
      for ( j = a_size - 1; j >= idx; j--)
        a[j + 1] = a[j];        
      a[idx] = element;
      if ( destination)
        ((GlowNode *)element)->set_level( destination_level + 1);
      else
        ((GlowNode *)element)->set_level( destination_level);
      a_size++;
      break;
    case glow_eDest_IntoLast:
      for ( i = idx; i < a_size; i++)
      {
        if ( ((GlowNode *)a[i])->get_level() <= destination_level)
          break;
      }
      idx = i;
      for ( j = a_size - 1; j >= idx; j--)
        a[j + 1] = a[j];        
      a[idx] = element;
      if ( destination)
        ((GlowNode *)element)->set_level( destination_level + 1);
      else
        ((GlowNode *)element)->set_level( destination_level);
      a_size++;
      break;
    case glow_eDest_After:
      for ( i = idx; i < a_size; i++)
      {
        if ( ((GlowNode *)a[i])->get_level() >= destination_level)
          break;
      }
      idx = i;
      for ( j = a_size - 1; j >= idx; j--)
        a[j + 1] = a[j];        
      a[idx] = element;
      ((GlowNode *)element)->set_level( destination_level);
      a_size++;
    case glow_eDest_Before:
      if ( idx > 0)
        idx--;
      for ( j = a_size - 1; j >= idx; j--)
        a[j + 1] = a[j];        
      a[idx] = element;
      ((GlowNode *)element)->set_level( destination_level);
      a_size++;
      break;
    default:
      ;
  }
  return 1;
}

void GlowArray::remove( GlowArrayElem *element)
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

void GlowArray::brow_remove( void *ctx, GlowArrayElem *element)
{
  brow_close( ctx, element);
  ((GlowCtx *)ctx)->delete_object( element);
}

void GlowArray::brow_close( void *ctx, GlowArrayElem *element)
{
  int		i;
  int		idx, next_idx;
  int		found;
  int		level;
  GlowArrayElem *e;

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
  level = ((GlowNode *)a[idx])->get_level();
  for ( i = idx + 1; i < a_size; i++)
  {
    if (((GlowNode *)a[i])->get_level() <= level)
      break;
  }
  next_idx = i;
  if ( next_idx == idx + 1)
    return;

  for ( i = idx + 1; i < next_idx; i++)
  {
    e = a[i];
    ((GlowCtx *)ctx)->delete_object( a[i]);
    i--;
    next_idx--;
  }
}

int GlowArray::brow_get_parent( GlowArrayElem *element, GlowArrayElem **parent)
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
    return GLOW__NOELEM;

  // Find previous element with lower level
  found = 0;
  level = ((GlowNode *)a[idx])->get_level();
  for ( i = idx - 1; i >= 0; i--)
  {
    if (((GlowNode *)a[i])->get_level() < level)
    {
      found = 1;
      break;
    }
  }
  if ( !found)
    return GLOW__NOPARENT;
  *parent = a[i];
  return 1;
}

void GlowArray::zoom()
{
  int		i;

  for ( i = 0; i < a_size; i++)
  {
    a[i]->zoom();
  }
}

void GlowArray::nav_zoom()
{
  int		i;

  for ( i = 0; i < a_size; i++)
  {
    a[i]->nav_zoom();
  }
}

void GlowArray::print_zoom()
{
  int		i;

  for ( i = 0; i < a_size; i++)
  {
    a[i]->print_zoom();
  }
}

void GlowArray::traverse( int x, int y)
{
  int		i;

  for ( i = 0; i < a_size; i++)
  {
    a[i]->traverse( x, y);
  }
}

void GlowArray::conpoint_select( void *pos, int x, int y, double *distance, 
		void **cp)
{
  int		i;

  for ( i = 0; i < a_size; i++)
  {
    a[i]->conpoint_select( pos, x, y, distance, cp);
  }
}

void GlowArray::conpoint_select( GlowTransform *t, int x, int y, double *distance, 
		void **cp, int *pix_x, int *pix_y)
{
  int		i;

  for ( i = 0; i < a_size; i++)
  {
    a[i]->conpoint_select( t, x, y, distance, cp, pix_x, pix_y);
  }
}

void GlowArray::print( void *pos, void *node) 
{
  int i;

  for ( i = 0; i < a_size; i++)
  {
    a[i]->print( pos, node);
  }
}

void GlowArray::save( ofstream& fp, glow_eSaveMode mode) 
{
  int i;

  fp <<	int(glow_eSave_Array) << endl;
  for ( i = 0; i < a_size; i++)
  {
    if ( a[i]->type() != glow_eObjectType_Con)
      a[i]->save( fp, mode);
  }
  for ( i = 0; i < a_size; i++)
  {
    if ( a[i]->type() == glow_eObjectType_Con)
      a[i]->save( fp, mode);
  }
  fp <<	int(glow_eSave_End) << endl;
}

void GlowArray::open( void *ctx, ifstream& fp) 
{
  int		type;
  int 		end_found = 0;
  char		dummy[40];
  int 		errcnt = 0;

  for (;;)
  {
    fp >> type;
    switch( type) {
      case glow_eSave_Array: break;
      case glow_eSave_Rect: 
      {
        GlowRect *n = new GlowRect( (GlowCtx *) ctx);
	n->open( fp);
        insert( n);
        break;
      }
      case glow_eSave_Line: 
      {
        GlowLine *n = new GlowLine( (GlowCtx *) ctx);
	n->open( fp);
        insert( n);
        break;
      }
      case glow_eSave_PolyLine: 
      {
        GlowPolyLine *n = new GlowPolyLine( (GlowCtx *) ctx, (glow_sPoint *) NULL, 0);
	n->open( fp);
        insert( n);
        break;
      }
      case glow_eSave_Arc: 
      {
        GlowArc *n = new GlowArc( (GlowCtx *) ctx);
	n->open( fp);
        insert( n);
        break;
      }
      case glow_eSave_Text: 
      {
        GlowText *n = new GlowText( (GlowCtx *) ctx, "");
	n->open( fp);
        insert( n);
        break;
      }
      case glow_eSave_Pixmap: 
      {
        GlowPixmap *n = new GlowPixmap( (GlowCtx *) ctx, 
		(glow_sPixmapData *) NULL);
	n->open( fp);
        insert( n);
        break;
      }
      case glow_eSave_AnnotPixmap: 
      {
        GlowAnnotPixmap *n = new GlowAnnotPixmap( (GlowCtx *) ctx, 0);
	n->open( fp);
        insert( n);
        break;
      }
      case glow_eSave_Radiobutton: 
      {
        GlowRadiobutton *n = new GlowRadiobutton( (GlowCtx *) ctx);
	n->open( fp);
        insert( n);
        break;
      }
      case glow_eSave_NodeClass: 
      {
        char name[32];
	GlowArrayElem *element;       

        GlowNodeClass *n = new GlowNodeClass( (GlowCtx *) ctx, "");
	n->open( fp);

        //  Check if this NodeClass already is loaded
	n->get_object_name( name);
	if ( find_by_name( name, &element))
          delete n;
        else
          insert( n);
        break;
      }
      case glow_eSave_ConClass: 
      {
        char name[32];
	GlowArrayElem *element;       

        GlowConClass *n = new GlowConClass( (GlowCtx *) ctx, "", 
		glow_eConType_Straight, glow_eCorner_Right, glow_eDrawType_Line, 
		1);
	n->open( fp);

        //  Check if this ConClass already is loaded
	n->get_object_name( name);
	if ( find_by_name( name, &element))
          delete n;
        else
          insert( n);
        break;
      }
      case glow_eSave_ConPoint: 
      {
        GlowConPoint *n = new GlowConPoint( (GlowCtx *) ctx);
	n->open( fp);
        insert( n);
        break;
      }
      case glow_eSave_Annot: 
      {
        GlowAnnot *n = new GlowAnnot( (GlowCtx *) ctx);
	n->open( fp);
        insert( n);
        break;
      }
      case glow_eSave_Arrow: 
      {
        GlowArrow *n = new GlowArrow( (GlowCtx *) ctx,0,0,0,0,0,0,glow_eDrawType_Line);
	n->open( fp);
        insert( n);
        break;
      }
      case glow_eSave_Node: 
      {
        GlowNode *n = new GlowNode( (GlowCtx *) ctx, "", 0, 0, 0);
	n->open( fp);
        if ( n->nc)
          insert( n);
        else
          delete n;
        break;
      }
      case glow_eSave_Con: 
      {
        GlowCon *n = new GlowCon( (GlowCtx *) ctx, "", (GlowConClass *)0, 
		(GlowNode *)0, (GlowNode *)0, 0, 0);
	n->open( fp);
        insert( n);
        break;
      }
      case glow_eSave_Point: 
      {
        GlowPoint *n = new GlowPoint( (GlowCtx *) ctx);
	n->open( fp);
        insert( n);
        break;
      }
      case glow_eSave_GrowRect:
      {
        GrowRect *r = new GrowRect( (GlowCtx *) ctx, "");
	r->open( fp);
        insert( r);
        break;
      }
      case glow_eSave_GrowRectRounded:
      {
        GrowRectRounded *r = new GrowRectRounded( (GlowCtx *) ctx, "");
	r->open( fp);
        insert( r);
        break;
      }
      case glow_eSave_GrowImage:
      {
        GrowImage *r = new GrowImage( (GlowCtx *) ctx, "");
	r->open( fp);
        insert( r);
        break;
      }
      case glow_eSave_GrowAxis:
      {
        GrowAxis *r = new GrowAxis( (GlowCtx *) ctx, "");
	r->open( fp);
        insert( r);
        break;
      }
      case glow_eSave_GrowConGlue:
      {
        GrowConGlue *r = new GrowConGlue( (GlowCtx *) ctx, "");
	r->open( fp);
        insert( r);
        break;
      }
      case glow_eSave_GrowLine: 
      {
        GrowLine *l = new GrowLine( (GlowCtx *) ctx, "");
	l->open( fp);
        insert( l);
        break;
      }
      case glow_eSave_GrowPolyLine: 
      {
        GrowPolyLine *l = new GrowPolyLine( (GlowCtx *) ctx, "", (glow_sPoint*) NULL, 0);
	l->open( fp);
        insert( l);
        break;
      }
      case glow_eSave_GrowArc: 
      {
        GrowArc *a = new GrowArc( (GlowCtx *) ctx, "");
	a->open( fp);
        insert( a);
        break;
      }
      case glow_eSave_GrowConPoint: 
      {
        GrowConPoint *n = new GrowConPoint( (GlowCtx *) ctx, "");
	n->open( fp);
        insert( n);
        break;
      }
      case glow_eSave_GrowAnnot: 
      {
        GrowAnnot *n = new GrowAnnot( (GlowCtx *) ctx);
	n->open( fp);
        insert( n);
        break;
      }
      case glow_eSave_GrowSubAnnot: 
      {
        GrowSubAnnot *n = new GrowSubAnnot( (GlowCtx *) ctx, "");
	n->open( fp);
        insert( n);
        break;
      }
      case glow_eSave_GrowText: 
      {
        GrowText *n = new GrowText( (GlowCtx *) ctx, "", "");
	n->open( fp);
        insert( n);
        break;
      }
      case glow_eSave_GrowBar: 
      {
        GrowBar *n = new GrowBar( (GlowCtx *) ctx, "");
	n->open( fp);
        insert( n);
        break;
      }
      case glow_eSave_GrowTrend: 
      {
        GrowTrend *n = new GrowTrend( (GlowCtx *) ctx, "");
	n->open( fp);
        insert( n);
        break;
      }
      case glow_eSave_GrowWindow: 
      {
        GrowWindow *n = new GrowWindow( (GlowCtx *) ctx, "");
	n->open( fp);
        insert( n);
        break;
      }
      case glow_eSave_GrowTable: 
      {
        GrowTable *n = new GrowTable( (GlowCtx *) ctx, "");
	n->open( fp);
        insert( n);
        break;
      }
      case glow_eSave_GrowFolder: 
      {
        GrowFolder *n = new GrowFolder( (GlowCtx *) ctx, "");
	n->open( fp);
        insert( n);
        break;
      }
      case glow_eSave_GrowNode: 
      {
        GrowNode *n = new GrowNode( (GlowCtx *) ctx, "", 0, 0, 0);
	n->open( fp);
        if ( n->nc)
          insert( n);
        else
          delete n;
        break;
      }
      case glow_eSave_GrowGroup:
      {
        GrowGroup *n = new GrowGroup( (GlowCtx *) ctx, "");
	n->open( fp);
        insert( n);
        break;
      }
      case glow_eSave_GrowSlider: 
      {
        GrowSlider *n = new GrowSlider( (GlowCtx *) ctx, "", 0, 0, 0);
	n->open( fp);
        if ( n->nc)
          insert( n);
        else
          delete n;
        break;
      }
      case glow_eSave_End: end_found = 1; break;
      default:
        cout << "GlowArray:open syntax error" << endl;
        fp.getline( dummy, sizeof(dummy));
	errcnt++;
	if ( errcnt > 20)
	  exit(0);
    }
    if ( end_found)
      break;
  }
}

void GlowArray::draw( void *pos, int highlight, int hot, void *node) 
{
  int i;

  for ( i = 0; i < a_size; i++)
  {
    a[i]->draw( pos, highlight, hot, node);
  }
}

void GlowArray::draw( GlowTransform *t, int highlight, int hot, void *node, 
		      void *colornode) 
{
  int i;

  for ( i = 0; i < a_size; i++)
  {
    a[i]->draw( t, highlight, hot, node, colornode);
  }
}

void GlowArray::draw_inverse( void *pos, int hot, void *node) 
{
  int i;

  for ( i = 0; i < a_size; i++)
  {
    a[i]->draw_inverse( pos, hot, node);
  }
}

void GlowArray::erase( void *pos, int hot, void *node) 
{
  int i;

  for ( i = 0; i < a_size; i++)
  {
    a[i]->erase( pos, hot, node);
  }
}

void GlowArray::erase( GlowTransform *t, int hot, void *node) 
{
  int i;

  for ( i = 0; i < a_size; i++)
  {
    a[i]->erase( t, hot, node);
  }
}

void GlowArray::nav_draw( void *pos, int highlight, void *node) 
{
  int i;

  for ( i = 0; i < a_size; i++)
  {
    a[i]->nav_draw( pos, highlight, node);
  }
}

void GlowArray::nav_draw( GlowTransform *t, int highlight, void *node, 
			  void *colornode) 
{
  int i;

  for ( i = 0; i < a_size; i++)
  {
    a[i]->nav_draw( t, highlight, node, colornode);
  }
}

void GlowArray::nav_erase( void *pos, void *node) 
{
  int i;

  for ( i = 0; i < a_size; i++)
  {
    a[i]->nav_erase( pos, node);
  }
}

void GlowArray::nav_erase( GlowTransform *t, void *node) 
{
  int i;

  for ( i = 0; i < a_size; i++)
  {
    a[i]->nav_erase( t, node);
  }
}

int GlowArray::find( GlowArrayElem *element) 
{
  int i;

  for ( i = 0; i < a_size; i++)
  {
    if ( a[i] == element)
      return 1;
  }
  return 0;
}

int GlowArray::find_by_name( char *name, GlowArrayElem **element) 
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

void GlowArray::set_highlight( int on) 
{
  int i;

  for ( i = 0; i < a_size; i++)
  {
    a[i]->set_highlight( on);
  }
}

void GlowArray::set_hot( int on) 
{
  int i;

  for ( i = 0; i < a_size; i++)
  {
    a[i]->set_hot( on);
  }
}

void GlowArray::select_region_insert( double ll_x, double ll_y, double ur_x, double ur_y, glow_eSelectPolicy select_policy)
{
  int i;

  for ( i = 0; i < a_size; i++)
  {
    a[i]->select_region_insert( ll_x, ll_y, ur_x, ur_y, select_policy);
  }
}

void GlowArray::get_borders(
	double *x_right, double *x_left, double *y_high, double *y_low)
{
  int i;

  for ( i = 0; i < a_size; i++)
  {
    a[i]->get_borders( x_right, x_left, y_high, y_low);
  }
}

void GlowArray::get_borders( double pos_x, double pos_y, double *x_right, 
	double *x_left, double *y_high, double *y_low, void *node)
{
  int i;

  for ( i = 0; i < a_size; i++)
  {
    a[i]->get_borders( pos_x, pos_y, x_right, x_left, y_high, y_low, node);
  }
}

void GlowArray::get_borders( GlowTransform *t, double *x_right, 
	double *x_left, double *y_high, double *y_low)
{
  int i;

  for ( i = 0; i < a_size; i++)
  {
    a[i]->get_borders( t, x_right, x_left, y_high, y_low);
  }
}

void GlowArray::move( double delta_x, double delta_y, int grid)
{
  int i;

  for ( i = 0; i < a_size; i++)
  {
    a[i]->move( delta_x, delta_y, grid);
  }
}

void GlowArray::shift( void *pos, double delta_x, double delta_y, int highlight, int hot)
{
  int i;

  for ( i = 0; i < a_size; i++)
  {
    a[i]->shift( pos, delta_x, delta_y, highlight, hot);
  }
}

void GlowArray::move_noerase( int delta_x, int delta_y, int grid)
{
  int i;

  for ( i = 0; i < a_size; i++)
  {
    a[i]->move_noerase( delta_x, delta_y, grid);
  }
}

void GlowArray::conpoint_refcon_redraw( void *node, int conpoint)
{
  int i;

  for ( i = 0; i < a_size; i++)
  {
    a[i]->conpoint_refcon_redraw( node, conpoint);
  }
}

void GlowArray::conpoint_refcon_erase( void *node, int conpoint)
{
  int i;

  for ( i = 0; i < a_size; i++)
  {
    a[i]->conpoint_refcon_redraw( node, conpoint);
  }
}

void GlowArray::set_inverse( int on)
{
  int i;

  for ( i = 0; i < a_size; i++)
  {
    a[i]->set_inverse( on);
  }
}

int GlowArray::event_handler( glow_eEvent event, int x, int y)
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

int GlowArray::event_handler( glow_eEvent event, int x, int y, double fx,
	double fy)
{
  int		i;
  int		sts;

  for ( i = a_size - 1; i >= 0; i--)
  {
    sts = a[i]->event_handler( event, x, y, fx, fy);
    if ( sts) return sts;
  }
  return 0;
}

int GlowArray::event_handler( glow_eEvent event, double fx, double fy)
{
  int		i;
  int		sts;

  for ( i = a_size - 1; i >= 0; i--)
  {
    sts = a[i]->event_handler( event, fx, fy);
    if ( sts) return sts;
  }
  return 0;
}

int GlowArray::event_handler( void *pos, glow_eEvent event, int x, int y, 
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

int GlowArray::event_handler( void *pos, glow_eEvent event, int x, int y, 
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

void GlowArray::configure()
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

void GlowArray::move_widgets( int x, int y)
{
  int		i;

  for ( i = 0; i < a_size; i++)
  {
      a[i]->move_widgets( x, y);
  }
}

int GlowArray::get_next( GlowArrayElem *element, GlowArrayElem **next) 
{
  int i;

  for ( i = 0; i < a_size; i++)
  {
    if ( a[i] == element)
    {
      if ( i == a_size - 1)
        return GLOW__NONEXT;
      *next = a[i+1];
      return 1;
    }
  }
  return GLOW__NOELEM;
}

int GlowArray::get_previous( GlowArrayElem *element, GlowArrayElem **prev) 
{
  int i;

  for ( i = 0; i < a_size; i++)
  {
    if ( a[i] == element)
    {
      if ( i == 0)
        return GLOW__NOPREVIOUS;
      *prev = a[i-1];
      return 1;
    }
  }
  return GLOW__NOELEM;
}

int GlowArray::get_first( GlowArrayElem **first) 
{
  if ( a_size == 0)
    return GLOW__NOELEM;
  *first = a[0];
  return 1;
}

int GlowArray::get_last( GlowArrayElem **last) 
{
  if ( a_size == 0)
    return GLOW__NOELEM;
  *last = a[a_size - 1];
  return 1;
}

void GlowArray::exec_dynamic()
{
  for ( int i = 0; i < a_size; i++)
    a[i]->exec_dynamic();
}

void GlowArray::set_fill_color( glow_eDrawType drawtype) 
{
  for ( int i = 0; i < a_size; i++)
    a[i]->set_fill_color( drawtype);
}

void GlowArray::reset_fill_color() 
{
  for ( int i = 0; i < a_size; i++)
    a[i]->reset_fill_color();
}

void GlowArray::set_border_color( glow_eDrawType drawtype) 
{
  for ( int i = 0; i < a_size; i++)
    a[i]->set_border_color( drawtype);
}

void GlowArray::reset_border_color()
{
  for ( int i = 0; i < a_size; i++)
    a[i]->reset_border_color();
}

void GlowArray::set_original_fill_color( glow_eDrawType drawtype) 
{
  for ( int i = 0; i < a_size; i++)
    a[i]->set_original_fill_color( drawtype);
}

void GlowArray::set_original_text_color( glow_eDrawType drawtype) 
{
  for ( int i = 0; i < a_size; i++)
    a[i]->set_original_text_color( drawtype);
}

void GlowArray::set_original_border_color( glow_eDrawType drawtype) 
{
  for ( int i = 0; i < a_size; i++)
    a[i]->set_original_border_color( drawtype);
}

void GlowArray::set_original_color_tone( glow_eDrawTone tone) 
{
  for ( int i = 0; i < a_size; i++)
    a[i]->set_original_color_tone( tone);
}

void GlowArray::set_color_tone( glow_eDrawTone tone) 
{
  for ( int i = 0; i < a_size; i++)
    a[i]->set_color_tone( tone);
}

void GlowArray::reset_color_tone() 
{
  for ( int i = 0; i < a_size; i++)
    a[i]->reset_color_tone();
}

void GlowArray::set_original_color_lightness( int lightness) 
{
  for ( int i = 0; i < a_size; i++)
    a[i]->set_original_color_lightness( lightness);
}

void GlowArray::incr_original_color_lightness( int lightness) 
{
  for ( int i = 0; i < a_size; i++)
    a[i]->incr_original_color_lightness( lightness);
}

void GlowArray::set_color_lightness( int lightness) 
{
  for ( int i = 0; i < a_size; i++)
    a[i]->set_color_lightness( lightness);
}

void GlowArray::reset_color_lightness() 
{
  for ( int i = 0; i < a_size; i++)
    a[i]->reset_color_lightness();
}

void GlowArray::set_original_color_intensity( int intensity) 
{
  for ( int i = 0; i < a_size; i++)
    a[i]->set_original_color_intensity( intensity);
}

void GlowArray::incr_original_color_intensity( int intensity) 
{
  for ( int i = 0; i < a_size; i++)
    a[i]->incr_original_color_intensity( intensity);
}

void GlowArray::set_color_intensity( int intensity) 
{
  for ( int i = 0; i < a_size; i++)
    a[i]->set_color_intensity( intensity);
}

void GlowArray::reset_color_intensity() 
{
  for ( int i = 0; i < a_size; i++)
    a[i]->reset_color_intensity();
}

void GlowArray::set_original_color_shift( int shift) 
{
  for ( int i = 0; i < a_size; i++)
    a[i]->set_original_color_shift( shift);
}

void GlowArray::incr_original_color_shift( int shift) 
{
  for ( int i = 0; i < a_size; i++)
    a[i]->incr_original_color_shift( shift);
}

void GlowArray::incr_color_shift( int shift) 
{
  for ( int i = 0; i < a_size; i++)
    a[i]->incr_original_color_shift( shift);
}

void GlowArray::set_color_shift( int shift) 
{
  for ( int i = 0; i < a_size; i++)
    a[i]->set_color_shift( shift);
}

void GlowArray::reset_color_shift() 
{
  for ( int i = 0; i < a_size; i++)
    a[i]->reset_color_shift();
}

void GlowArray::set_linewidth( int linewidth) 
{
  for ( int i = 0; i < a_size; i++)
    a[i]->set_linewidth( linewidth);
}

void GlowArray::set_fill( int fill) 
{
  for ( int i = 0; i < a_size; i++)
    a[i]->set_fill( fill);
}

void GlowArray::set_border( int border) 
{
  for ( int i = 0; i < a_size; i++)
    a[i]->set_border( border);
}

void GlowArray::set_shadow( int shadow) 
{
  for ( int i = 0; i < a_size; i++)
    a[i]->set_shadow( shadow);
}

void GlowArray::set_drawtype( glow_eDrawType drawtype) 
{
  for ( int i = 0; i < a_size; i++)
    a[i]->set_drawtype( drawtype);
}

void GlowArray::set_transform_from_stored( GlowTransform *t)
{
  for ( int i = 0; i < a_size; i++)
    a[i]->set_transform_from_stored( t);
}

void GlowArray::set_transform( GlowTransform *t)
{
  for ( int i = 0; i < a_size; i++)
    a[i]->set_transform( t);
}

void GlowArray::store_transform()
{
  for ( int i = 0; i < a_size; i++)
    a[i]->store_transform();
}

void GlowArray::pop( GlowArrayElem *element)
{
  remove( element);
  insert( element);
}

void GlowArray::push( GlowArrayElem *element)
{
  remove( element);
  for ( int j = a_size - 1; j >= 0; j--)
    a[j + 1] = a[j];        
  a[0] = element;
  a_size++;
}

void GlowArray::get_node_borders()
{
  for ( int i = 0; i < a_size; i++)
    a[i]->get_node_borders();
}

void GlowArray::align( double x, double y, glow_eAlignDirection direction)
{
  for ( int i = 0; i < a_size; i++)
    a[i]->align( x, y, direction);
}

int GlowArray::find_nc( GlowArrayElem *nc)
{

  for ( int i = 0; i < a_size; i++) {
    if ( a[i]->find_nc( nc))
      return 1;
  }
  return 0;
}

int GlowArray::find_cc( GlowArrayElem *cc)
{

  for ( int i = 0; i < a_size; i++) {
    if ( a[i]->find_cc( cc))
      return 1;
  }
  return 0;
}

void GlowArray::trace_scan()
{

  for ( int i = 0; i < a_size; i++)
    a[i]->trace_scan();
}


int GlowArray::trace_init()
{
  for ( int i = 0; i < a_size; i++)
    a[i]->trace_init();
  return 1;
}

void GlowArray::trace_close()
{

  for ( int i = 0; i < a_size; i++)
    a[i]->trace_close();
}

void GlowArray::get_nodegroups( void *a_ng)
{
  for ( int i = 0; i < a_size; i++)
    a[i]->get_nodegroups( a_ng);
}

void GlowArray::set_last_group( char *name)
{
  for ( int i = 0; i < a_size; i++)
    a[i]->set_last_group( name);
}

char *GlowArray::get_last_group()
{
  char *name;
  static char groups[10][32];
  char member_cnt[10];
  int group_cnt;
  int found;
  int max_members;
  int max_idx;
  int i, j;

  memset( member_cnt, 0, sizeof(member_cnt));
  group_cnt = 0;

  for ( i = 0; i < a_size; i++)
  {
    name = a[i]->get_last_group();
    if ( strcmp( name, "") != 0) {
      // Find group and increment member count
      found = 0;
      for ( j = 0; j < group_cnt; j++) {
        if ( strcmp( groups[j], name) == 0) {
          member_cnt[j]++;
          found = 1;
          break;
        }
      }
      if ( !found && group_cnt <= 10) {
        strcpy( groups[group_cnt], name);
        member_cnt[group_cnt]++;
        group_cnt++;
      }
    }
  }

  // Analyse result
  max_members = 0;
  for ( i = 0; i < group_cnt; i++) {
    if ( member_cnt[i] > max_members) {
      max_members = member_cnt[i];
      max_idx = i;
    }
  }
  if ( max_members > 0)
    return groups[max_idx];

  return "";
}

int GlowArray::get_background_object_limits( GlowTransform *t, 
		    glow_eTraceType type,
		    double x, double y, GlowArrayElem **background,
		    double *min, double *max, glow_eDirection *direction)
{
  int sts;

  for ( int i = 0; i < a_size; i++) {
    sts = a[i]->get_background_object_limits( t, type, x, y, background,
					      min, max, direction);
    if ( ODD(sts))
      break;
  }
  return sts;
}

void GlowArray::flip( double x0, double y0, glow_eFlipDirection dir)
{
  for ( int i = 0; i < a_size; i++)
    a[i]->flip( x0, y0, dir);
}

void GlowArray::convert( glow_eConvert version)
{
  for ( int i = 0; i < a_size; i++)
    a[i]->convert( version);
}

void GlowArray::set_rootnode( void *node)
{
  for ( int i = 0; i < a_size; i++)
    a[i]->set_rootnode( node);
}

void GlowArray::set_linetype( glow_eLineType type)
{
  for ( int i = 0; i < a_size; i++)
    a[i]->set_linetype( type);
}


