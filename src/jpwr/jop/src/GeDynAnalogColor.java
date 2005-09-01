/* 
 * Proview   $Id: GeDynAnalogColor.java,v 1.3 2005-09-01 14:57:50 claes Exp $
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
 */

package jpwr.jop;
import jpwr.rt.*;

public class GeDynAnalogColor extends GeDynElem {
  String attribute;
  double limit;
  int limitType;
  int color;

  boolean attrFound;
  PwrtRefId subid;
  int p;
  public int typeId;
  float oldValueF;
  int oldValueI;
  boolean firstScan = true;
  boolean oldState;
  boolean isMainInstance = false;

  public GeDynAnalogColor( GeDyn dyn, String attribute, 
			   double limit, int limitType, int color) {
    super( dyn, GeDyn.mDynType_AnalogColor, GeDyn.mActionType_No);
    this.attribute = attribute;
    this.limit = limit;
    this.limitType = limitType;
    this.color = color;
  }
  public void connect() {
    // Find the main instance
    GeDynAnalogColor mainInstance = null;
    for ( int i = dyn.elements.length - 1; i >= 0; i--) {
      if ( dyn.elements[i].getDynType() == GeDyn.mDynType_AnalogColor) {
	mainInstance = (GeDynAnalogColor)dyn.elements[i];
	break;
      }
    }

    if ( mainInstance != null) {
      if ( !mainInstance.attrFound) {
        String attrName = dyn.getAttrName( mainInstance.attribute);
        if ( attrName.compareTo("") != 0) {
          GdhrRefObjectInfo ret = dyn.en.gdh.refObjectInfo( attrName);
          if ( ret.evenSts())
	    System.out.println( "AnalogColor: " + attrName);
          else
	    mainInstance.attrFound = true;
	  mainInstance.p = ret.id;
	  mainInstance.subid = ret.refid;
	  mainInstance.typeId = ret.typeId;
	  mainInstance.isMainInstance = true;
        }
      }
      p = mainInstance.p;
      attrFound = mainInstance.attrFound;
      typeId = mainInstance.typeId;
    }
  }
  public void disconnect() {
    if ( isMainInstance && attrFound)
      dyn.en.gdh.unrefObjectInfo( subid);
  }
  public void scan() {
    if ( !attrFound || dyn.ignoreColor)
      return;

    boolean state = false;
    boolean set_color = false;
    boolean reset_color = false;

    if ( typeId == Pwr.eType_Float32) {
      float value = dyn.en.gdh.getObjectRefInfoFloat( p);
      int i;
      if ( !firstScan) {
        if ( !dyn.resetColor && value == oldValueF) {
	  if ( oldState)
	    dyn.ignoreColor = true;
	  return;
        }
      }
      else
        firstScan = false;
  
      switch ( limitType) {
      case GeDyn.eLimitType_Gt:
        state = value > limit;
	break;
      case GeDyn.eLimitType_Lt:
        state = value < limit;
	break;
      case GeDyn.eLimitType_Ge:
        state = value >= limit;
	break;
      case GeDyn.eLimitType_Le:
        state = value >= limit;
	break;
      case GeDyn.eLimitType_Eq:
        state = value == limit;
	break;      
      }
      oldValueF = value;
    }
    else if ( typeId == Pwr.eType_Int32) {
      int value = dyn.en.gdh.getObjectRefInfoInt( p);
      int i;
      if ( !firstScan) {
        if ( !dyn.resetColor && value == oldValueI) {
	  if ( oldState)
	    dyn.ignoreColor = true;
	  return;
        }
      }
      else
        firstScan = false;
  
      switch ( limitType) {
      case GeDyn.eLimitType_Gt:
        state = value > limit;
	break;
      case GeDyn.eLimitType_Lt:
        state = value < limit;
	break;
      case GeDyn.eLimitType_Ge:
        state = value >= limit;
	break;
      case GeDyn.eLimitType_Le:
        state = value >= limit;
	break;
      case GeDyn.eLimitType_Eq:
        state = value == limit;
	break;      
      }
      oldValueI = value;
    }

    if ( state != oldState || dyn.resetColor || firstScan) {
      if ( state) {
	set_color = true;
	dyn.ignoreColor = true;
      }
      else {
	reset_color = true;
	dyn.resetColor = true;
      }
      oldState = state;
    }

    else if ( state)
      dyn.ignoreColor = true;

    if ( !set_color && !reset_color)
      return;

    if ( (dyn.dynType & GeDyn.mDynType_Tone) != 0) {
      if ( set_color) {
	if ( color <= GeColor.COLOR_TONE_MAX)
	  dyn.comp.tsetColorTone( color);
	else
	  dyn.comp.tsetFillColor( color);
	dyn.ignoreColor = true;
      }
      else {
	dyn.comp.resetFillColor();
	dyn.comp.resetColorTone();
	dyn.resetColor = true;
      }
      dyn.repaintNow = true;
    }
    else {
      if ( set_color) {
	dyn.comp.tsetFillColor( color);
	dyn.ignoreColor = true;
      }
      else {
	dyn.comp.resetFillColor();
	dyn.resetColor = true;
      }
      dyn.repaintNow = true;
    }
  }
}




