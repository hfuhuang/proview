/* 
 * Proview   $Id: DynamicObj.java,v 1.4 2005-12-06 11:17:01 claes Exp $
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
import javax.swing.tree.DefaultMutableTreeNode;
import jpwr.jop.JopDynamic;
import jpwr.jop.JopEngine;
import jpwr.rt.*;
/**
 *  Description of the Class
 *
 *@author     JN3920
 *@created    November 12, 2002
 */
public class DynamicObj extends TreeObj
{
  /**  Description of the Field */
  public XttObjAttr objAttr;
  /**  Description of the Field */
  int classid;
  /**  Description of the Field */
//  String fullName;
  /**  Description of the Field */
  static JopEngine en;
  
  static boolean initDone = false;


  /**
   *  Sets the objectAttributeValue attribute of the DynamicObj object
   *
   *@param  obj  The new objectAttributeValue value
   */
  public void setObjectAttributeValue(AttrObj obj)
  {
    switch (obj.type)
    {
      case Pwr.eType_Float32:
      case Pwr.eType_Float64:
        Logg.logg("getObjectRefInfoFloat(" + obj.refObj.id + ")", 6);
        obj.setValue(en.gdh.getObjectRefInfoFloat(obj.refObj.id));
        break;
      case Pwr.eType_Int8:
      case Pwr.eType_Int16:
      case Pwr.eType_Int32:
      case Pwr.eType_Int64:
      case Pwr.eType_UInt8:
      case Pwr.eType_UInt16:
      case Pwr.eType_UInt32:
      case Pwr.eType_UInt64:
      case Pwr.eType_Status:
      case Pwr.eType_NetStatus:
      case Pwr.eType_Enum:
      case Pwr.eType_Mask:
        Logg.logg("getObjectRefInfoInt(" + obj.refObj.id + ")", 6);
        obj.setValue(en.gdh.getObjectRefInfoInt(obj.refObj.id));
        break;
      case Pwr.eType_Boolean:
        Logg.logg("getObjectRefInfoBoolean(" + obj.refObj.id + ")", 6);
        obj.setValue(en.gdh.getObjectRefInfoBoolean(obj.refObj.id));
        break;
      default:
	  //	  Logg.logg(obj.name + " getObjectRefInfoString(" + obj.refObj.id + /*" " + obj.type +*/ ")", 1);
	  //        Logg.logg("getObjectRefInfoString: " + en.gdh.getObjectRefInfoString(obj.refObj.id, obj.type), 1);

        obj.setValue(en.gdh.getObjectRefInfoString(obj.refObj.id, obj.type));
        break;
    }
  }



  /**
   *  Sets the typeIdString attribute of the DynamicObj object
   *
   *@param  type  The new typeIdString value
   *@param  size  The new typeIdString value
   *@return       Description of the Return Value
   */
  public String setTypeIdString(int type, int size)
  {
    String suffix = "##";
    switch (type)
    {
      case Pwr.eType_Boolean:
        suffix += "BOOLEAN";
        break;
      case Pwr.eType_Float32:
        suffix += "FLOAT32";
        break;
      case Pwr.eType_Int32:
        suffix += "INT32";
        break;
      case Pwr.eType_UInt32:
        suffix += "UINT32";
        break;
      case Pwr.eType_Int16:
        suffix += "INT16";
        break;
      case Pwr.eType_UInt16:
        suffix += "UINT16";
        break;
      case Pwr.eType_Int8:
        suffix += "INT8";
        break;
      case Pwr.eType_UInt8:
        suffix += "UINT8";
        break;
      case Pwr.eType_Status:
        suffix += "STATUS";
        break;
      case Pwr.eType_NetStatus:
        suffix += "NETSTATUS";
        break;
      case Pwr.eType_Enum:
        suffix += "ENUM";
        break;
      case Pwr.eType_Mask:
        suffix += "MASK";
        break;
      case Pwr.eType_Char:
        suffix += "CHAR";
        break;
      case Pwr.eType_Float64:
        suffix += "FLOAT64";
        break;
      case Pwr.eType_Objid:
        suffix += "OBJID";
        break;
      case Pwr.eType_String:
        suffix += "STRING" + size;
        break;
      case Pwr.eType_Time:
        suffix += "TIME";
        break;
      case Pwr.eType_DeltaTime:
        suffix += "DELTATIME";
        break;
      case Pwr.eType_AttrRef:
        suffix += "ATTRREF";
        break;
      default:
        suffix += "STRING" + size;
        break;
    }
    return suffix;
  }
  static void init(JopEngine en)
  {
    if(!DynamicObj.initDone)
    {
      DynamicObj.initDone = true;
      DynamicObj.en = en;
    }
  }
}


