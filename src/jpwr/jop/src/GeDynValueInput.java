/* 
 * Proview   $Id: GeDynValueInput.java,v 1.4 2005-09-01 14:57:50 claes Exp $
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
import javax.swing.*;

public class GeDynValueInput extends GeDynElem {
  double minValue;
  double maxValue;

  boolean attrFound = false;
  int typeId;
  GeDynValue valueElement;

  public GeDynValueInput( GeDyn dyn, double minValue, double maxValue) {
    super( dyn, GeDyn.mDynType_No, GeDyn.mActionType_ValueInput);
    this.minValue = minValue;
    this.maxValue = maxValue;
  }
  public void connect() {
    // Find the value element
    if ( dyn.elements == null)
      return;

    for ( int i = 0; i < dyn.elements.length; i++) {
      if ( dyn.elements[i].getDynType() == GeDyn.mDynType_Value) {
	valueElement = (GeDynValue)dyn.elements[i];
	typeId = valueElement.typeId;
	attrFound = valueElement.attrFound;
	break;
      }
    }
    if ( !attrFound)
      System.out.println("ValueInput: attribute not found");
  }
  public void disconnect() {
  }
  public void action( int eventType, MouseEvent e) {
    switch ( eventType) {
    case GeDyn.eEvent_FocusLost: {
      if (!attrFound)
	break;

      valueElement.firstScan = true;
      break;
    }
    case GeDyn.eEvent_ValueChanged: {
      if (!attrFound)
	break;

      if ( (dyn.actionType & GeDyn.mActionType_Confirm) != 0)
	break;

      String text = ((JTextField)dyn.comp).getText();
      PwrtStatus sts;

      try {
	if ( typeId == Pwr.eType_Float32) {
	  float inputValue = Float.parseFloat( text.trim());
	  valueElement.oldValueF = inputValue;
	  if ( minValue == 0 && maxValue == 0) {
	    String attrName = dyn.getAttrNameNoSuffix( valueElement.attribute);
	    if ( !valueElement.localDb)
	      sts = dyn.en.gdh.setObjectInfo( attrName, inputValue);
	    else
	      sts = dyn.en.ldb.setObjectInfo( dyn.comp.dynamicGetRoot(), attrName, inputValue);
	    if ( sts.evenSts())
	      System.out.println( "setObjectInfoError " + sts);
	  }
	  else {
	    if ( inputValue >= minValue && inputValue <= maxValue ) {
	      String attrName = dyn.getAttrNameNoSuffix( valueElement.attribute);        
	      if ( !valueElement.localDb)
	        sts = dyn.en.gdh.setObjectInfo( attrName, inputValue);
	      else
	        sts = dyn.en.ldb.setObjectInfo( dyn.comp.dynamicGetRoot(), attrName, inputValue);
	      if ( sts.evenSts())
		System.out.println( "setObjectInfoError " + attrName + " " + sts);
	    }
	    else
	      valueElement.oldValueF = -10000;
	  }
	}
	else if ( typeId == Pwr.eType_Int32 ||
		  typeId == Pwr.eType_UInt32 ||
		  typeId == Pwr.eType_Int16 ||
		  typeId == Pwr.eType_UInt16 ||
		  typeId == Pwr.eType_Int8 ||
		  typeId == Pwr.eType_UInt8) {
	  int inputValue = Integer.parseInt( text.trim(), 10);
	  valueElement.oldValueI = inputValue;
	  if ( minValue == 0 && maxValue == 0) {
	    String attrName = dyn.getAttrNameNoSuffix( valueElement.attribute);        
	    if ( !valueElement.localDb)
	      sts = dyn.en.gdh.setObjectInfo( attrName, inputValue);
	    else
	      sts = dyn.en.ldb.setObjectInfo( dyn.comp.dynamicGetRoot(), attrName, inputValue);
	    if ( sts.evenSts())
	      System.out.println( "setObjectInfoError " + sts);
	  }
	  else {
	    if ( inputValue >= minValue && inputValue <= maxValue ) {
	      String attrName = dyn.getAttrNameNoSuffix( valueElement.attribute);        
	      if ( !valueElement.localDb)
	        sts = dyn.en.gdh.setObjectInfo( attrName, inputValue);
	      else
	        sts = dyn.en.ldb.setObjectInfo( dyn.comp.dynamicGetRoot(), attrName, inputValue);
	      if ( sts.evenSts())
		System.out.println( "setObjectInfoError " + sts);
	    }
	    else
	      valueElement.oldValueI = -10000;
	  }
	}
	else if ( typeId == Pwr.eType_Boolean) {
	  int inputValueInt = Integer.parseInt( text.trim(), 10);
	  boolean inputValue;
	  if ( inputValueInt == 0)
	    inputValue = false;
	  else if ( inputValueInt == 1)
	    inputValue = true;
          else
	    break;

	  valueElement.oldValueB = inputValue;
	  String attrName = dyn.getAttrNameNoSuffix( valueElement.attribute);
	  if ( !valueElement.localDb)
	    sts = dyn.en.gdh.setObjectInfo( attrName, inputValue);
	  else
	    sts = dyn.en.ldb.setObjectInfo( dyn.comp.dynamicGetRoot(), attrName, inputValue);
	  if ( sts.evenSts())
	    System.out.println( "setObjectInfoError " + sts);
	}
	else if ( typeId == Pwr.eType_String) {
	  valueElement.oldValueS = text;
	  String attrName = dyn.getAttrNameNoSuffix( valueElement.attribute);        
	  if ( !valueElement.localDb)
	    sts = dyn.en.gdh.setObjectInfo( attrName, text);
	  else
	    sts = dyn.en.gdh.setObjectInfo( attrName, text);
	  if ( sts.evenSts())
	    System.out.println( "setObjectInfoError " + sts);
	}
      }
      catch(NumberFormatException ex) {
	  System.out.println( ex.toString() );
      }
      break;
    }
    }
  }
}









