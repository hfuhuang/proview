package jpwr.jop;
import jpwr.rt.*;
import java.awt.event.*;
import java.awt.*;
import javax.swing.*;

public class GeDynRadioButton extends GeDynElem {
  String attribute;

  boolean attrFound;
  PwrtRefId subid;
  int p;
  boolean inverted;
  boolean oldValue;
  boolean firstScan = true;

  public GeDynRadioButton( GeDyn dyn, String attribute) {
    super( dyn, GeDyn.mDynType_No, GeDyn.mActionType_RadioButton);
    this.attribute = attribute;
  }
  public void connect() {
    String attrName = dyn.getAttrName( attribute);
    if ( attrName.compareTo("") != 0) {
      GdhrRefObjectInfo ret = dyn.en.gdh.refObjectInfo( attrName);
      if ( ret.evenSts())
	System.out.println( "RadioButton: " + attrName);
      else {
	attrFound = true;
	p = ret.id;
	subid = ret.refid;
	inverted = GeDyndata.getAttrInverted( attribute);
      }
    }
  }
  public void disconnect() {
    if ( attrFound)
      dyn.en.gdh.unrefObjectInfo( subid);
  }
  public void scan() {
    if ( !attrFound)
      return;

    boolean value0 = dyn.en.gdh.getObjectRefInfoBoolean( p);

    if ( firstScan) {
      if ( (!inverted && value0) || (inverted && !value0)) { 
        dyn.comp.setLastPage();
        dyn.repaintNow = true;
      }
      oldValue = value0;
      firstScan = false;
    } 

    if ( oldValue != value0) {
      // Shift to first or last page
      if ( (!inverted && value0) || (inverted && !value0))
        dyn.comp.setLastPage();
      else if ( (!inverted && !value0) || (inverted && value0))
        dyn.comp.setFirstPage();
      dyn.repaintNow = true;
    }
    oldValue = value0;
  }

  public void action( int eventType, MouseEvent e) {
    switch ( eventType) {
    case GeDyn.eEvent_MB1Down:
      dyn.comp.setColorInverse( 1);
      dyn.repaintNow = true;
      break;
    case GeDyn.eEvent_MB1Up:
      dyn.comp.setColorInverse( 0);
      dyn.repaintNow = true;
      break;
    case GeDyn.eEvent_MB1Click:
      if ( (dyn.actionType & GeDyn.mActionType_Confirm) != 0)
	break;

      // Get all radiobutton in the same group
      Container parent = ((JComponent)dyn.comp).getParent();
      if ( parent == null) break;

      PwrtStatus sts;
      String attrName;
      for ( int i = 0; i < parent.getComponentCount(); i++) {
	GeComponentIfc c = (GeComponentIfc) parent.getComponent(i);
	if ( (((GeDyn)c.getDd()).actionType & GeDyn.mActionType_RadioButton) != 0) {
	  // Get the attribute
	  for ( int j = 0; j < ((GeDyn)c.getDd()).elements.length; j++) {
	    if ( ((GeDyn)c.getDd()).elements[j].getActionType() == 
		  GeDyn.mActionType_RadioButton) {
	      String name = ((GeDynRadioButton)((GeDyn)c.getDd()).elements[j]).attribute;
	      if ( name.equals( attribute))
	        break;
	      attrName = dyn.getAttrName( name);        
	      sts = dyn.en.gdh.setObjectInfo( attrName, false);
	      break;
	    }
	  }	
	}
      }
      attrName = dyn.getAttrName( attribute);        
      sts = dyn.en.gdh.setObjectInfo( attrName, true);
      if ( sts.evenSts())
        System.out.println( "RadioButton: " + attribute);
      break;
    }
  }
}








