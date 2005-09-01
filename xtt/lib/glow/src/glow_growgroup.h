/* 
 * Proview   $Id: glow_growgroup.h,v 1.3 2005-09-01 14:57:53 claes Exp $
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

#ifndef glow_growgroup_h
#define glow_growgroup_h

#include "glow_grownode.h"

/*! \file glow_growgroup.h
    \brief Contains the GrowGroup class. */
/*! \addtogroup Glow */
/*@{*/


//! Class for handling a group.
/*! A GrowGroup object is an object that is built of other object or groups.
  The GrowGroup class contains function for drawing the object, and handle events when the 
  object is clicked on, moved etc.
*/
class GrowGroup : public GrowNode {
 public:

  //! Constuctor
  /*!
    \param glow_ctx 	The glow context.
    \param name		Name (max 31 char).
    \param array	Array with group member objects.
    \param nodraw	Don't draw the object now.
  */
  GrowGroup( GlowCtx *glow_ctx, char *name, GlowArray& array, 
	       int nodraw = 0);

  //! Noargs constructor.
  GrowGroup() {};

  GrowGroup( GlowCtx *glow_ctx, char *name);
  ~GrowGroup();

  //! Make this object a copy of another image object.
  /*!
    \param n	Object to copy.
  */
  void copy_from( const GrowGroup& n); 

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

  //! Dissolve the group.
  /*! Add the transform of the group to the transformation of all members, and
    store the name of the group as last group in all members.
  */
  void ungroup();

  //! Scan trace
  /*! Calls the trace scan callback for the group and all members.
   */
  void trace_scan();

  //! Init trace
  /*! Calls the trace connect callback for the group and all members.
   */
  int trace_init();

  //! Close trace
  /*! Calls the trace disconnect callback for the group and all members.
   */
  void trace_close();

  //! Add nc for this group to the nodegroup array
  /*!
    \param a	Array where the nc is inserted.
  */
  void get_nodegroups( void *a);

  //! Check if an object is a member of this group.
  /*!
    \param object	Object to check.
    \param group	If the object is found, this group is return here.
  */
  int get_object_group( GlowArrayElem *object, GlowArrayElem **group);

  //! Get list of group members.
  /*!
    \param list		Returned list of members.
    \param size		Number of objects in list.
  */
  void get_objectlist( GlowArrayElem ***list, int *size)
		{ *list = nc->a.a; *size = nc->a.size();};

  //! Find a member object with matching dyntype and coordinates.
  /*!
    \param t		Transform of parent group.
    \param type		Dyntype for the background object.
    \param x		x coordinate for foreground object.
    \param y		y coordinate for foreground object.
    \param background	Found background object.
    \param min		Min limit for background object.
    \param max		Max limit for background object.
    \param direction	Direction of background object.
    \return		Return 1 if the object is found, else 0.
  */
  int get_background_object_limits(GlowTransform *t,
		    glow_eTraceType type,
		    double x, double y, GlowArrayElem **background,
		    double *min, double *max, glow_eDirection *direction);

  //! Find a member object with the specified name.
  /*!
    \param name		Name of object.
    \return		Returns the object with matching name. Return 0 if it isn't found.
  */
  GlowArrayElem *get_node_from_name( char *name);

  //! Redraw all connections connected to member objects of the group.
  virtual void call_redraw_node_cons();

  //! Link member objects in list for connection calculations.
  virtual void link_insert( void **start);

  //! Conversion between different versions of Glow
  /*!
    \param version	Version to convert to.
  */
  void convert( glow_eConvert version);

  //! Set root node.
  /*!
    \param node		Rootnode.
  */
  void set_rootnode( void *node);
};

/*@}*/
#endif









