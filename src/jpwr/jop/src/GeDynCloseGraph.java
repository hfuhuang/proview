/* 
 * Proview   $Id: GeDynCloseGraph.java,v 1.2 2005-09-01 14:57:50 claes Exp $
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
import java.awt.*;
import javax.swing.*;

public class GeDynCloseGraph extends GeDynElem {
  String command;

  public GeDynCloseGraph( GeDyn dyn) {
    super( dyn, GeDyn.mDynType_No, GeDyn.mActionType_CloseGraph);
    this.command = command;
  }
  public void action( int eventType, MouseEvent e) {
    switch ( eventType) {
    case GeDyn.eEvent_MB1Down:
      ((GeComponent)dyn.comp).colorInverse = 1;
      dyn.repaintNow = true;
      break;
    case GeDyn.eEvent_MB1Up:
      ((GeComponent)dyn.comp).colorInverse = 0;
      dyn.repaintNow = true;
      break;
    case GeDyn.eEvent_MB1Click:
      if ( (dyn.actionType & GeDyn.mActionType_Confirm) != 0)
	break;

      JopDynamic comp = (JopDynamic)dyn.comp;
      if ( comp.dynamicGetRoot() instanceof JFrame)
	((JFrame)comp.dynamicGetRoot()).dispose();
      break;
    }
  }
}









