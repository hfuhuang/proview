/* 
 * Proview   $Id: GeDynIncrAnalog.java,v 1.4 2006-04-24 13:21:46 claes Exp $
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
import java.awt.event.*;

public class GeDynIncrAnalog extends GeDynElem {
  String attribute;
  double increment;
  double min_value;
  double max_value;

  public GeDynIncrAnalog( GeDyn dyn, String attribute, double increment, double min_value,
			  double max_value) {
    super( dyn, GeDyn.mDynType_No, GeDyn.mActionType_IncrAnalog);
    this.attribute = attribute;
    this.increment = increment;
    this.min_value = min_value;
    this.max_value = max_value;
  }
  public void action( int eventType, MouseEvent e) {
    switch ( eventType) {
    case GeDyn.eEvent_MB1Down:
      dyn.comp.setColorInverse( 1);
      dyn.repaintNow = true;
      break;
    case GeDyn.eEvent_MB1Up:
      dyn.comp.setColorInverse( 0);
      dyn.repaintNow = true;
      break;
    case GeDyn.eEvent_MB1Click:
      if ( (dyn.actionType & GeDyn.mActionType_Confirm) != 0)
	break;

      String attrName = dyn.getAttrName( attribute);        
      int typeId = GeDyn.getTypeId( attribute);
      if ( typeId < 0)
	typeId = Pwr.eType_Float32;
      switch ( typeId) {
      case Pwr.eType_Int32: {
	CdhrInt ret = dyn.en.gdh.getObjectInfoInt( attrName);
	if ( ret.evenSts()) {
	  System.out.println( "IncrAnalog " + attrName);
	  break;
	}
	ret.value += (int)increment;
	if ( !( min_value == 0 && max_value == 0)) {
	  if ( ret.value < (int)min_value)
	    ret.value = (int) min_value;
	  if ( ret.value > (int)max_value)
	    ret.value = (int) max_value;
	}
	PwrtStatus sts = dyn.en.gdh.setObjectInfo( attrName, ret.value);
	if ( sts.evenSts())
	  System.out.println( "IncrAnalog " + attrName);
	break;
      }
      default: {
	CdhrFloat ret = dyn.en.gdh.getObjectInfoFloat( attrName);
	if ( ret.evenSts()) {
	  System.out.println( "IncrAnalog " + attrName);
	  break;
	}
	ret.value += increment;
	if ( !( min_value == 0 && max_value == 0)) {
	  if ( ret.value < min_value)
	    ret.value = (float) min_value;
	  if ( ret.value > max_value)
	    ret.value = (float) max_value;
	}
	PwrtStatus sts = dyn.en.gdh.setObjectInfo( attrName, ret.value);
	if ( sts.evenSts())
	  System.out.println( "IncrAnalog " + attrName);
	}
      }
      break;
    }
  }
}




