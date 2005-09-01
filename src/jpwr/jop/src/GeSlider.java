/* 
 * Proview   $Id: GeSlider.java,v 1.2 2005-09-01 14:57:50 claes Exp $
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
import java.awt.*;
import java.awt.geom.*;
import java.awt.image.*;
import java.awt.font.*;
import javax.swing.*;
import java.awt.event.*;
import jpwr.rt.*;

public class GeSlider extends JComponent implements JopDynamic, ActionListener{
  Timer timer = new Timer(500, this);
  public Object root;
  public JopEngine en;
  JopSession session;
  public GeDyndata dd = new GeDyndata();
  Point offset = new Point();
  public GeSlider()
  {
    timer.start();
  }
  public void actionPerformed(ActionEvent e) {
    boolean engine_found = false;

    root = getTopLevelAncestor();
    if ( root != null) {
      if ( root instanceof JopApplet)
	session = ((JopApplet)root).session;
      else if ( root instanceof JopFrame)
	session = ((JopFrame) root).session;
      en = session.getEngine();

      if ( !en.isReady())
        return;
      en.add(this);
      engine_found = true;
    }
    if ( engine_found) {
      timer.stop();
      timer = null;    

      if ( en.gdh.isAuthorized( access)) {
        this.addMouseListener(new MouseAdapter() {
          public void mouseClicked(MouseEvent e) {
          }
          public void mousePressed(MouseEvent e) {
            moveActive = true;
//            colorInverse = 1;
//	    repaint();
            offset = e.getPoint();
          }
          public void mouseReleased(MouseEvent e) {
            moveActive = false;
//            colorInverse = 0;
//	    repaint();
          }
        });

        this.addMouseMotionListener( new MouseMotionAdapter() {
          public void mouseDragged(MouseEvent e) {
  	    float value;
	    PwrtStatus sts;
	    Point new_loc = new Point();
            Point ePoint = e.getPoint();
//	    System.out.println("Mouse dragged: " + thisPoint.x + ", " +
//	     thisPoint.y);
            Point loc = getLocation();
            switch ( direction) {
	      case Ge.DIRECTION_RIGHT:
	        new_loc.x = loc.x + ePoint.x - offset.x;
                new_loc.y = loc.y;
                if ( new_loc.x > maxPos)
	          new_loc.x = (int) maxPos;
	        if ( new_loc.x < minPos)
	          new_loc.x = (int) minPos;
                value = (float)((maxPos - new_loc.x) / (maxPos - minPos) *
	        	(maxValue - minValue) + minValue); 
                break;
	      case Ge.DIRECTION_LEFT:
	        new_loc.x = loc.x + ePoint.x - offset.x;
	        new_loc.y = loc.y;
                if ( new_loc.x > maxPos)
	          new_loc.x = (int) maxPos;
	        if ( new_loc.x < minPos)
	          new_loc.x = (int) minPos;
                value = (float)((new_loc.x - minPos) / (maxPos - minPos) * 
	        	(maxValue - minValue) + minValue);
                break;
	      case Ge.DIRECTION_UP:
	        new_loc.y = loc.y + ePoint.y - offset.y;
	        new_loc.x = loc.x;
                if ( new_loc.y > maxPos)
	          new_loc.y = (int) maxPos;
	        if ( new_loc.y < minPos)
	          new_loc.y = (int) minPos;
	        value = (float)((new_loc.y - minPos) / (maxPos - minPos) *
	        	(maxValue - minValue) + minValue);
      System.out.println("old_y: " + ePoint.y + " new_y: " + new_loc.y + "v: " + value);
                break;
	      default:
	        new_loc.y = loc.y + ePoint.y - offset.y;
		System.out.println( "loc.y " + loc.y + " eP.y " + ePoint.y + " offset.y " + offset.y + " new_loc.y " + new_loc.y + " maxPos " + maxPos + " minPos " + minPos);
	        new_loc.x = loc.x;
                if ( new_loc.y > maxPos)
	          new_loc.y = (int) maxPos;
	        if ( new_loc.y < minPos)
	          new_loc.y = (int) minPos;
                value = (float)((maxPos - new_loc.y) / (maxPos - minPos) *
	      		(maxValue - minValue) + minValue);
      System.out.println("old_y: " + ePoint.y + " new_y: " + new_loc.y + "v: " + value);
            }
            setLocation(new_loc);
	
            sts = en.gdh.setObjectInfo( pwrAttribute, value);
            if ( sts.evenSts())
              System.out.println( "setObjectInfoError " + sts);        
          }
          public void mouseMoved(MouseEvent e) {}
        });
      }
    }
  }
  String pwrAttribute = new String();
  public void setPwrAttribute( String pwrAttribute) { this.pwrAttribute = pwrAttribute;}
  public String getPwrAttribute() { return pwrAttribute;}
  int access;
  public void setAccess( int access) {
    this.access = access;
  }
  public int getAccess() {
    return access;
  }
  float minValue = 0;
  float maxValue = 100;
  double minPos = 0;
  double maxPos = 100;
  int direction = Ge.DIRECTION_UP;
  public void setMinValue( float minValue) {
    this.minValue = minValue;
  }
  public float getMinValue() {
    return minValue;
  }
  public void setMaxValue( float maxValue) {
    this.maxValue = maxValue;
  }
  public float getMaxValue() {
    return maxValue;
  }
  public void setMinPos( float minPos) {
    this.minPos = minPos;
  }
  public double getMinPos() {
    return minPos;
  }
  public void setMaxPos( float maxPos) {
    this.maxPos = maxPos;
  }
  public double getMaxPos() {
    return maxPos;
  }
  public void setDirection( int direction) {
    this.direction = direction;
  }
  public int getDirection() {
    return direction;
  }
  public int fillColor = 9999;
  public int originalFillColor = 9999;
  public int borderColor = 9999;
  public int originalBorderColor = 9999;
  public int colorTone = 0;
  public int originalColorTone = 0;
  public int colorShift = 0;
  public int originalColorShift = 0;
  public int colorBrightness = 0;
  public int originalColorBrightness = 0;
  public int colorIntensity = 0;
  public int originalColorIntensity = 0;
  public int colorInverse = 0;
  public int originalColorInverse = 0;
  public void setColorTone( int colorTone) {
    this.colorTone = colorTone;
    originalColorTone = colorTone;
  }
  public int getColorTone() {
    return colorTone;
  }
  public void setColorShift( int colorShift) {
    this.colorShift = colorShift;
    originalColorShift = colorShift;
  }
  public int getColorShift() {
    return colorShift;
  }
  public void setColorBrightness( int colorBrightness) {
    this.colorBrightness = colorBrightness;
    originalColorBrightness = colorBrightness;
  }
  public int getColorBrightness() {
    return colorBrightness;
  }
  public void setColorIntensity( int colorIntensity) {
    this.colorIntensity = colorIntensity;
    originalColorIntensity = colorIntensity;
  }
  public int getColorIntensity() {
    return colorIntensity;
  }
  public void setFillColor( int fillColor) {
    this.fillColor = fillColor;
    this.originalFillColor = fillColor;
  }
  public void resetFillColor() {
    fillColor = originalFillColor;
  }
  public int getFillColor() {
    return fillColor;
  }
  public void setBorderColor( int borderColor) {
    this.borderColor = borderColor;
    this.originalBorderColor = borderColor;
  }
  public int getBorderColor() {
    return borderColor;
  }
  public int animationCount = 1;
  public double rotate;
  public void setRotate( double rotate) {
    if ( rotate < 0)
      this.rotate = rotate % 360 + 360;
    else
      this.rotate = rotate % 360;
  }
  public double getRotate() { return rotate;}
  boolean moveActive = false;
  float valueOld;
  float value;
  boolean firstScan = true;
  GdhrRefObjectInfo retValue = null;
  boolean attrFound = false;
  public Object dynamicGetRoot() {
    return root;
  }
  public void dynamicOpen() {
    if ( pwrAttribute.compareTo("") != 0) {
      retValue = en.gdh.refObjectInfo( pwrAttribute);
      if ( retValue.evenSts())
        System.out.println( "refObjectInfoError retColor");
      else
        attrFound = true;
    }
  }
  public void dynamicClose() {
    if ( attrFound)
      en.gdh.unrefObjectInfo( retValue.refid);
  }
  public void dynamicUpdate( boolean animationOnly) {
    if ( animationOnly)
      return;
    if ( attrFound && !moveActive) {
      value = en.gdh.getObjectRefInfoFloat( retValue.id);
      if ( valueOld != value || firstScan) {
        Point loc = getLocation();
        int pos;
	
	switch ( direction) {
	  case Ge.DIRECTION_RIGHT:
            pos = (int)((maxValue - value)/(maxValue - minValue) *
		(maxPos - minPos) + minPos);
            loc.x = pos;
	    break;
	  case Ge.DIRECTION_LEFT:
            pos = (int)(value /(maxValue - minValue) *
		(maxPos - minPos) + minPos);
            loc.x = pos;
	    break;
	  case Ge.DIRECTION_UP:
            pos = (int)((value - minValue)/(maxValue - minValue) *
		(maxPos - minPos) + minPos);
            loc.y = pos;
	    break;
	  default:
            pos = (int)((maxValue - value)/(maxValue - minValue) *
		(maxPos - minPos) + minPos);
	    loc.y = pos;
	}
	setLocation( loc);
        repaint();
      }
      valueOld = value;
    }
    if ( firstScan)
      firstScan = false;
  }
}






