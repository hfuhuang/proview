/* 
 * Proview   $Id: FlowPoint.java,v 1.2 2005-09-01 14:57:50 claes Exp $
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
import java.io.*;
import java.util.*;


public class FlowPoint {
  double x;
  double y;
  FlowCmn cmn;
  
  public FlowPoint( FlowCmn cmn) {
    this.cmn = cmn;
  }

  public void open( BufferedReader reader) {
    String line;
    StringTokenizer token;
    boolean end = false;

    try {
      while( (line = reader.readLine()) != null) {
	token = new StringTokenizer(line);
	int key = new Integer(token.nextToken()).intValue();
	if ( cmn.debug) System.out.println( "line : " + key);
	
	switch ( key) {
	case Flow.eSave_Point:
	  break;
	case Flow.eSave_Point_x:
	  x  = new Double(token.nextToken()).doubleValue();
	  break;
	case Flow.eSave_Point_y:
	  y  = new Double(token.nextToken()).doubleValue();
	  break;
	case Flow.eSave_End:
	  end = true;
	  break;
	default:
	  System.out.println("Syntax error in FlowPoint");
	  break;
	}
	if ( end)
	  break;
      }
    } catch ( Exception e) {
      System.out.println( "IOExeption FlowRect");
    }
  }
}












