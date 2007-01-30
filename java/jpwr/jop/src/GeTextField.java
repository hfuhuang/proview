/* 
 * Proview   $Id: GeTextField.java,v 1.10 2007-01-30 13:03:06 claes Exp $
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
import jpwr.jop.*;
import java.awt.event.*;
import java.awt.*;
import javax.swing.*;
import javax.swing.Timer;

public class GeTextField extends JTextField implements GeComponentIfc, 
			       JopDynamic, JopConfirm, ActionListener
{
  public boolean isFocusable() { return root != null;}

  Dimension size;
  Object root;
  public Timer timer = new Timer(500, this);
  JopSession session;
  public JopEngine en;
  public GeDyn dd = new GeDyn( this);

  String undoKey = new String( "Ej i bruk." );
  public boolean focus = false;
  public boolean confirmActive = false;
  public Color  normalColor = null;
  public GeTextField component = this;
  public GeTextField( JopSession session)
  {
    this.session = session;
    dd.setSession( session);
    size = new Dimension( 102, 36);
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

      if ( en.gdh.isAuthorized( dd.access)) {
        this.addKeyListener(new java.awt.event.KeyAdapter() {
          public void keyPressed(KeyEvent e) {
            if ( e.getKeyCode() == KeyEvent.VK_ESCAPE ) {
              keyPressedEvent(e);
            }
          }
        });
        this.addActionListener(new java.awt.event.ActionListener() {
          public void actionPerformed(ActionEvent e) {
	    dd.action( GeDyn.eEvent_ValueChanged, null);
	    if ( (dd.actionType & GeDyn.mActionType_Confirm) != 0)
	      confirmActive = true;

	    // newValueEvent(e);
	    component.getParent().requestFocus();
          }
        });
      }
      this.addFocusListener(new java.awt.event.FocusAdapter() {
        public void focusGained(FocusEvent e) {
	  if ( en.gdh.isAuthorized( dd.access)) {
            focus = true;
            selectAll();
          }
          else
            component.getParent().requestFocus();
        }
        public void focusLost(FocusEvent e) {
	  if ( !confirmActive) {
            focus = false;
	    dd.action( GeDyn.eEvent_FocusLost, null);
	  }
        }
      });
      
      if ( dd.actionType != 0 && en.gdh.isAuthorized( dd.access)) {
        this.addMouseListener(new MouseAdapter() {
          public void mouseReleased(MouseEvent e) {
	    if ( e.isPopupTrigger())
	      dd.action( GeDyn.eEvent_MB3Press, e);
	    else if ((e.getModifiers() & MouseEvent.BUTTON1_MASK) != 0 &&
		     en.gdh.isAuthorized( dd.access))
	      dd.action( GeDyn.eEvent_MB1Up, e);
	  }

          public void mousePressed(MouseEvent e) {
	    if ( e.isPopupTrigger())
	      dd.action( GeDyn.eEvent_MB3Press, e);
	    else if ((e.getModifiers() & MouseEvent.BUTTON1_MASK) != 0 &&
		     en.gdh.isAuthorized( dd.access))
	      dd.action( GeDyn.eEvent_MB1Down, e);
	  }
        });
      }
      
    }
  }
  public void confirmYes() {
    PwrtStatus sts;
    String attrName;

    dd.confirmedAction( GeDyn.eEvent_ValueChanged, null);
    confirmActive = false;
    focus = false;
    dd.action( GeDyn.eEvent_FocusLost, null);
    component.getParent().requestFocus();
  }
  public void confirmNo() {
    focus = false;
    confirmActive = false;
    dd.action( GeDyn.eEvent_FocusLost, null);
    component.getParent().requestFocus();
  }

  // GeComponents Ifc
  public void tsetFillColor( int fillColor) { 

    this.fillColor = fillColor;
    normalColor = GeColor.getColor( fillColor, colorTone,
	 colorShift, colorIntensity, colorBrightness, colorInverse, fillColor, false);
    setBackground( normalColor);
  }
  public void tsetColorTone( int colorTone) {
    this.colorTone = colorTone;
    normalColor = GeColor.getColor( fillColor, colorTone,
	 colorShift, colorIntensity, colorBrightness, colorInverse, GeColor.NO_COLOR, false);
    setBackground( normalColor);
  }
  public void setShadow( int shadow) {}
  public void tsetBorderColor( int borderColor) {}
  public void tsetTextColor( int borderColor) {}
  public void setColorInverse( int colorInverse) {} 
  public void resetFillColor() { 
    fillColor = originalFillColor;
    normalColor = GeColor.getColor( fillColor, colorTone,
	 colorShift, colorIntensity, colorBrightness, colorInverse, fillColor, false);
    setBackground( normalColor);
  }
  public void resetColorTone() { 
    colorTone = originalColorTone;
    normalColor = GeColor.getColor( fillColor, colorTone,
	 colorShift, colorIntensity, colorBrightness, colorInverse, fillColor, false);
    setBackground( normalColor);
  }
  public void resetBorderColor() {}
  public void resetTextColor() {}
  public String getAnnot1() {
    return getText();
  }
  public void setAnnot1( String s) { 
    setText( s);
  }
  public void setLastPage() {}
  public void setFirstPage() {}
  public void setPage( int page) {}
  public int setNextPage() { return 1;}
  public int setPreviousPage() { return 1;}
  public Object getDd() { return dd;}
  public void setFillLevel( float fillLevel) {}
  public void setLevelDirection( int levelDirection) {}
  public void setLevelColorTone( int levelColorTone) {}
  public void setLevelFillColor( int levelFillColor) {}
  public void setVisibility( int visibility) {}

  Font annot1Font = new Font("Helvetica", Font.BOLD, 14);
  public Font annotFont = annot1Font;
  public int annotBackground = 30;
  public void setAnnot1Font( Font font) { annot1Font = font; 
                                          annotFont = font; setFont(font);}
  public Font getAnnot1Font() { return annot1Font;}
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
  public GdhrRefObjectInfo ret = null;
  public StringBuffer sb = new StringBuffer();
  float oldValueF;
  int oldValueI;
  String oldValueS;
  int typeId;
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
    this.annotBackground = fillColor;
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
  public double rotate;
  public void setRotate( double rotate) {
    if ( rotate < 0)
      this.rotate = rotate % 360 + 360;
    else
      this.rotate = rotate % 360;
  }
  public double getRotate() { return rotate;}

  public Object dynamicGetRoot() {
    return root;
  }
  public void dynamicOpen() {
    if ( en.isInstance())
      dd.setInstance( en.getInstance());

    dd.connect();

    normalColor = GeColor.getColor( fillColor, colorTone,
	 colorShift, colorIntensity, colorBrightness, colorInverse, fillColor, false);
    setBackground( normalColor);
  }
  public void dynamicClose() {
    dd.disconnect();
  }
  public void dynamicUpdate( boolean animationOnly) {
    if ( animationOnly || focus)
      return;

    dd.scan();

  }

  void keyPressedEvent(KeyEvent e) {
    if ( e.getKeyCode() == KeyEvent.VK_ESCAPE )
    {
      this.getParent().requestFocus();
    }
  }

  public String getUndoKey() {
    return undoKey;
  }

  public void setUndoKey(String text) {
    undoKey = text;
  }

  public void repaintForeground() {
    Graphics g = getGraphics();
    if ( g == null) {
      System.out.println("repaintForeground: can't get Graphic object");
      return;
    }

    // if ( dd.invisible && !dd.invisibleOld) {
    //   setVisible( false);
    //   dd.invisibleOld = dd.invisible;
    // }
    // else if ( !dd.invisible && dd.invisibleOld) {
    //   setVisible( true);
    //   dd.invisibleOld = dd.invisible;
    // }
    paintComponent(g);
    paintChildren(g);
  }
}















