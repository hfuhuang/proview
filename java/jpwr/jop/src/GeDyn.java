/* 
 * Proview   $Id: GeDyn.java,v 1.9 2006-06-14 05:06:05 claes Exp $
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
import java.awt.event.*;

public class GeDyn {
    public static final int mDynType_No			= 0;
    public static final int mDynType_Inherit		= 1 << 0;
    public static final int mDynType_Tone		= 1 << 1;
    public static final int mDynType_DigLowColor       	= 1 << 2;
    public static final int mDynType_DigColor		= 1 << 3;
    public static final int mDynType_DigError		= 1 << 4;
    public static final int mDynType_DigWarning		= 1 << 5;
    public static final int mDynType_DigFlash		= 1 << 6;
    public static final int mDynType_Invisible		= 1 << 7;
    public static final int mDynType_DigBorder		= 1 << 8;
    public static final int mDynType_DigText		= 1 << 9;
    public static final int mDynType_Value		= 1 << 10;
    public static final int mDynType_AnalogColor       	= 1 << 11;
    public static final int mDynType_Rotate		= 1 << 12;
    public static final int mDynType_Move	       	= 1 << 13;
    public static final int mDynType_AnalogShift       	= 1 << 14;
    public static final int mDynType_DigShift		= 1 << 15;
    public static final int mDynType_Animation		= 1 << 16;
    public static final int mDynType_Bar	       	= 1 << 17;
    public static final int mDynType_Trend		= 1 << 18;
    public static final int mDynType_SliderBackground	= 1 << 19;
    public static final int mDynType_Video		= 1 << 20;
    public static final int mDynType_FillLevel		= 1 << 21;
    public static final int mDynType_FastCurve		= 1 << 22;
    public static final int mDynType_AnalogText		= 1 << 23;
    public static final int mDynType_Table		= 1 << 24;
    public static final int mDynType_StatusColor       	= 1 << 25;
    public static final int mDynType_HostObject       	= 1 << 26;

    public static final int mActionType_No		= 0;
    public static final int mActionType_Inherit		= 1 << 0;
    public static final int mActionType_PopupMenu      	= 1 << 1;
    public static final int mActionType_SetDig		= 1 << 2;
    public static final int mActionType_ResetDig       	= 1 << 3;
    public static final int mActionType_ToggleDig      	= 1 << 4;
    public static final int mActionType_StoDig		= 1 << 5;
    public static final int mActionType_Command		= 1 << 6;
    public static final int mActionType_CommandDoubleClick = 1 << 7;
    public static final int mActionType_Confirm		= 1 << 8;
    public static final int mActionType_IncrAnalog	= 1 << 9;
    public static final int mActionType_RadioButton	= 1 << 10;
    public static final int mActionType_Slider		= 1 << 11;
    public static final int mActionType_ValueInput	= 1 << 12;
    public static final int mActionType_TipText		= 1 << 13;
    public static final int mActionType_CloseGraph     	= 1 << 18;
    public static final int mActionType_PulldownMenu    = 1 << 19;
    public static final int mActionType_OptionMenu     	= 1 << 20;

    public static final int eAnimSequence_Cycle		= 1;
    public static final int eAnimSequence_Dig		= 2;
    public static final int eAnimSequence_ForwBack	= 3;

    public static final int eLimitType_Gt		= 0;
    public static final int eLimitType_Lt		= 1;
    public static final int eLimitType_Ge		= 2;
    public static final int eLimitType_Le		= 3;
    public static final int eLimitType_Eq		= 4;

    public static final int eEvent_MB1Up		= 0;
    public static final int eEvent_MB1Down		= 1;
    public static final int eEvent_MB1Click		= 2;
    public static final int eEvent_MB3Press		= 3;
    public static final int eEvent_SliderMoved		= 4;
    public static final int eEvent_ValueChanged		= 5;
    public static final int eEvent_FocusLost		= 6;

    public static final int eType_Bit 			= (1 << 15) + 1;

    public int 		dynType;
    public int		actionType;
    public int		access;
    public GeDynElemIfc[] elements;
    public GeComponentIfc comp;
    public String       instance; 
    public String	hostObject;
    public boolean	ignoreColor;
    public boolean	resetColor;
    public boolean	repaintNow;
    public JopSession	session;
    public JopEngine	en;

    public double       rotate;
    public double       x0;
    public double       y0;

    public GeDyn( Object comp) {
	this.comp = (GeComponentIfc) comp;
    }
    public GeDyn( Object comp, int dynType, int actionType, int access, GeDynElemIfc[] elements) {
	this.comp = (GeComponentIfc) comp;
	this.dynType = dynType;
	this.actionType = actionType;
	this.access = access;
	this.elements = elements;
    }
    public void setSession( JopSession session) {
	this.session = session;
	en = session.getEngine();
    }
    public void setDynType( int dynType) {
	this.dynType = dynType;
    }
    public void setActionType( int actionType) {
	this.actionType = actionType;
    }
    public void setAccess( int access) {
	this.access = access;
    }
    public void setInstance( String instance) {
	this.instance = instance;
    }
    public void setHostObject( String hostObject) {
	this.hostObject = hostObject;
    }
    public void setElements( GeDynElemIfc[] elements) {
	this.elements = elements;
    }
    public void connect() {
	if ( elements == null)
	    return;

	for ( int i = 0; i < elements.length; i++)
	    elements[i].connect();
    }
    public void disconnect() {
	if ( elements == null)
	    return;

	for ( int i = 0; i < elements.length; i++)
	    elements[i].disconnect();
    }
    public void scan() {
	if ( elements == null)
	    return;
	resetColor = false;
	ignoreColor = false;
	repaintNow = false;
	for ( int i = 0; i < elements.length; i++)
	    elements[i].scan();
	if ( repaintNow)
	    comp.repaintForeground();
    }
    public void action( int eventType, MouseEvent e) {
	if ( elements == null)
	    return;
	repaintNow = false;
	for ( int i = 0; i < elements.length; i++)
	    elements[i].action( eventType, e);
	if ( repaintNow)
	    comp.repaintForeground();
    }

    public void confirmedAction( int eventType, MouseEvent e) {
	if ( elements == null)
	    return;
	actionType = actionType & ~mActionType_Confirm;
	for ( int i = 0; i < elements.length; i++)
	    elements[i].action( eventType, e);
	actionType = actionType | mActionType_Confirm;
    }

    public String getAttrName( String str) {
	if ( str.toLowerCase().startsWith("$node")) {
	    int nix;
	    if ( instance == null)
	        nix = 0;
	    else {
	        CdhrObjid cdhr_inst = en.gdh.nameToObjid( instance);
	        nix = cdhr_inst.objid.vid;
	    }
	    CdhrObjid cdhr_node = en.gdh.getNodeObject(nix);
	    if ( cdhr_node.evenSts()) return str;
	    CdhrString cdhr_ns = en.gdh.objidToName( cdhr_node.objid, Cdh.mName_volumeStrict);
	    if ( cdhr_ns.evenSts()) return str;
	    String s = RtUtilities.strReplace( str, "$node", cdhr_ns.str);
	    return s;
	}
	if ( instance == null) {
	    if ( hostObject == null) {
		if ( str.startsWith("!"))
		    return str.substring(1);
		else
		    return str;
	    }
	    else {
		String s = RtUtilities.strReplace( str, "$hostobject", hostObject);
		if ( s.startsWith("!"))
		    return s.substring(1);
		else
		    return s;
	    }
	}
	else {
	    String s = RtUtilities.strReplace( str, "$object", instance);
	    if ( s.startsWith("!"))
		return s.substring(1);
	    else
		return s;
	}
    }

    public String getAttrNameNoSuffix( String str) {
	int startIdx;
	String s;
	if ( instance == null) {
	    if ( hostObject == null)
		s = str;
	    else
		s = RtUtilities.strReplace( str, "$hostobject", hostObject);
	}
	else
	    s = RtUtilities.strReplace( str, "$object", instance);
   
	if ( s.startsWith("!"))
	    startIdx = 1;
	else
	    startIdx = 0;

	int idx1 = s.indexOf('#');
	if ( idx1 != -1) {
	    int idx2 = s.indexOf('[');
	    if ( idx2 != -1)
		return s.substring( startIdx, idx1) + s.substring(idx2);
	    else
		return s.substring( startIdx, idx1);
	}
	else
	    return s.substring(startIdx);
    }
    static public boolean getAttrInverted( String str) {
	return str.startsWith("!");
    }
    static public int getTypeId( String s) {
        String suffix;
	int idx1 = s.indexOf("##");
	if ( idx1 != -1) {
	    int idx2 = s.indexOf('[');
	    if ( idx2 != -1)
		suffix = s.substring( idx1+2, idx2);
	    else
	        suffix = s.substring( idx1+2);
	}
	else
	    return -1;

	if ( suffix.equalsIgnoreCase("Float32"))
	     return Pwr.eType_Float32;
	else if ( suffix.equalsIgnoreCase("Int32"))
	     return Pwr.eType_Int32;
	else if ( suffix.substring(0,6).equalsIgnoreCase("String"))
	     return Pwr.eType_String;
	else if ( suffix.equalsIgnoreCase("UInt32"))
	     return Pwr.eType_UInt32;
	else if ( suffix.equalsIgnoreCase("Int16"))
	     return Pwr.eType_Int16;
	else if ( suffix.equalsIgnoreCase("UInt32"))
	     return Pwr.eType_UInt16;
	else if ( suffix.equalsIgnoreCase("Int8"))
	     return Pwr.eType_Int8;
	else if ( suffix.equalsIgnoreCase("UInt8"))
	     return Pwr.eType_UInt8;
	else if ( suffix.equalsIgnoreCase("Boolean"))
	     return Pwr.eType_Boolean;
	else if ( suffix.equalsIgnoreCase("Char"))
	     return Pwr.eType_Char;
	else if ( suffix.equalsIgnoreCase("Float64"))
	     return Pwr.eType_Float64;
	else if ( suffix.equalsIgnoreCase("Objid"))
	     return Pwr.eType_Objid;
	else if ( suffix.equalsIgnoreCase("Time"))
	     return Pwr.eType_Time;
	else if ( suffix.equalsIgnoreCase("DeltaTime"))
	     return Pwr.eType_DeltaTime;
	else if ( suffix.equalsIgnoreCase("AttrRef"))
	     return Pwr.eType_AttrRef;
	else if ( suffix.equalsIgnoreCase("Bit"))
	     return GeDyn.eType_Bit;
	return -1;
    }

    boolean isLocalDb( String attribute) {
      return (attribute.startsWith("$local.") && en.ldb != null);
    }
}









