/* 
 * Proview   $Id: JopEngine.java,v 1.3 2005-09-01 14:57:50 claes Exp $
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

/**
 * Title:	<p>
 * Description:	<p>
 * Copyright:	<p>
 * Company	SSAB<p>
 * @author	CS
 * @version	1.0
 */

package jpwr.jop;
import java.util.*;
import javax.swing.Timer;
import java.awt.event.*;
import jpwr.rt.*;

public class JopEngine implements ActionListener {
  int scanTime;
  int animationScanTime;
  Vector updateVector;
  Timer timer;
  public Gdh gdh;
  public LocalDb ldb = null;
  boolean initDone = false;
  boolean ready = false;
  boolean gdhReady = false;
  boolean frameReady = false;
  int lockRejected = 0;
  int scanCount = 0;
  int totalScanCount = 0;
  boolean closingDown = false;
  String instance = null;
   
  public JopEngine( int scantime, Object root) {
    scanTime = scantime;
    animationScanTime = scantime;
    updateVector = new Vector( 50, 50);
    timer = new Timer( scantime, this); 
    timer.start();
    gdh = new Gdh( root);
    gdh.checkUser();
    gdhReady = true;
    if ( frameReady) {
      System.out.println("JopEngine: ready");
      ready = true;
    }
  }

  public void actionPerformed( ActionEvent e) {
    if ( closingDown) {
      close();
      return;
    }
    totalScanCount++;
    if ( scanCount >= scanTime / animationScanTime)
      scanCount = 0;
    scanCount++;
    if ( !initDone) {
      if ( !ready)
        return;
      init();
//      UnLock();
      initDone = true;
    }
    if ( scanCount == 1)
      gdh.getObjectRefInfoAll();
    for ( int i = 0; i < updateVector.size(); i++)
      ((JopDynamic) updateVector.get(i)).dynamicUpdate( scanCount != 1);
    if ( totalScanCount == 100)
      gdh.printStatistics( lockRejected);
  }
 
  public void close() {
      if ( gdh.isBusy()) {
      closingDown = true;
      return;
    }
    timer.stop();

    // This will unref all for applets, not for applications
    gdh.unrefObjectInfoAll();
    for ( int i = 0; i < updateVector.size(); i++) {
      ((JopDynamic) updateVector.get(i)).dynamicClose();
    }
    gdh.close();
  }
  
  public void init() {
    gdh.refObjectInfoList(); 
  }
    
  public void add( Object o) {
    if ( o instanceof JopDynamic) { 
      updateVector.add( o);
      ((JopDynamic) o).dynamicOpen();
    }
  } 

  public void remove( Object o) {
    ((JopDynamic) o).dynamicClose();
    updateVector.remove( o);
  }

  public void removeWindow( Object window) {
    for ( int i = 0; i < updateVector.size(); i++) {
      Object o = updateVector.get(i);
      if ( ((JopDynamic) o).dynamicGetRoot() == window) {
        ((JopDynamic) o).dynamicClose();
	updateVector.remove( i);
	i--;
      }
    }
  }

  public boolean isReady() {
    if ( !ready)
      lockRejected++;
    return ready;
  }

  public boolean Lock() {
    if ( ready) {
      ready = false;
      return true;
    }
    lockRejected++;
    return false;
  }
  
  public void UnLock() {
    ready = true;
  }
  
  public void setFrameReady() {
    frameReady = true;
    if ( gdhReady) {
      System.out.println("JopEngine: ready");
      ready = true;
    }
  }

  public void setScanTime( int scanTime) {
    this.scanTime = scanTime;
    if ( scanTime < animationScanTime) {
      animationScanTime = scanTime;
      timer.setDelay( animationScanTime);
    }
  }

  public void setAnimationScanTime( int animationScanTime) {
    this.animationScanTime = animationScanTime;
    // timer.setDelay( animationScanTime);
    timer.setDelay( scanTime); // Temporary fix...
  }

  public void setInstance( String instance) {
    this.instance = instance;
    if ( ldb == null) {
      ldb = new LocalDb();
    }
  }
  public String getInstance() {
    return instance;
  }
  public boolean isInstance() {
    return ( instance != null);
  }
  public void setLocalDb( LocalDb ldb) {
    if ( this.ldb == null)
      this.ldb = ldb;
  }

}






