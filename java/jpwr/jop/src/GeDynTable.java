/* 
 * Proview   $Id: GeDynTable.java,v 1.7 2007-05-23 08:05:19 claes Exp $
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
import java.awt.event.*;

public class GeDynTable extends GeDynElem {
  public String[] attribute;
  public String[] selAttribute;
  int rows;
  int columns;

  public GeCFormat[] cFormat;
  public boolean[] attrFound;
  public boolean[] selAttrFound;
  PwrtRefId[] subid;
  int[] p;
  public int[] elements;
  public int[] size;
  public int[] typeId;
  public boolean[][] oldValueB;
  public float[][] oldValueF;
  public int[][] oldValueI;
  public String[][] oldValueS;
  boolean firstScan = true;
  StringBuffer sb = new StringBuffer();
  boolean[] isHeaderRef;
  int[][] headerRefp;
  PwrtRefId[][] headerRefSubid;
  public GeDynTable( GeDyn dyn, String[] attribute, String[] format, String[] selAttribute,
		     int rows, int columns) {
    super( dyn, GeDyn.mDynType_Table, GeDyn.mActionType_No);
    this.rows = rows;
    this.columns = columns;
    this.attribute = attribute;
    this.selAttribute = selAttribute;
    cFormat = new GeCFormat[columns];
    for ( int i = 0; i < columns; i++)
      cFormat[i] = new GeCFormat( format[i]);
    p = new int[columns];
    subid = new PwrtRefId[columns];
    typeId = new int[columns];
    elements = new int[columns];
    size = new int[columns];
    attrFound = new boolean[columns];
    selAttrFound = new boolean[columns];
    oldValueB = new boolean[columns][];
    oldValueF = new float[columns][];
    oldValueI = new int[columns][];
    oldValueS = new String[columns][];
    isHeaderRef = new boolean[columns];
    headerRefp = new int[columns][];
    headerRefSubid = new PwrtRefId[columns][];
  }
  public void connect() {
    GdhrRefObjectInfo ret;

    for ( int i = 0; i < columns; i++) {
      String attrName = dyn.getAttrName( attribute[i]);

      if ( attrName.compareTo("") != 0) {
	if ( attrName.startsWith("$header.")) {
	  // NYI TODO !!!
          // Replace $header with the object in the header column

	  isHeaderRef[i] = true;
	  headerRefp[i] = new int[rows];
	  headerRefSubid[i] = new PwrtRefId[rows];

	  for ( int j = 0; j < rows; j++) {
	    String hName = dyn.getAttrNameNoSuffix( attribute[0]);
	    hName = hName + "[" + j + "]";

	    CdhrString cstr = dyn.en.gdh.getObjectInfoString( hName);
	    if (cstr.evenSts()) {
	      headerRefp[i][j] = 0;
	      // headerRefSubid[i][j] = null;
	      continue;
	    }
	    String cellAttrName = cstr.str + attrName.substring(7);
	    ret = dyn.en.gdh.refObjectInfo( cellAttrName);
	    if ( ret.evenSts()) {
	      System.out.println( "Table: " + cellAttrName);
	    }
	    else {
	      attrFound[i] = true;
	      headerRefp[i][j] = ret.id;
	      headerRefSubid[i][j] = ret.refid;
	      typeId[i] = ret.typeId;
	    }
	  }
	}
	else {
	  isHeaderRef[i] = false;

	  ret = dyn.en.gdh.refObjectInfo( attrName);
	  if ( ret.evenSts()) {
	    System.out.println( "Table: " + attrName);
	    attrFound[i] = false;
	  }
	  else {
	    attrFound[i] = true;
	    p[i] = ret.id;
	    subid[i] = ret.refid;
	    typeId[i] = ret.typeId;
	    elements[i] = ret.getElements();
	    size[i] = ret.getSize();
	    if ( elements[i] > rows)
	      elements[i] = rows;
	  }
	}
	if ( typeId[i] == Pwr.eType_Float32) {
          oldValueF[i] = new float[rows];
	}
	else if ( typeId[i] == Pwr.eType_Boolean) {
          oldValueB[i] = new boolean[rows];
	}
	else if ( typeId[i] == Pwr.eType_Int32 ||
		  typeId[i] == Pwr.eType_UInt32 ||
		  typeId[i] == Pwr.eType_Int16 ||
		  typeId[i] == Pwr.eType_UInt16 ||
		  typeId[i] == Pwr.eType_Int8 ||
		  typeId[i] == Pwr.eType_UInt8) {
	  oldValueI[i] = new int[rows];
	}
	else if ( typeId[i] == Pwr.eType_String || 
		  typeId[i] == Pwr.eType_Objid ||
		  typeId[i] == Pwr.eType_Time ||
		  typeId[i] == Pwr.eType_DeltaTime) {
	  oldValueS[i] = new String[rows];
	}
      }
    }
  }
  public void disconnect() {
    for ( int i = 0; i < columns; i++) {
      if ( attrFound[i]) {
	if ( isHeaderRef[i]) {
	  for ( int j = 0; j < rows; j++) {
	    if ( headerRefSubid[i][j] != null) 
	      dyn.en.gdh.unrefObjectInfo( headerRefSubid[i][j]);
	  }
	}
	else
	  dyn.en.gdh.unrefObjectInfo( subid[i]);
      }
    }
  }
  public void scan() {

    for ( int i = 0; i < columns; i++) {
      if ( !attrFound[i])
	continue;

      if ( isHeaderRef[i]) {
        if ( typeId[i] == Pwr.eType_Float32) {
	  for ( int j = 0; j < rows; j++) {
	    if ( headerRefp[i][j] != 0) {
              float value0 = dyn.en.gdh.getObjectRefInfoFloat( headerRefp[i][j]);
	      if ( value0 != oldValueF[i][j] || firstScan) {
		sb = cFormat[i].format( value0, sb);
		((GeTable)dyn.comp).setValueAt(new String(sb), j, i);
		// dyn.repaintNow = true;
		oldValueF[i][j] = value0;
	      }
	    }
          }
        }
        else if ( typeId[i] == Pwr.eType_Boolean) {
	  for ( int j = 0; j < rows; j++) {
	    if ( headerRefp[i][j] != 0) {
              boolean value0 = dyn.en.gdh.getObjectRefInfoBoolean( headerRefp[i][j]);
	      if ( value0 != oldValueB[i][j] || firstScan) {
		sb = cFormat[i].format( value0, sb);
		((GeTable)dyn.comp).setValueAt(new String(sb), j, i);
		// dyn.repaintNow = true;
		oldValueB[i][j] = value0;
	      }
	    }
          }
        }
        else if ( typeId[i] == Pwr.eType_Int32 ||
	          typeId[i] == Pwr.eType_UInt32 ||
                  typeId[i] == Pwr.eType_Int16 ||
	          typeId[i] == Pwr.eType_UInt16 ||
                  typeId[i] == Pwr.eType_Int8 ||
	          typeId[i] == Pwr.eType_UInt8) {
	  for ( int j = 0; j < rows; j++) {
	    if ( headerRefp[i][j] != 0) {
              int value0 = dyn.en.gdh.getObjectRefInfoInt( headerRefp[i][j]);
	      if ( value0 != oldValueI[i][j] || firstScan) {
		sb = cFormat[i].format( value0, sb);
		((GeTable)dyn.comp).setValueAt(new String(sb), j, i);
		// dyn.repaintNow = true;
		oldValueI[i][j] = value0;
	      }
	    }
          }
        }
        else if ( typeId[i] == Pwr.eType_String || 
	          typeId[i] == Pwr.eType_Objid ||
  		  typeId[i] == Pwr.eType_Time ||
		  typeId[i] == Pwr.eType_DeltaTime) {
	  for ( int j = 0; j < rows; j++) {
	    String value0 = dyn.en.gdh.getObjectRefInfoString( headerRefp[i][j], typeId[i]);
	    if ( firstScan || value0.compareTo( oldValueS[i][j]) != 0) {
	      sb = cFormat[i].format( value0, sb);
	      ((GeTable)dyn.comp).setValueAt(new String(sb), j, i);
	      // dyn.repaintNow = true;

	      oldValueS[i][j] = value0;
	    }
	  }
        }
      }
      else {
        if ( typeId[i] == Pwr.eType_Float32) {
          float[] value0 = dyn.en.gdh.getObjectRefInfoFloatArray( p[i], elements[i]);
	  if ( value0 != null) {
	    for ( int j = 0; j < value0.length; j++) {
	      if (j >= rows)
	        break;
              if ( value0[j] != oldValueF[i][j] || firstScan) {
	        sb = cFormat[i].format( value0[j], sb);
	        ((GeTable)dyn.comp).setValueAt(new String(sb), j, i);
                // dyn.repaintNow = true;
                oldValueF[i][j] = value0[j];
	      }
	    }
          }
        }
        else if ( typeId[i] == Pwr.eType_Boolean) {
          boolean[] value0 = dyn.en.gdh.getObjectRefInfoBooleanArray( p[i], elements[i]);
	  if ( value0 != null) {
	    for ( int j = 0; j < value0.length; j++) {
	      if (j >= rows)
	        break;
              if ( value0[j] != oldValueB[i][j] || firstScan) {
	        if ( value0[j])
	          ((GeTable)dyn.comp).setValueAt("1", j, i);
	        else
	          ((GeTable)dyn.comp).setValueAt("0", j, i);
                // dyn.repaintNow = true;
                oldValueB[i][j] = value0[j];
	      }
	    }
          }
        }
        else if ( typeId[i] == Pwr.eType_Int32 ||
	          typeId[i] == Pwr.eType_UInt32 ||
                  typeId[i] == Pwr.eType_Int16 ||
	          typeId[i] == Pwr.eType_UInt16 ||
                  typeId[i] == Pwr.eType_Int8 ||
	          typeId[i] == Pwr.eType_UInt8) {
	  int value0[] = dyn.en.gdh.getObjectRefInfoIntArray( p[i], elements[i]);
	  if ( value0 != null) {
	    for ( int j = 0; j < value0.length; j++) {
	      if (j >= rows)
	        break;
	      if ( value0[j] != oldValueI[i][j] || firstScan) {
	        sb = cFormat[i].format( value0[j], sb);
	        ((GeTable)dyn.comp).setValueAt(new String(sb), j, i);
	        // dyn.repaintNow = true;
	        oldValueI[i][j] = value0[j];
	      }
            }
          }
        }
        else if ( typeId[i] == Pwr.eType_String || 
	          typeId[i] == Pwr.eType_Objid ||
  		  typeId[i] == Pwr.eType_Time ||
		  typeId[i] == Pwr.eType_DeltaTime) {
	  String[] value0 = dyn.en.gdh.getObjectRefInfoStringArray( p[i], typeId[i], size[i], elements[i]);
	  if ( value0 != null) {
	    for ( int j = 0; j < value0.length; j++) {
	      if (j >= rows)
	        break;
	      if ( firstScan || value0[j].compareTo( oldValueS[i][j]) != 0) {
	        sb = cFormat[i].format( value0[j], sb);
	        ((GeTable)dyn.comp).setValueAt(new String(sb), j, i);
	        // dyn.repaintNow = true;

	        oldValueS[i][j] = value0[j];
	      }
            }
	  }
        }
      }
    }
    if ( firstScan)
      firstScan = false;
  }

  public void action( int eventType, MouseEvent e) {
    int row;
    int column;

    switch ( eventType) {
    case GeDyn.eEvent_MB1Click:
      column = ((GeTable)dyn.comp).getSelectedColumn();
      row = ((GeTable)dyn.comp).getSelectedRow();
      System.out.println( "MB1Click " + row + " " + column);

      break;
    case GeDyn.eEvent_MB3Press:
	System.out.println("mb3 press");
      // Get the selected object
      column = ((GeTable)dyn.comp).getSelectedColumn();
      row = ((GeTable)dyn.comp).getSelectedRow();
      System.out.println("Row " + row + " Column " + column);

      if ( row == -1 || column == -1)
        break;

      if ( typeId[column] == Pwr.eType_Objid) {
	// Name of an attribute that contains the objid of the reference object
 	String name = dyn.getAttrNameNoSuffix( attribute[column]);
	name = name + "[" + row + "]";
       	CdhrObjid reto = dyn.en.gdh.getObjectInfoObjid( name);
	
	if ( reto.oddSts() && !reto.objid.isNull()) {
	  CdhrString rets = dyn.en.gdh.objidToName( reto.objid, Cdh.mName_volumeStrict);
	  System.out.println( "str: " + rets.str + " " + rets.getSts());
	  if ( rets.oddSts() && ! rets.str.equals(""))
	    new JopMethodsMenu( dyn.session, rets.str, JopUtility.GRAPH, 
				    (Component)dyn.comp, e.getX(), e.getY());
	}
      }
      break;
    }
  }

}









