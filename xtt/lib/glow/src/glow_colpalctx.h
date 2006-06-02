/* 
 * Proview   $Id: glow_colpalctx.h,v 1.5 2006-06-02 11:09:51 claes Exp $
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

#ifndef glow_colpalctx_h
#define glow_colpalctx_h

#include <iostream.h>
#include <fstream.h>

#include "glow.h"
#include "glow_growctx.h"

 
/*! \file glow_colpalctx.h
    \brief Contains the ColPalCtx class. */
/*! \addtogroup Glow */
/*@{*/


//! Context for the color palette.
/*! The color palette consists of 300 colors, where the bordercolor, fillcolor and the textcolor
  is selected. The selected colors are displayed above the actual palette. Below the color palette there is
  a colortone palette where a color tone can be selected. There is also a reset button.

  When a color entry is activated, an event is sent to the registred callback routine for the event.
*/

class ColPalCtx : public GrowCtx {
 public:
  
  //! Constructor.
  /*! 
    \param ctx_name	Name of the context.
    \param zoom_fact	Initial zoomfactor.
  */
  ColPalCtx( char *ctx_name, double zoom_fact = 100) :
    	GrowCtx( ctx_name, zoom_fact), columns(30), 
	current_fill( glow_eDrawType_LineGray), current_border( glow_eDrawType_Line),
	current_text(glow_eDrawType_Line), entry_width(0.3), entry_height(1), display_entry_width(3)
	{ ctx_type = glow_eCtxType_ColPal; grid_on = 0; };

  //! Destructor
  ~ColPalCtx() {};

  //! Configure the palette.
  /*! This function creates all color rectangles and texts in the palette, and gives them
    appropriate names, so they can be identified when an event is recieved.
  */
  void configure();

  //! Update the layout of the scrollbars.
  void change_scrollbar();

  //! Redraw the window.
  /*! Redraw all objects in the window.
   */
  void redraw();

  //! Adjust the pixel coordinates to the current zoomfactor.
  void zoom( double factor);

  //! Reset to base zoomfactor.
  void unzoom() { zoom( base_zoom_factor / zoom_factor_y);};

  //! Not used.
  void print( char *filename);

  //! Event handler
  /*!
    \param event	Current event.
    \param x		x coordinate of event.
    \param y		y coordinate of event.
    \param w		Width of exposure event.
    \param h		Height of exposure event.
    \return		Returns always 1.

    Handling of event. Detects if any color or colortone entry is hit. Send event to
    a callback function, if there is one registred for this event.
  */
  int event_handler( glow_eEvent event, int x, int y, int w, int h);

  int columns;			//!< Number of columns in the color palette.
  glow_eDrawType current_fill;	//!< The currently selected fill color.
  glow_eDrawType current_border; //!< The currently selected border color.
  glow_eDrawType current_text;	//!< The currently selected text color.
  GlowArrayElem *display_fill;	//!< The rectangle object to display the current fillcolor.
  GlowArrayElem *display_border; //!< The rectangle object to display the current border color.
  GlowArrayElem *display_text;	//!< The rectangle object to display the current text color.
  double entry_width;		//!< Width of a color palette entry.
  double entry_height;		//!< Height of a color palette entry.
  double display_entry_width;	//!< Width of a display entry.
};

//! Scroll horizontal.
/*!
  \param ctx	Pointer to ColPalCtx object.
  \param value	Scrollbar value.
  \param bottom	Scrollbar is in bottom position.
*/
void colpal_scroll_horizontal( ColPalCtx *ctx, int value, int bottom);

//! Scroll vertical.
/*!
  \param ctx	Pointer to ColPalCtx object.
  \param value	Scrollbar value.
  \param bottom	Scrollbar is in bottom position.
*/
void colpal_scroll_vertical( ColPalCtx *ctx, int value, int bottom);

/*@}*/
#endif






