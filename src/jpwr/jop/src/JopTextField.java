
/**
 * Title:        <p>
 * Description:  Ut�ka befintliga JButton till PwrButton.<p>
 * Copyright:    Copyright (c) <p>
 * Company:      <p>
 * @author
 * @version 1.0
 */

import java.awt.event.*;
import java.awt.*;
import javax.swing.*;

public class JopTextField extends JTextField implements JopDynamic
{
  Dimension size;
  Object root;
  Timer timer = new Timer(500, this);
  JopEngine en;
  String pwrAttribute = new String();
  public void setPwrAttribute( String pwrAttribute) { 
    this.pwrAttribute = pwrAttribute;
  }
  public String getPwrAttribute() { 
    return pwrAttribute;
  }

  String undoKey = new String( "Ej i bruk." );
  float minValue = 0;
  float maxValue = 0;
  float alarmLimitLow = 0;
  float alarmLimitHigh = 100000;
  boolean focus = false;
  Color  normalColor = null;
  Color  alarmColor = new Color( 255, 0, 0 );

  public JopTextField()
  {
    try {
      jbInit(f);
    }
    catch(Exception e) {
      e.printStackTrace();
    }
  }

  private void jbInit() throws Exception
  {
    size = new Dimension( 102, 36);
    timer.start();
    this.addKeyListener(new java.awt.event.KeyAdapter() {
      public void keyPressed(KeyEvent e) {
        if ( e.getKeyCode() == e.VK_ESCAPE ) {
          keyPressedEvent(e);
        }
      }
    });
    this.addFocusListener(new java.awt.event.FocusAdapter() {
      public void focusGained(FocusEvent e) {
        focus = true;
      }
      public void focusLost(FocusEvent e) {
        focus = false;
      }
    });
    this.addActionListener(new java.awt.event.ActionListener() {
      public void actionPerformed(ActionEvent e) {
        newValueEvent(e);
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

  boolean firstScan = true;
  GdhrRefObjectInfo retAttr = null;
  boolean attrFound = false;
  float valueAttr;
  float valueAttrOld;

  public void dynamicOpen() {
    if ( pwrAttribute.compareTo("") != 0) {
      retAttr = en.gdh.refObjectInfo( pwrAttribute);
      if ( retColor.evenSts())
        System.out.println( "refObjectInfoError retColor");
      else
        attrFound = true;
    }
  }
  public void dynamicClose() {
    if ( attrFound)
      en.gdh.unrefObjectInfo( retAttr.refid);
  }
  public void dynamicUpdate( boolean animationOnly) {
    if ( animationOnly)
      return;
    boolean valueB = false;

    if ( attrFound && !focus) {
      valueF = en.gdh.getObjectRefInfoFloat( retAttr.id);
      if ( valueAttrOld != valueAttr || firstScan) {
        if ( valueAttr < alarmLimitLow || valueAttr > alarmLimitHigh)
          setBackground( AlarmColor);
        else
          setBackground( normalColor);
        setText(new Float(valueAttr).toString());
        valueAttrOld = valueAttr;
      }
    }
    if ( firstScan)
      firstScan = false;
  }

  void newValueEvent(ActionEvent e) {
    String text = this.getText();
    PwrtStatus sts;

    try {
      float f = Float.parseFloat( text );
      if ( minValue == 0 && maxValue == 0) {
        sts = Gdh.setObjectInfo( pwrAttribute, f );
        if ( sts.evenSts())
          System.out.println( "setObjectInfoError " + sts);
      }
      else {
        if ( f > minValue && f < maxValue ) {
          sts = Gdh.setObjectInfo( pwrAttribute, f );
          if ( sts.evenSts())
            System.out.println( "setObjectInfoError " + sts);
        }
        else {
          // TODO... Information dialog
        }
      }
    }
    catch(NumberFormatException ex) {
      System.out.println( ex.toString() );
    }

    this.getParent().requestFocus();
  }

  void keyPressedEvent(KeyEvent e) {
    System.out.println("Nu har en knapp tryckts ner!");
    if ( e.getKeyCode() == e.VK_ESCAPE )
    {
      this.getParent().requestFocus();
      System.out.println("Nu har en ESC tryckts ner!");
    }
  }

  public String getUndoKey() {
    return undoKey;
  }

  public void setUndoKey(String text) {
    undoKey = text;
  }

  public float getMaxValue() {
    return maxValue;
  }

  public void setMaxValue(float maxValue) {
    this.maxValue = maxValue;
  }

  public float getMinValue() {
    return minValue;
  }

  public void setMinValue(float minValue) {
    this.minValue = minValue;
  }

  public float getAlarmLimitHigh() {
    return alarmLimitHigh;
  }

  public void setAlarmLimitHigh(float alarmLimitHigh) {
    this.alarmLimitHigh = alarmLimitHigh;
  }

  public float getAlarmLimitLow() {
    return alarmLimitLow;
  }

  public void setAlarmLimitLow(float alarmLimitLow)  {
    this.alarmLimitLow = alarmLimitLow;
  }

  public Color getAlarmColor() {
    return alarmColor;
  }

  public void setAlarmColor(Color alarmColor) {
    this.alarmColor = alarmColor;
  }

}
