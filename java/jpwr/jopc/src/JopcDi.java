package jpwr.jopc;
import jpwr.rt.*;
import jpwr.jop.*;
import java.awt.*;
import java.awt.geom.*;
import java.awt.image.*;
import java.awt.font.*;
import javax.swing.*;
import java.awt.event.*;

public class JopcDi extends JopcDiGen implements JopDynamic {
  boolean hold = false;
  int pHold;
  float scanTimeOld = -1F;
  int pScanTime;
  LocalDb ldb;
  public JopcDi( JopSession session, String instance, boolean scrollbar) {
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

  }
  public void dynamicClose() {
    engine.ldb.unrefObjectInfo( pHold);
    engine.ldb.unrefObjectInfo( pScanTime);
  }
  public void dynamicUpdate( boolean animationOnly) {
    if ( animationOnly)
      return;

    boolean holdValue = engine.ldb.getObjectRefInfoBoolean( pHold);
    float scanTime = engine.ldb.getObjectRefInfoFloat( pScanTime);
    if ( holdValue) {
      engine.ldb.setObjectInfo( this, "$local.TrendHold##Boolean", false);
      hold = !hold;
      jopTrend2.setHold(hold);
      if ( hold)
        jopButtontoggle6.tsetFillColor( GeColor.COLOR_115);
      else
        jopButtontoggle6.resetFillColor();
      jopButtontoggle6.repaintForeground();
    }
    if ( scanTime != scanTimeOld) {
      // TODO
      engine.ldb.setObjectInfo( this, "$local.ScanTime##Float32", 100.0F);
      scanTimeOld = scanTime;
    }
    
  }
}







