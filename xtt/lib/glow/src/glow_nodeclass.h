#ifndef glow_nodeclass_h
#define glow_nodeclass_h

#include <iostream.h>
#include <fstream.h>
#include "glow_point.h"
#include "glow_rect.h"
#include "glow_array_elem.h"
#include "glow_array.h"


/*! \file glow_nodeclass.h
    \brief Contains the GlowNodeClass class. */
/*! \addtogroup Glow */
/*@{*/


//! Class object for nodes.
/*! The GlowNodeClass object contains the definition of a GrowNode object.
  Every GrowNode belongs to a node class, which contains the elements the node is build of.
  A NodeClass is originally a SubGraph that is saved as a nodeclass in a .pwsg-file. The
  node class contains all the components with its original transform. When it is draw as
  a GrowNode, it is drawn through the GrowNodes transform as well.
*/

class GlowNodeClass : public GlowArrayElem {
 public:

  //! Constructor
  /*!
    \param glow_ctx	Glow context.
    \param name		Name of the nodeclass.
    \param grp		Node group.
  */
  GlowNodeClass( GlowCtx *glow_ctx, char *name, 
	glow_eNodeGroup grp = glow_eNodeGroup_Common);

  //! Copy constructor
  /*!
    \param nc	Nodeclass to copy.
  */
  GlowNodeClass( const GlowNodeClass& nc);

  //! Insert a new element in the nodeclass.
  /*!
    \param element	New element to insert.
  */
  void insert( GlowArrayElem *element) 
		{ a.insert( element);};

  void zoom() { a.zoom();};
  void nav_zoom() { a.nav_zoom();};
  void print_zoom() { a.print_zoom();};
  void traverse( int x, int y) { a.traverse( x, y);};
  void get_borders( double pos_x, double pos_y, double *x_right, 
		double *x_left, double *y_high, double *y_low, void *node)
	{ a.get_borders(pos_x, pos_y, x_right, x_left, y_high, y_low, node);};

  //! Get borders of a node.
  /*!
    \param t		Transform of the node.
    \param x_right	Right border.
    \param x_left	Left border.
    \param y_high	High border.
    \param y_low	Low border.

    Loops trough all elements, which extend the size of the borders, if it exceeds the border.
  */
  void get_borders( GlowTransform *t, double *x_right, 
		      double *x_left, double *y_high, double *y_low);

  void get_obstacle_borders( double pos_x, double pos_y, double *x_right, 
		double *x_left, double *y_high, double *y_low, void *node);
  int	event_handler( void *pos, glow_eEvent event, int x, int y, void *node);

  //! Event handler
  /*!
    \param event	Event.
    \param fx		x coordinate in the nodeclass coordinate system.
    \param fy		y coordinate in the nodeclass coordinate system.

    Checks if any of the elements covers the coordinate.
  */
  int event_handler( glow_eEvent event, double fx, double fy);

  //! Print as postscript. Not used.
  void print( GlowPoint *pos, void *node);

  //! Save the content of the object to file.
  /*!
    \param fp	Ouput file.
    \param mode	Save as graph or subgraph.
  */
  void save( ofstream& fp, glow_eSaveMode mode);

  //! Read the content of the object from file.
  /*! \param fp	Input file. */
  void open( ifstream& fp);

  void draw( GlowPoint *pos, int highlight, int hot, void *node);
  void nav_draw( GlowPoint *pos, int highlight, void *node);
  void draw_inverse( GlowPoint *pos, int hot, void *node);
  void erase( GlowPoint *pos, int hot, void *node);
  void nav_erase( GlowPoint *pos, void *node);

  //! Draw the calling node.
  /*!
    \param t		Transform of the node.
    \param highlight	Draw with highlight colors.
    \param hot		Draw as hot, with larger line width.
    \param node		Parent node. Can be zero.
    \param colornode	The node that controls the color of the object. Can be zero.

    Call the draw fuction for each element.
  */
  void draw( GlowTransform *t, int highlight, int hot, void *node, void *colornode);

  //! Draw the calling node in navigation window.
  /*!
    \param t		Transform of the node.
    \param highlight	Draw with highlight colors.
    \param node		Parent node. Can be zero.
    \param colornode	The node that controls the color of the object. Can be zero.

    Call the draw fuction for each element.
  */
  void nav_draw( GlowTransform *t, int highlight, void *node, void *colornode);

  //! Erase the calling node.
  /*!
    \param t		Transform of the node.
    \param hot		Draw as hot, with larger line width.
    \param node		Parent node. Can be zero.

    Call the erase fuction for each element.
  */
  void erase( GlowTransform *t, int hot, void *node);

  //! Erase the calling node in the navigation window.
  /*!
    \param t		Transform of the node.
    \param node		Parent node. Can be zero.

    Call the erase fuction for each element.
  */
  void nav_erase( GlowTransform *t, void *node);

  int get_conpoint( int num, double *x, double *y, glow_eDirection *dir);

  //! Get info for a connection point.
  /*!
    \param t		Transform of the node.
    \param num		Connection point number for which info should be returned.
    \param flip_horizontal The node is flipped horizontal.
    \param flip_vertical The node is flipped vertical.
    \param x		x coordinate for connection point.
    \param y		y coordinate for connection point.
    \param dir		Direction of the connection point.
  */
  int get_conpoint( GlowTransform *t, int num, bool flip_horizontal,
		      bool flip_vertical, double *x, double *y, glow_eDirection *dir);

  //! Get the object type
  /*! \return The type of the object. */
  glow_eObjectType type() { return glow_eObjectType_NodeClass;};
  void erase_annotation( void *pos, int highlight, int hot, 
	void *node, int num);
  void draw_annotation( void *pos, int highlight, int hot, 
	void *node, int num);

  //! Erase an annotation
  /*!
    \param t		Transform of parent node.
    \param highlight	Node is highlighted.
    \param hot		Node is hot.
    \param node		Parent node. Can be zero.
    \param num		Annotation number.
  */
  void erase_annotation( GlowTransform *t, int highlight, int hot, 
	void *node, int num);

  //! Draw an annotation
  /*!
    \param t		Transform of parent node.
    \param highlight	Node is highlighted.
    \param hot		Node is hot.
    \param node		Parent node. Can be zero.
    \param num		Annotation number.
  */
  void draw_annotation( GlowTransform *t, int highlight, int hot, 
	void *node, int num);

  void configure_annotations( void *pos, void *node);

  //! Get width and height for the text of an annotation.
  /*!
    \param num		Annotation number.
    \param text		Text of the annotation.
    \param width	Width of the text.
    \param height	Hight of the text.
  */
  void measure_annotation( int num, char *text, double *width,
	double *height);

  //! Check if an annotation with this number exist
  /*! \param num	Annotation number. */
  int check_annotation( int num);

  //! Get nodeclass name
  /*! \param name	Returned name. */
  void get_object_name( char *name);

  void open_annotation_input( void *pos, void *node, int num);
  void close_annotation_input( void *node, int num);
  int get_annotation_input( void *node, int num, char **text);
  void move_widgets( void *node, int x, int y);

  //! Set fill color for all nodeclass elements.
  /*! \param drawtype	Color. */
  void set_fill_color( glow_eDrawType drawtype);

  //! Set original fill color for all nodeclass elements.
  /*! \param drawtype	Color. */
  void set_original_fill_color( glow_eDrawType drawtype);

  //! Reset fillcolor to original fillcolor for all nodeclass elements.
  /*! Reset temporary fillcolor at runtime. */
  void reset_fill_color();

  //! Set border color for all nodeclass elements.
  /*! \param drawtype	Color. */
  void set_border_color( glow_eDrawType drawtype);

  //! Set original border color for all nodeclass elements.
  /*! \param drawtype	Color. */
  void set_original_border_color( glow_eDrawType drawtype);

  //! Reset border color to original bordercolor for all nodeclass elements.
  /*! Reset temporary bordercolor at runtime. */
  void reset_border_color();

  //! Set line width.
  /*! \param linewidth	Linewidth. */
  void set_linewidth( int linewidth);

  //! Check if nodeclass is a slider.
  /*! \return Returns 1 if nodeclass i a slider, else 0. */
  int is_slider() { return slider;};

  //! Find and redraw the background rectangle for an annotation
  /*!
    \param t		Transform of node.
    \param node		Node.
    \param x		x coordinate for annotation.
    \param y		y coordinate for annotation.
  */
  int draw_annot_background( GlowTransform *t, void *node, double x, double y);

  //! Find the background rectangle for an annotation
  /*!
    \param t		Transform of node.
    \param node		Node.
    \param background	Returned background object.
  */
  int get_annot_background( GlowTransform *t, void *node,
		glow_eDrawType *background);

  //! Set java name for the nodeclass.
  /*! \param name	Java name. */
  void set_java_name( char *name) { strcpy( java_name, name);};

  //! Get java name for the nodeclass.
  /*!
    \param name		Java name. If no java name is set, the nodeclass name is returned.
    \return		Returns 1 if a java name is set, else 0.
  */
  int get_java_name( char *name);

  //! Get the numbers of all annotations in the nodeclass
  /*!
    \param numbers      Buffer with an array with the annotation numbers.
    \param cnt		Number of annotations.

    The buffer should be freed by the caller.
  */
  void get_annotation_numbers( int **numbers, int *cnt);

  //! Measure the extent in pixel of the nodeclass for the current zoomfactor.
  /*!
    \param pix_x_right		Right border in pixel.
    \param pix_x_left		Left border in pixel.
    \param pix_y_high		High border in pixel.
    \param pix_y_low		Low border in pixel.
  */
  void measure_javabean( double *pix_x_right, double *pix_x_left, 
	double *pix_y_high, double *pix_y_low);

  //! Mark if there are unsaved changes in the nodeclass.
  /*! \param value	1: There are no changes, 0: There are changes. */
  void set_saved( int value) { saved = value;};

  //! Check if this is a next page
  /*! \return 1 if this is a next page, 0 if this is the first page. */
  int is_next() { return prev_nc != 0;};

  //! Get number of pages.
  /*! \return Number of pages for this nodeclass. */
  int get_pages();

  //! Get the list of elements in the nodeclass
  /*!
    \param list		Returns a pointer to the list of elements.
    \param size		Number of elements in list.
  */
  void get_objectlist( GlowArrayElem ***list, int *size)
		{ *list = a.a; *size = a.size();}; 

  //! Get the base nodeclass, i.e. nodeclass of first page.
  /*! \return The base nodeclass. */
  GlowNodeClass *get_base_nc();

  //! Set nodeclass extern or intern.
  /*!
    \param ext	1 set extern, 0 set intern.

    If the nodeclass is extern it is not saved in the Glow context file, and read from
    the subgraph file of the nodeclass, the next time the graph is opened.
  */
  void set_extern( int ext) { nc_extern = ext;};

  //! Return origo for the nodeclass in the coordinate system of transform t.
  /*!
    \param t		Transform for which the coordinate of origo should be returned.
    \param x		x coordinate.
    \param y		y coordinate.
  */
  void get_origo( GlowTransform *t, double *x, double *y);


  //! Conversion between different versions of Glow
  /*!
    \param version	Version to convert to.
  */
  void convert( glow_eConvert version);

  //! Check if an element node has the specified nodeclass as nodeclass.
  /*! \param nodeclass		Nodeclass to check. */
  int find_nc( GlowArrayElem *nodeclass);

  //! Get text size an color of an annotation.
  /*!
    \param node		Node.
    \param num		Annotation number.
    \param t_size	Annotation text size.
    \param t_drawtype Annotation text drawtype.
    \param t_color	Annotation text color.
    \return		0 if annotation doesn't exist, else 1.
  */
  int get_annotation_info( void *node, int num, int *t_size, glow_eDrawType *t_drawtype,
			   glow_eDrawType *t_color);

  GlowCtx	*ctx;		//!< Glow context.
  GlowArray 	a;		//!< Array of nodeclass elements.
  char		nc_name[32];	//!< Name of nodeclass.
  glow_eNodeGroup group;	//!< Group the nodeclass belongs to.
  char 		*dynamic;	//!< Dynamic code.
  int 		dynamicsize;	//!< Size of dynamic code.
  int		arg_cnt;	//!< Number of arguments to dynamic code.
  char		argname[20][32]; //!< Arguments to dynamic code.
  int		argtype[20];	//!< Type of arguments to dynamic code.
  int		nc_extern;	//!< Nodeclass is extern, i.e loaded from pwsg file.
  int		dyn_type;	//!< Dynamic type.
  int		dyn_action_type; //!< Action type.
  glow_eDrawType dyn_color[4];	//!< Dynamic colors.
  int		dyn_attr[4];	//!< Dynamic attributes.
  int		no_con_obstacle; //!< Node of this nodeclass are not obstacles for routed connections.
  int		slider;		//!< Nodeclass i a slider.
  char		java_name[40];	//!< Name of java bean, when the nodeclass is exported as java.
  char		next_nodeclass[40]; //!< Name of nodeclass for the next page.
  int		animation_count; //!< Number of scans in an animation, that this page will be displayed.

  //! I the extension is larger than its elements, y0 is the low border.
  /*! y0 and y1 have special meaning for: 
    1. sliders where y0 corresponds to the value of the slider.
    2. slider backgrounds, where y0 is lower limit and y1 upper limit for the motion of the slider.
    3. nodeclasses with dyntype FillLevel where y0 and y1 are upper and lower limit for the fill level.
  */
  double      	y0;
  double      	y1;		//!< I the extension is larger than its elements, y1 is the high border.
  double      	x0;		//!< I the extension is larger than its elements, x0 is the left border.
  double      	x1;		//!< I the extension is larger than its elements, x1 is the right border.
  GlowArrayElem *next_nc;	//!< Pointer to nodeclass of next page.
  GlowArrayElem *prev_nc;	//!< Pointer to nodeclass of previous page.
  int		saved;		//!< The nodeclass doesn't have any unsaved changes.
  glow_eCycle 	cycle;		//!< Cycle, i.e. if dynamics is executed at fast or slow scantime.
  glow_eInputFocusMark input_focus_mark; //!< How input focus in marked.
};

/*@}*/
#endif







