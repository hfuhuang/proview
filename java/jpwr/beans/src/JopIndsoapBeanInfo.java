package jpwr.beans;
import jpwr.jop.*;
import java.beans.*;
public class JopIndsoapBeanInfo extends SimpleBeanInfo {
  Class beanClass = JopIndsoap.class;
  String iconColor16x16Filename = "jopindsoap16.gif";
  String iconColor32x32Filename = "jopindsoap32.gif";
  String iconMono16x16Filename;
  String iconMono32x32Filename;

  public JopIndsoapBeanInfo() {
  }
  public PropertyDescriptor[] getPropertyDescriptors() {
    try {
      PropertyDescriptor _toolTipText = new PropertyDescriptor("toolTipText",
        beanClass, "getToolTipText", "setToolTipText");
      _toolTipText.setDisplayName("toolTipText");
      _toolTipText.setShortDescription("toolTipText");
      PropertyDescriptor _fillColor = new PropertyDescriptor("fillColor",
        beanClass, "getFillColor", "setFillColor");
      _fillColor.setDisplayName("fillColor");
      _fillColor.setShortDescription("fillColor");
      _fillColor.setPropertyEditorClass(GeColorEditor.class);
      PropertyDescriptor _borderColor = new PropertyDescriptor("borderColor",
        beanClass, "getBorderColor", "setBorderColor");
      _borderColor.setDisplayName("borderColor");
      _borderColor.setShortDescription("borderColor");
      _borderColor.setPropertyEditorClass(GeColorEditor.class);
      PropertyDescriptor _colorTone = new PropertyDescriptor("colorTone",
        beanClass, "getColorTone", "setColorTone");
      _colorTone.setDisplayName("colorTone");
      _colorTone.setShortDescription("colorTone");
      _colorTone.setPropertyEditorClass(GeColorToneEditor.class);
      PropertyDescriptor _colorShift = new PropertyDescriptor("colorShift",
        beanClass, "getColorShift", "setColorShift");
      _colorShift.setDisplayName("colorShift");
      _colorShift.setShortDescription("colorShift");
      _colorShift.setPropertyEditorClass(GeColorShiftEditor.class);
      PropertyDescriptor _colorBrightness = new PropertyDescriptor("colorBrightness",
        beanClass, "getColorBrightness", "setColorBrightness");
      _colorBrightness.setDisplayName("colorBrightness");
      _colorBrightness.setShortDescription("colorBrightness");
      _colorBrightness.setPropertyEditorClass(GeColorBrightnessEditor.class);
      PropertyDescriptor _colorIntensity = new PropertyDescriptor("colorIntensity",
        beanClass, "getColorIntensity", "setColorIntensity");
      _colorIntensity.setDisplayName("colorIntensity");
      _colorIntensity.setShortDescription("colorIntensity");
      _colorIntensity.setPropertyEditorClass(GeColorIntensityEditor.class);
      PropertyDescriptor _rotate = new PropertyDescriptor("rotate",
        beanClass, "getRotate", "setRotate");
      _rotate.setDisplayName("rotate");
      _rotate.setShortDescription("rotate");
      PropertyDescriptor _lowTone = new PropertyDescriptor("lowTone",
        beanClass, "getLowTone", "setLowTone");
      _lowTone.setDisplayName("lowTone");
      _lowTone.setShortDescription("lowTone");
      _lowTone.setPropertyEditorClass(GeColorToneEditor.class);
      PropertyDescriptor _pwrAttribute = new PropertyDescriptor("pwrAttribute",
        beanClass, "getPwrAttribute", "setPwrAttribute");
      _pwrAttribute.setDisplayName("pwrAttribute");
      _pwrAttribute.setShortDescription("pwrAttribute");
      PropertyDescriptor _annot1 = new PropertyDescriptor("annot1",
        beanClass, "getAnnot1", "setAnnot1");
      _annot1.setDisplayName("annot1");
      _annot1.setShortDescription("annot1");
      PropertyDescriptor _annot1Font = new PropertyDescriptor("annot1Font",
        beanClass, "getAnnot1Font", "setAnnot1Font");
      _annot1Font.setDisplayName("annot1Font");
      _annot1Font.setShortDescription("annot1Font");
      PropertyDescriptor[] pds = new PropertyDescriptor[] {
	_toolTipText, _fillColor, _borderColor, _colorTone, _colorShift,
	_colorBrightness, _colorIntensity, _rotate,
	_lowTone, _pwrAttribute, _annot1, _annot1Font};
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
//    return new BeanInfo[] { superBeanInfo };
      return null;
    }
    catch(IntrospectionException ex) {
      ex.printStackTrace();
      return null;
    }
  }
}
