
package jpwr.jop;
import jpwr.rt.*;
import java.awt.*;
import javax.swing.*;
import javax.swing.event.*;
import java.awt.event.*;

public class JopMethodsMenu implements ActionListener, PopupMenuListener,
       JopDynamic {

  JPopupMenu popup;
  JopSession session;
  Gdh gdh;
  String object;
  PwrtObjid objid;
  int classid;
  int utility;

  JopMethodsMenu( JopSession session, String object, int utility, 
		  Component invoker, int x, int y) {
    this.session = session;
    gdh = session.getEngine().gdh;
    this.object = object;
    this.utility = utility;

    if ( object == null)
      return;
    CdhrObjid oret = gdh.nameToObjid( object);
    if ( oret.evenSts()) return;

    objid = oret.objid;

    CdhrClassId cret = gdh.getObjectClass( objid);
    if ( cret.evenSts()) return;

    classid = cret.classId;

    popup = new JPopupMenu();

    JMenuItem item;

    if ( openGraphFilter()) {
      popup.add( item = new JMenuItem( "Graph"));
      item.addActionListener( this);
    }

    if ( helpFilter()) {
      popup.add( item = new JMenuItem( "Help"));
      item.addActionListener( this);
    }

    if ( dataSheetFilter()) {
      popup.add( item = new JMenuItem( "DataSheet"));
      item.addActionListener( this);
    }

    if ( navigatorFilter()) {
      popup.add( item = new JMenuItem( "Navigator"));
      item.addActionListener( this);
    }

    if ( openTraceFilter()) {
      popup.add( item = new JMenuItem( "Open Plc"));
      item.addActionListener( this);
    }

    if ( openCrossrefFilter()) {
      popup.add( item = new JMenuItem( "Crossreferences"));
      item.addActionListener( this);
    }

    if ( classHelpFilter()) {
      popup.add( item = new JMenuItem( "Class Help"));
      item.addActionListener( this);
    }
    popup.addPopupMenuListener( this);
    popup.show( invoker, x,  y);
    session.getEngine().add(this);
  }

  public void popupMenuWillBecomeVisible( PopupMenuEvent e) {}
  public void popupMenuWillBecomeInvisible( PopupMenuEvent e) {}
  public void popupMenuCanceled( PopupMenuEvent e) {}

  public void actionPerformed( ActionEvent event) {

    if ( event.getActionCommand().equals("Navigator")) {
      navigator();
    }
    else if ( event.getActionCommand().equals("Open Plc")) {
      openTrace();
    }
    else if ( event.getActionCommand().equals("Graph")) {
      openGraph();
    }
    else if ( event.getActionCommand().equals("Help")) {
      help();
    }
    else if ( event.getActionCommand().equals("DataSheet")) {
      dataSheet();
    }
    else if ( event.getActionCommand().equals("Crossreferences")) {
      openCrossref();
    }
    else if ( event.getActionCommand().equals("Class Help")) {
      classHelp();
    }
  }  

  //
  // Methods and filter metods for object
  //
  public boolean navigatorFilter() {
    if ( utility == JopUtility.NAVIGATOR)
      return false;

    return true;
  }
  public void navigator() {
    session.openNavigator( objid);
  }

  public boolean openCrossrefFilter() {
    return true;
  }
  public void openCrossref() {
    session.openCrrFrame( object);
  }

  public boolean openTraceFilter() {
    if ( utility == JopUtility.TRACE)
      return false;

    if ( classid == Pwrb.cClass_plc ||
	 classid == Pwrb.cClass_windowplc ||
	 classid == Pwrb.cClass_windowsubstep ||
	 classid == Pwrb.cClass_windoworderact ||
	 classid == Pwrb.cClass_windowplc)
      return true;

    CdhrObjid pret = gdh.getParent( objid);
    if ( pret.evenSts())
      return false;

    CdhrClassId cret = gdh.getObjectClass( pret.objid);

     if ( cret.evenSts()) return false;

    if ( cret.classId == Pwrb.cClass_windowplc ||
	 cret.classId == Pwrb.cClass_windowsubstep ||
	 cret.classId == Pwrb.cClass_windoworderact ||
	 cret.classId == Pwrb.cClass_windowplc)
      return true;

    return false;
  }

  public void openTrace() {
    if ( classid == Pwrb.cClass_windowplc ||
	 classid == Pwrb.cClass_windowsubstep ||
	 classid == Pwrb.cClass_windoworderact ||
	 classid == Pwrb.cClass_windowplc) {
      session.openFlowFrame( objid, null);
    }
    else if ( classid == Pwrb.cClass_plc) {
      // Open child 
      CdhrObjid child = gdh.getChild( objid);
      if ( child.evenSts()) return;

      CdhrClassId cret = gdh.getObjectClass( child.objid);
      if ( cret.evenSts()) return;

      if ( cret.classId == Pwrb.cClass_windowplc ||
	   cret.classId == Pwrb.cClass_windowsubstep ||
	   cret.classId == Pwrb.cClass_windoworderact ||
	   cret.classId == Pwrb.cClass_windowplc) {
        session.openFlowFrame( child.objid, null);
      }
    }
    else {
      // Open parent window and center object
      CdhrObjid parent = gdh.getParent( objid);
      if ( parent.evenSts()) return;

      int idx = object.lastIndexOf('-');
      if ( idx == -1) return;
      String name = object.substring( idx + 1);

      session.openFlowFrame( parent.objid, name);
    }
  }

  public boolean openGraphFilter() {
    if ( utility == JopUtility.GRAPH) {
      // Check caller name...
    }

    String attr = object + ".DefGraph";
    CdhrString sret = gdh.getObjectInfoString( attr);
    if ( sret.evenSts())
      return false;
    if ( sret.str.equals(""))
      return false;

    return true;
  }
  public void openGraph() {
    String attr = object + ".DefGraph";
    CdhrString xttgraph = gdh.getObjectInfoString( attr);
    if ( xttgraph.evenSts()) return;

    attr = xttgraph.str + ".Action";
    CdhrString actionRet = gdh.getObjectInfoString( attr);
    if ( actionRet.evenSts()) return;

    int idx = actionRet.str.lastIndexOf( '.');
    String action;
    if ( idx != -1)
      action = actionRet.str.substring(0,1).toUpperCase() +
	actionRet.str.substring(1, idx);
    else
      action = actionRet.str.substring(0,1).toUpperCase() +
	actionRet.str.substring(1);

    String instance = null;
    attr = xttgraph.str + ".Object";
    CdhrString instanceRet = gdh.getObjectInfoString( attr);
    if ( instanceRet.oddSts() && ! instanceRet.str.equals(""))
      instance = instanceRet.str;      

    session.openGraphFrame( action, instance, false);
  }

  public boolean helpFilter() {
    String attr = object + ".HelpTopic";
    CdhrString sret = gdh.getObjectInfoString( attr);
    if ( sret.evenSts() || sret.str.equals(""))
      return false;

    return true;
  }
  public void help() {
    String attr = object + ".HelpTopic";
    CdhrString sret = gdh.getObjectInfoString( attr);
    if ( sret.evenSts()) return;

    String cmd = "help " + sret.str;
    session.executeCommand( cmd);
  }

  public boolean dataSheetFilter() {
    String attr = object + ".DataSheet";
    CdhrString sret = gdh.getObjectInfoString( attr);
    if ( sret.evenSts() || sret.str.equals(""))
      return false;

    return true;
  }
  public void dataSheet() {
    String attr = object + ".DataSheet";
    CdhrString sret = gdh.getObjectInfoString( attr);
    if ( sret.evenSts()) return;

    String cmd = "open url \"" + sret.str + "\"";
    System.out.println( "dataSheet: " + cmd);
    session.executeCommand( cmd);
  }

  public boolean classHelpFilter() {
    return true;
  }
  public void classHelp() {
    CdhrObjid oret = gdh.classIdToObjid( classid);
    if ( oret.evenSts()) return;

    CdhrString sret = gdh.objidToName( oret.objid, Cdh.mName_volumeStrict); 
    if ( sret.evenSts()) return;

    int idx = sret.str.lastIndexOf(":");
    if ( idx == -1) return;

    String volume = sret.str.substring( 0, idx);
    idx = sret.str.lastIndexOf("-");
    if ( idx == -1) return;

    String className = sret.str.substring(idx + 1);

    String url = volume.toLowerCase() + "_" + className.toLowerCase() + ".html";
    String cmd = "open url \"" + url + "\"";
    session.executeCommand( cmd);
  }

  public boolean isVisible() {
    return popup.isVisible();
  }
  public void repaint() {
    popup.repaint();
  }

  public void dynamicUpdate( boolean animationOnly) {
    if ( !isVisible())
      session.getEngine().remove(this);
    else
      repaint();
  }
  public void dynamicOpen() {}
  public void dynamicClose() {}
  public Object dynamicGetRoot() {
    return this;
  }

}











