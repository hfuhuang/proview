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

public class JopIndsoap extends JComponent implements JopDynamic, ActionListener{
  Dimension size;
  Object root;
  JopEngine en;
  Timer timer = new Timer(500, this);
  public JopIndsoap()
  {
    try {
      jbInit();
    }
    catch(Exception e) {
      e.printStackTrace();
    }
  }

  private void jbInit() throws Exception {
    size = new Dimension( 77, 31);
    timer.start();
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
  Font annot1Font = new Font("Helvetica", Font.BOLD, 12);
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
  int original_width = 77;
  int original_height = 31;
  double rotate;
  public void setRotate( double rotate) {
    if ( rotate < 0)
      this.rotate = rotate % 360 + 360;
    else
      this.rotate = rotate % 360;
  }
  public double getRotate() { return rotate;}
  Shape[] shapes = new Shape[] { 
    new Arc2D.Float(52.6171F, 7.07536F, 23.1486F, 22.5806F, 270F, 180F, Arc2D.PIE),
    new Arc2D.Float(6.31986F, 7.07536F, 23.1486F, 22.5806F, 90F, 180F, Arc2D.PIE),
    new Rectangle2D.Float(17.1795F, 7.5496F, 48F, 22F),
    new Rectangle2D.Float(12.9248F, 2.555F, 48F, 22F),
    new Arc2D.Float(2F, 2F, 23.1486F, 22.5806F, 90F, 180F, Arc2D.PIE),
    new Arc2D.Float(48.2973F, 2F, 23.1486F, 22.5806F, 270F, 180F, Arc2D.PIE),
    new Line2D.Float( 59.8715F, 2F, 13.5743F, 2F),
    new Line2D.Float( 59.8715F, 24.5806F, 13.5743F, 24.5806F),
    new Polygon( new int[] { 4, 10, 15, 59, 65, 67, 68, 70, 68, 65, 58, 12, 7, 3}, new int[] {15, 19, 20, 20, 17, 13, 7, 12, 18, 22, 23, 23, 21, 15}, 14),
    new Polygon( new int[] { 8, 11, 15, 41, 37, 18, 11, 8}, new int[] {7, 4, 4, 4, 6, 6, 8, 8}, 8),
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
    g.setColor(GeColor.getColor(13, colorTone,
	 colorShift, colorIntensity, colorBrightness, colorInverse, fillColor));
    g.fill( shapes[0]);
    g.setColor(GeColor.getColor(13, colorTone,
	 colorShift, colorIntensity, colorBrightness, colorInverse, fillColor));
    g.fill( shapes[1]);
    g.setColor(GeColor.getColor(13, colorTone,
	 colorShift, colorIntensity, colorBrightness, colorInverse, fillColor));
    g.fill( shapes[2]);
    g.setColor(GeColor.getColor(96, colorTone,
	 colorShift, colorIntensity, colorBrightness, colorInverse, fillColor));
    g.fill( shapes[3]);
    ((Arc2D)shapes[4]).setArcType(Arc2D.PIE);
    g.setColor(GeColor.getColor(96, colorTone,
	 colorShift, colorIntensity, colorBrightness, colorInverse, fillColor));
    g.fill( shapes[4]);
    ((Arc2D)shapes[4]).setArcType(Arc2D.OPEN);
    g.setStroke( new BasicStroke(1F));
    g.setColor(GeColor.getColor(0, colorTone,
	 colorShift, colorIntensity, colorBrightness, colorInverse, borderColor));
    g.draw( shapes[4]);
    ((Arc2D)shapes[5]).setArcType(Arc2D.PIE);
    g.setColor(GeColor.getColor(96, colorTone,
	 colorShift, colorIntensity, colorBrightness, colorInverse, fillColor));
    g.fill( shapes[5]);
    ((Arc2D)shapes[5]).setArcType(Arc2D.OPEN);
    g.setStroke( new BasicStroke(1F));
    g.setColor(GeColor.getColor(0, colorTone,
	 colorShift, colorIntensity, colorBrightness, colorInverse, borderColor));
    g.draw( shapes[5]);
    g.setStroke( new BasicStroke(1F));
    g.setColor(GeColor.getColor(0, colorTone,
	 colorShift, colorIntensity, colorBrightness, colorInverse, borderColor));
    g.draw( shapes[6]);
    g.setStroke( new BasicStroke(1F));
    g.setColor(GeColor.getColor(0, colorTone,
	 colorShift, colorIntensity, colorBrightness, colorInverse, borderColor));
    g.draw( shapes[7]);
    g.setColor(GeColor.getColor(97, colorTone,
	 colorShift, colorIntensity, colorBrightness, colorInverse, fillColor));
    g.fill( shapes[8]);
    g.setColor(GeColor.getColor(90, colorTone,
	 colorShift, colorIntensity, colorBrightness, colorInverse, fillColor));
    g.fill( shapes[9]);
    g.setColor(Color.black);
    g.setFont( annot1Font);
    save_tmp = g.getTransform();
    g.setRenderingHint(RenderingHints.KEY_ANTIALIASING,RenderingHints.VALUE_ANTIALIAS_OFF);
    g.transform( AffineTransform.getScaleInstance( original_width/width *
      		height/original_height, 1));
    g.drawString( annot1, 14 * original_height / height * width / original_width, 17F);
    g.setTransform( save_tmp);
    g.setTransform(save);
  }
  public Dimension getPreferredSize() { return size;}
  public Dimension getMinimumSize() { return size;}
  int lowTone = 18;
  public void setLowTone( int lowTone) { this.lowTone = lowTone;}
  public int getLowTone() { return lowTone;}
  String pwrAttribute = new String();
  public void setPwrAttribute( String pwrAttribute) { this.pwrAttribute = pwrAttribute;}
  public String getPwrAttribute() { return pwrAttribute;}
  boolean valueColor;
  boolean valueColorOld;
  boolean firstScan = true;
  GdhrRefObjectInfo retColor = null;
  boolean colorAttrFound = false;
  public void dynamicOpen() {
    if ( pwrAttribute.compareTo("") != 0) {
      retColor = en.gdh.refObjectInfo( pwrAttribute);
      if ( retColor.evenSts())
        System.out.println( "refObjectInfoError retColor");
      else
        colorAttrFound = true;
    }
  }
  public void dynamicClose() {
    if ( colorAttrFound)
      en.gdh.unrefObjectInfo( retColor.refid);
  }
  public void dynamicUpdate( boolean animationOnly) {
    if ( animationOnly)
      return;
    if ( colorAttrFound) {
      valueColor = en.gdh.getObjectRefInfoBoolean( retColor.id);
      if ( valueColorOld != valueColor || firstScan) {
        if ( valueColor) {
          colorTone = originalColorTone;
          fillColor = originalFillColor;
          repaint();
        }
        else {
          if ( lowTone <= GeColor.COLOR_TONE_MAX)
            colorTone = lowTone;
          else
            fillColor = lowTone;
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
