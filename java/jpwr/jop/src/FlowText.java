/* 
 * Proview   $Id: FlowText.java,v 1.2 2005-09-01 14:57:50 claes Exp $
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
import java.awt.*;
import java.awt.geom.*;
import javax.swing.*;


public class FlowText implements FlowArrayElem {
  FlowPoint p;
  int draw_type;
  int text_size;
  String text;
  FlowCmn cmn;

  public FlowText( FlowCmn cmn) {
    this.cmn = cmn;
    p = new FlowPoint(cmn);
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
	case Flow.eSave_Text:
	  break;
	case Flow.eSave_Text_text_size:
	  text_size = new Integer(token.nextToken()).intValue();
	  break;
	case Flow.eSave_Text_draw_type:
	  draw_type = new Integer(token.nextToken()).intValue();
	  break;
	case Flow.eSave_Text_text:
	  text = token.nextToken();
	  break;
	case Flow.eSave_Text_p:
	  p.open( reader);
	  break;
	case Flow.eSave_End:
	  end = true;
	  break;
	default:
	  System.out.println( "Syntax error in FlowText");
	  break;
	}
	if ( end)
	  break;
      }
    } catch ( Exception e) {
      System.out.println( "IOExeption FlowText");
    }
  }

  public void draw( Graphics2D g, FlowPoint p0, String[] annotv, boolean highlight) {
    int tsize;
    int idx = (int)(cmn.zoom_factor / cmn.base_zoom_factor * (text_size +4) - 4);
    if ( idx < 0) return;
    // if ( idx > 8)
    //  idx = 8;

    switch( idx) {
    case 0: tsize = 8; break;
    case 1: tsize = 10; break;
    case 2: tsize = 12; break;
    case 3: tsize = 14; break;
    case 4: tsize = 14; break;
    case 5: tsize = 18; break;
    case 6: tsize = 18; break;
    case 7: tsize = 18; break;
    default: tsize = 3 * idx;
    }
    tsize -= tsize/5;

    Font f = new Font("Helvetica", Font.BOLD, tsize);

    g.setColor( Color.black);
    if ( highlight)
      g.setColor( Color.red);
    g.setFont( f);
    g.drawString( text, (float)((p.x + p0.x) * cmn.zoom_factor), (float)((p.y + p0.y) * cmn.zoom_factor) - tsize/4);
  }

}








