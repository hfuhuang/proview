/* 
 * Proview   $Id: JopButtonimageBeanInfo.java,v 1.2 2005-09-01 14:57:49 claes Exp $
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

package jpwr.beans;
import jpwr.jop.*;
import java.beans.*;
public class JopButtonimageBeanInfo extends SimpleBeanInfo {
  Class beanClass = JopButtonimage.class;
  String iconColor16x16Filename = "jopbuttonimage16.gif";
  String iconColor32x32Filename = "jopbuttonimage32.gif";
  String iconMono16x16Filename;
  String iconMono32x32Filename;

  public JopButtonimageBeanInfo() {
  }
  public PropertyDescriptor[] getPropertyDescriptors() {
    try {
      PropertyDescriptor _image = new PropertyDescriptor("image",
        beanClass, "getImage", "setImage");
      _image.setDisplayName("image");
      _image.setShortDescription("image");
      PropertyDescriptor _imageLow = new PropertyDescriptor("imageLow",
        beanClass, "getImageLow", "setImageLow");
      _imageLow.setDisplayName("imageLow");
      _imageLow.setShortDescription("imageLow");
      PropertyDescriptor _imagePress = new PropertyDescriptor("imagePress",
        beanClass, "getImagePress", "setImagePress");
      _imagePress.setDisplayName("imagePress");
      _imagePress.setShortDescription("imagePress");
      PropertyDescriptor _pwrAttribute = new PropertyDescriptor("pwrAttribute",
        beanClass, "getPwrAttribute", "setPwrAttribute");
      _pwrAttribute.setDisplayName("pwrAttribute");
      _pwrAttribute.setShortDescription("pwrAttribute");
      PropertyDescriptor _pwrAttrImage = new PropertyDescriptor("pwrAttrImage",
        beanClass, "getPwrAttrImage", "setPwrAttrImage");
      _pwrAttrImage.setDisplayName("pwrAttrImage");
      _pwrAttrImage.setShortDescription("pwrAttrImage");
      PropertyDescriptor _clickAction = new PropertyDescriptor("clickAction",
        beanClass, "getClickAction", "setClickAction");
      _clickAction.setDisplayName("clickAction");
      _clickAction.setShortDescription("clickAction");
      _clickAction.setPropertyEditorClass(ClickActionEditor.class);
      PropertyDescriptor _command = new PropertyDescriptor("command",
        beanClass, "getCommand", "setCommand");
      _command.setDisplayName("command");
      _command.setShortDescription("command");
      PropertyDescriptor _confirm = new PropertyDescriptor("confirm",
        beanClass, "getConfirm", "setConfirm");
      _confirm.setDisplayName("confirm");
      _confirm.setShortDescription("confirm");
      PropertyDescriptor _confirmText = new PropertyDescriptor("confirmText",
        beanClass, "getConfirmText", "setConfirmText");
      _confirmText.setDisplayName("confirmText");
      _confirmText.setShortDescription("confirmText");
      PropertyDescriptor[] pds = new PropertyDescriptor[] {
	_image, _imageLow, _imagePress, _pwrAttribute, _pwrAttrImage, 
	_clickAction, _command, _confirm, _confirmText};
      return pds;

    }
    catch(IntrospectionException ex) {
      ex.printStackTrace();
      return null;
    }
  }
  public java.awt.Image getIcon(int iconKind) {
    switch (iconKind) {
    case BeanInfo.ICON_COLOR_16x16:
        return iconColor16x16Filename != null ? loadImage(iconColor16x16Filename) : null;
    case BeanInfo.ICON_COLOR_32x32:
        return iconColor32x32Filename != null ? loadImage(iconColor32x32Filename) : null;
    case BeanInfo.ICON_MONO_16x16:
        return iconMono16x16Filename != null ? loadImage(iconMono16x16Filename) : null;
    case BeanInfo.ICON_MONO_32x32:
        return iconMono32x32Filename != null ? loadImage(iconMono32x32Filename) : null;
        }
    return null;
  }
  public BeanInfo[] getAdditionalBeanInfo() {
    Class superclass = beanClass.getSuperclass();
    try {
      BeanInfo superBeanInfo = Introspector.getBeanInfo(superclass);
//      return new BeanInfo[] { superBeanInfo };
      return null;
    }
    catch(IntrospectionException ex) {
      ex.printStackTrace();
      return null;
    }
  }
}
