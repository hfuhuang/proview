package jpwr.jop;
//package navigator;
import java.util.Enumeration;
import java.util.Vector;
import javax.swing.tree.DefaultMutableTreeNode;
import javax.swing.tree.DefaultTreeModel;
//import jpwr.jop.JopDynamic;
//import jpwr.jop.JopEngine;
import jpwr.rt.*;

/**
 *  Description of the Class
 *
 *@author     JN3920
 *@created    November 12, 2002
 */
public class XttObj extends DynamicObj implements JopDynamic
{
  /**  Description of the Field */
  public Vector attrVector = null;
  /**  Description of the Field */
  public CdhrClassId classId = null;
  /**  Description of the Field */
  public String className = null;
  /**  Description of the Field */
  public boolean debug = false;
  /**  Description of the Field */
  public String description = " ";
  /**  Description of the Field */
  public JopEngine en;
  /**  Description of the Field */
//  public String fullName = null;
  /**  Description of the Field */
  public Gdh gdh;
  /**  Description of the Field */
  public boolean hasBeenReferenced = false;
  /**  Description of the Field */
  public boolean hasChildren = false;
  /**  Description of the Field */
  public int lengthToSecondCol = 24;
  /**  Description of the Field */
//  public String name = null;
  /**  Description of the Field */
  public CdhrObjid objId = null;
  /**  Description of the Field */
  public XttRefObj refObj = null;
  /**  Description of the Field */
  public int sts = 2;
  /**  Description of the Field */
  DefaultMutableTreeNode treeNode;
//  public String showableClassName = "Dv";
  /**  Description of the Field */
  private static String isXttObjStr = "XttObj";


  /**
   *  Constructor for the XttObj object
   *
   *@param  gdh        Description of the Parameter
   *@param  en         Description of the Parameter
   *@param  fullName   Description of the Parameter
   *@param  name       Description of the Parameter
   *@param  className  Description of the Parameter
   *@param  objId      Description of the Parameter
   *@param  classId    Description of the Parameter
   */
  public XttObj(Gdh gdh, JopEngine en, String fullName, String name, String className, CdhrObjid objId, CdhrClassId classId)
  {
    this.gdh = gdh;
    this.en = en;
    this.fullName = fullName;
    this.name = name;
    this.className = className;
    this.objId = objId;
    this.classId = classId;
    CdhrString ret = en.gdh.getObjectInfoString(this.fullName + "." + "Description");
    if(ret.oddSts())
    {
      this.description = ret.str;
    }
  }


  /**
   *  Constructor for the XttObj object
   *
   *@param  dummystring  Description of the Parameter
   */
  public XttObj(String dummystring)
  {
    this.name = dummystring;
  }


  /**
   *  Constructor for the XttObj object
   *
   *@param  obj  Description of the Parameter
   *@param  gdh  Description of the Parameter
   *@param  en   Description of the Parameter
   */
  public XttObj(GdhrGetXttObj obj, Gdh gdh, JopEngine en)
  {
    this.name = obj.name;
    this.fullName = obj.fullName;
    this.description = obj.description;
    this.className = obj.className;
    this.objId = obj.cdhrObjId;
    this.classId = obj.cdhrClassId;
    this.sts = obj.sts;
    this.hasChildren = obj.hasChildren;
    this.gdh = gdh;
    this.en = en;
  }


  /**
   *  Adds a feature to the AttrVector attribute of the XttObj object
   *
   *@param  attrVector  The feature to be added to the AttrVector attribute
   */
  public void addAttrVector(Vector attrVector)
  {
    if(!this.hasBeenReferenced)
    {
      this.attrVector = attrVector;
      en.add(this);
      this.hasBeenReferenced = true;
    }
  }



  public Object dynamicGetRoot() 
  {
    return null;
  }

  /**  Description of the Method */
  public void dynamicClose()
  {
    Logg.logg("XttObj: dynamic close", 6);
  }



  /**  Description of the Method */
  public void dynamicOpen()
  {
    Logg.logg("XttObj: Dynamic open", 6);
    String s = null;
    XttObjAttr obj;
    String suffix;
    GdhrRefObjectInfo ret;
    Vector refVec = new Vector();
    for(int i = 0; i < attrVector.size(); i++)
    {
      obj = ((XttObjAttr)attrVector.get(i));
      suffix = this.setTypeIdString(obj.type, obj.size);
      s = this.fullName + "." + obj.name + suffix;
      obj.fullName = s;
      Logg.logg("XttObj:  Name " + s + "Type " + this.className, 6);
      if((obj.flags & Pwr.mAdef_array) > 0)
      {
        Logg.logg("XttObj:  Hittat array, vad ska jag g�ra nu?? " + s, 6);
        XttArrayAttr arrayAttr = new XttArrayAttr(s);
        DefaultMutableTreeNode arrayChildNode = new DefaultMutableTreeNode(arrayAttr);
        obj.treeNode.add(arrayChildNode);
      }
      //p� grund av bugg i nuvarande proviewversion DataPointer borde vara PRIVATE
      else if(obj.name.compareTo("DataPointer") != 0)
      {
        refVec.add(s);
      }
    }
    Logg.logg("Innan refObjectInfo_Vector", 6);
    Vector retVec = gdh.refObjectInfo_Vector(refVec);
    Logg.logg("Efter refObjectInfo_Vector", 6);
    if(retVec == null)
    {
      Logg.logg("XttObj: refObjectInfo_Vector(" + s + ") returnerar null", 1);
      return;
    }
    else
    {
      int j = 0;
      for(int i = 0; i < retVec.size(); i++)
      {
        ret = (GdhrRefObjectInfo)retVec.get(i);
        if(ret.evenSts())
        {
          Logg.logg("XttObj:  refObjectInfo_Vector(" + s + ") Error ", 4);
        }

        //p� grund av bugg i nuvarande proviewversion DataPointer borde vara PRIVATE
        while(j < attrVector.size() &&
            (((XttObjAttr)attrVector.get(j)).name.compareTo("DataPointer") == 0 ||
            ((((XttObjAttr)attrVector.get(j)).flags & Pwr.mAdef_array) > 0)))
        {
          Logg.logg("Hittat datapointer", 4);
          j++;
        }
        if(j < attrVector.size())
        {
          obj = ((XttObjAttr)attrVector.get(j));
          obj.refObj = ret;
        }
        j++;
      }
      en.setFrameReady();
    }
    Logg.logg("refVec=" + refVec.size() + " retVec=" + retVec.size() + "attrVector=" + attrVector.size(), 6);
  }


  /**
   *  Description of the Method
   *
   *@param  animationOnly  Description of the Parameter
   */
  public void dynamicUpdate(boolean animationOnly)
  {
    if(animationOnly)
    {
      Logg.logg("XttObj: animationOnly", 6);
      return;
    }
    Logg.logg("XttObj:  dynamicUpdate", 6);
    if(this.attrVector == null)
    {
      Logg.logg("XttObj:  null-attrVector hittad i XttObj dynamicUpdate()", 1);
      return;
    }
    for(int i = 0; i < this.attrVector.size(); i++)
    {
      XttObjAttr obj = (XttObjAttr)this.attrVector.get(i);
      if((obj.flags & Pwr.mAdef_array) > 0)
      {
        Enumeration enum = obj.treeNode.children();
        while(enum.hasMoreElements())
        {
          DefaultMutableTreeNode child = (DefaultMutableTreeNode)enum.nextElement();
          XttArrayAttr arrayObj = (XttArrayAttr)child.getUserObject();
          if(arrayObj.elements == obj.elements)
          {
            this.setObjectAttributeValue(arrayObj);
          }
        }

      }
      else if(obj.name.compareTo("DataPointer") != 0)
      {
        Logg.logg("obj.type " + obj.type + " i= " + i, 6);
        this.setObjectAttributeValue(obj);
      }
    }
  }


  /**
   *  Description of the Method
   *
   *@param  debug  Description of the Parameter
   */
  public void init(boolean debug)
  {
    if(!debug)
    {
      return;
    }
    if(!XttRefObj.initDone)
    {
      XttRefObj.init(this.en);
    }
    int index = XttRefObj.checkTypeName(this.className);
    if(index >= 0)
    {
      this.refObj = new XttRefObj(this.fullName, this.en, this.classId.classId, this.treeNode, index);
    }
  }


  /**  Description of the Method */
  public void localDynamicClose()
  {
    Logg.logg("XttObj: localDynamic close_vector", 6);
    Vector unref_vec = new Vector();
    for(int i = 0; i < attrVector.size(); i++)
    {
      XttObjAttr obj = (XttObjAttr)attrVector.get(i);
      if((obj.flags & Pwr.mAdef_array) > 0)
      {
        Enumeration enum = obj.treeNode.children();
        while(enum.hasMoreElements())
        {
          DefaultMutableTreeNode child = (DefaultMutableTreeNode)enum.nextElement();
          XttArrayAttr arrayObj = (XttArrayAttr)child.getUserObject();
          if(arrayObj.elements == obj.elements)
          {
            unref_vec.add(arrayObj.refObj.refid);
          }
        }
      }
      //p� grund av bugg i nuvarande proviewversion DataPointer borde vara PRIVATE
      else if(obj.name.compareTo("DataPointer") != 0)
      {
        Logg.logg(" " + obj.refObj.refid.rix, 6);
        unref_vec.add(obj.refObj.refid);
      }
    }
    gdh.unrefObjectInfo_Vector(unref_vec);

  }


  /**  Description of the Method */
  public void removeAttrVector()
  {
    this.debug = false;
    if(this.hasBeenReferenced)
    {
      en.remove(this);
      this.localDynamicClose();
      this.attrVector = null;
      this.hasBeenReferenced = false;
    }
    else if(this.refObj != null)
    {
      this.refObj.localDynamicClose();
    }

  }


  /**
   *  A unit test for JUnit
   *
   *@return    Description of the Return Value
   */
  public String test()
  {
    return "Det gick ju bra";
  }


  /**
   *  Description of the Method
   *
   *@return    Description of the Return Value
   */
  public String toString()
  {
    int spaceLength = this.lengthToSecondCol - this.name.length();
    String space = " ";
    while(spaceLength > 1)
    {
      spaceLength--;
      space += " ";
    }
    if(this.refObj != null)
    {
      return this.name + space + this.className + "  " + this.refObj.toString();
    }
    else
    {
      return this.name + space + this.className + "    " + this.description;
    }
  }


  /**
   *  Gets the xttObj attribute of the XttObj class
   *
   *@param  o  Description of the Parameter
   *@return    The xttObj value
   */
  public static boolean isXttObj(Object o)
  {
    return o.getClass().getName() == isXttObjStr;
  }
}

