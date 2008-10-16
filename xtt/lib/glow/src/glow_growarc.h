/* 
 * Proview   $Id: glow_growarc.h,v 1.7 2008-10-16 08:58:11 claes Exp $
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

#ifndef glow_growarc_h
#define glow_growarc_h

#include "glow_arc.h"
#include "glow_transform.h"

class GlowExportFlow;

/*! \file glow_growarc.h
    \brief Contains the GrowArc class. */
/*! \addtogroup Glow */
/*@{*/


//! Class for drawing an arc.
/*! An arc object for drawing a arc, cicle or elipse, with specified bordercolor, linewidth, fill and fillcolor.
  Detection of event when the object is clicked on, moved etc.
*/
class GrowArc : public GlowArc {
  public:
  //! Constuctor
  /*!
    \param glow_ctx 	The glow context.
    \param name		Name (max 31 char).
    \param x1		x coordinate for first corner of a rectangle that surrouds the elipse of the arc.
    \param y1		y coordinate for first corner of a rectangle that surrouds the elipse of the arc.
    \param x2		x coordinate for second corner of a rectangle that surrouds the elipse of the arc.
    \param y2		y coordinate for second corner of a rectangle that surrouds the elipse of the arc.
    \param ang1		Start angle of the arc in degrees from the x axis.
    \param ang2		Lengh of the arc in degrees from the start angle.
    \param border_d_type Border color.
    \param line_w	Linewidth of border.
    \param fill_arc	Arc is filled.
    \param display_border Border is visible.
    \param display_shadow Shadow is visible.
    \param fill_d_type	Fill color.
    \param nodraw	Don't draw the object now.
  */
  GrowArc( GrowCtx *glow_ctx, char *name, double x1 = 0, double y1 = 0, 
	     double x2 = 0, double y2 = 0, 
	     int ang1 = 0, int ang2 = 0,
	     glow_eDrawType border_d_type = glow_eDrawType_Line, 
	     int line_w = 1,
	     int fill_arc = 0, int display_border = 1, int display_shadow = 0,
	     glow_eDrawType fill_d_type = glow_eDrawType_Line, int nodraw = 0);

  //! Event handler
  /*!
    \param event	Current event.
    \param x		Not used.
    \param y		Not used.
    \param fx		x coordinate of event.
    \param fy		y coordinate of event.
    \return		Returns 1 if the object is hit, else 0.

    Detects if the objet is hit by the event, and if it is, performes the appropriate
    action: changes the cursor, draws the object hot, and registers the object as
    current callback object.
  */
  int	event_handler( GlowWind *w, glow_eEvent event, int x, int y, double fx, double fy);

  //! Detects if the object is hit by an event in transformed coordinates
  /*!
    \param event	Current event.
    \param fx		x coordinate of event in the objects transformed coordinate system.
    \param fy		y coordinate of event in the objects transformed coordinate system.
    \return 		Returns 1 if the object is hit, else 0.

    Compares the coordinates of the event with the borders of the object.
    If the event is inside the borders, 1 is returned, otherwise 0 is returned.
  */
  int event_handler( GlowWind *w, glow_eEvent event, double fx, double fy);

  //! Detects if the object is hit by an event in local coordinates
  /*!
    \param event	Current event.
    \param x		x of event coordinate in the objects local coordinate system.
    \param y		y of event coordinate in the objects local coordinate system.
    \return 		Returns 1 if the object is hit, else 0.

    Compares the coordinates of the event with the borders of the object.
    If the event is inside the borders, 1 is returned, otherwise 0 is returned.
  */
  int local_event_handler( glow_eEvent event, double x, double y);

  //! Calculate the border for a set of objects or for a parent node.
  /*!
    \param t		Transform.
    \param x1_right	Right limit.
    \param x1_left	Left limit.
    \param y1_high	High limit.
    \param y1_low	Low limit.
    
    If the borders of the objects exceeds a limit, the limit is adjusted to the
    border of the object.
  */
  void get_borders( GlowTransform *t,
	double *x1_right, double *x1_left, double *y1_high, double *y1_low);

  //! Calculate the border for a set of objects, without transformation.
  /*!
    \param x1_right	Right limit.
    \param x1_left	Left limit.
    \param y1_high	High limit.
    \param y1_low	Low limit.
    
    If the borders of the objects exceeds a limit, the limit is adjusted to the
    border of the object.
  */
  void get_borders(
	double *x1_right, double *x1_left, double *y1_high, double *y1_low)
	{ get_borders( (GlowTransform *)NULL, x1_right, x1_left, y1_high, y1_low);};

  //! Update the borders of the object.
  /*! The borders of the object is stored in x_right, x_left, y_high and y_low.
   */
  void get_node_borders()
	{ x_left = y_low = 1e37; x_right = y_high = -1e37;
	  get_borders( (GlowTransform *)NULL, &x_right, &x_left, &y_high, &y_low);};

  //! Not implemented
  void print( double ll_x, double ll_y, double ur_x, double ur_y) {};

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

  //! Draw the objects if any part is inside the drawing area.
  /*!
    \param ll_x		Lower left x coordinate of drawing area.
    \param ll_y		Lower left y coordinate of drawing area.
    \param ur_x		Upper right x coordinate of drawing area.
    \param ur_y		Upper right y coordinate of drawing area.
  */
  void draw( GlowWind *w, int ll_x, int ll_y, int ur_x, int ur_y);

  //! Draw the objects if any part is inside the drawing area, and extends the drawing area.
  /*!
    \param ll_x		Lower left x coordinate of drawing area.
    \param ll_y		Lower left y coordinate of drawing area.
    \param ur_x		Upper right x coordinate of drawing area.
    \param ur_y		Upper right y coordinate of drawing area.

    If some part of object is inside the drawing area, and also outside the drawing area,
    the drawingarea is extended so it contains the whole objects.
  */
  void draw( GlowWind *w, int *ll_x, int *ll_y, int *ur_x, int *ur_y);

  //! Erase the object
  void erase( GlowWind *w)
	{ erase( w, (GlowTransform *)NULL, hot, NULL);};

  //! Move the object.
  /*!
    \param delta_x	Moved distance in x direction.
    \param delta_y	Moved distance in y direction.
    \param grid		Position object on grid point.
  */
  void move( double delta_x, double delta_y, int grid);


  //! Move the object without erase.
  /*!
    \param delta_x	Moved distance in x direction.
    \param delta_y	Moved distance in y direction.
    \param grid		Position object on grid point.
  */
  void move_noerase( int delta_x, int delta_y, int grid);

  //! Set object highlight.
  /*!
    \param on	If 1, set highlight. If 0, reset highlight.
  */
  void set_highlight( int on);

  //! Get object highlight.
  /*!
    \return Return 1 if object is highlighted, else 0.
  */
  int get_highlight() {return highlight;};

  //! Not used.
  void set_hot( int on) {};

  //! Insert object in select list, if it is inside the selection area. 
  /*!
    \param ll_x		Lower left x coordinate of selection area.
    \param ll_y		Lower left y coordinate of selection area.
    \param ur_x		Upper right x coordinate of selection area.
    \param ur_y		Upper right y coordinate of selection area.
    \param select_policy Current select policy.
  */
  void select_region_insert( double ll_x, double ll_y, double ur_x, 
		double ur_y, glow_eSelectPolicy select_policy);

  //! Get the object type
  /*!
    \return The type of the object.
  */
  glow_eObjectType type() { return glow_eObjectType_GrowArc;};

  //! Measure the extent of the object.
  /*!
    \param ll_x		Left border of the object.
    \param ll_y		Low border of the object.
    \param ur_x		Right border of the object.
    \param ur_y		High border of the object.
  */
  void	measure( double *ll_x, double *ll_y, double *ur_x, double *ur_y)
	{ *ll_x = x_left; *ll_y = y_low; *ur_x = x_right; *ur_y = y_high;};

  //! Get the object name
  /*!
    \param name		The name of the object.
  */
  void get_object_name( char *name) { strcpy( name, n_name);};

  //! Set the object name
  /*!
    \param name		The name of the object. Max 31 char.
  */
  void set_object_name( char *name) { strcpy( n_name, name);};

  double	x_right;	//!< Right border of object.
  double	x_left;		//!< Left border of object.
  double	y_high;		//!< High border of object.
  double	y_low;		//!< Low border of object.
  int		hot;		//!< Object is hot, i.e. the cursor is currently on the object.
  GlowPoint 	pzero;		//!< Not used.
  GlowPoint	stored_pos;	//!< Not used.
  char		n_name[32];	//!< Object name.	
  int		highlight;	//!< Object is highlighted, i.e. painted with red color.
  int		inverse;	//!< Not implemented.
  void 		*user_data;    	//!< User data.
  glow_eDrawType original_border_drawtype; //!< The original border color, i.e. color drawn at edit time.
  glow_eDrawType original_fill_drawtype; //!< The original fill color, i.e. color drawn at edit time.
  glow_eDrawType fill_drawtype; //!< Fill color.
  int		border;		//!< Display border.
  char 		*dynamic;	//!< Dynamic code.
  int 		dynamicsize;	//!< Size of dynamic code.
  GlowTransform trf;		//!< Transformation matrix of object.
  char        	last_group[32];	//!< The last group the object was a member of.
  int 		shadow;		//!< Draw shadow.
  double 	shadow_width;	//!< Width of shadow, in percentage of minimum of width and height.
  glow_eRelief 	relief;		//!< Type of relief.
  int 		shadow_contrast; //!< Shadow contrast.
  int		disable_shadow;	//!< Disable shadow, even if parent node has shadow.

  //! Set user data.
  /*!
    \param data User data.
  */
  void set_user_data( void *data) { user_data = data;};

  //! Get user data.
  /*!
    \param data User data.
  */
  void get_user_data( void **data) { *data = user_data;};

  //! Set dynamic code
  /*!
    \param code		Dynamic code.
    \param size		Size of dynamic code.
  */
  void set_dynamic( char *code, int size);
  //! Get dynamic code
  /*!
    \param code		Dynamic code.
    \param size		Size of dynamic code.
  */
  void get_dynamic( char **code, int *size) { *code = dynamic; 
	*size = dynamicsize;};

  //! Get grow context.
  /*!
    \return The context.
  */
  void *get_ctx() { return this->ctx;};

  //! Execute the dynamic code.
  void exec_dynamic();
  //! Set the position of the object, in absoute coordinates.
  /*!
    \param x	x coordinate.
    \param y	y coordinate.
  */
  void set_position( double x, double y);

  //! Set the scale of the object. The scale is made from the last stored transform.
  /*!
    \param scale_x	Scale factor in x direction.
    \param scale_y	Scale factor in y direction.
    \param x0		x coordinate of scale point.
    \param y0		y coordinate of scale point.
    \param type		Type of scaling.
  */
  void set_scale( double scale_x, double scale_y, double x0, double y0, 
		glow_eScaleType type);

  //! Rotate the object. The rotation is made form the last stored transform.
  /*!
    \param angle	Rotation angle in degrees.
    \param x0		x coordinate of rotation point.
    \param y0		y coordinate of rotation point.
    \param type		Type of rotation point.
  */
  void set_rotation( double angle, 
		double x0, double y0, glow_eRotationPoint type);

  //! Set the fill color.
  /*!
    \param drawtype	Fill color.
  */
  void set_fill_color( glow_eDrawType drawtype) 
	{ fill_drawtype = drawtype; draw();};

  //! Reset the fill color to the original fill color.
  void reset_fill_color() 
	{ fill_drawtype = original_fill_drawtype; draw();};

  //! Set the border color.
  /*!
    \param drawtype	Border color.
  */
  void set_border_color( glow_eDrawType drawtype) 
	{ draw_type = drawtype; draw();};

  //! Reset the border color to the original border color.
  void reset_border_color()
	{ draw_type = original_border_drawtype; draw();};

  //! Set the original fill color.
  /*!
    \param drawtype	Fill color.
  */
  void set_original_fill_color( glow_eDrawType drawtype) 
	{ original_fill_drawtype = drawtype; reset_fill_color();};

  //! Set the original border color.
  /*!
    \param drawtype	Border color.
  */
  void set_original_border_color( glow_eDrawType drawtype) 
	{ original_border_drawtype = drawtype; reset_border_color();};

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
  void draw( GlowWind *w, GlowTransform *t, int highlight, int hot, void *node, void *colornode);

  //! Erase the object.
  /*!
    \param t		Transform of parent node.
    \param hot		Draw as hot, with larger line width.
    \param node		Parent node. Can be zero.
  */
  void erase( GlowWind *w, GlowTransform *t, int hot, void *node);

  //! Redraw the area inside the objects border.
  void draw();

  //! Add a transform to the current transform.
  /*!
    \param t		Transform.

    The transform is multiplied to the current transform, to give the new transform for the object.
  */
  void set_transform( GlowTransform *t);

  //! Add a transform to the stored transform.
  /*!
    \param t		Transform.

    The transorm is multiplied with the stored transform, to give to new transform for the object.
  */
  void set_transform_from_stored( GlowTransform *t) 
	{ trf.set_from_stored( t), get_node_borders(); };

  //! Store the current transform
  /*! The stored transform can be used as starting point for future scaling or rotations.
   */
  void store_transform() { trf.store(); };

  //! Set the linewidth.
  /*!
    \param linewidth	Linewidth in range 0 to 8. 0 gives a linewidth of 1 pixel at original zoom. 1 -> 2 pixel etc.
  */
  void set_linewidth( int linewidth);

  //! Set fill.
  /*!
    \param fillval	If 1 the object will be draw with fill, if 0 the object will be drawn without fill.
  */
  void set_fill( int fillval);

  //! Set border.
  /*!
    \param borderval	If 1 the object will be draw with border, if 0 the object will be drawn without border.
  */
  void set_border( int borderval);

  //! Set shadow.
  /*!
    \param shadowval	If 1 the object will be draw with shadow, if 0 the object will be drawn without shadow.
  */
  void set_shadow( int shadowval) { shadow = shadowval; draw();}

  //! Set shadow width.
  /*!
    \param width	Width of shadow.
  */
  void set_shadow_width( double width) { shadow_width = width; draw();}

  void get_ctx( void **c) { *c = (void *)ctx;}; 	//!< Should be replace by get_ctx() !!

  //! Moves object to alignment line or point.
  /*!
    \param x	x coordinate of alignment point.
    \param y	y coordinate of alignment point.
    \param direction Type of alignment.
  */
  void align( double x, double y, glow_eAlignDirection direction);

  //! Export the object as a java shape.
  /*!
    \param t		Transform of parent node. Can be zero.
    \param node		Parent node. Can be zero.
    \param pass		Export pass.
    \param shape_cnt	Current index in a shape vector.
    \param node_cnt	Counter used for javabean name. Not used for this kind of object.
    \param in_nc	Member of a nodeclass. Not used for this kind of object.
    \param fp		Output file.

    The object is transformed to the current zoom factor, and GlowExportJBean is used to generate
    java code for the shape.
  */
  void export_javabean( GlowTransform *t, void *node,
	glow_eExportPass pass, int *shape_cnt, int node_cnt, int in_nc, ofstream &fp);

  //! Register the name of the last group
  /*!
    \param name		The name of the group the object until now was a member of.

    The last group is used to recover data for a group that is desolved, when the members are regrouped.
  */
  void set_last_group( char *name) { strcpy( last_group, name); };

  //! Return the name of the last group the object was a member of
  /*!
    \return The name of the last group the object was a member of.
  */
  char *get_last_group() { return last_group; };

  //! Mirror the object around a horizontal or vertical mirror line.
  /*!
    \param x0	x coordinate of mirror line.
    \param y0	y coordinate of mirror line.
    \param dir	Direction of mirror.
  */
  void flip( double x0, double y0, glow_eFlipDirection dir);

  //! Conversion between different versions of Glow
  /*!
    \param version	Version to convert to.
  */
  void convert( glow_eConvert version);

  void export_flow( GlowExportFlow *ef);
  
  //! Destructor
  ~GrowArc();
};

/*@}*/
#endif

