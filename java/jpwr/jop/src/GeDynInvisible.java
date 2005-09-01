/* 
 * Proview   $Id: GeDynInvisible.java,v 1.3 2005-09-01 14:57:50 claes Exp $
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

public class GeDynInvisible extends GeDynElem {
  String attribute;
  int dimmed;

  boolean attrFound;
  PwrtRefId subid;
  int typeId;
  int p;
  boolean inverted;
  boolean oldValueB;
  String oldValueS;
  boolean firstScan = true;
  boolean cmd = false;
  boolean valueCmd = false;

  public GeDynInvisible( GeDyn dyn, String attribute, int dimmed) {
    super( dyn, GeDyn.mDynType_Invisible, GeDyn.mActionType_No);
    this.attribute = attribute;
    this.dimmed = dimmed;
  }
  public void connect() {
    String attrName = dyn.getAttrName( attribute);
    if ( attrName.compareTo("") != 0) {
      if ( attrName.toLowerCase().startsWith("$cmd(")) {
	cmd = true;
	valueCmd = false;
      }
      else {
        GdhrRefObjectInfo ret = dyn.en.gdh.refObjectInfo( attrName);
        if ( ret.evenSts())
	  System.out.println( "DigInvisible: " + attrName);
        else {
	  attrFound = true;
	  p = ret.id;
	  subid = ret.refid;
	  typeId = ret.typeId;
	  inverted = GeDyndata.getAttrInverted( attribute);
        }
      }
    }
  }
  public void disconnect() {
    if ( attrFound)
      dyn.en.gdh.unrefObjectInfo( subid);
  }
  public void scan() {
    if ( cmd) {
      if ( firstScan) {
        if ( dimmed == 0)
          dyn.comp.setVisibility( Ge.VISIBILITY_INVISIBLE);
        else
          dyn.comp.setVisibility( Ge.VISIBILITY_DIMMED);
	firstScan = false;
      }
      return;
    }
    if ( !attrFound)
      return;
    switch ( typeId) {
    case Pwr.eType_Boolean: {
      boolean value = dyn.en.gdh.getObjectRefInfoBoolean( p);
      if ( !firstScan) {
        if ( value == oldValueB) {
	  // No change since last time
	  return;
        }
      }
      else
        firstScan = false;

      if ( (!inverted && value) || (inverted && !value)) {
        if ( dimmed == 0)
          dyn.comp.setVisibility( Ge.VISIBILITY_INVISIBLE);
        else
          dyn.comp.setVisibility( Ge.VISIBILITY_DIMMED);
      }
      else {
        dyn.comp.setVisibility( Ge.VISIBILITY_VISIBLE);
      }
      dyn.repaintNow = true;
      oldValueB = value;
      break;
    }
    case Pwr.eType_String: {
      String value0;
      value0 = dyn.en.gdh.getObjectRefInfoString( p, typeId);

      if ( firstScan || value0.compareTo( oldValueS) != 0) {
        if ( value0.compareTo("") == 0) {
          if ( dimmed == 0)
            dyn.comp.setVisibility( Ge.VISIBILITY_INVISIBLE);
          else
            dyn.comp.setVisibility( Ge.VISIBILITY_DIMMED);
        }
        else {
          dyn.comp.setVisibility( Ge.VISIBILITY_VISIBLE);
	}
        dyn.repaintNow = true;
	oldValueS = value0;
	if ( firstScan)
	  firstScan = false;
      }
      break;
    }
    }
  }
}








