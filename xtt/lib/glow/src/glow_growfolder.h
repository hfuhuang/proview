/* 
 * Proview   $Id: glow_growfolder.h,v 1.5 2006-05-16 11:50:27 claes Exp $
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

#ifndef glow_growfolder_h
#define glow_growfolder_h

#include "glow_growwindow.h"
#include "glow_tracedata.h"

/*! \file glow_growfolder.h
    \brief Contains the GrowFolder class. */
/*! \addtogroup Glow */
/*@{*/

#define MAX_FOLDERS 12

//! Class for drawing a folder object.
/*! A GrowFolder is a component that displays another graph, inside its borders.

  The GrowFolder class contains function for drawing the object, and handle events when the 
  object is clicked on, moved etc.
*/

class GrowFolder : public GrowWindow {
 public:
  //! Constuctor
  /*!
    \param glow_ctx 	The glow context.
    \param name		Name (max 31 char).
    \param x		x coordinate for position.
    \param y		y coordinate for position.
    \param w		Width.
    \param h		Height.
    \param border_d_type Border color.
    \param line_w	Linewidth of border.
    \param sel_color	Color for selected tab.
    \param unsel_color	Color for unselected tab.
    \param display_lev	Displaylevel when this object is visible.
    \param nodraw	Don't draw the object now.
  */
  GrowFolder( GlowCtx *glow_ctx, char *name, double x = 0, double y = 0, 
		double w = 0, double h = 0, 
		glow_eDrawType border_d_type = glow_eDrawType_Line, 
	        int line_w = 1, glow_eDrawType sel_color = glow_eDrawType_LightGray,
	        glow_eDrawType unsel_color = glow_eDrawType_MediumGray,
		glow_mDisplayLevel display_lev = glow_mDisplayLevel_1,
		int nodraw = 0);

  //! Destructor
  /*! Remove the object from context, and erase it from the screen.
   */
  ~GrowFolder();


  //! Save the content of the object to file.
  /*!
    \param fp	Ouput file.
    \param mode	Not used.
  */
  void save( ofstream& fp, glow_eSaveMode mode);

  //! Read the content of the object from file.
  /*!
    \param fp	Input file.
  */
  void open( ifstream& fp);

  //! Erase the object
  void erase()
	{ erase( (GlowTransform *)NULL, hot, NULL);};

  //! Erase the object in the navigator window.
  void nav_erase()
	{ nav_erase( (GlowTransform *)NULL, NULL);};

  //! Draw the objects if any part is inside the drawing area.
  /*!
    \param ll_x		Lower left x coordinate of drawing area.
    \param ll_y		Lower left y coordinate of drawing area.
    \param ur_x		Upper right x coordinate of drawing area.
    \param ur_y		Upper right y coordinate of drawing area.
  */
  void draw( int ll_x, int ll_y, int ur_x, int ur_y);

  //! Draw the objects if any part is inside the drawing area, and extends the drawing area.
  /*!
    \param ll_x		Lower left x coordinate of drawing area.
    \param ll_y		Lower left y coordinate of drawing area.
    \param ur_x		Upper right x coordinate of drawing area.
    \param ur_y		Upper right y coordinate of drawing area.

    If some part of object is inside the drawing area, and also outside the drawing area,
    the drawingarea is extended so it contains the whole objects.
  */
  void draw( int *ll_x, int *ll_y, int *ur_x, int *ur_y);

  //! Drawing in the navigation window. See the corresponding draw function.
  void nav_draw(int ll_x, int ll_y, int ur_x, int ur_y);

  //! Set object highlight.
  /*!
    \param on	If 1, set highlight. If 0, reset highlight.
  */
  void set_highlight( int on);

  //! Get the object type
  /*!
    \return The type of the object.
  */
  glow_eObjectType type() { return glow_eObjectType_GrowFolder;};

  int			folders;		//!< Number of folders.
  int			text_size;		//!< Header text size.
  glow_eDrawType        text_drawtype;		//!< Header text drawtype.
  glow_eDrawType       	text_color_drawtype;	//!< Header text color.
  double		header_height;		//!< Header height.
  char			folder_file_names[MAX_FOLDERS][80]; //!< Filenames for the folders.
  char			folder_text[MAX_FOLDERS][80]; //!< Folder text.
  double	       	folder_scale[MAX_FOLDERS];	//!< Scale for each folder.
  int			folder_v_scrollbar[MAX_FOLDERS]; //!< Vertical scrollbar for each folder.
  int			folder_h_scrollbar[MAX_FOLDERS]; //!< Horizontal scrollbar for each folder.
  char			folder_owner[MAX_FOLDERS][256]; //!< Owner for each folder.
  int			current_folder;		//!< The currently selected folder.
  glow_eDrawType	color_selected;		//!< Color of selected folder.
  glow_eDrawType        color_unselected;	//!< Color of unselected folder.
  
  //! Draw the object.
  /*!
    \param t		Transform of parent node. Can be zero.
    \param highlight	Draw with highlight colors.
    \param hot		Draw as hot, with larger line width.
    \param node		Parent node. Can be zero.
    \param colornode	The node that controls the color of the object. Can be zero.

    The object is drawn with border, fill and shadow. If t is not zero, the current tranform is
    multiplied with the parentnodes transform, to give the appropriate coordinates for the drawing.
  */
  void draw( GlowTransform *t, int highlight, int hot, void *node, void *colornode);

  //! Erase the object.
  /*!
    \param t		Transform of parent node.
    \param hot		Draw as hot, with larger line width.
    \param node		Parent node. Can be zero.
  */
  void erase( GlowTransform *t, int hot, void *node);

  //! Redraw the area inside the objects border.
  void draw();

  //! Draw the object in the navigation window.
  /*!
    \param t		Transform of parent node. Can be zero.
    \param highlight	Draw with highlight colors.
    \param node		Parent node. Can be zero.
    \param colornode	The node that controls the color of the object. Can be zero.
  */
  void nav_draw( GlowTransform *t, int highlight, void *node, void *colornode);

  //! Erase the object in the navigation window.
  /*!
    \param t		Transform of parent node.
    \param node		Parent node. Can be zero.
  */
  void nav_erase( GlowTransform *t, void *node);

  //! Export the object as a javabean.
  /*!
    \param t		Transform of parent node. Can be zero.
    \param node		Parent node. Can be zero.
    \param pass		Export pass.
    \param shape_cnt	Current index in a shape vector.
    \param node_cnt	Counter used for javabean name. Not used for this kind of object.
    \param in_nc	Member of a nodeclass. Not used for this kind of object.
    \param fp		Output file.

    The object is transformed to the current zoom factor, and GlowExportJBean is used to generate
    java code for the bean.
  */
  void export_javabean( GlowTransform *t, void *node,
	glow_eExportPass pass, int *shape_cnt, int node_cnt, int in_nc,  ofstream &fp);


  //! Conversion between different versions of Glow
  /*!
    \param version	Version to convert to.
  */
  void convert( glow_eConvert version);

  int event_handler( glow_eEvent event, int x, int y, double fx,
		     double fy);

  //! Check if new filename
  void update_attributes();

  //! Set text size
  /*!
    \param size		Size index 0-8.
  */
  void set_textsize( int size);

  //! Set text bold
  /*!
    \param bold		Text is bold.
  */
  void set_textbold( int bold);

  //! Set original text color.
  /*!
    \param drawtype	Text color.
  */
  void set_original_text_color( glow_eDrawType drawtype) 
	{ text_color_drawtype = drawtype; draw();};

  //! Set displayed folder.
  /*!
    \param idx	Folder index.
  */
  int set_folder( int idx);
};


/*@}*/
#endif









