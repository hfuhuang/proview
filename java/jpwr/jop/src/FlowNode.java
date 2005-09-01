/* 
 * Proview   $Id: FlowNode.java,v 1.2 2005-09-01 14:57:50 claes Exp $
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
import java.io.*;
import java.util.*;
import java.awt.*;
import java.awt.geom.*;
import javax.swing.*;
import java.awt.event.*;


public class FlowNode extends JComponent implements FlowComponent, JopDynamic {
  static final int OFFSET = 2;
  double x_right;
  double x_left;
  double y_high;
  double y_low;
  FlowCmn cmn;
  FlowNodeClass nc;
  FlowPoint pos;
  String n_name;
  String annotv[] = new String[10];
  int annotsize[] = new int[10];
  String trace_object;
  String trace_attribute;
  int trace_attr_type;
  Dimension size;
  boolean highlight;
  boolean select;
  Component component;

  public FlowNode( FlowCmn cmn) {
    component = this;
    this.cmn = cmn;
    pos = new FlowPoint( cmn);
    size = new Dimension(40,40); // .....todo


  }

  public boolean getSelect() {
    return select;
  }
  public void setSelect( boolean select) {
    boolean redraw = (this.select != select);
    this.select = select;
    if ( redraw)
      repaint();
  }

  public String getName() {
    return n_name;
  }
  public String getTraceObject() {
    return trace_object;
  }
  public void setHighlight( boolean highlight) {
    boolean redraw = (this.highlight != highlight);
    this.highlight = highlight;
    if ( redraw)
      repaint();
  }
  public void open( BufferedReader reader) {
    String line;
    StringTokenizer token;
    boolean end = false;
    boolean found = false;
    int i;

    try {
      while( (line = reader.readLine()) != null) {
	token = new StringTokenizer(line);
	int key = new Integer(token.nextToken()).intValue();
	if ( cmn.debug) System.out.println( "line : " + line);

	switch ( key) {
	case Flow.eSave_Node_nc:
	  String nc_name = token.nextToken();
	  found = false;
          for ( i = 0; i < cmn.a_nc.size(); i++) {
	    if ( ((FlowNodeClass)cmn.a_nc.get(i)).nc_name.equals( nc_name)) {
	      nc = (FlowNodeClass) cmn.a_nc.get(i);
	      found = true;
	      break;
	    }
	  }
	  if ( !found)
	    System.out.println( "FlowNode: NodeClass not found: " + nc_name);
	  break;
	case Flow.eSave_Node_n_name:
	  if ( token.hasMoreTokens())
	    n_name = token.nextToken();
          else
	    n_name = new String();
	  break;
	case Flow.eSave_Node_refcon_cnt:
	  for ( i = 0; i < 32; i++)
	    reader.readLine();
	  break;
	case Flow.eSave_Node_x_right:
	  x_right = new Double( token.nextToken()).doubleValue();
	  break;
	case Flow.eSave_Node_x_left:
	  x_left = new Double( token.nextToken()).doubleValue();
	  break;
	case Flow.eSave_Node_y_high:
	  y_high = new Double( token.nextToken()).doubleValue();
	  break;
	case Flow.eSave_Node_y_low:
	  y_low = new Double( token.nextToken()).doubleValue();
	  break;
	case Flow.eSave_Node_annotsize:
	  for ( i = 0; i < 10; i++) {
	    line = reader.readLine();
	    token = new StringTokenizer(line);
	    annotsize[i] = new Integer( token.nextToken()).intValue();
	  }
	  break;
	case Flow.eSave_Node_annotv:
	  // Annotation are surrouded by quotes. A quote inside a
	  // annotation is preceded by a backslash. The size is calculated
	  // without backslashes
	  for ( i = 0; i < 10; i++) {
	    if ( annotsize[i] > 0) {
	      StringBuffer buf = new StringBuffer();
	      char c_old = 0;
	      char c;
	      reader.read();
	      for ( int j = 0; j < annotsize[i]; j++) {
		c = (char) reader.read();
		if ( c == '"') {
		  if ( c_old == '\\') {
		    buf.setLength( buf.length() - 1);
		    j--;
		  }
		  else
		    break;
		}
		buf.append(c);
                c_old = c;
              }
	      annotv[i] = new String( buf);
	      reader.readLine();  // Read linefeed
	    }
	  }
	  break;
	case Flow.eSave_Node_pos:
	  pos.open( reader);
	  break;
	case Flow.eSave_Node_trace_object:
	  if ( token.hasMoreTokens())
	    trace_object = token.nextToken();
	  break;
	case Flow.eSave_Node_trace_attribute:
	  if ( token.hasMoreTokens())
	    trace_attribute = token.nextToken();
	  break;
	case Flow.eSave_Node_trace_attr_type:
	  trace_attr_type = new Integer( token.nextToken()).intValue();
	  break;
	case Flow.eSave_Node_obst_x_right:
	case Flow.eSave_Node_obst_x_left:
	case Flow.eSave_Node_obst_y_high:
	case Flow.eSave_Node_obst_y_low:
	  break;
	case Flow.eSave_End:
	  end = true;
	  break;
	default:
	  System.out.println( "Syntax error in FlowNode");
	  break;
	}
	if ( end)
	  break;
      }
    } catch ( Exception e) {
      System.out.println( "IOException FlowNode");
    }
    if ( nc.group == Flow.eNodeGroup_Common) {
      this.addMouseListener(new MouseAdapter() {
	    public void mouseReleased(MouseEvent e) {
	      if ( e.isPopupTrigger()) {
		new JopMethodsMenu( cmn.session, trace_object, JopUtility.TRACE, 
				    component, e.getX(), e.getY());
		return;
	      }
	    }
	    public void mousePressed(MouseEvent e) {
		System.out.println( "Mouse event" + n_name);
	      if ( e.isPopupTrigger()) {
		new JopMethodsMenu( cmn.session, trace_object, JopUtility.TRACE, 
				    component, e.getX(), e.getY());
		return;
	      }
	      if ( select) {
		setSelect( false);
	      }
	      else {
		cmn.unselect();
		setSelect( true);
	      }
	    }
	    public void mouseClicked(MouseEvent e) {
	      // Detect double click
	      if ( e.getClickCount() == 2) {
                if ( trace_object == null || trace_object.equals(""))
		  return;
                cmn.session.openCrrFrame( trace_object);
	      }
	    }
        });
    }
  }

  public void setBounds() {
      int x = (int)((x_left - cmn.x_left) * cmn.zoom_factor) - Flow.DRAWOFFSET;
      int y = (int)((y_low - cmn.y_low) * cmn.zoom_factor) - Flow.DRAWOFFSET;
      int width = (int)((x_right - x_left) * cmn.zoom_factor) + 2 * Flow.DRAWOFFSET;
      int height = (int)((y_high - y_low) * cmn.zoom_factor) + 2 * Flow.DRAWOFFSET;
      setBounds( x, y, width, height);

  }

  public void paintComponent( Graphics g1) {
    Graphics2D g = (Graphics2D) g1;
    if ( cmn.antiAliasing)
      g.setRenderingHint(RenderingHints.KEY_ANTIALIASING,RenderingHints.VALUE_ANTIALIAS_ON);

    if ( select) {
      // Draw blue background
      Rectangle2D.Double rect = new Rectangle2D.Double( 0, 0,
						   (x_right - x_left) * cmn.zoom_factor + Flow.DRAWOFFSET * 2,
						   (y_high - y_low) * cmn.zoom_factor + Flow.DRAWOFFSET * 2);
      g.setColor( new Color(230,230,255));  // Select color lightblue
      g.fill( rect);
    }

    // Adjust pos to javabean koordinates
    FlowPoint p = new FlowPoint(cmn);
    p.x = pos.x - x_left + ((double) Flow.DRAWOFFSET) / cmn.zoom_factor;
    p.y = pos.y - y_low + ((double) Flow.DRAWOFFSET) / cmn.zoom_factor;
    nc.draw( g, p, annotv, highlight);
  }

  boolean attrFound;
  PwrtRefId subid;
  int refid;  
  boolean oldValue; 
  boolean firstScan;

  public Object dynamicGetRoot() {
    return null;
  }
  public void dynamicOpen() {
    if ( trace_object == null || trace_attribute == null ||
	   trace_object.equals(""))
      return;

    if ( trace_attr_type != Flow.eTraceType_Boolean)
      return;

    String attrName = trace_object + "." + trace_attribute;

    GdhrRefObjectInfo ret = cmn.gdh.refObjectInfo( attrName);
    if ( ret.evenSts())
      System.out.println( "ObjectInfoError " + attrName);
    else {
      attrFound = true;
      refid = ret.id;
      subid = ret.refid;
    }
  }

  public void dynamicClose() {
    if ( attrFound)
      cmn.gdh.unrefObjectInfo( subid);
  }

  public void dynamicUpdate( boolean animationOnly) {
    if ( attrFound) {
      boolean value = cmn.gdh.getObjectRefInfoBoolean( refid);
      if ( value != oldValue || firstScan) {
	setHighlight( value);
	oldValue = value;
      }  
    }
  }

  public Dimension getPreferredSize() { return size;}
  public Dimension getMinimumSize() { return size;}
}







