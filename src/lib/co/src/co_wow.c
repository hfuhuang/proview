/* co_wow -- useful windows

   PROVIEW/R
   Copyright (C) 1996 by Comator Process AB.

   <Description>.  */

#if !defined OS_ELN

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include <Xm/Xm.h>
#include <Xm/Text.h>
#include <Mrm/MrmPublic.h>
#include <Xm/List.h>
#include <Xm/PushB.h>
#include <X11/Intrinsic.h>
#include <Xm/MessageB.h>
#include <Xm/MainW.h>
#include <Xm/FileSB.h>
#include <Xm/Form.h>

#include "pwr.h"
#include "co_dcli.h"
#include "co_wow.h"

#define WOW_MAXNAMES 400

typedef struct
{
  void			*ctx;
  void			*data;
  void	         	(* questionbox_ok) ();
  void	         	(* questionbox_cancel) ();
} wow_t_question_cb;

static void wow_question_ok_cb (
  Widget dialog, 
  XtPointer data, 
  XmAnyCallbackStruct *cbs
);

static void wow_question_cancel_cb (
  Widget dialog, 
  XtPointer data, 
  XmAnyCallbackStruct *cbs
);

static void wow_error_ok_cb (Widget w);

static void wow_error_ok_cb (Widget w)
{
    XtDestroyWidget( w);
}

static void wow_question_ok_cb (
  Widget dialog, 
  XtPointer data, 
  XmAnyCallbackStruct *cbs
)
{
  wow_t_question_cb *cbdata = (wow_t_question_cb *) data;
  
  if (cbdata->questionbox_ok)
    (cbdata->questionbox_ok)( cbdata->ctx, cbdata->data);

  XtFree ((char *)cbdata);
  XtDestroyWidget (dialog);
}

static void wow_question_cancel_cb (
  Widget dialog, 
  XtPointer data, 
  XmAnyCallbackStruct *cbs
)
{
  wow_t_question_cb *cbdata = (wow_t_question_cb *) data;
  
  if (cbdata->questionbox_cancel)
    (cbdata->questionbox_cancel)( cbdata->ctx, cbdata->data);

  XtFree ((char *)cbdata);
  XtDestroyWidget (dialog);
}


/************************************************************************
*
* Name: wow_DisplayQuestion		
*
* Description:	Displays an question box widget 
*
*************************************************************************/

void wow_DisplayQuestion (
void	    *ctx,
Widget	    father,
char	    *title,
char	    *text,
void	    (* questionbox_ok) (),
void	    (* questionbox_cancel) (),
void	    *data
)
{
    Arg	    arg[10];
    Widget  question_widget, w;
    XmString CStr2, TitleStr, okstr, cancelstr;
    wow_t_question_cb *cbdata;
    XmFontList fontlist;
    XFontStruct *font;
    XmFontListEntry fontentry;

    // Set default fontlist
    font = XLoadQueryFont( XtDisplay(father),
	      "-*-Helvetica-Bold-R-Normal--12-*-*-*-P-*-ISO8859-1");
    fontentry = XmFontListEntryCreate( "tag1", XmFONT_IS_FONT, font);
    fontlist = XmFontListAppendEntry( NULL, fontentry);
    XtFree( (char *)fontentry);

    CStr2 = XmStringCreateLtoR( text, XmSTRING_DEFAULT_CHARSET);
    TitleStr = XmStringCreateLtoR( title, XmSTRING_DEFAULT_CHARSET);    
    okstr = XmStringCreateLtoR( " Yes ", XmSTRING_DEFAULT_CHARSET );    
    cancelstr = XmStringCreateLtoR( " No  ", XmSTRING_DEFAULT_CHARSET );    
    XtSetArg(arg[0],XmNheight,75);
    XtSetArg(arg[1],XmNwidth,200);
    XtSetArg(arg[2],XmNmessageString, CStr2);
    XtSetArg(arg[3],XmNx,400);
    XtSetArg(arg[4],XmNy,300);
    XtSetArg(arg[5],XmNdialogTitle,TitleStr);
    XtSetArg(arg[6], XmNokLabelString, okstr);
    XtSetArg(arg[7], XmNcancelLabelString, cancelstr);
    XtSetArg(arg[8], XmNbuttonFontList, fontlist);
    XtSetArg(arg[9], XmNlabelFontList, fontlist);

    cbdata = (wow_t_question_cb *) XtCalloc( 1, sizeof(*cbdata));
    cbdata->questionbox_ok = questionbox_ok;
    cbdata->questionbox_cancel = questionbox_cancel;
    cbdata->ctx = ctx;
    cbdata->data = data;

    question_widget = XmCreateQuestionDialog( father,"questionDialog",arg,10);
    XtAddCallback( question_widget, XmNokCallback,
		(XtCallbackProc) wow_question_ok_cb, cbdata);
    XtAddCallback( question_widget, XmNcancelCallback, 
		(XtCallbackProc) wow_question_cancel_cb, cbdata);

    XmStringFree( CStr2);
    XmStringFree( TitleStr);
    XmStringFree( okstr);
    XmStringFree( cancelstr);
    XmFontListFree( fontlist);
    
    XtManageChild( question_widget);	       
    
    w = XmMessageBoxGetChild( question_widget, XmDIALOG_HELP_BUTTON);
    XtUnmanageChild(w);    

}


/************************************************************************
*
* Name: DisplayError(Father,Text)		
*
* Type:	void	
*
* Widget	Father		I	The widget that is the father of
*					the message box
* char		*Text		I	Message text
*
* Description:	Displays an error message box widget 
*
*************************************************************************/

void wow_DisplayError (
Widget	    father,
char	    *title,
char	    *text
)
{
    Arg	    arg[10];
    Widget  err_widget, w;
    XmString cstr, ctitle;
    XmFontList fontlist;
    XFontStruct *font;
    XmFontListEntry fontentry;

    // Set default fontlist
    font = XLoadQueryFont( XtDisplay(father),
	      "-*-Helvetica-Bold-R-Normal--12-*-*-*-P-*-ISO8859-1");
    fontentry = XmFontListEntryCreate( "tag1", XmFONT_IS_FONT, font);
    fontlist = XmFontListAppendEntry( NULL, fontentry);
    XtFree( (char *)fontentry);

    cstr = XmStringCreateLtoR( text, XmSTRING_DEFAULT_CHARSET);
    ctitle = XmStringCreateLtoR( title, XmSTRING_DEFAULT_CHARSET);    
    XtSetArg(arg[0],XmNheight,75);
    XtSetArg(arg[1],XmNwidth,200);
    XtSetArg(arg[2],XmNmessageString, cstr);
    XtSetArg(arg[3],XmNx,400);
    XtSetArg(arg[4],XmNy,300);
    XtSetArg(arg[5],XmNdialogTitle, ctitle);
    XtSetArg(arg[6], XmNbuttonFontList, fontlist);
    XtSetArg(arg[7], XmNlabelFontList, fontlist);

    err_widget = XmCreateErrorDialog( father,"err_widget",arg,8);
    XtAddCallback(err_widget, XmNokCallback, 
		(XtCallbackProc) wow_error_ok_cb, NULL);

    XmStringFree( cstr);
    XmStringFree( ctitle);
    XmFontListFree( fontlist);
      
    XtManageChild(err_widget);	       
    
    w = XmMessageBoxGetChild(err_widget, XmDIALOG_CANCEL_BUTTON);
    XtUnmanageChild( w);    
    
    w = XmMessageBoxGetChild(err_widget, XmDIALOG_HELP_BUTTON);
    XtUnmanageChild( w);    

} /* END DisplayErrorBox */

/************************************************************************
*
* Name: CreatePushButton(Father,Text,Callback)
*
* Type:	void	
*
* Widget	Father		I	The widget that is the father of
*					the message box
* char		*Text		I	Message text
*
* Description: Create a pushbutton 
*
*************************************************************************/

void wow_CreatePushButton (
Widget	    father,
char	    *text,
char	    *widget_name,
void	    (callback)(),
void	    *ctx
)
{
    Arg	    arg[10];
    Widget  widget;
    XmString cstr;


    cstr = XmStringCreateLtoR( text, XmSTRING_DEFAULT_CHARSET);
    XtSetArg(arg[0],XmNlabelString, cstr);

    widget = XmCreatePushButton( father,widget_name,arg,1);
    XtAddCallback( widget, XmNactivateCallback,
		(XtCallbackProc) callback, ctx);

    XmStringFree( cstr);
    
    XtManageChild(widget);	       

} /* END CreatePushButton */


/************************************************************************
*
* Description: Create a window with a scrolled list and Ok and Cancel
*              buttons.
*
*************************************************************************/

static void wow_list_ok_cb (
  Widget w, 
  XtPointer data, 
  XmAnyCallbackStruct *cbs
)
{
  wow_tListCtx ctx = (wow_tListCtx) data;
  int 		*pos_list, pos_cnt;
  static char   selected_text[80];
  
  if ( ctx->action_cb) {
    if (XmListGetSelectedPos( ctx->list, &pos_list, &pos_cnt)) {
      strcpy( selected_text, ctx->texts + (pos_list[0] - 1) * 80);
      (ctx->action_cb)( ctx->parent_ctx, selected_text);

      XtFree( (char *)pos_list);
    }
  }

  XtDestroyWidget( ctx->toplevel);
  free( ctx->texts);
  free( ctx);
}

static void wow_list_cancel_cb (
  Widget w, 
  XtPointer data, 
  XmAnyCallbackStruct *cbs
)
{
  wow_tListCtx ctx = (wow_tListCtx) data;
  
  XtDestroyWidget( ctx->toplevel);
  free( ctx->texts);
  free( ctx);
}

static void wow_list_action_cb (
  Widget w,
  XtPointer data,
  XmListCallbackStruct *cbs)
{
  wow_tListCtx ctx = (wow_tListCtx) data;
  char          *item_str;
  static char   action_text[80];
  
  if ( cbs->event->type == KeyPress)
    // The ok callback will be called later
    return;

  if ( ctx->action_cb) {
    XmStringGetLtoR( cbs->item, XmSTRING_DEFAULT_CHARSET, &item_str);
    strcpy( action_text, item_str);

    XtFree( item_str);
    (ctx->action_cb)( ctx->parent_ctx, action_text);
  }

  XtDestroyWidget( ctx->toplevel);
  free( ctx->texts);
  free( ctx);
}

wow_tListCtx wow_CreateList (
  Widget    parent,
  char	    *title,
  char      *texts,
  void	    (action_cb)( void *, char *),
  void	    *parent_ctx
)
{
  Arg	    args[15];
  XmString cstr;
  Widget mainwindow;
  Widget ok_button;
  Widget cancel_button;
  Widget form;
  char *name_p;
  int i;
  wow_tListCtx ctx;
  XmFontList fontlist;
  XFontStruct *font;
  XmFontListEntry fontentry;

  ctx = (wow_tListCtx) calloc( 1, sizeof(*ctx));
  ctx->action_cb = action_cb;
  ctx->parent_ctx = parent_ctx;
  
  i=0;
  XtSetArg( args[i], XmNiconName, title); i++;

  ctx->toplevel = XtCreatePopupShell (
        title, topLevelShellWidgetClass, parent, args, i);

  // Set default fontlist
  font = XLoadQueryFont( XtDisplay(ctx->toplevel),
	      "-*-Helvetica-Bold-R-Normal--12-*-*-*-P-*-ISO8859-1");
  fontentry = XmFontListEntryCreate( "tag1", XmFONT_IS_FONT, font);
  fontlist = XmFontListAppendEntry( NULL, fontentry);
  XtFree( (char *)fontentry);

  i=0;
  XtSetArg( args[i], XmNbuttonFontList, fontlist); i++;
  XtSetArg( args[i], XtNallowShellResize, TRUE); i++;
  XtSetValues( ctx->toplevel, args, i);

  mainwindow = XmCreateMainWindow( ctx->toplevel, "mainWindow", NULL, 0);
  XtManageChild( mainwindow);

  i=0;
  XtSetArg(args[i],XmNwidth, 200);i++;
  XtSetArg(args[i],XmNheight, 400);i++;
  XtSetArg(args[i],XmNresizePolicy,XmRESIZE_NONE); i++;

  form = XmCreateForm( mainwindow, "form", args, i);
  XtManageChild( form);

  cstr = XmStringCreateLtoR( "Ok", XmSTRING_DEFAULT_CHARSET);

  i=0;
  XtSetArg( args[i], XmNbottomAttachment, XmATTACH_FORM); i++;
  XtSetArg( args[i], XmNbottomOffset, 20); i++;
  XtSetArg( args[i], XmNleftAttachment, XmATTACH_FORM); i++;
  XtSetArg( args[i], XmNleftOffset, 20); i++;
  XtSetArg( args[i], XmNwidth, 50); i++;
  XtSetArg( args[i], XmNlabelString, cstr); i++;

  ok_button = XmCreatePushButton( form, "okButton", args, i);
  XtAddCallback( ok_button, XmNactivateCallback,
		(XtCallbackProc) wow_list_ok_cb, ctx);
  XtManageChild( ok_button);

  XmStringFree( cstr);

  cstr = XmStringCreateLtoR( "Cancel", XmSTRING_DEFAULT_CHARSET);

  i=0;
  XtSetArg( args[i], XmNbottomAttachment, XmATTACH_FORM); i++;
  XtSetArg( args[i], XmNbottomOffset, 20); i++;
  XtSetArg( args[i], XmNrightAttachment, XmATTACH_FORM); i++;
  XtSetArg( args[i], XmNrightOffset, 20); i++;
  XtSetArg( args[i], XmNwidth, 50); i++;
  XtSetArg( args[i], XmNlabelString, cstr); i++;

  cancel_button = XmCreatePushButton( form, "okButton", args, i);
  XtAddCallback( cancel_button, XmNactivateCallback,
		(XtCallbackProc) wow_list_cancel_cb, ctx);
  XtManageChild( cancel_button);

  XmStringFree( cstr);

  i = 0;
  XtSetArg( args[i], XmNdefaultButton, ok_button); i++;
  XtSetArg( args[i], XmNcancelButton, cancel_button); i++;
  XtSetValues( form, args, i);

  i=0;
  XtSetArg( args[i], XmNbottomAttachment, XmATTACH_WIDGET); i++;
  XtSetArg( args[i], XmNbottomWidget, ok_button); i++;
  XtSetArg( args[i], XmNbottomOffset, 15); i++;
  XtSetArg( args[i], XmNrightAttachment, XmATTACH_FORM); i++;
  XtSetArg( args[i], XmNrightOffset, 15); i++;
  XtSetArg( args[i], XmNtopAttachment, XmATTACH_FORM); i++;
  XtSetArg( args[i], XmNtopOffset, 15); i++;
  XtSetArg( args[i], XmNleftAttachment, XmATTACH_FORM); i++;
  XtSetArg( args[i], XmNleftOffset, 15); i++;
  XtSetArg( args[i], XmNselectionPolicy, XmSINGLE_SELECT); i++;
  XtSetArg( args[i], XmNfontList, fontlist); i++;
  ctx->list = XmCreateScrolledList( form, "scrolledList", args, i);
  XtAddCallback( ctx->list, XmNdefaultActionCallback,
		(XtCallbackProc) wow_list_action_cb, ctx);

  XmFontListFree( fontlist);
  XtManageChild( ctx->list);

  name_p = texts;
  i = 0;
  while ( strcmp( name_p, "") != 0) {
    cstr = XmStringCreateSimple( name_p);
    XmListAddItemUnselected( ctx->list, cstr, 0);
    XmStringFree(cstr);	  
    name_p += 80;
    i++;
  }

  ctx->texts = calloc( i+1, 80);
  memcpy( ctx->texts, texts, (i+1) * 80); 
  XtPopup( ctx->toplevel, XtGrabNone);

  // Set input focus to the scrolled list widget
  XmProcessTraversal( ctx->list, XmTRAVERSE_CURRENT);


  return ctx;
}


void wow_file_ok_cb( Widget widget, XtPointer udata, XtPointer data)
{
  XmFileSelectionBoxCallbackStruct *cbs = (XmFileSelectionBoxCallbackStruct *) data;
  wow_tFileSelCtx ctx = (wow_tFileSelCtx) udata;
  char *filename;

  if ( !XmStringGetLtoR( cbs->value, XmFONTLIST_DEFAULT_TAG, &filename))
    return;

  if ( !filename)
    return;
  
  if ( ctx->file_selected_cb)
    (ctx->file_selected_cb)( ctx->parent_ctx, filename, ctx->file_type);
  free( (char *)ctx);
  XtDestroyWidget( widget);
  XtFree( filename);
}

void wow_file_cancel_cb( Widget widget, XtPointer udata, XtPointer data)
{
  free( (char *)udata);
  XtDestroyWidget( widget);
}

void wow_file_search_cb( Widget widget, XtPointer data)
{
  XmFileSelectionBoxCallbackStruct *cbs = (XmFileSelectionBoxCallbackStruct *) data;
  XmString names[WOW_MAXNAMES];
  char *mask;
  char found_file[200];
  int sts;
  int file_cnt;
  int i;

  if ( !XmStringGetLtoR( cbs->mask, XmFONTLIST_DEFAULT_TAG, &mask))
    return;

  file_cnt = 0;
  sts = dcli_search_file( mask, found_file, DCLI_DIR_SEARCH_INIT);
  while( ODD(sts)) {
    if ( file_cnt >= WOW_MAXNAMES)
      break;
    names[file_cnt++] = XmStringCreateLocalized( found_file);
    sts = dcli_search_file( mask, found_file, DCLI_DIR_SEARCH_NEXT);
  }
  sts = dcli_search_file( mask, found_file, DCLI_DIR_SEARCH_END);
  XtFree( mask);

  if ( file_cnt) {
    XtVaSetValues( widget, 
		   XmNfileListItems, names,
		   XmNfileListItemCount, file_cnt,
		   XmNdirSpec, names[0],
		   XmNlistUpdated, True,
		   NULL);
    for ( i = 0; i < file_cnt; i++)
      XmStringFree( names[i]);
  }
  else
    XtVaSetValues( widget,
		   XmNfileListItems, NULL,
		   XmNfileListItemCount, 0,
		   XmNlistUpdated, True,
		   NULL);
}

void wow_CreateFileSelDia( Widget parent_wid,
			   char *title,
			   void *parent_ctx,
			   void (*file_selected_cb)(void *, char *, wow_eFileSelType),
			   wow_eFileSelType file_type)
{
  Arg args[10];
  XmString ctitle, cdirectory, cpattern;
  char directory[200];
  int i;
  wow_tFileSelCtx ctx;
  Widget help;

  ctx = (wow_tFileSelCtx) calloc( 1, sizeof(*ctx));
  ctx->file_selected_cb = file_selected_cb;
  ctx->parent_ctx = parent_ctx;
  ctx->file_type = file_type;

  ctitle = XmStringCreateLtoR( title, XmSTRING_DEFAULT_CHARSET);    

  i = 0;
  XtSetArg( args[i], XmNdialogTitle, ctitle); i++;
  XtSetArg( args[i], XmNfileTypeMask, XmFILE_REGULAR); i++;
  XtSetArg( args[i], XmNfileSearchProc, wow_file_search_cb); i++;

  switch( file_type) {
    case wow_eFileSelType_All:
      break;
    case wow_eFileSelType_Dbs:
      dcli_translate_filename( directory, "$pwrp_load/");
      cdirectory = XmStringCreateLtoR( directory, XmSTRING_DEFAULT_CHARSET);
      cpattern = XmStringCreateLtoR( "*.dbs", XmSTRING_DEFAULT_CHARSET);
      XtSetArg( args[i], XmNdirectory, cdirectory); i++;
      XtSetArg( args[i], XmNpattern, cpattern); i++;
      break;
    case wow_eFileSelType_Wbl:
      dcli_translate_filename( directory, "$pwrp_db/");
      cdirectory = XmStringCreateLtoR( directory, XmSTRING_DEFAULT_CHARSET);
      cpattern = XmStringCreateLtoR( "*.wb_load", XmSTRING_DEFAULT_CHARSET);
      XtSetArg( args[i], XmNdirectory, cdirectory); i++;
      XtSetArg( args[i], XmNpattern, cpattern); i++;
      break;
  }

  ctx->dialog = XmCreateFileSelectionDialog( parent_wid, "fileseldia", args, i);
  XtAddCallback( ctx->dialog, XmNokCallback, wow_file_ok_cb, ctx);
  XtAddCallback( ctx->dialog, XmNcancelCallback, wow_file_cancel_cb, ctx);

  help = XmFileSelectionBoxGetChild( ctx->dialog, XmDIALOG_HELP_BUTTON);
  XtUnmanageChild( help);
  XtManageChild( ctx->dialog);

  XmStringFree( ctitle);
  XmStringFree( cdirectory);
  XmStringFree( cpattern);
}

#endif









