package jpwr.beans;
import java.awt.*;
import java.awt.geom.*;
import java.awt.image.*;
import java.awt.font.*;
import javax.swing.*;
import jpwr.jop.*;

public class JopCon_v extends JComponent {
  Dimension size;
  public JopCon_v()
  {
    try {
      jbInit();
    }
    catch(Exception e) {
      e.printStackTrace();
    }
  }

  private void jbInit() throws Exception {
    size = new Dimension( 40, 40);
  }
  int borderColor = 9999;
  int lineWidth = 1;
  public void setBorderColor( int borderColor) {
    this.borderColor = borderColor;
  }
  public int getBorderColor() {
    return borderColor;
  }
  public void setLineWidth( int lineWidth) {
    this.lineWidth = lineWidth;
  }
  public int getLineWidth() {
    return lineWidth;
  }
  int original_width = 40;
  int original_height = 40;
  public double rotate;
  public void setRotate( double rotate) { this.rotate = rotate;}
  public double getRotate() { return rotate;}
  Shape[] shapes = new Shape[] { null, null,};
  float widthOld = 0;
  float heightOld = 0;
  double rotateOld = 0;
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
    g.setRenderingHint(RenderingHints.KEY_ANTIALIASING,RenderingHints.VALUE_ANTIALIAS_ON);

    float width = getWidth();
    float height = getHeight();
    if ( shapes[0] == null) {
      shapes[0] = new Line2D.Float();
      shapes[1] = new Line2D.Float();
    }
    if ( width != widthOld || height != heightOld || rotate != rotateOld) {
      if ( 45.0 <= rotate && rotate < 135.0) {
        ((Line2D.Float)shapes[0]).setLine(width - 10F, 10F, width - 10F, height);
        ((Line2D.Float)shapes[1]).setLine(0F, 10F, width - 10F, 10F);
      }
      else if ( 135.0 <= rotate && rotate < 225.0) {
       ((Line2D.Float)shapes[0]).setLine(10F, 10F, width, 10F);
       ((Line2D.Float)shapes[1]).setLine(10F, 10F, 10F, height);
      }
      else if ( 225.0 <= rotate && rotate < 315.0) {
        ((Line2D.Float)shapes[0]).setLine(10F, 0F, 10F, height - 10F);
        ((Line2D.Float)shapes[1]).setLine(10F, height - 10F, width, height - 10F);
      }
      else {
       ((Line2D.Float)shapes[0]).setLine(0F, height - 10F, width - 10F, height - 10F);
       ((Line2D.Float)shapes[1]).setLine(width - 10F, height - 10F, width - 10F, 0F);
      }

      widthOld = width;
      heightOld = height;
      rotateOld = rotate;
    }

    g.setStroke( new BasicStroke((float)lineWidth));
    g.setColor(GeColor.getColor(0, borderColor));
    g.draw( shapes[0]);
    g.draw( shapes[1]);
  }
  public Dimension getPreferredSize() { return size;}
  public Dimension getMinimumSize() { return size;}
}
