/* 
 * Proview   $Id: glow_text.h,v 1.5 2008-10-31 12:51:36 claes Exp $
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

#ifndef glow_text_h
#define glow_text_h

#include <iostream>
#include "glow.h"
#include "glow_growctx.h"
#include "glow_point.h"
#include "glow_array_elem.h"


/*! \file glow_text.h
    \brief Contains the GlowText class. */
/*! \addtogroup Glow */
/*@{*/


//! Base class for a text.
/*! The full implementation of a text is in the GrowText class. 
  The GlowText class is still used by GlowCon, when drawing reference connections.
*/

class GlowText : public GlowArrayElem {
 public:
  //! Constuctor
  /*!
    \param glow_ctx 	The glow context.
    \param text1	Text.
    \param x		x coordinate for position.
    \param y		y coordinate for position.
    \param d_type 	Text drawtype.
    \param color_d_type Color drawtype.
    \param t_size	Text size.
    \param display_lev	Level when the object is visible.
  */
  GlowText( GrowCtx *glow_ctx, const char *text1, double x = 0, double y = 0,
	glow_eDrawType d_type = glow_eDrawType_TextHelveticaBold, 
	glow_eDrawType color_d_type = glow_eDrawType_Line,
	int t_size = 2, glow_mDisplayLevel display_lev = glow_mDisplayLevel_1) : 
	ctx(glow_ctx), p(glow_ctx,x,y), draw_type(d_type), text_size(t_size),
        display_level(display_lev), color_drawtype( color_d_type)
	{ strncpy( text, text1, sizeof(text));};

  friend ostream& operator<< ( ostream& o, const GlowText t);

  //! Adjust pixel coordinates to current zoom factor.
  void zoom();

  //! Adjust pixel coordinates for navigaion window to current zoom factor.
  void nav_zoom();

  void print_zoom();		//!< Not used.
  void traverse( int x, int y);	//!< Not used.

  //! Event handler
  /*!
    \param pos		Position of object. Should be zero.
    \param event	Current event.
    \param x		x coordinate of event.
    \param y		y coordinate of event.
    \param node		Parent node. Can be zero.
    \return		Returns 1 if the object is hit, else 0.

    Detects if the object is hit by the event.
  */
  int	event_handler( GlowWind *w, void *pos, glow_eEvent event, int x, int y, void *node);

  //! Not implemented
  void conpoint_select( void *pos, int x, int y, double *distance, 
		void **cp) {};

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

  //! Draw the object.
  /*!
    \param pos		Position of object. Should be zero.
    \param highlight	Draw with highlight colors.
    \param hot		Draw as hot, with larger line width.
    \param node		Parent node. Can be zero.

    Draw the object, without borders or shadow.
  */
  void draw( GlowWind *w, void *pos, int highlight, int hot, void *node);

  //! Erase the object.
  /*!
    \param pos		Position of object. Should be zero.
    \param hot		Draw as hot, with larger line width.
    \param node		Parent node. Can be zero.
  */
  void erase( GlowWind *w, void *pos, int hot, void *node);

  //! Move the text to the specified coordinates.
  /*!
    \param pos		Position. Should be zero.
    \param x		x coordinate of position.
    \param y		y coordinate of position.
    \param highlight	Draw with highlight colors.
    \param hot		Draw as hot.
  */
  void move( void *pos, double x, double y, int highlight, int hot);

  //! Move the text.
  /*!
    \param pos		Position. Should be zero.
    \param delta_x	Moved distance in x direction.
    \param delta_y	Moved distance in y direction.
    \param highlight	Draw with highlight colors.
    \param hot		Draw as hot.
  */
  void shift( void *pos, double delta_x, double delta_y,
	int highlight, int hot);

  //! Calculate the border for a set of objects or for a parent node.
  /*!
    \param pos_x        x coordinate for position.
    \param pos_y        y coordinate for position.
    \param x_right	Right limit.
    \param x_left	Left limit.
    \param y_high	High limit.
    \param y_low	Low limit.
    \param node		Parent node. Can be zero.
    
    If the borders of the objects exceeds a limit, the limit is adjusted to the
    border of the object.
  */
  void get_borders( double pos_x, double pos_y, double *x_right, 
		double *x_left, double *y_high, double *y_low, void *node);

  int get_conpoint( int num, double *x, double *y, glow_eDirection *dir) 
		{ return 0;};

  //! Get the object type
  /*!
    \return The type of the object.
  */
  glow_eObjectType type() { return glow_eObjectType_Text;};

  GrowCtx *ctx;    	//!< Glow context.
  GlowPoint p;		//!< Position point.
  char text[80];	//!< The text.
  glow_eDrawType draw_type;	//!< Drawtype for the text.
  int	 text_size;	//!< Text size.
  glow_mDisplayLevel display_level; //!< Display level when the object is visible.
  glow_eDrawType color_drawtype; //!< Text color.
};

/*@}*/
#endif
