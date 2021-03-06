/* 
 * Proview   $Id: JopOpWindow.java,v 1.8 2008-06-24 13:35:11 claes Exp $
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
import java.awt.*;
import javax.swing.*;
import java.awt.event.*;

public class JopOpWindow extends JPanel {

  JopSession session;
  JopEngine en;
  Object root;
  JLabel label = null;

  public JopOpWindow( JopSession session, Object root) {
    // super( BoxLayout.Y_AXIS);
    // createGlue();
    this.session = session;
    this.root = root;
    en = session.getEngine();

    CdhrObjid oretWebH = en.gdh.getClassList( Pwrb.cClass_WebHandler);
    if ( oretWebH.evenSts()) return;

    CdhrString sret = en.gdh.objidToName( oretWebH.objid, Cdh.mName_volumeStrict);            
    if ( sret.evenSts()) return;

    String s = sret.str + ".Title";
    CdhrString srettxt = en.gdh.getObjectInfoString( s);
    if ( srettxt.evenSts()) return;

    JLabel llabel = new JLabel( srettxt.str);
    Font f = new Font("Helvetica", Font.BOLD, 24);
    llabel.setFont( f);
    this.add( llabel);

    s = sret.str + ".Text";
    srettxt = en.gdh.getObjectInfoString( s);
    if ( srettxt.evenSts()) return;

    llabel = new JLabel( srettxt.str);
    f = new Font("Helvetica", Font.BOLD, 16);
    llabel.setFont( f);
    this.add( llabel);

    this.add( new JSeparator());

    s = sret.str + ".EnableLogin";
    CdhrInt iret = en.gdh.getObjectInfoInt( s);
    if ( iret.evenSts()) return;

    label = new JLabel();
    this.add( label);

    OpWindButton button;
    if ( iret.value != 0) {

      button = new OpWindButton( session, "", "Login",
				 OpWindButton.LOGIN);
      this.add( button);

      button = new OpWindButton( session, "", "Logout",
				 OpWindButton.LOGOUT);
      this.add( button);
    }
      
    s = sret.str + ".EnableAlarmList";
    iret = en.gdh.getObjectInfoInt( s);
    if ( iret.evenSts()) return;

    if ( iret.value != 0) {
      button = new OpWindButton( session, "", "Alarm and Eventlist",
				 OpWindButton.ALARMLIST);
      this.add( button);
    }
      
    s = sret.str + ".EnableEventLog";
    iret = en.gdh.getObjectInfoInt( s);
    if ( iret.evenSts()) return;

    if ( iret.value != 0) {
      button = new OpWindButton( session, "", "Eventlog",
				 OpWindButton.EVENTLOG);
      this.add( button);
    }
      
    s = sret.str + ".EnableNavigator";
    iret = en.gdh.getObjectInfoInt( s);
    if ( iret.evenSts()) return;

    if ( iret.value != 0) {
      button = new OpWindButton( session, "", "Navigator",
				 OpWindButton.NAVIGATOR);
      this.add( button);
    }
      
    button = new OpWindButton( session, "", "Help",
			       OpWindButton.HELP);
    this.add( button);
    button = new OpWindButton( session, "", "Proview",
			       OpWindButton.PROVIEW);
    this.add( button);
    this.add( new JSeparator());

    CdhrString sretName = null;
    CdhrString sretText = null;
    CdhrString sretURL = null;

    CdhrObjid oret = en.gdh.getChild( oretWebH.objid);
    while ( oret.oddSts()) {
      CdhrClassId retCid = en.gdh.getObjectClass( oret.objid);
      if ( retCid.evenSts()) return;

      switch( retCid.classId) {
        case Pwrb.cClass_WebGraph:
          sret = en.gdh.objidToName( oret.objid, Cdh.mName_volumeStrict);            
          if ( sret.evenSts()) return;

          s = sret.str + ".Name";
          sretName = en.gdh.getObjectInfoString( s);

          s = sret.str + ".Text";
          sretText = en.gdh.getObjectInfoString( s);

 
          button = new OpWindButton( session, sretName.str, sretText.str,
						  OpWindButton.WEBGRAPH);
          this.add( button);
          break;
      }
      oret = en.gdh.getNextSibling( oret.objid);
    }

    this.add( new JSeparator());

    oret = en.gdh.getChild( oretWebH.objid);
    while ( oret.oddSts()) {
      CdhrClassId retCid = en.gdh.getObjectClass( oret.objid);
      if ( retCid.evenSts()) return;

      switch( retCid.classId) {
        case Pwrb.cClass_WebLink:
          sret = en.gdh.objidToName( oret.objid, Cdh.mName_volumeStrict);
          if ( sret.evenSts()) return;

          s = sret.str + ".URL";
          sretURL = en.gdh.getObjectInfoString( s);

          s = sret.str + ".Text";
          sretText = en.gdh.getObjectInfoString( s);

 
          button = new OpWindButton( session, sretURL.str, sretText.str,
						  OpWindButton.WEBLINK);
          this.add( button);
          break;
      }
      oret = en.gdh.getNextSibling( oret.objid);
    }
  }

  class OpWindButton extends JButton {
    public static final int WEBGRAPH = 1;
    public static final int WEBLINK = 2;
    public static final int LOGIN = 3;
    public static final int LOGOUT = 4;
    public static final int NAVIGATOR = 5;
    public static final int ALARMLIST = 6;
    public static final int EVENTLOG = 7;
    public static final int HELP = 8;
    public static final int PROVIEW = 9;
    JopSession session;
    String action;
    int type;
    boolean scrollbar;
    String instance;

    public Dimension getPreferredSize() {
      return new Dimension( 200, 25);
    }
    public Dimension getMininumSize() { return getPreferredSize();}
    public Dimension getMaximumSize() { return getPreferredSize();}

    public OpWindButton( JopSession bsession, String name, String text, int btype) {
      this.session = bsession;
      this.action = name;
      this.type = btype;
      setText( text);
      setHorizontalTextPosition( SwingConstants.LEFT);
      this.addMouseListener(new MouseAdapter() {
        public void mouseReleased(MouseEvent e) {
	  switch ( type) {
	    case NAVIGATOR:
	      if ( ! en.gdh.isAuthorized( Pwr.mAccess_AllPwr))
		break;
	      session.openNavigator( null);
	      break;
	    case LOGIN:
	      session.openLogin();
	      break;
	    case LOGOUT:
	      en.gdh.logout();
	      setLabelText( " ");
	      break;
	    case ALARMLIST:
	      if ( ! en.gdh.isAuthorized( Pwr.mAccess_AllPwr))
		break;
	      session.openAlarmList();
	      break;
	    case EVENTLOG:
	      if ( ! en.gdh.isAuthorized( Pwr.mAccess_AllPwr))
		break;
	      session.openEventLog();
	      break;
	    case HELP:
	      if ( ! en.gdh.isAuthorized( Pwr.mAccess_AllPwr))
		break;
	      session.executeCommand("help index");
	      break;
	    case PROVIEW:
	      if ( ! en.gdh.isAuthorized( Pwr.mAccess_AllPwr))
		break;
	      session.executeCommand("open url \"$pwr_doc/sv_se/index.html\"");
	      break;
	    case WEBGRAPH:
	      if ( ! en.gdh.isAuthorized( Pwr.mAccess_AllPwr))
		break;
	      session.openGraphFrame( action, instance, scrollbar, false);
	      break;
	    case WEBLINK:
	      if ( ! en.gdh.isAuthorized( Pwr.mAccess_AllPwr))
		break;
	      String cmd = "open url \"" + action + "\"";
	      session.executeCommand( cmd);
	      break;
	  }
	  System.out.println( "Action: " + action);
	}
	  });
    }

    public void setScrollbar( boolean scrollbar) {
      this.scrollbar = scrollbar;
    }
    public void setInstance( String instance) {
      this.instance = instance;
    }
  }

  public void setLabelText( String text) {
    label.setText( text);
  }
}













