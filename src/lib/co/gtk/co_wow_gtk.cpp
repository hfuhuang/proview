/** 
 * Proview   $Id: co_wow_gtk.cpp,v 1.15 2008-12-01 16:42:35 claes Exp $
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

/* co_wow_gtk.cpp -- useful windows */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <gdk/gdkkeysyms.h>


#include "pwr.h"
#include "co_wow_gtk.h"
#include "co_dcli.h"
#include "co_cdh.h"
#include "co_api.h"
#include "co_lng.h"

#define WOW_MAXNAMES 400

typedef struct {
  char		        str[200];
  int 			len;
  int			received;
  pwr_tStatus		sts;
  GdkAtom	       	atom;
  int			new_main;
} wow_sSelection;


void CoWowGtk::question_ok_cb( GtkWidget *w, gpointer data)
{
  wow_t_question_cb *cbdata = (wow_t_question_cb *) data;
  
  if (cbdata->questionbox_ok)
    (cbdata->questionbox_ok)( cbdata->ctx, cbdata->data);

  free( (char *)cbdata);
  gtk_widget_destroy( cbdata->question_widget);
}

void CoWowGtk::question_cancel_cb( GtkWidget *w, gpointer data)
{
  wow_t_question_cb *cbdata = (wow_t_question_cb *) data;
  
  if (cbdata->questionbox_cancel)
    (cbdata->questionbox_cancel)( cbdata->ctx, cbdata->data);

  free( (char *)cbdata);
  gtk_widget_destroy( cbdata->question_widget);
}

static gint question_delete_event( GtkWidget *w, GdkEvent *event, gpointer data)
{
  CoWowGtk::question_cancel_cb( w, data);
  return TRUE;
}


/************************************************************************
*
* Name: DisplayQuestion		
*
* Description:	Displays an question box widget 
*
*************************************************************************/

void CoWowGtk::DisplayQuestion( void *ctx, const char *title, const char *text,
				void (* questionbox_ok) ( void *, void *),
				void (* questionbox_cancel) ( void *, void *),
				void *data)
{
  GtkWidget *question_widget;
  GtkWidget *question_label;
  wow_t_question_cb *cbdata;
  
  cbdata = (wow_t_question_cb *) calloc( 1, sizeof(*cbdata));
  cbdata->questionbox_ok = questionbox_ok;
  cbdata->questionbox_cancel = questionbox_cancel;
  cbdata->questionbox_help = 0;
  cbdata->ctx = ctx;
  cbdata->data = data;


  // Create a question window
  question_widget = (GtkWidget *) g_object_new( GTK_TYPE_WINDOW, 
						"default-height", 150,
						"default-width", 400,
						"title", title,
						"window-position", GTK_WIN_POS_CENTER,
						NULL);
  cbdata->question_widget = question_widget;
  g_signal_connect( question_widget, "delete_event", G_CALLBACK(question_delete_event), cbdata);


  char *textutf8 = g_convert( text, -1, "UTF-8", "ISO8859-1", NULL, NULL, NULL);
  question_label = gtk_label_new(textutf8);
  g_free( textutf8);

  GtkWidget *question_image = (GtkWidget *)g_object_new( GTK_TYPE_IMAGE, 
				"stock", GTK_STOCK_DIALOG_QUESTION,
				"icon-size", GTK_ICON_SIZE_DIALOG,
				"xalign", 0.5,
				"yalign", 1.0,
				NULL);

  GtkWidget *question_ok = gtk_button_new_with_label( translate_utf8("Yes"));
  gtk_widget_set_size_request( question_ok, 70, 28);
  g_signal_connect( question_ok, "clicked", 
  		    G_CALLBACK(question_ok_cb), cbdata);

  GtkWidget *question_no = gtk_button_new_with_label( translate_utf8("No"));
  gtk_widget_set_size_request( question_no, 70, 28);
  g_signal_connect( question_no, "clicked", 
  		    G_CALLBACK(question_cancel_cb), cbdata);

  GtkWidget *question_hboxtext = gtk_hbox_new( FALSE, 0);
  gtk_box_pack_start( GTK_BOX(question_hboxtext), question_image, FALSE, FALSE, 15);
  gtk_box_pack_start( GTK_BOX(question_hboxtext), question_label, TRUE, TRUE, 15);

  GtkWidget *question_hboxbuttons = gtk_hbox_new( TRUE, 40);
  gtk_box_pack_start( GTK_BOX(question_hboxbuttons), question_ok, FALSE, FALSE, 0);
  gtk_box_pack_end( GTK_BOX(question_hboxbuttons), question_no, FALSE, FALSE, 0);

  GtkWidget *question_vbox = gtk_vbox_new( FALSE, 0);
  gtk_box_pack_start( GTK_BOX(question_vbox), question_hboxtext, TRUE, TRUE, 30);
  gtk_box_pack_start( GTK_BOX(question_vbox), gtk_hseparator_new(), FALSE, FALSE, 0);
  gtk_box_pack_end( GTK_BOX(question_vbox), question_hboxbuttons, FALSE, FALSE, 15);
  gtk_container_add( GTK_CONTAINER(question_widget), question_vbox);
  gtk_widget_show_all( question_widget);
}

/************************************************************************
*
* Description: Create an Error message dialog.
*
*************************************************************************/

static void displayerror_ok_cb( GtkWidget *w, gint arg1, gpointer data)
{
  gtk_widget_destroy( w);
}

void CoWowGtk::DisplayError( const char *title, const char *text)
{
  GtkWidget *parent = m_parent;
  if ( parent) {
    while( !GTK_IS_WINDOW(parent))
      parent = gtk_widget_get_parent( parent);
  }

  char *textutf8 = g_convert( text, -1, "UTF-8", "ISO8859-1", NULL, NULL, NULL);
  GtkWidget *dialog = gtk_message_dialog_new( GTK_WINDOW(parent),
					      GTK_DIALOG_MODAL, 
					      GTK_MESSAGE_ERROR,
					      GTK_BUTTONS_OK, textutf8);
  g_free( textutf8);
  g_signal_connect( dialog, "response", 
 		    G_CALLBACK(displayerror_ok_cb), NULL);
  gtk_window_set_title( GTK_WINDOW(dialog), title);
  gtk_widget_show_all( dialog);
}

/************************************************************************
*
* Description: Create a text window.
*
*************************************************************************/

static void displaytext_ok_cb( GtkWidget *w, gint arg1, gpointer data)
{
  gtk_widget_destroy( w);
}

void CoWowGtk::DisplayText( const char *title, const char *text)
{
  GtkWidget *parent = m_parent;
  if ( parent) {
    while( !GTK_IS_WINDOW(parent))
      parent = gtk_widget_get_parent( parent);
  }

  char *textutf8 = g_convert( text, -1, "UTF-8", "ISO8859-1", NULL, NULL, NULL);
  GtkWidget *dialog = gtk_message_dialog_new( GTK_WINDOW(parent),
					      GTK_DIALOG_DESTROY_WITH_PARENT, 
					      GTK_MESSAGE_OTHER,
					      GTK_BUTTONS_CLOSE, textutf8);
  g_free( textutf8);
  g_signal_connect( dialog, "response", 
 		    G_CALLBACK(displaytext_ok_cb), NULL);
  gtk_window_set_title( GTK_WINDOW(dialog), title);
  gtk_widget_show_all( dialog);
}

/************************************************************************
*
* Description: Create a window with a scrolled list and Ok and Cancel
*              buttons.
*
*************************************************************************/

class WowListCtx {
 public:
  GtkWidget    *toplevel;
  GtkWidget    *list;
  char      *texts;
  void      (* action_cb) ( void *, char *);
  void      (* cancel_cb) ( void *);
  void      *parent_ctx;
};

void CoWowGtk::list_row_activated_cb( GtkTreeView *tree_view, 
				      GtkTreePath *path,
				      GtkTreeViewColumn *column,
				      gpointer data)
{
  list_ok_cb( 0, data);
}

void CoWowGtk::list_apply_cb (
  GtkWidget *w, 
  gpointer data
)
{
  WowListCtx *ctx = (WowListCtx *) data;
  char		*text;
  static char   selected_text[80];
  GtkTreeIter   iter;
  GtkTreeModel  *store;
  
  g_object_get( ctx->list, "model", &store, NULL);

  GtkTreeSelection *selection = gtk_tree_view_get_selection( 
				 GTK_TREE_VIEW(ctx->list));
  if ( gtk_tree_selection_get_selected( selection, NULL, &iter)) {
    gtk_tree_model_get( GTK_TREE_MODEL( store), &iter, 0, 
			&text, -1);
    strcpy( selected_text, text);
  }
  if ( ctx->action_cb) {
    char *textiso = g_convert( selected_text, -1, "ISO8859-1", "UTF-8", NULL, NULL, NULL);
    strcpy( selected_text, textiso);
    g_free( textiso);

    (ctx->action_cb)( ctx->parent_ctx, selected_text);
  }

}

void CoWowGtk::list_ok_cb (
  GtkWidget *w, 
  gpointer data
)
{
  WowListCtx *ctx = (WowListCtx *) data;

  list_apply_cb( w, data);

  gtk_widget_destroy( ctx->toplevel);
  free( ctx->texts);
  delete ctx;
}

void CoWowGtk::list_cancel_cb (
  GtkWidget *w, 
  gpointer data
)
{
  WowListCtx *ctx = (WowListCtx *) data;
  
  if ( ctx->cancel_cb)
    (ctx->cancel_cb)( ctx->parent_ctx);

  gtk_widget_destroy( ctx->toplevel);
  free( ctx->texts);
  delete ctx;
}

static gboolean list_action_inputfocus( GtkWidget *w, GdkEvent *event, gpointer data)
{
  gtk_window_present( GTK_WINDOW(w));
  return FALSE;
}

static gboolean list_delete_event( GtkWidget *w, GdkEvent *event, gpointer data)
{
  CoWowGtk::list_cancel_cb( w, data);
  return TRUE;
}

void *CoWowGtk::CreateList (
  const char	    *title,
  const char      *texts,
  void	    (action_cb)( void *, char *),
  void	    (cancel_cb)( void *),
  void	    *parent_ctx,
  int	    show_apply_button
)
{
  char *name_p;
  int i;
  WowListCtx *ctx;
  GtkListStore *store;
  GtkTreeIter iter;
  GtkCellRenderer *text_renderer;
  GtkTreeViewColumn *name_column;

  ctx = new WowListCtx();
  ctx->action_cb = action_cb;
  ctx->cancel_cb = cancel_cb;
  ctx->parent_ctx = parent_ctx;
  
 
  ctx->toplevel = (GtkWidget *) g_object_new( GTK_TYPE_WINDOW, 
					      "default-height", 400,
					      "default-width", 200,
					      "title", translate_utf8(title),
					      "window-position", GTK_WIN_POS_CENTER,
					      NULL);

  g_signal_connect( ctx->toplevel, "focus-in-event", G_CALLBACK(list_action_inputfocus), ctx);
  g_signal_connect( ctx->toplevel, "delete_event", G_CALLBACK(list_delete_event), ctx);

  store = gtk_list_store_new( 1, G_TYPE_STRING);
  name_p = (char *)texts;
  i = 0;
  while ( strcmp( name_p, "") != 0) {
    char *nameutf8 = g_convert( name_p, -1, "UTF-8", "ISO8859-1", NULL, NULL, NULL);

    gtk_list_store_append( store, &iter);
    gtk_list_store_set( store, &iter, 0, nameutf8, -1);
    name_p += 80;
    i++;
    g_free( nameutf8);
  }

  ctx->list = (GtkWidget *) g_object_new(GTK_TYPE_TREE_VIEW,
				 "model", store,
				 "rules-hint", TRUE,
				 "headers-clickable", TRUE,
				 "reorderable", TRUE,
				 "enable-search", TRUE,
				 "search-column", 0,
				 NULL);

  text_renderer = gtk_cell_renderer_text_new();
  name_column = gtk_tree_view_column_new_with_attributes( "",
							  text_renderer,
							  "text", 0,
							  NULL);
  g_object_set( name_column,
		"resizable", TRUE,
		"clickable", TRUE,
		NULL);

  gtk_tree_view_append_column( GTK_TREE_VIEW(ctx->list), name_column);
  g_signal_connect( ctx->list, "row-activated", 
 		    G_CALLBACK(CoWowGtk::list_row_activated_cb), ctx);

  GtkWidget *ok_button = gtk_button_new_with_label( translate_utf8("Ok"));
  gtk_widget_set_size_request( ok_button, 70, 28);
  g_signal_connect( ok_button, "clicked", 
 		    G_CALLBACK(CoWowGtk::list_ok_cb), ctx);

  GtkWidget *apply_button;
  if ( show_apply_button) {
    apply_button = gtk_button_new_with_label( translate_utf8("Apply"));
    gtk_widget_set_size_request( apply_button, 70, 28);
    g_signal_connect( apply_button, "clicked", 
		      G_CALLBACK(CoWowGtk::list_apply_cb), ctx);
  }
  GtkWidget *cancel_button = gtk_button_new_with_label( translate_utf8("Cancel"));
  gtk_widget_set_size_request( cancel_button, 70, 28);
  g_signal_connect( cancel_button, "clicked", 
 		    G_CALLBACK(CoWowGtk::list_cancel_cb), ctx);

  GtkWidget *hboxbuttons = gtk_hbox_new( TRUE, 40);
  if ( show_apply_button)
    gtk_box_pack_start( GTK_BOX(hboxbuttons), apply_button, FALSE, FALSE, 20);
  gtk_box_pack_start( GTK_BOX(hboxbuttons), ok_button, FALSE, FALSE, 20);
  gtk_box_pack_end( GTK_BOX(hboxbuttons), cancel_button, FALSE, FALSE, 20);

  GtkWidget *scrolled_window = gtk_scrolled_window_new( NULL, NULL);
  gtk_container_add( GTK_CONTAINER( scrolled_window), ctx->list);

  GtkWidget *vbox = gtk_vbox_new( FALSE, 0);
  gtk_box_pack_start( GTK_BOX(vbox), scrolled_window, TRUE, TRUE, 0);
  gtk_box_pack_end( GTK_BOX(vbox), hboxbuttons, FALSE, FALSE, 10);

  gtk_container_add( GTK_CONTAINER(ctx->toplevel), vbox);
  gtk_widget_show_all( ctx->toplevel);

  // GtkTreeSelection *selection = gtk_tree_view_get_selection( GTK_TREE_VIEW(ctx->list));
  // gtk_tree_selection_set_mode( selection, GTK_SELECTION_SINGLE);

  ctx->texts = (char *) calloc( i+1, 80);
  memcpy( ctx->texts, texts, (i+1) * 80); 

  // Set input focus to the scrolled list widget
  gtk_widget_grab_focus( ctx->list);

  return ctx;
}

void CoWowGtk::warranty_cb( GtkWidget *w, gint response, gpointer data)
{
  wow_t_question_cb *cbdata = (wow_t_question_cb *)data;

  switch( response) {
  case GTK_RESPONSE_ACCEPT:
    ((CoWowGtk *)cbdata->ctx)->DisplayLicense();
    break;
  case GTK_RESPONSE_REJECT:
    exit(0);
  case GTK_RESPONSE_HELP:
    gtk_widget_destroy( GTK_WIDGET(cbdata->data));
    free( cbdata);
    break;
  default: ;
  }
}

static gint warranty_delete_event( GtkWidget *w, GdkEvent *event, gpointer data)
{
  exit(0);
  return FALSE;
}

int CoWowGtk::DisplayWarranty()
{
  char    text[4000];
  char 	title[80];
  FILE 	*fp;
  int 	i;
  pwr_tFileName fname;
  wow_t_question_cb *cbdata;

  // Display only once
  if ( HideWarranty())
    return 1;

  sprintf( fname, "$pwr_exe/%s/acceptlicense.txt", lng_get_language_str());
  dcli_translate_filename( fname, fname);

  fp = fopen( fname, "r");
  if ( !fp) {
    strcpy( fname, "$pwr_exe/en_us/acceptlicense.txt");
    dcli_translate_filename( fname, fname);
    fp = fopen( fname, "r");
    if ( !fp) return 1;
  }

  for ( i = 0; i < (int)sizeof(text) - 1; i++) {
    text[i] = fgetc( fp);
    if ( text[i] == EOF)
      break;
  }
  text[i] = 0;
  fclose( fp);

  gchar *s = g_convert( text, -1, "UTF-8", "ISO8859-1", NULL, NULL, NULL);
  if ( s) {
    strncpy( text, s, sizeof(text));
    g_free(s);
  }

  strcpy( title, translate_utf8("Accept License Terms"));

  char show_license_str[200];
  char i_accept_str[200];
  strcpy( show_license_str, translate_utf8("Show License"));
  strcpy( i_accept_str, translate_utf8("I Accept"));

  GtkDialog *dialog = (GtkDialog *)gtk_dialog_new_with_buttons( title, GTK_WINDOW(m_parent),
			   GTK_DIALOG_MODAL, 
			   translate_utf8("Quit"), GTK_RESPONSE_REJECT, 
			   show_license_str, GTK_RESPONSE_ACCEPT, 
			   i_accept_str, GTK_RESPONSE_HELP,
			   NULL);

  cbdata = (wow_t_question_cb *) calloc( 1, sizeof(*cbdata));
  cbdata->ctx = this;
  cbdata->data = dialog;

  g_signal_connect( dialog, "delete_event", G_CALLBACK(warranty_delete_event), cbdata);
  g_signal_connect( dialog, "response", G_CALLBACK(warranty_cb), cbdata);

  GtkWidget *dialog_label = gtk_label_new( text);
  dcli_translate_filename( fname, "$pwr_exe/proview_icon2.png");
  GtkWidget *dialog_image = gtk_image_new_from_file( fname);
  g_object_set( dialog_image,
		"xalign", 0.5,
		"yalign", 0.0,
		NULL);
  GtkWidget *hbox = gtk_hbox_new( FALSE, 0);
  gtk_box_pack_start( GTK_BOX(hbox), dialog_image, FALSE, FALSE, 15);
  gtk_box_pack_start( GTK_BOX(hbox), dialog_label, TRUE, TRUE, 15);
  gtk_box_pack_start( GTK_BOX(dialog->vbox), hbox, TRUE, TRUE, 30);

  gtk_widget_show_all( GTK_WIDGET(dialog));
  return 1;
}

void CoWowGtk::license_cb( GtkWidget *w, gint response, gpointer data)
{
  gtk_widget_destroy( GTK_WIDGET(data));
}

void CoWowGtk::DisplayLicense()
{
  char text[20000];
  char title[80];
  FILE *fp;
  char fname[200];
  int i;

  strcpy( title, translate_utf8("License"));

  sprintf( fname, "$pwr_exe/%s/license.txt", lng_get_language_str());
  dcli_translate_filename( fname, fname);

  fp = fopen( fname, "r");
  if ( !fp) {
    strcpy( fname, "$pwr_exe/en_us/lincense.txt");
    dcli_translate_filename( fname, fname);
    fp = fopen( fname, "r");
    if ( !fp)
      return;
  }

  for ( i = 0; i < (int)sizeof(text) - 1; i++) {
    text[i] = fgetc( fp);
    if ( text[i] == EOF)
      break;
  }
  fclose( fp);
  text[i] = 0;

  gchar *s = g_convert( text, -1, "UTF-8", "ISO8859-1", NULL, NULL, NULL);
  if ( s) {
    strncpy( text, s, sizeof(text));
    g_free( s);
  }
  
  GtkDialog *dialog = (GtkDialog *)gtk_dialog_new_with_buttons( title, GTK_WINDOW(m_parent),
						   GTK_DIALOG_MODAL, 
						   translate_utf8("Close"), GTK_RESPONSE_REJECT, 
						   NULL);
  g_object_set( dialog, 
		"default-height", 600,
		"default-width", 600, NULL);
  g_signal_connect( dialog, "response", 
 		    G_CALLBACK(license_cb), dialog);

  GtkWidget *dialog_label = gtk_label_new( text);

  GtkWidget *hbox = gtk_hbox_new( FALSE, 0);
  gtk_box_pack_start( GTK_BOX(hbox), dialog_label, TRUE, TRUE, 15);

  GtkWidget *scrolled_window = gtk_scrolled_window_new( NULL, NULL);
  gtk_scrolled_window_add_with_viewport( GTK_SCROLLED_WINDOW( scrolled_window), hbox);

  gtk_box_pack_start( GTK_BOX(dialog->vbox), scrolled_window, TRUE, TRUE, 30);

  gtk_widget_show_all( GTK_WIDGET(dialog));

}

void CoWowGtk::GetAtoms( GdkAtom *graph_atom, GdkAtom *objid_atom, GdkAtom *attrref_atom)
{
  if ( graph_atom)
    *graph_atom = gdk_atom_intern( "PWR_GRAPH", FALSE); 
  if ( objid_atom)
    *objid_atom = gdk_atom_intern( "PWR_OBJID", FALSE); 
  if ( attrref_atom)
    *attrref_atom = gdk_atom_intern( "PWR_ATTRREF", FALSE); 
}

static void wow_get_selection_cb( GtkWidget *w, GtkSelectionData *selection_data,
				  guint time, gpointer clientdata)
{
  wow_sSelection *data = (wow_sSelection *)clientdata;

  if ( selection_data->length > 0 && selection_data->target == data->atom) {
    if ( selection_data->length > int(sizeof(data->str) - 1)) {
      data->sts = 0;
      return;
    }
    strncpy( data->str, (char *)selection_data->data, selection_data->length);
    data->str[selection_data->length] = 0;
    data->len = selection_data->length;
    data->sts = 1;
  }
  else
    data->sts = 0;
  data->received = 1;
  if ( data->new_main)
    gtk_main_quit();
}

int CoWowGtk::GetSelection( GtkWidget *w, char *str, int size, GdkAtom atom)
{
  static wow_sSelection data;

  data.received = 0;
  data.sts = 0;
  data.atom = atom;
  data.new_main = 0;
  
  gulong id = g_signal_connect( w, "selection_received", 
				G_CALLBACK(wow_get_selection_cb), &data);
  gtk_selection_convert( w, GDK_SELECTION_PRIMARY, atom, GDK_CURRENT_TIME);
  

  while ( !data.received) {
    // From other application
    data.new_main = 1;
    gtk_main();
  }

  g_signal_handler_disconnect( w, id);
  if ( data.sts && data.len < size)
    strcpy( str, data.str);
  return data.sts;
}


void CoWowGtk::CreateFileSelDia( const char *title, void *parent_ctx,
				 void (*file_selected_cb)(void *, char *, wow_eFileSelType),
				 wow_eFileSelType file_type) 
{
  GtkWidget *dialog;
  pwr_tFileName fname;
  
  
  dialog = gtk_file_chooser_dialog_new( title, NULL,
					GTK_FILE_CHOOSER_ACTION_OPEN,
					GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL,
					GTK_STOCK_OPEN, GTK_RESPONSE_ACCEPT,
					NULL);

  if ( file_type == wow_eFileSelType_Dbs) {
    pwr_tFileName folder;
    dcli_translate_filename( folder, "$pwrp_load");
    gtk_file_chooser_set_current_folder( GTK_FILE_CHOOSER(dialog), folder);

    GtkFileFilter *filter = gtk_file_filter_new();
    gtk_file_filter_set_name( filter,  "*.dbs");
    gtk_file_filter_add_pattern( filter, "*.dbs");
    gtk_file_chooser_add_filter( GTK_FILE_CHOOSER(dialog), filter);
  }
  else if ( file_type == wow_eFileSelType_Wbl) {
    pwr_tFileName folder;
    dcli_translate_filename( folder, "$pwrp_db");
    gtk_file_chooser_set_current_folder( GTK_FILE_CHOOSER(dialog), folder);

    GtkFileFilter *filter = gtk_file_filter_new();
    gtk_file_filter_set_name( filter,  "*.wb_load,*.wb_dmp");
    gtk_file_filter_add_pattern( filter, "*.wb_load");
    gtk_file_filter_add_pattern( filter, "*.wb_dmp");
    gtk_file_chooser_add_filter( GTK_FILE_CHOOSER(dialog), filter);

    filter = gtk_file_filter_new();
    gtk_file_filter_set_name( filter,  "All Files");
    gtk_file_filter_add_pattern( filter, "*");
    gtk_file_chooser_add_filter( GTK_FILE_CHOOSER(dialog), filter);
  }
  else if ( file_type == wow_eFileSelType_Graph) {    
    pwr_tFileName folder;
    dcli_translate_filename( folder, "$pwrp_pop");
    gtk_file_chooser_set_current_folder( GTK_FILE_CHOOSER(dialog), folder);

    GtkFileFilter *filter = gtk_file_filter_new();
    gtk_file_filter_set_name( filter,  "*.pwg");
    gtk_file_filter_add_pattern( filter, "*.pwg");
    gtk_file_chooser_add_filter( GTK_FILE_CHOOSER(dialog), filter);
  }
  else if ( file_type == wow_eFileSelType_Image) {
    pwr_tFileName folder;
    dcli_translate_filename( folder, "$pwrp_pop");
    gtk_file_chooser_set_current_folder( GTK_FILE_CHOOSER(dialog), folder);

    GtkFileFilter *filter = gtk_file_filter_new();
    gtk_file_filter_set_name( filter,  "*.png,*.jpg,*.gif");
    gtk_file_filter_add_pattern( filter, "*.png");
    gtk_file_filter_add_pattern( filter, "*.jpg");
    gtk_file_filter_add_pattern( filter, "*.gif");
    gtk_file_chooser_add_filter( GTK_FILE_CHOOSER(dialog), filter);

    filter = gtk_file_filter_new();
    gtk_file_filter_set_name( filter,  "All Files");
    gtk_file_filter_add_pattern( filter, "*");
    gtk_file_chooser_add_filter( GTK_FILE_CHOOSER(dialog), filter);
  }

  if ( gtk_dialog_run( GTK_DIALOG(dialog)) == GTK_RESPONSE_ACCEPT) {
    char *filename;

    filename = gtk_file_chooser_get_filename( GTK_FILE_CHOOSER(dialog));
    if ( filename)
      strcpy( fname, filename);
    g_free( filename);

    if ( file_selected_cb)
      (file_selected_cb)( parent_ctx, fname, file_type);
  }
  gtk_widget_destroy( dialog);
}

CoWowTimer *CoWowGtk::timer_new()
{
  return new CoWowTimerGtk();
}

CoWowTimerGtk::~CoWowTimerGtk()
{
  if ( m_timerid)
    g_source_remove( m_timerid);
}

void CoWowTimerGtk::add( int time, void (* callback)(void *data), void *data)
{
  m_callback = callback;
  m_data = data;
  m_timerid = g_timeout_add( time, timer_cb, this); 
}

void CoWowTimerGtk::remove()
{
  if ( m_timerid) {
    g_source_remove( m_timerid);
    m_timerid = 0;
  }
}

gboolean CoWowTimerGtk::timer_cb( void *data)
{
  CoWowTimerGtk *timer = (CoWowTimerGtk *)data;
  timer->m_timerid = 0;
  (timer->m_callback)( timer->m_data);
  return FALSE;
}

//
// Calculate position for a popup menu
//
void CoWowGtk::PopupPosition( GtkWidget *parent, int x_event, int y_event, int *x, int *y)
{
  gint x0, y0, x1, y1;
  GtkWidget *grandparent;
  x1 = y1 = 0;

  x0 = (gint) x_event;
  y0 = (gint) y_event;

  grandparent = gtk_widget_get_parent( parent);
  while( !GTK_IS_WINDOW(grandparent)) {
    if ( !x1  && !y1 && (parent->allocation.x || parent->allocation.y)) {
      // Add first widget with nonzero position
      x1 = parent->allocation.x;
      y1 = parent->allocation.y;
      x0 += x1;
      y0 += y1;
    }
    parent = grandparent;
    grandparent = gtk_widget_get_parent( parent);
  }
  if ( grandparent) {
    // Add window widget position
    gtk_window_get_position( GTK_WINDOW(grandparent), &x1, &y1);
    x0 += x1;
    y0 += y1;
  }

  *x = x0;
  *y = y0;
}

//
//  FocusTimer
//
void CoWowFocusTimerGtk::disable( int time) 
{
  set_focus_disabled++;
  if ( request_cnt > 1)
    request_cnt = 0;
  focus_timerid = g_timeout_add( time, enable_set_focus, this); 
}

int CoWowFocusTimerGtk::disabled() {
  // Return false on first request after disable
  request_cnt++;
  return (request_cnt > 1);
}
    
gboolean CoWowFocusTimerGtk::enable_set_focus( void *ft) 
{
  ((CoWowFocusTimerGtk *)ft)->set_focus_disabled--;
  ((CoWowFocusTimerGtk *)ft)->request_cnt = 0;
  return FALSE;
}

CoWowFocusTimerGtk::~CoWowFocusTimerGtk() 
{
  if ( set_focus_disabled)
    g_source_remove( focus_timerid);
}

CoWowEntryGtk::CoWowEntryGtk( CoWowRecall *re) 
  : m_re(re), m_re_alloc(false)
{
  if ( !m_re) {
    m_re = new CoWowRecall();
    m_re_alloc = true;
  }
  w = gtk_entry_new();
  g_object_set( w, 
		"editable", TRUE, 
		"max-length", m_re->m_line_size - 1,
		NULL);
  g_signal_connect( w, "key-press-event", 
  		    G_CALLBACK(event_cb), this);
}

CoWowEntryGtk::~CoWowEntryGtk()
{
  if ( m_re_alloc)
    delete m_re;
}

gboolean CoWowEntryGtk::event_cb( GtkWidget *w, GdkEvent *event, gpointer data)
{
  CoWowEntryGtk *en = (CoWowEntryGtk *)data;
  int i;
  gboolean sts;

  guint keysym = event->key.keyval;
  gchar *text = gtk_editable_get_chars( GTK_EDITABLE(w), 0, -1);

  switch ( keysym) {
  case GDK_Return:
  case GDK_KP_Enter:
  case GDK_Linefeed: {
    // Insert in recall buffer
    if ( strcmp( text, "") != 0) {
      for ( i = en->m_re->m_recall_size - 2; i >= 0; i--)
	strcpy( en->m_re->m_recall[i+1], en->m_re->m_recall[i]);
      strncpy( en->m_re->m_recall[0], text, en->m_re->m_line_size);
      en->m_re->m_recall[0][en->m_re->m_line_size-1] = 0;
    }
    en->m_re->m_current_recall_line = 0;
    sts = FALSE;
    break;
  }
  case GDK_Up: {
    en->m_re->m_current_recall_line++;
    if ( en->m_re->m_current_recall_line > en->m_re->m_recall_size - 1)
      en->m_re->m_current_recall_line = en->m_re->m_recall_size - 1;

    gint pos = 0;
    gtk_editable_delete_text( GTK_EDITABLE(w), 0, -1);
    gtk_editable_insert_text( GTK_EDITABLE(w), 
			      en->m_re->m_recall[en->m_re->m_current_recall_line - 1], 
			      strlen(en->m_re->m_recall[en->m_re->m_current_recall_line - 1]), 
			      &pos);
    gtk_editable_set_position( GTK_EDITABLE(w), -1);
    sts = TRUE;
    break;
  }
  case GDK_Down: {
    if ( en->m_re->m_current_recall_line == 0)
      gtk_editable_delete_text( GTK_EDITABLE(w), 0, -1);
    else if ( en->m_re->m_current_recall_line == 1) {
      gtk_editable_delete_text( GTK_EDITABLE(w), 0, -1);
      en->m_re->m_current_recall_line--;
    }
    else {
      en->m_re->m_current_recall_line--;
      gint pos = 0;
      gtk_editable_delete_text( GTK_EDITABLE(w), 0, -1);
      gtk_editable_insert_text( GTK_EDITABLE(w), 
				en->m_re->m_recall[en->m_re->m_current_recall_line - 1], 
				strlen(en->m_re->m_recall[en->m_re->m_current_recall_line - 1]), 
				&pos);
      gtk_editable_set_position( GTK_EDITABLE(w), -1);
    }
    sts = TRUE;
    break;
  }
  default:
    sts = FALSE;
  }
  g_free( text);
  return sts;
}

void CoWowGtk::SetWindowIcon( GtkWidget *w)
{
  pwr_tFileName fname;
  static GdkPixbuf *icon = 0;

  if ( !icon) {
    dcli_translate_filename( fname, "$pwr_exe/proview_icon.png");
    icon = gdk_pixbuf_new_from_file( fname, 0);
  }
  gtk_window_set_icon( GTK_WINDOW(w), icon);
}

char *CoWowGtk::translate_utf8( const char *str)
{
  static char result[400];
  gchar *s;

  s = g_convert( Lng::translate( str), -1, "UTF-8", "ISO8859-1", NULL, NULL, NULL);
  if ( s) {
    strncpy( result, s, sizeof(result));
    g_free( s);
  }
  else 
    strcpy( result, "");
  return result;
}

char *CoWowGtk::convert_utf8( const char *str)
{
  static char result[400];
  gchar *s;

  s = g_convert( str, -1, "UTF-8", "ISO8859-1", NULL, NULL, NULL);
  if ( s) {
    strncpy( result, s, sizeof(result));
    g_free( s);
  }
  else 
    strcpy( result, "");
  return result;
}

//
//  Modal Dialog with text and tree buttons
//
static void modaldia_button1_cb( GtkWidget *w, gpointer data)
{
  *(int *)data = wow_eModalDialogReturn_Button1;
  gtk_main_quit();
}
static void modaldia_button2_cb( GtkWidget *w, gpointer data)
{
  *(int *)data = wow_eModalDialogReturn_Button2;
  gtk_main_quit();
}
static void modaldia_button3_cb( GtkWidget *w, gpointer data)
{
  *(int *)data = wow_eModalDialogReturn_Button3;
  gtk_main_quit();
}
static gboolean modaldia_delete_event( GtkWidget *w, GdkEvent *event, gpointer data)
{
  *(int *)data = wow_eModalDialogReturn_Deleted;
  gtk_main_quit();

  return TRUE;
}

int CoWowGtk::CreateModalDialog( const char *title, const char *text, const char *button1, 
				 const char *button2, const char *button3,
				 const char *image)
{
  int status = 0;
  GtkWidget *image_w;
  pwr_tFileName fname;

  // Create a question window
  GtkWidget *dialog_w = (GtkWidget *) g_object_new( GTK_TYPE_WINDOW, 
							 "default-height", 150,
							 "default-width", 400,
							 "title", title,
							 "window-position", GTK_WIN_POS_CENTER,
							 NULL);

  g_signal_connect( dialog_w, "delete_event", G_CALLBACK(modaldia_delete_event), &status);
  GtkWidget *text_w = gtk_label_new(text);


  if ( image) {
    dcli_translate_filename( fname, image);
    image_w = gtk_image_new_from_file( fname);
  }
  if ( !image) {
    image_w = (GtkWidget *)g_object_new( GTK_TYPE_IMAGE, 
				"stock", GTK_STOCK_DIALOG_QUESTION,
				"icon-size", GTK_ICON_SIZE_DIALOG,
				"xalign", 0.5,
				"yalign", 1.0,
				NULL);
  }

  GtkWidget *hboxtext = gtk_hbox_new( FALSE, 0);
  gtk_box_pack_start( GTK_BOX(hboxtext), image_w, FALSE, FALSE, 15);
  gtk_box_pack_start( GTK_BOX(hboxtext), text_w, TRUE, TRUE, 15);

  GtkWidget *hboxbuttons = gtk_hbox_new( TRUE, 40);

  if ( button1) {
    GtkWidget *button1_w = gtk_button_new_with_label( translate_utf8(button1));
    gtk_widget_set_size_request( button1_w, -1, 28);
    g_signal_connect( button1_w, "clicked", 
		      G_CALLBACK(modaldia_button1_cb), &status);
    gtk_box_pack_start( GTK_BOX(hboxbuttons), button1_w, FALSE, FALSE, 0);
  }
  if ( button2) {
    GtkWidget *button2_w = gtk_button_new_with_label( translate_utf8(button2));
    gtk_widget_set_size_request( button2_w, -1, 28);
    g_signal_connect( button2_w, "clicked", 
		      G_CALLBACK(modaldia_button2_cb), &status);
    gtk_box_pack_start( GTK_BOX(hboxbuttons), button2_w, FALSE, FALSE, 0);
  }
  if ( button3) {
    GtkWidget *button3_w = gtk_button_new_with_label( translate_utf8(button3));
    gtk_widget_set_size_request( button3_w, -1, 28);
    g_signal_connect( button3_w, "clicked", 
		      G_CALLBACK(modaldia_button3_cb), &status);
    gtk_box_pack_end( GTK_BOX(hboxbuttons), button3_w, FALSE, FALSE, 0);
  }


  GtkWidget *vbox = gtk_vbox_new( FALSE, 0);
  gtk_box_pack_start( GTK_BOX(vbox), hboxtext, TRUE, TRUE, 30);
  gtk_box_pack_start( GTK_BOX(vbox), gtk_hseparator_new(), FALSE, FALSE, 0);
  gtk_box_pack_end( GTK_BOX(vbox), hboxbuttons, FALSE, FALSE, 15);
  gtk_container_add( GTK_CONTAINER(dialog_w), vbox);
  gtk_widget_show_all( dialog_w);

  gtk_main();
  gtk_widget_destroy( dialog_w);

  return status;
}

//
//  Modal Dialog with text, inputentry  and three buttons.
//  The returned data should be freed with free().
//

static gboolean modaldia_keypress_cb( GtkWidget *w, GdkEvent *event, gpointer data)
{
  gboolean sts;
  guint keysym = event->key.keyval;
  gchar *text = gtk_editable_get_chars( GTK_EDITABLE(w), 0, -1);

  switch ( keysym) {
  case GDK_Return:
  case GDK_KP_Enter:
  case GDK_Linefeed: {
    // Terminate
    *(int *)data = wow_eModalDialogReturn_ReturnPressed;
    gtk_main_quit();
    sts = FALSE;
    break;
  }
  default:
    sts = FALSE;
  }

  g_free( text);
  return sts;
}

wow_sModalInputDialog *CoWowGtk::CreateModalInputDialog( const char *title, const char *text, const char *button1, 
							 const char *button2, const char *button3,
							 const char *image, int input_length)
{
  int status = 0;
  GtkWidget *image_w;
  pwr_tFileName fname;

  // Create a question window
  GtkWidget *dialog_w = (GtkWidget *) g_object_new( GTK_TYPE_WINDOW, 
							 "default-height", 150,
							 "default-width", 800,
							 "title", title,
							 "window-position", GTK_WIN_POS_CENTER,
							 NULL);

  g_signal_connect( dialog_w, "delete_event", G_CALLBACK(modaldia_delete_event), &status);
  GtkWidget *text_w = gtk_label_new(text);


  if ( image) {
    dcli_translate_filename( fname, image);
    image_w = gtk_image_new_from_file( fname);
  }
  if ( !image) {
    image_w = (GtkWidget *)g_object_new( GTK_TYPE_IMAGE, 
				"stock", GTK_STOCK_DIALOG_QUESTION,
				"icon-size", GTK_ICON_SIZE_DIALOG,
				"xalign", 0.5,
				"yalign", 1.0,
				NULL);
  }

  GtkWidget *textinput = gtk_entry_new_with_max_length( input_length);
  g_signal_connect( textinput, "key-press-event", 
  		    G_CALLBACK(modaldia_keypress_cb), &status);

  GtkWidget *hboxtext = gtk_hbox_new( FALSE, 0);
  gtk_box_pack_start( GTK_BOX(hboxtext), image_w, FALSE, FALSE, 15);
  gtk_box_pack_start( GTK_BOX(hboxtext), text_w, FALSE, FALSE, 15);
  gtk_box_pack_start( GTK_BOX(hboxtext), textinput, TRUE, TRUE, 15);

  GtkWidget *hboxbuttons = gtk_hbox_new( TRUE, 40);

  if ( button1) {
    GtkWidget *button1_w = gtk_button_new_with_label( translate_utf8(button1));
    gtk_widget_set_size_request( button1_w, 90, 28);
    g_signal_connect( button1_w, "clicked", 
		      G_CALLBACK(modaldia_button1_cb), &status);
    gtk_box_pack_start( GTK_BOX(hboxbuttons), button1_w, FALSE, FALSE, 0);
  }
  if ( button2) {
    GtkWidget *button2_w = gtk_button_new_with_label( translate_utf8(button2));
    gtk_widget_set_size_request( button2_w, 90, 28);
    g_signal_connect( button2_w, "clicked", 
		      G_CALLBACK(modaldia_button2_cb), &status);
    gtk_box_pack_start( GTK_BOX(hboxbuttons), button2_w, FALSE, FALSE, 0);
  }
  if ( button3) {
    GtkWidget *button3_w = gtk_button_new_with_label( translate_utf8(button3));
    gtk_widget_set_size_request( button3_w, 90, 28);
    g_signal_connect( button3_w, "clicked", 
		      G_CALLBACK(modaldia_button3_cb), &status);
    gtk_box_pack_end( GTK_BOX(hboxbuttons), button3_w, FALSE, FALSE, 0);
  }


  GtkWidget *vbox = gtk_vbox_new( FALSE, 0);
  gtk_box_pack_start( GTK_BOX(vbox), hboxtext, TRUE, TRUE, 30);
  gtk_box_pack_start( GTK_BOX(vbox), gtk_hseparator_new(), FALSE, FALSE, 0);
  gtk_box_pack_end( GTK_BOX(vbox), hboxbuttons, FALSE, FALSE, 15);
  gtk_container_add( GTK_CONTAINER(dialog_w), vbox);
  gtk_widget_show_all( dialog_w);

  gtk_main();

  wow_sModalInputDialog *ret = (wow_sModalInputDialog *) calloc( 1, sizeof(*ret));
  ret->status = status;

  char *valueutf8 = gtk_editable_get_chars( GTK_EDITABLE(textinput), 0, -1);
  char *value = g_convert( valueutf8, -1, "ISO8859-1", "UTF-8", NULL, NULL, NULL);
  strncpy( ret->input_str, value, sizeof(ret->input_str));
  g_free( valueutf8);
  g_free( value);


  gtk_widget_destroy( dialog_w);

  return ret;
}

pwr_tStatus CoWowGtk::CreateMenuItem( const char *name, void *menu, int pixmap, int append, void *w)
{
  char name_array[10][40];
  int name_cnt;
  char label[80];
  GtkWidget *child;
  pwr_tFileName fname;
  
  name_cnt = dcli_parse( name, "-", "",
			 (char *) name_array, sizeof( name_array)/sizeof( name_array[0]), 
			 sizeof( name_array[0]), 0);
  if ( !name_cnt) return 0;

  if ( name_cnt == 1) {
    if ( pixmap == wow_ePixmap_No || pixmap >= wow_ePixmap__)
      *(GtkWidget **)w = gtk_menu_item_new_with_label( translate_utf8(name_array[0]));    
    else {
      *(GtkWidget **)w = gtk_image_menu_item_new_with_label( translate_utf8(name_array[0]));
      switch ( pixmap) {
      case wow_ePixmap_Graph:
	dcli_translate_filename( fname, "$pwr_exe/wtt_ge.png");
	break;
      default:
	dcli_translate_filename( fname, "$pwr_exe/wtt_ge.png");
      }
      gtk_image_menu_item_set_image( GTK_IMAGE_MENU_ITEM(*(GtkWidget **)w), 
				     gtk_image_new_from_file( fname));
    }
    if ( append)
      gtk_menu_shell_append(GTK_MENU_SHELL(menu), *(GtkWidget **)w);
    else
      gtk_menu_shell_prepend(GTK_MENU_SHELL(menu), *(GtkWidget **)w);
    gtk_widget_show( GTK_WIDGET(*(GtkWidget **)w));
  }
  else {
    // Find or create the parent menues
    GtkContainer *parent = (GtkContainer *)menu;

    for ( int i = 0; i < name_cnt; i++) {
      GList *chlist = gtk_container_get_children( GTK_CONTAINER(parent));

      int found = 0;
      for( GList *ch = chlist; ch; ch = ch->next) {
	GtkWidget *labelw = gtk_bin_get_child( GTK_BIN(ch->data));			   
	strncpy( label, (char *)gtk_label_get_text( GTK_LABEL(labelw)), sizeof(label)); 
	if ( cdh_NoCaseStrcmp( label, name_array[i]) == 0) {
	  child = GTK_WIDGET(ch->data);
	  found = 1;
	  break;
	}
      }
      if ( !found) {
	// Create the menu item
	GtkWidget *item;
	if ( i == name_cnt - 1 && pixmap != wow_ePixmap_No && pixmap <= wow_ePixmap__) {
	  item = gtk_image_menu_item_new_with_label( translate_utf8(name_array[i]));
	  switch ( pixmap) {
	  case wow_ePixmap_Graph:
	    dcli_translate_filename( fname, "$pwr_exe/wtt_ge.png");
	    break;
	  default:
	    dcli_translate_filename( fname, "$pwr_exe/wtt_ge.png");
	  }
	  gtk_image_menu_item_set_image( GTK_IMAGE_MENU_ITEM(item), 
					 gtk_image_new_from_file( fname));
	}
	else
	  item = gtk_menu_item_new_with_label( translate_utf8(name_array[i]));    

	if ( append)
	  gtk_menu_shell_append(GTK_MENU_SHELL(parent), item);
	else
	  gtk_menu_shell_prepend(GTK_MENU_SHELL(parent), item);
	gtk_widget_show( GTK_WIDGET(item));
	if ( i != name_cnt - 1) {
	  GtkMenu *submenu = (GtkMenu *) g_object_new( GTK_TYPE_MENU, NULL);
	  gtk_menu_item_set_submenu(GTK_MENU_ITEM(item), GTK_WIDGET(submenu));
	  parent = GTK_CONTAINER(submenu);
	  gtk_widget_show( GTK_WIDGET(submenu));
	}
	else 
	  parent = (GtkContainer *)item;
      }
      else
	parent = (GtkContainer *)gtk_menu_item_get_submenu(GTK_MENU_ITEM(child));

      if ( !parent) return 0;
	
      g_list_free( chlist);
    }
    *(GtkContainer **)w = parent;
  }
  return 1;
}

pwr_tStatus CoWowGtk::DeleteMenuItem( const char *name, void *menu)
{
  char name_array[10][40];
  int name_cnt;
  char label[80];
  GtkWidget *child;

  name_cnt = dcli_parse( name, "-", "",
			 (char *) name_array, sizeof( name_array)/sizeof( name_array[0]), 
			 sizeof( name_array[0]), 0);
  if ( !name_cnt) return 0;

  // Find the parent menues
  GtkContainer *parent = (GtkContainer *)menu;

  for ( int i = 0; i < name_cnt; i++) {
    GList *chlist = gtk_container_get_children( GTK_CONTAINER(parent));

    int found = 0;
    for( GList *ch = chlist; ch; ch = ch->next) {
      GtkWidget *labelw = gtk_bin_get_child( GTK_BIN(ch->data));			   
      strncpy( label, (char *)gtk_label_get_text( GTK_LABEL(labelw)), sizeof(label)); 
      if ( cdh_NoCaseStrcmp( label, name_array[i]) == 0) {
	child = GTK_WIDGET(ch->data);
	found = 1;
	break;
      }
    }
    if ( !found)
      return 0;

    if ( i == name_cnt - 1)
      gtk_widget_destroy( child);
    else
      parent = (GtkContainer *)gtk_menu_item_get_submenu(GTK_MENU_ITEM(child));

    if ( !parent) return 0;
	
    g_list_free( chlist);
  }
  return 1;
}








