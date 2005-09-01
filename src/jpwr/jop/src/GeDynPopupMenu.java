/* 
 * Proview   $Id: GeDynPopupMenu.java,v 1.2 2005-09-01 14:57:50 claes Exp $
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
import java.awt.*;
import java.awt.event.*;

public class GeDynPopupMenu extends GeDynElem {
  String refObject;

  public GeDynPopupMenu( GeDyn dyn, String refObject) {
    super( dyn, GeDyn.mDynType_No, GeDyn.mActionType_PopupMenu);
    this.refObject = refObject;
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
    case GeDyn.eEvent_MB3Press:
      if ( refObject.startsWith("!")) {
	// Name of an attribute that contains the objid of the reference object
       	CdhrObjid reto = dyn.en.gdh.getObjectInfoObjid( refObject.substring(1));
	
	if ( reto.oddSts() && !reto.objid.isNull()) {
	  CdhrString rets = dyn.en.gdh.objidToName( reto.objid, Cdh.mName_volumeStrict);
	  System.out.println( "str: " + rets.str + " " + rets.getSts());
	  if ( rets.oddSts() && ! rets.str.equals(""))
	    new JopMethodsMenu( dyn.session, rets.str, JopUtility.GRAPH, 
				    (Component)dyn.comp, e.getX(), e.getY());
	}
      }
      else {
	new JopMethodsMenu( dyn.session, refObject, JopUtility.GRAPH, 
			    (Component)dyn.comp, e.getX(), e.getY());
      }
      break;
    }
  }
}
