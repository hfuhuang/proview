package jpwr.jopc;
import jpwr.rt.*;
import jpwr.jop.*;
import java.awt.*;
import java.awt.geom.*;
import java.awt.image.*;
import java.awt.font.*;
import javax.swing.*;
import java.awt.event.*;

public class JopcIo extends JopcIoGen implements JopDynamic {
  boolean slider = false;
  int pSlider;
  boolean hold = false;
  int pHold;
  float scanTimeOld = -1F;
  int pScanTime;
  float presMaxLimitOld = 0;
  float presMinLimitOld = 0;
  int pPresMaxLimit;
  int pPresMinLimit;
  PwrtRefId presMaxLimitSubid;
  PwrtRefId presMinLimitSubid;
  LocalDb ldb;
  public JopcIo( JopSession session, String instance, boolean scrollbar) {
    super( session, instance, scrollbar, true);

    // engine = new JopEngine( 1000, session.getRoot());
    // ldb = new LocalDb();
    // engine.setLocalDb(ldb);
    geInit();

    engine.add(this);

  }
  public Object dynamicGetRoot() {
    return this;
  }
  GeDyn dd;
  public void dynamicOpen() {
    GdhrRefObjectInfo ret;
    ret = engine.ldb.refObjectInfo( this, "$local.SliderDisable##Boolean");
    if ( ret.oddSts())
      pSlider = ret.id;
    else
      System.out.println("$local.Slider not found");

    ret = engine.ldb.refObjectInfo( this, "$local.TrendHold##Boolean");
    if ( ret.oddSts())
      pHold = ret.id;
    else
      System.out.println("$local.Hold not found");

    ret = engine.ldb.refObjectInfo( this, "$local.ScanTime##Float32");
    if ( ret.oddSts())
      pScanTime = ret.id;
    else
      System.out.println("$local.ScanTime not found");

    dd = new GeDyn( null);
    dd.setSession( session);
    dd.setInstance( engine.getInstance()); 

    String attrName = dd.getAttrName( "$object.PresMinLimit##Float32");
    ret = engine.gdh.refObjectInfo( attrName);
    if ( ret.evenSts())
      System.out.println( "JopcAv: " + attrName);
    else {
      presMinLimitSubid = ret.refid;
      pPresMinLimit = ret.id;
    }

    attrName = dd.getAttrName( "$object.PresMaxLimit##Float32");
    ret = engine.gdh.refObjectInfo( attrName);
    if ( ret.evenSts())
      System.out.println( "JopcAv: " + attrName);
    else {
      presMaxLimitSubid = ret.refid;
      pPresMaxLimit = ret.id;
    }

    pwr_slider113.setActionDisabled(true);
  }
  public void dynamicClose() {
    engine.gdh.unrefObjectInfo( presMaxLimitSubid);
    engine.gdh.unrefObjectInfo( presMinLimitSubid);
    engine.ldb.unrefObjectInfo( pSlider);
    engine.ldb.unrefObjectInfo( pHold);
    engine.ldb.unrefObjectInfo( pScanTime);
  }
  public void dynamicUpdate( boolean animationOnly) {
    if ( animationOnly)
      return;

    boolean sliderValue = engine.ldb.getObjectRefInfoBoolean( pSlider);
    boolean holdValue = engine.ldb.getObjectRefInfoBoolean( pHold);
    float scanTime = engine.ldb.getObjectRefInfoFloat( pScanTime);
    float presMinLimit = engine.gdh.getObjectRefInfoFloat( pPresMinLimit);
    float presMaxLimit = engine.gdh.getObjectRefInfoFloat( pPresMaxLimit);
    if ( engine.gdh.isAuthorized( jopButtontoggle10.dd.access) ) {
      if ( sliderValue) {
	engine.ldb.setObjectInfo( this, "$local.SliderDisable##Boolean", false);
	slider = !slider;
	pwr_slider113.setActionDisabled(!slider);
	if ( slider)
	  jopButtontoggle14.tsetFillColor( GeColor.COLOR_115);
	else
	  jopButtontoggle14.resetFillColor();
	jopButtontoggle14.repaintForeground();
      }
    }
    if ( holdValue) {
      engine.ldb.setObjectInfo( this, "$local.TrendHold##Boolean", false);
      hold = !hold;
      jopTrend5.setHold(hold);
      if ( hold)
	jopButtontoggle10.tsetFillColor( GeColor.COLOR_115);
      else
	jopButtontoggle10.resetFillColor();
      jopButtontoggle10.repaintForeground();
    }
    if ( presMaxLimit != presMaxLimitOld || presMinLimit != presMinLimitOld) {
      jopTrend5.setMinValue1(presMinLimit);
      jopTrend5.setMaxValue1(presMaxLimit);
      jopTrend5.reset();
      jopBar4.setMinValue(presMinLimit);
      jopBar4.setMaxValue(presMaxLimit);
      jopBar4.update();

      GeDyn dyn = pwr_slider113.dd;
      for ( int i = 0; i < dyn.elements.length; i++) {
	if ( dyn.elements[i].getActionType() == GeDyn.mActionType_Slider) {
	  GeDynSlider elem = (GeDynSlider)dyn.elements[i];
	  elem.setMinValue( presMinLimit);
	  elem.setMaxValue( presMaxLimit);
	  elem.update();
	}
      }

      presMaxLimitOld = presMaxLimit;
      presMinLimitOld = presMinLimit;
    }
    if ( scanTime != scanTimeOld) {
      // TODO
      engine.ldb.setObjectInfo( this, "$local.ScanTime##Float32", 100.0F);
      scanTimeOld = scanTime;
    }
    
  }
}







