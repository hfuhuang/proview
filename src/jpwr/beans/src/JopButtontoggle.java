/* 
 * Proview   $Id: JopButtontoggle.java,v 1.3 2005-09-01 14:57:49 claes Exp $
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

package jpwr.beans;
import jpwr.rt.*;
import jpwr.jop.*;
import java.awt.*;
import java.awt.geom.*;
import java.awt.image.*;
import java.awt.font.*;
import javax.swing.*;
import javax.swing.Timer;
import java.awt.event.*;

public class JopButtontoggle extends JComponent implements JopDynamic, ActionListener{
  Dimension size;
  Object root;
  JopEngine en;
  JopSession session;
  int clickAction = Jop.BUTTON_ACTION_TOGGLE;
  public int getClickAction() { return clickAction;}
  public void setClickAction(int clickAction) { this.clickAction = clickAction;}
  String command = new String();
  public String getCommand() { return command;}
  public void setCommand( String command) { this.command = command;}
  Timer timer = new Timer(500, this);
  public JopButtontoggle()
  {
    try {
      jbInit();
    }
    catch(Exception e) {
      e.printStackTrace();
    }
  }

  private void jbInit() throws Exception {
    size = new Dimension( 102, 36);
    timer.start();
    this.addMouseListener(new MouseAdapter() {
      public void mouseClicked(MouseEvent e) {
        PwrtStatus sts;
        if (clickAction == Jop.BUTTON_ACTION_SET) {
          sts = en.gdh.setObjectInfo( pwrAttribute, true);
          if ( sts.evenSts())
            System.out.println( "setObjectInfoError " + sts);
        }
        else if(clickAction == Jop.BUTTON_ACTION_RESET) {
          sts = en.gdh.setObjectInfo( pwrAttribute, false);
          if ( sts.evenSts())
            System.out.println( "setObjectInfoError " + sts);
        }
        else if(clickAction == Jop.BUTTON_ACTION_TOGGLE) {
          sts = en.gdh.toggleObjectInfo(pwrAttribute);
          if ( sts.evenSts())
            System.out.println( "setObjectInfoError " + sts);
        }
        else if(clickAction == Jop.BUTTON_ACTION_COMMAND) {
          Jop.executeCommand( session, command);
        }
      }
      public void mousePressed(MouseEvent e) {
        colorInverse = 1;
	repaint();
      }
      public void mouseReleased(MouseEvent e) {
        colorInverse = 0;
	repaint();
      }
    });
  }
  public void actionPerformed(ActionEvent e) {
    boolean engine_found = false;
    Container parent = getParent();
    while ( parent != null) {
      if ( parent instanceof JopFrame) {
        en = ((JopFrame)parent).engine;
        if ( !en.isReady())
          break;
        en.add(this);
        root = parent;
	session = ((JopFrame)root).session;
        engine_found = true;
        break;
      }
      parent = parent.getParent();
    }
    if ( !engine_found) {
      parent = getParent();
      while ( parent != null) {
        if ( parent instanceof JopApplet) {
          en = ((JopApplet)parent).engine;
          if ( !en.isReady())
            break;
          en.add(this);
          root = parent;
	  session = ((JopApplet)root).session;
          engine_found = true;
          break;
        }
        parent = parent.getParent();
      }
    }
    if ( engine_found) {
      timer.stop();
      timer = null;    
    }
  }
  String annot1 = new String();
  public String getAnnot1() { return annot1;}
  public void setAnnot1( String s) { annot1 = s;}
  Font annot1Font = new Font("Helvetica", Font.BOLD, 14);
  public void setAnnot1Font( Font font) { annot1Font = font;}
  public Font getAnnot1Font() { return annot1Font;}
  int fillColor = 9999;
  int originalFillColor = 9999;
  int borderColor = 9999;
  int colorTone = 0;
  int originalColorTone = 0;
  int colorShift = 0;
  int originalColorShift = 0;
  int colorBrightness = 0;
  int originalColorBrightness = 0;
  int colorIntensity = 0;
  int originalColorIntensity = 0;
  int colorInverse = 0;
  int originalColorInverse = 0;
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
  }
  public int getBorderColor() {
    return borderColor;
  }
  int original_width = 102;
  int original_height = 36;
  double rotate;
  public void setRotate( double rotate) {
    if ( rotate < 0)
      this.rotate = rotate % 360 + 360;
    else
      this.rotate = rotate % 360;
  }
  public double getRotate() { return rotate;}
  Shape[] shapes = new Shape[] { 
    new Rectangle2D.Float(2F, 2F, 98F, 32F),
    new Polygon( new int[] { 2, 8, 8, 94, 100, 2, 2}, new int[] {34, 28, 8, 8, 2, 2, 34}, 7),
    new Polygon( new int[] { 2, 8, 94, 94, 100, 100, 2}, new int[] {34, 28, 28, 8, 2, 34, 34}, 7),
    new Rectangle2D.Float(2F, 2F, 98F, 32F),
  };
  public void paint(Graphics g1) {
    Graphics2D g = (Graphics2D) g1;
    Component c;
    Point p;
    paintComponent(g);
    for ( int i = 0; i < getComponentCount(); i++) {
      AffineTransform save = g.getTransform();
      c = getComponent(i);
      p = c.getLocation();
      g.translate((int)p.getX(), (int)p.getY());
      c.paint(g);
      g.setTransform(save);
    }
  }
  public void paintComponent(Graphics g1) {
    Graphics2D g = (Graphics2D) g1;
    float width = getWidth();
    float height = getHeight();
    AffineTransform save = g.getTransform();
    AffineTransform save_tmp;
    g.setRenderingHint(RenderingHints.KEY_ANTIALIASING,RenderingHints.VALUE_ANTIALIAS_ON);
    if ( 45.0 <= rotate && rotate < 135.0) {
      g.translate( width, 0.0); 
      g.rotate( Math.PI * rotate/180, 0.0, 0.0);
      g.transform( AffineTransform.getScaleInstance( height/original_width,
      		width/original_height));
    }
    else if ( 135.0 <= rotate && rotate < 225.0)
    {
      g.rotate( Math.PI * rotate/180, width/2, height/2);
      g.transform( AffineTransform.getScaleInstance( width/original_width,
      		height/original_height));
    }
    else if ( 225.0 <= rotate && rotate < 315.0)
    {
      g.translate( -height, 0.0);
      g.rotate( Math.PI * rotate/180, height, 0.0);
      g.transform( AffineTransform.getScaleInstance( height/original_width,
      		width/original_height));
    }
    else
      g.transform( AffineTransform.getScaleInstance( width/original_width,
      		height/original_height));
    g.setColor(GeColor.getColor(22, colorTone,
	 colorShift, colorIntensity, colorBrightness, colorInverse, fillColor, false));
    g.fill( shapes[0]);
    g.setColor(GeColor.getColor(20, colorTone,
	 colorShift, colorIntensity, colorBrightness, colorInverse, fillColor, false));
    g.fill( shapes[1]);
    g.setColor(GeColor.getColor(23, colorTone,
	 colorShift, colorIntensity, colorBrightness, colorInverse, fillColor, false));
    g.fill( shapes[2]);
    g.setStroke( new BasicStroke(1F));
    g.setColor(GeColor.getColor(0, borderColor));
    g.draw( shapes[3]);
    g.setColor(Color.black);
    g.setFont( annot1Font);
    save_tmp = g.getTransform();
    g.transform( AffineTransform.getScaleInstance( original_width/width *
      		height/original_height * 0.75, 1));
    g.drawString( annot1, 13 * original_height / height * width / original_width, 22F);
    g.setTransform( save_tmp);
    g.setTransform(save);
  }
  public Dimension getPreferredSize() { return size;}
  public Dimension getMinimumSize() { return size;}
  int lowColor = 0;
  public void setLowColor( int lowColor) { this.lowColor = lowColor;}
  public int getLowColor() { return lowColor;}
  String pwrAttribute = new String();
  public void setPwrAttribute( String pwrAttribute) { this.pwrAttribute = pwrAttribute;}
  public String getPwrAttribute() { return pwrAttribute;}
  String pwrAttrColor = new String();
  public void setPwrAttrColor( String pwrAttrColor) { this.pwrAttrColor = pwrAttrColor;}
  public String getPwrAttrColor() { return pwrAttrColor;}
  String pwrAttrText = new String();
  public void setPwrAttrText( String pwrAttrText) { this.pwrAttrText = pwrAttrText;}
  public String getPwrAttrText() { return pwrAttrText;}
  String textLow = new String();
  public void setTextLow( String textLow) { this.textLow = textLow;}
  public String getTextLow() { return textLow;}
  String textHigh = new String();
  public void setTextHigh( String textHigh) { this.textHigh = textHigh;}
  public String getTextHigh() { return textHigh;}
  boolean valueColor;
  boolean valueColorOld;
  boolean valueText;
  boolean valueTextOld;
  boolean firstScan = true;
  GdhrRefObjectInfo retColor = null;
  GdhrRefObjectInfo retText = null;
  boolean colorAttrFound = false;
  boolean textAttrFound = false;
  public void dynamicOpen() {
    if ( pwrAttrColor.compareTo("") != 0) {
      retColor = en.gdh.refObjectInfo( pwrAttrColor);
      if ( retColor.evenSts())
        System.out.println( "refObjectInfoError retColor");
      else
        colorAttrFound = true;
    }
    if ( pwrAttrText.compareTo("") != 0) {
      retText = en.gdh.refObjectInfo( pwrAttrText);
      if ( retText.evenSts())
        System.out.println( "refObjectInfoError retText");
      else
        textAttrFound = true;
    }
  }
  public void dynamicClose() {
    if ( colorAttrFound)
      en.gdh.unrefObjectInfo( retColor.refid);
    if ( textAttrFound)
      en.gdh.unrefObjectInfo( retText.refid);
  }
  public void dynamicUpdate( boolean animationOnly) {
    if ( animationOnly)
      return;
    if ( textAttrFound) {
      valueText = en.gdh.getObjectRefInfoBoolean( retText.id);
      if ( valueText != valueTextOld || firstScan) {
        if ( valueText) {
          annot1 = textHigh;
          repaint();
        }
        else {
          annot1 = textLow;
          repaint();
        }
      }
      valueTextOld = valueText;
    }
    if ( colorAttrFound) {
      valueColor = en.gdh.getObjectRefInfoBoolean( retColor.id);
      if ( valueColorOld != valueColor || firstScan) {
        if ( valueColor) {
          fillColor = originalFillColor;
          repaint();
        }
        else {
          fillColor = lowColor;
          repaint();
        }
        valueColorOld = valueColor;
      }
    }
    if ( firstScan)
      firstScan = false;
  }
  public Object dynamicGetRoot() {
    return null;
  }
}
