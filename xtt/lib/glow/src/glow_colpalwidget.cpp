#include "glow_std.h"

#include <stdio.h>
#include <stdlib.h>

#ifdef __OS_VMS
#pragma member_alignment
#endif

#include <Xm/Xm.h>
#include <Xm/XmP.h>
#include <Xm/ScrollBar.h>
#include <Xm/Form.h>
#include <Mrm/MrmPublic.h>
#ifndef _XtIntrinsic_h
#include <X11/Intrinsic.h>
#endif
#include <X11/Xlib.h>
#include <X11/Xutil.h>

#include "glow.h"
#include "glow_colpalctx.h"
#include "glow_draw.h"
#include "glow_colpalwidget.h"

typedef struct {
	Widget  colpal;
	Widget	form;
	Widget	scroll_h;
	Widget	scroll_v;
	int	scroll_h_managed;
	int	scroll_v_managed;
	} colpalwidget_sScroll;

static XtGeometryResult GeometryManager( 
		Widget w, 
		XtWidgetGeometry *request,
		XtWidgetGeometry *reply);
static void Notify( Widget w, XEvent *event);
static void Destroy( Widget w);
static void Realize( Widget w, unsigned long *dum, XSetWindowAttributes *swa);
static void Initialize( Widget rec, Widget new_widget, ArgList arg, int *args);
static void Redisplay( Widget w, XEvent *event, Region region);
static Boolean SetValues( Widget old, Widget request, Widget new_widget);

static char defaultTranslations[] = "#replace \n\
<Btn1Up>: notify()\n\
<Btn2Up>: notify()\n\
<Btn3Up>: notify()\n\
<Btn1Down>: notify()\n\
<Btn2Down>: notify()\n\
<Btn3Down>: notify()\n\
<BtnMotion>: notify()\n\
<EnterWindow>: notify()\n\
<LeaveWindow>: notify()\n\
<VisibilityNotify>: notify()\n\
<MotionNotify>: notify()\n\
<FocusIn>: notify()\n\
<Key>Up: notify()\n\
<Key>Down: notify()\n\
<KeyDown>: notify()";


static XtActionsRec actionsList[] = { {"notify", (XtActionProc) Notify}};

ColPalClassRec colpalClassRec = {
  { /* Core class part */
    (WidgetClass) &compositeClassRec,	/* superclass */
    "ColPal",				/* class name */
    sizeof(ColPalRec),			/* widget size */
    NULL,				/* class initialize */
    NULL, 				/* class part initialize */
    FALSE, 				/* class inited */
    (XtInitProc) Initialize,		/* initialize */
    NULL, 				/* initialize hook */
    Realize,				/* realize */
    actionsList, 			/* actions */
    XtNumber( actionsList),		/* num actions */
    NULL, 				/* resourses */
    0,					/* num resourses */
    NULLQUARK,				/* xrm class */
    TRUE,				/* compress motion */
    TRUE,				/* compress expsure */
    TRUE,				/* compress enterleave */
    FALSE,				/* visible interest */
    Destroy,				/* destroy */
    XtInheritResize,			/* resize */
    Redisplay,				/* expose */
    (XtSetValuesFunc)SetValues,		/* set values */
    NULL,				/* set values hook */
    XtInheritSetValuesAlmost,		/* set values almost */
    NULL,				/* get values hook */
    NULL, 				/* accept focus */
    XtVersionDontCheck,			/* version */
    NULL, 				/* callback offsets */
    defaultTranslations,		/* tm_table */
    NULL,				/* geometry */
    NULL,				/* disp accelerators */
    NULL				/* extension */
  },
  { /* composite class record */
    (XtGeometryHandler) GeometryManager, /* geometry manager */
    NULL,				/* change managed */
    XtInheritInsertChild,		/* insert child */
    XtInheritDeleteChild,		/* delete child */
    NULL				/* extension */
  },
  { /* colpal class record */
    NULL,
    0
  }
};

WidgetClass colpalWidgetClass = (WidgetClass) &colpalClassRec;

static void scroll_h_action( 	Widget 		w,
				XtPointer 	client_data,
				XtPointer 	call_data);
static void scroll_v_action( 	Widget 		w,
				XtPointer 	client_data,
				XtPointer 	call_data);
static void scroll_callback( glow_sScroll *data);

static int colpal_init_proc( Widget w, GlowCtx *fctx, void *client_data)
{
  colpalwidget_sScroll *scroll_data;
  ColPalCtx	*ctx;

  scroll_data = (colpalwidget_sScroll *) malloc( sizeof( colpalwidget_sScroll));
  scroll_data->colpal = w;
  scroll_data->scroll_h = ((ColPalWidget) w)->colpal.scroll_h;
  scroll_data->scroll_v = ((ColPalWidget) w)->colpal.scroll_v;
  scroll_data->form = ((ColPalWidget) w)->colpal.form;
  scroll_data->scroll_h_managed = 1;
  scroll_data->scroll_v_managed = 1;

  ctx = (ColPalCtx *) ((ColPalWidget) w)->colpal.colpal_ctx;
  ctx->register_scroll_callback( (void *) scroll_data, scroll_callback);

  XtAddCallback( scroll_data->scroll_h, XmNvalueChangedCallback, scroll_h_action, w);
  XtAddCallback( scroll_data->scroll_h, XmNdragCallback, scroll_h_action, w);
  XtAddCallback( scroll_data->scroll_h, XmNincrementCallback, scroll_h_action, w);
  XtAddCallback( scroll_data->scroll_h, XmNdecrementCallback, scroll_h_action, w);
  XtAddCallback( scroll_data->scroll_h, XmNpageIncrementCallback, scroll_h_action, w);
  XtAddCallback( scroll_data->scroll_h, XmNpageDecrementCallback, scroll_h_action, w);
  XtAddCallback( scroll_data->scroll_h, XmNtoTopCallback, scroll_h_action, w);
  XtAddCallback( scroll_data->scroll_h, XmNtoBottomCallback, scroll_h_action, w);

  XtAddCallback( scroll_data->scroll_v, XmNvalueChangedCallback, scroll_v_action, w);
  XtAddCallback( scroll_data->scroll_v, XmNdragCallback, scroll_v_action, w);
  XtAddCallback( scroll_data->scroll_v, XmNincrementCallback, scroll_v_action, w);
  XtAddCallback( scroll_data->scroll_v, XmNdecrementCallback, scroll_v_action, w);
  XtAddCallback( scroll_data->scroll_v, XmNpageIncrementCallback, scroll_v_action, w);
  XtAddCallback( scroll_data->scroll_v, XmNpageDecrementCallback, scroll_v_action, w);
  XtAddCallback( scroll_data->scroll_v, XmNtoTopCallback, scroll_v_action, w);
  XtAddCallback( scroll_data->scroll_v, XmNtoBottomCallback, scroll_v_action, w);

  ctx->configure();

  if ( ((ColPalWidget) w)->colpal.init_proc)
    return (((ColPalWidget) w)->colpal.init_proc)( ctx, client_data);
  else
    return 1;
}

static void scroll_h_action( 	Widget 		w,
				XtPointer 	client_data,
				XtPointer 	call_data)
{
  XmScrollBarCallbackStruct *cbs = (XmScrollBarCallbackStruct *) call_data;
  ColPalCtx *ctx = (ColPalCtx *) ((ColPalWidget) client_data)->colpal.colpal_ctx;

  switch( cbs->reason)
  {
    case XmCR_DRAG:
    case XmCR_VALUE_CHANGED:
    case XmCR_INCREMENT:
    case XmCR_DECREMENT:
    case XmCR_PAGE_INCREMENT:
    case XmCR_PAGE_DECREMENT:
    case XmCR_TO_TOP:
    case XmCR_TO_BOTTOM:
      colpal_scroll_horizontal( ctx, cbs->value, 0);
      break;
  }
}

static void scroll_v_action( 	Widget 		w,
				XtPointer 	client_data,
				XtPointer 	call_data)
{

  XmScrollBarCallbackStruct *cbs = (XmScrollBarCallbackStruct *) call_data;
  ColPalCtx *ctx = (ColPalCtx *) ((ColPalWidget) client_data)->colpal.colpal_ctx;
  Arg 		arg[20];
  int		i;
  int		maximum, slider, value, bottom;


  // Calculate if position is bottom
  i = 0;
  XtSetArg( arg[i], XmNmaximum, &maximum);i++;
  XtSetArg( arg[i], XmNsliderSize, &slider);i++;
  XtSetArg( arg[i], XmNvalue, &value);i++;
  XtGetValues( w, arg, i);
  if ( slider + value == maximum)
    bottom = 1;
  else
    bottom = 0;

  switch( cbs->reason)
  {
    case XmCR_DRAG:
    case XmCR_VALUE_CHANGED:
    case XmCR_INCREMENT:
    case XmCR_DECREMENT:
    case XmCR_PAGE_INCREMENT:
    case XmCR_PAGE_DECREMENT:
    case XmCR_TO_TOP:
    case XmCR_TO_BOTTOM:
      colpal_scroll_vertical( ctx, cbs->value, bottom);
      break;
  }
}

static void scroll_callback( glow_sScroll *data)
{
  colpalwidget_sScroll *scroll_data;
  Arg 		arg[20];
  int		i;

  scroll_data = (colpalwidget_sScroll *) data->scroll_data;

  if ( data->total_width <= data->window_width)
  {
    if ( data->offset_x == 0)
      data->total_width = data->window_width;
    if ( scroll_data->scroll_h_managed)
    {
      // Remove horizontal scrollbar
    }
  }
  else
  {
    if ( !scroll_data->scroll_h_managed)
    {
      // Insert horizontal scrollbar
    }
  }

  if ( data->total_height <= data->window_height)
  {
    if ( data->offset_y == 0)
      data->total_height = data->window_height;
    if ( scroll_data->scroll_v_managed)
    {
      // Remove vertical scrollbar
    }
  }
  else
  {
    if ( !scroll_data->scroll_v_managed)
    {
      // Insert vertical scrollbar
    }
  }
  if ( data->offset_x < 0)
    data->offset_x = 0;
  if ( data->offset_y < 0)
    data->offset_y = 0;
  if ( data->total_height < data->window_height + data->offset_y)
    data->total_height = data->window_height + data->offset_y;
  if ( data->total_width < data->window_width + data->offset_x)
    data->total_width = data->window_width + data->offset_x;
  if ( data->window_width < 1)
    data->window_width = 1;
  if ( data->window_height < 1)
    data->window_height = 1;

  if ( scroll_data->scroll_h_managed)
  {
    i = 0;
    XtSetArg( arg[i], XmNmaximum, data->total_width);i++;
    XtSetArg( arg[i], XmNsliderSize, data->window_width);i++;
    XtSetArg( arg[i], XmNvalue, data->offset_x);i++;
    XtSetValues( scroll_data->scroll_h, arg, i);
  }

  if ( scroll_data->scroll_v_managed)
  {
    i = 0;
    XtSetArg( arg[i], XmNmaximum, data->total_height);i++;
    XtSetArg( arg[i], XmNsliderSize, data->window_height);i++;
    XtSetArg( arg[i], XmNvalue, data->offset_y);i++;
    XtSetValues( scroll_data->scroll_v, arg, i);
  }
}

static XtGeometryResult GeometryManager( 
		Widget w, 
		XtWidgetGeometry *request,
		XtWidgetGeometry *reply)
{
  if ( request->request_mode & CWX)
    w->core.x = request->x;
  if ( request->request_mode & CWY)
    w->core.y = request->y;
  if ( request->request_mode & CWWidth)
    w->core.width = request->width;
  if ( request->request_mode & CWHeight)
    w->core.height = request->height;
  if ( request->request_mode & CWBorderWidth)
    w->core.border_width = request->border_width;
  return XtGeometryYes;
}

static void Initialize( Widget rec, Widget new_widget, ArgList arg, int *args)
{
/*
  ColPalWidget w;

  XtManageChild( new_widget);
  w = (ColPalWidget) new_widget;
*/
}  

static void Redisplay( Widget w, XEvent *event, Region region)
{

  draw_event_handler( (ColPalCtx *)((ColPalWidget)w)->colpal.colpal_ctx, *event);
}

static void Notify( Widget w, XEvent *event)
{
  
  draw_event_handler( (ColPalCtx *)((ColPalWidget)w)->colpal.colpal_ctx, *event);
}

static Boolean SetValues( Widget old, Widget request, Widget new_widget)
{
  return 0;
}

static void Destroy( Widget w)
{
  if ( ((ColPalWidget) w)->colpal.is_navigator)
    return;
  glow_draw_delete( (GlowCtx *)((ColPalWidget)w)->colpal.colpal_ctx);
}

static void Realize( Widget w, unsigned long *mask, XSetWindowAttributes *swa)
{

  (* colpalWidgetClass->core_class.superclass->core_class.realize)
	(w, mask, swa);

  if ( ((ColPalWidget) w)->colpal.is_navigator)
  {
    if ( !((ColPalWidget) w)->colpal.colpal_ctx)
    {
      ColPalWidget main_colpal = (ColPalWidget) ((ColPalWidget) w)->colpal.main_colpal_widget;

      ((ColPalWidget) w)->colpal.colpal_ctx = main_colpal->colpal.colpal_ctx;
      glow_draw_init_nav( w, main_colpal->colpal.colpal_ctx);
    }
  }
  else
  {
    if ( !((ColPalWidget) w)->colpal.colpal_ctx)
    {
      glow_draw_init( w, &((ColPalWidget) w)->colpal.colpal_ctx, 
		colpal_init_proc, 
		((ColPalWidget) w)->colpal.client_data,
		glow_eCtxType_ColPal);
    }
  }
}

extern "C" Widget ColPalCreate( 
	Widget parent, 
	char *name, 
	ArgList args, 
	int argCount,
        int (*init_proc)(GlowCtx *ctx, void *client_data),
	void *client_data
	)
{
  ColPalWidget w;

  w = (ColPalWidget) XtCreateWidget( name, colpalWidgetClass, parent, args, 
		argCount);
  w->colpal.init_proc = init_proc;
  w->colpal.colpal_ctx = 0;
  w->colpal.is_navigator = 0;
  w->colpal.client_data = client_data;
  return (Widget) w;
}

extern "C" Widget ScrolledColPalCreate( 
	Widget parent, 
	char *name, 
	ArgList args, 
	int argCount,
        int	(*init_proc)(GlowCtx *ctx, void *client_data),
	void *client_data, 
	Widget *colpal_w
)
{
  Widget 	form, scroll_h, scroll_v;
  ColPalWidget 	colpal;
  Arg 		arg[20];
  int		i;
  int		scroll_width = 15;

  form = XtCreateWidget( name, xmFormWidgetClass, parent, args,
		argCount);

  i = 0;
  XtSetArg( arg[i], XmNorientation, XmHORIZONTAL); i++;
  XtSetArg( arg[i], XmNrightAttachment, XmATTACH_FORM);i++;
  XtSetArg( arg[i], XmNleftAttachment, XmATTACH_FORM);i++;
  XtSetArg( arg[i], XmNbottomAttachment, XmATTACH_FORM);i++;
  XtSetArg( arg[i], XmNheight, scroll_width);i++;
  XtSetArg( arg[i], XmNrightOffset, scroll_width);i++;
  scroll_h = XtCreateWidget( "scroll_horizontal", xmScrollBarWidgetClass,
		form, arg, i);
  XtManageChild( scroll_h);

  i = 0;
  XtSetArg( arg[i], XmNorientation, XmVERTICAL); i++;
  XtSetArg( arg[i], XmNtopAttachment, XmATTACH_FORM);i++;
  XtSetArg( arg[i], XmNrightAttachment, XmATTACH_FORM);i++;
  XtSetArg( arg[i], XmNbottomAttachment, XmATTACH_FORM);i++;
  XtSetArg( arg[i], XmNwidth, scroll_width);i++;
  XtSetArg( arg[i], XmNbottomOffset, scroll_width);i++;
  scroll_v = XtCreateWidget( "scroll_vertical", xmScrollBarWidgetClass,
		form, arg, i);
  XtManageChild( scroll_v);

  i = 0;
  XtSetArg( arg[i], XmNtopAttachment, XmATTACH_FORM);i++;
  XtSetArg( arg[i], XmNleftAttachment, XmATTACH_FORM);i++;
  XtSetArg( arg[i], XmNbottomAttachment, XmATTACH_WIDGET);i++;
  XtSetArg( arg[i], XmNbottomWidget, scroll_h);i++;
  XtSetArg( arg[i], XmNrightAttachment, XmATTACH_WIDGET);i++;
  XtSetArg( arg[i], XmNrightWidget, scroll_v);i++;
/*
  XtSetArg( arg[i], XmNbottomAttachment, XmATTACH_FORM);i++;
  XtSetArg( arg[i], XmNrightAttachment, XmATTACH_FORM);i++;
*/
  colpal = (ColPalWidget) ColPalCreate( form, "colpal", arg, i, init_proc, client_data);
  XtManageChild( (Widget) colpal);

  colpal->colpal.scroll_h = scroll_h;
  colpal->colpal.scroll_v = scroll_v;
  colpal->colpal.form = form;

  *colpal_w = (Widget) colpal;
  return (Widget) form;
}

Widget ColPalCreateNav( Widget parent, char *name, ArgList args, int argCount,
	Widget main_colpal)
{
  ColPalWidget w;

  w = (ColPalWidget) XtCreateWidget( name, colpalWidgetClass, parent, args, argCount);
  w->colpal.is_navigator = 1;
  w->colpal.colpal_ctx = 0;
  w->colpal.main_colpal_widget = main_colpal;
  return (Widget) w;
}

void ColPalCtxFromWidget( Widget w, void **ctx)
{
  *ctx = ((ColPalWidget) w)->colpal.colpal_ctx;
}

