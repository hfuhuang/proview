/* 
 * Proview   $Id: JopAxis.java,v 1.4 2005-11-04 11:42:05 claes Exp $
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

public class JopAxis extends JComponent {
  Dimension size;
  public JopAxis()
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
  }
  int fillColor = 9999;
  int borderColor = 9999;
  public int originalTextColor = 9999;
  public int textColor = 9999;
  int lineWidth = 1;
  float lineLength;
  int lines = 11;
  int longQuotient = 1;
  int valueQuotient = 1;
  GeCFormat	cFormat;
  Font font;
  public void setFillColor( int fillColor) {
    this.fillColor = fillColor;
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
  public void setTextColor( int textColor) {
    this.textColor = textColor;
    this.originalTextColor = textColor;
  }
  public int getTextColor() {
    return textColor;
  }
  public void setLineWidth( int lineWidth) {
    this.lineWidth = lineWidth;
  }
  public int getLineWidth() {
    return lineWidth;
  }
  public void setLineLength( float lineLength) {
    this.lineLength = lineLength;
  }
  public float getLineLength() {
    return lineLength;
  }
  public void setLines( int lines) {
    this.lines = lines;
  }
  public int getLines() {
    return lines;
  }
  public void setLongQuotient( int longQuotient) {
    this.longQuotient = longQuotient;
  }
  public int getLongQuotient() {
    return longQuotient;
  }
  public void setValueQuotient( int valueQuotient) {
    this.valueQuotient = valueQuotient;
  }
  public int getValueQuotient() {
    return valueQuotient;
  }
  public void setFormat( String format) {
    this.cFormat = new GeCFormat(format);
  }
  public void setFont( Font font) {
    this.font = font;
  }
  public Font getFont() {
    return font;
  }
  public double rotate;
  public void setRotate( double rotate) { this.rotate = rotate;}
  public double getRotate() { return rotate;}
  Shape[] shapes = null;
  Line2D.Float[] hLines;
  public void paint(Graphics g1) {
    super.paint(g1);
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

  float hTextPosX[];
  float hTextPosY[];
  String hText[];
  float oldWidth;
  float widthOrig = 0;
  public void paintComponent(Graphics g1) {
    int i, j;
    Graphics2D g = (Graphics2D) g1;
    float width = getWidth();
    float height = getHeight();
    AffineTransform save = g.getTransform();
    float delta;
    float value;
    boolean drawText = (minValue != maxValue);

    if ( widthOrig == 0)
      widthOrig = width;

    //g.setFont( font);
    g.setFont( font.deriveFont( width / widthOrig * font.getSize()));

    if ( shapes == null || width != oldWidth) {
      float lineLength = this.lineLength * width / widthOrig;
      shapes = new Shape[1];
      FontRenderContext frc = g.getFontRenderContext();

      if ( lines > 0)
      {
        hLines = new Line2D.Float[lines];
	if ( drawText) {
          hText = new String[lines/valueQuotient + 1];
          hTextPosY = new float[lines/valueQuotient + 1];
          hTextPosX = new float[lines/valueQuotient + 1];
	}

        if ( 315 <= rotate || rotate < 45.0) {
          shapes[0] = new Line2D.Float(width, 0F, width, height);
          delta = height / ( lines - 1);
	  for ( i = 0; i < lines; i++) {
            if ( i % longQuotient == 0)
	      hLines[i] = new Line2D.Float( width - lineLength, delta * i, 
					    width, delta * i);
            else
	      hLines[i] = new Line2D.Float( width - 2F/3F*lineLength, 
					  delta * i, width, delta * i);
	    if ( drawText && i % valueQuotient == 0) {
	      if ( maxValue > minValue)
	        value = maxValue - (maxValue - minValue) / ( lines - 1) * i;
              else
	        value = (minValue - maxValue) / ( lines - 1) * i;
	      hText[i/valueQuotient] = 
                 cFormat.format( value, new StringBuffer()).toString();
              Rectangle2D textBounds = 
		  g.getFont().getStringBounds( hText[i/valueQuotient], frc);
              float textHeight = (float) textBounds.getHeight();
              if ( i == lines - 1)
                hTextPosY[i/valueQuotient] = height;
              else if ( i == 0)
                hTextPosY[i/valueQuotient] = 0F + textHeight/2;
              else
                hTextPosY[i/valueQuotient] = delta * i + textHeight/4;
              hTextPosX[i/valueQuotient] = 0F;
            }
          }
        }
        else if ( 45.0 <= rotate && rotate < 135.0) {
          shapes[0] = new Line2D.Float(0F, height, width, height);
          delta = width / ( lines - 1);
	  for ( i = 0; i < lines; i++) {
            if ( i % longQuotient == 0)
	      hLines[i] = new Line2D.Float( delta * i, height - lineLength, 
					    delta * i, height);
            else
	      hLines[i] = new Line2D.Float( delta * i, 
			  height - 2F/3F*lineLength, delta * i, height);
	    if ( drawText && i % valueQuotient == 0) {
	      if ( maxValue > minValue)
	        value = (maxValue - minValue) / ( lines - 1) * i;
              else
	        value = minValue - (minValue - maxValue) / ( lines - 1) * i;
	      hText[i/valueQuotient] = 
                 cFormat.format( value, new StringBuffer()).toString();
              Rectangle2D textBounds = 
		  g.getFont().getStringBounds( hText[i/valueQuotient], frc);
              float textHeight = (float) textBounds.getHeight();
              float textWidth = (float) textBounds.getWidth();
              if ( i == lines - 1)
                hTextPosX[i/valueQuotient] = width - textWidth;
              else if ( i == 0)                
                hTextPosX[i/valueQuotient] = 0F;
              else
                hTextPosX[i/valueQuotient] = delta * i - textWidth/2;
              hTextPosY[i/valueQuotient] = 0F + textHeight * 0.75F;
            }
          }
        }
        else if ( 135.0 <= rotate && rotate < 225.0) {
          shapes[0] = new Line2D.Float(0F, 0F, 0F, height);
          delta = height / ( lines - 1);
	  for ( i = 0; i < lines; i++) {
            if ( i % longQuotient == 0)
	      hLines[i] = new Line2D.Float( 0F, delta * i, 
					    lineLength, delta * i);
            else
	      hLines[i] = new Line2D.Float( 0F, 
			      delta * i, 2F/3F*lineLength, delta * i);
	    if ( drawText && i % valueQuotient == 0) {
	      if ( maxValue > minValue)
	        value = (maxValue - minValue) / ( lines - 1) * i;
	      else
	        value = minValue - (minValue - maxValue) / ( lines - 1) * i;
	      hText[i/valueQuotient] = 
                 cFormat.format( value, new StringBuffer()).toString();
              Rectangle2D textBounds = 
		  g.getFont().getStringBounds( hText[i/valueQuotient], frc);
              float textHeight = (float) textBounds.getHeight();
              if ( i == lines - 1)
                hTextPosY[i/valueQuotient] = height;
              else if ( i == 0)
                hTextPosY[i/valueQuotient] = 0F + textHeight/2;
              else
                hTextPosY[i/valueQuotient] = delta * i + textHeight/4;
              hTextPosX[i/valueQuotient] = lineLength;
            }
          }
        }
        else if ( 225.0 <= rotate && rotate < 315.0) {
          shapes[0] = new Line2D.Float(0F, 0F, width, 0F);
          delta = width / ( lines - 1);
	  for ( i = 0; i < lines; i++) {
            if ( i % longQuotient == 0)
	      hLines[i] = new Line2D.Float( delta * i, 0F,
					    delta * i, lineLength);
            else
	      hLines[i] = new Line2D.Float( delta * i, 
			  0F , delta * i, 2F/3F*lineLength);
	    if ( drawText && i % valueQuotient == 0) {
	      if ( maxValue > minValue)
	        value = maxValue - (maxValue - minValue) / ( lines - 1) * i;
              else
	        value = (minValue - maxValue) / ( lines - 1) * i;
	      hText[i/valueQuotient] = 
                 cFormat.format( value, new StringBuffer()).toString();
              Rectangle2D textBounds = 
		  g.getFont().getStringBounds( hText[i/valueQuotient], frc);
              float textHeight = (float) textBounds.getHeight();
              float textWidth = (float) textBounds.getWidth();
              if ( i == lines - 1)
                hTextPosX[i/valueQuotient] = width - textWidth;
              else if ( i == 0)                
                hTextPosX[i/valueQuotient] = 0F;
              else
                hTextPosX[i/valueQuotient] = delta * i - textWidth/2;
              hTextPosY[i/valueQuotient] = height;
            }
          }
        }
      }
    }
    g.setRenderingHint(RenderingHints.KEY_ANTIALIASING,RenderingHints.VALUE_ANTIALIAS_ON);

    /*
    if ( 45.0 <= rotate && rotate < 135.0) {
      g.translate( -height, 0.0); 
      g.rotate( - Math.PI * rotate/180, height, 0.0);
      g.transform( AffineTransform.getScaleInstance( height/width, width/height));
    }
    else if ( 135.0 <= rotate && rotate < 225.0)
    {
      g.rotate( - Math.PI * rotate/180, width/2, height/2);
    }
    else if ( 225.0 <= rotate && rotate < 315.0)
    {
      g.translate( width, 0.0); 
      g.rotate( - Math.PI * rotate/180, 0.0, 0.0);
      g.transform( AffineTransform.getScaleInstance( height/width, width/height));      
    }
    */
    g.setStroke( new BasicStroke((float)lineWidth));
    g.setColor(GeColor.getColor(0, borderColor));
    g.draw( shapes[0]);
    for ( i = 0; i < lines; i++)
      g.draw( hLines[i]);
    if ( drawText) {
      for ( i = 0; i < lines; i++) {
        if ( i % valueQuotient == 0) {
	  g.drawString( hText[i/valueQuotient], hTextPosX[i/valueQuotient], 
			hTextPosY[i/valueQuotient]);
	}
      } 
    }
    oldWidth = width;
  }
  public Dimension getPreferredSize() { return size;}
  public Dimension getMinimumSize() { return size;}
  float minValue = 0;
  float maxValue = 100;
  public void setMinValue( float minValue) {
    this.minValue = minValue;
  }
  public void setMaxValue( float maxValue) {
    this.maxValue = maxValue;
  }
  public float getMaxValue() {
    return maxValue;
  }
  public float getMinValue() {
    return minValue;
  }
}








