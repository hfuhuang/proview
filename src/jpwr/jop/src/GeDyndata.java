package jpwr.jop;
import jpwr.rt.*;

public class GeDyndata {
  public static final int eTrace_Inherit            = 0;
  public static final int eTrace_Dig                = 1;
  public static final int eTrace_DigWithError       = 2;
  public static final int eTrace_DigTone            = 3;
  public static final int eTrace_DigToneWithError   = 4;
  public static final int eTrace_Annot              = 5;
  public static final int eTrace_DigWithText        = 6;
  public static final int eTrace_Bar                = 7;
  public static final int eTrace_Trend              = 8;
  public static final int eTrace_DigBorder          = 9;
  public static final int eTrace_AnnotWithTone      = 10;
  public static final int eTrace_DigTwo             = 11;
  public static final int eTrace_DigToneTwo         = 12;
  public static final int eTrace_Invisible          = 13;
  public static final int eTrace_Rotate             = 14;
  public static final int eTrace_AnalogShift        = 15;
  public static final int eTrace_Animation          = 16;
  public static final int eTrace_DigAnimation       = 17;
  public static final int eTrace_AnimationForwBack  = 18;
  public static final int eTrace_DigShift           = 19;
  public static final int eTrace_Move               = 20;
  public static final int eTrace_SetDig             = 1000;
  public static final int eTrace_ResetDig           = 1001;
  public static final int eTrace_ToggleDig          = 1002;
  public static final int eTrace_Slider             = 1003;
  public static final int eTrace_AnnotInput         = 1004;
  public static final int eTrace_Command            = 1005;
  public static final int eTrace_CommandConfirm     = 1006;
  public static final int eTrace_SetDigConfirm      = 1007;
  public static final int eTrace_ResetDigConfirm    = 1008;
  public static final int eTrace_ToggleDigConfirm   = 1009;
  public static final int eTrace_SetDigWithTone     = 1010;
  public static final int eTrace_ResetDigWithTone   = 1011;
  public static final int eTrace_ToggleDigWithTone  = 1012;
  public static final int eTrace_AnnotInputWithTone = 1013;
  public static final int eTrace_SetDigConfirmWithTone = 1014;
  public static final int eTrace_ResetDigConfirmWithTone = 1015;
  public static final int eTrace_ToggleDigConfirmWithTone = 1016;
  public static final int eTrace_DigWithCommand     = 1017;
  public static final int eTrace_DigWithErrorAndCommand = 1018;
  public static final int eTrace_DigToneWithCommand = 1019;
  public static final int eTrace_DigToneWithErrorAndCommand = 1020;
  public static final int eTrace_StoDigWithTone     = 1021;
  public static final int eTrace_DigTwoWithCommand  = 1022;
  public static final int eTrace_DigToneTwoWithCommand = 1023;
  public static final int eTrace_IncrAnalog         = 1024;
  public static final int eTrace_RadioButton        = 1025;
  public static final int eTrace_DigShiftWithToggleDig = 1026;

  public String[]	data = new String[6];
  public int		type;
  public int		color;
  public int		color2;
  public int		access;
  public boolean[]	attrFound = new boolean[6];
  public PwrtRefId[]	subid = new PwrtRefId[6];
  public int[]		typeId = new int[6];
  public GeCFormat	cFormat;
  public int[]		p = new int[6];
  public boolean[]	inverted = new boolean[6];
  public boolean[]	oldValueB = new boolean[6];
  public float[]	oldValueF = new float[4];
  public int		oldValueI;
  public String		oldValueS;
  public int[]		size = new int[6];
  public boolean	firstScan = true;
  public double		scanTime;
  public double		accTime;
  public int		annotTypeid;
  public int		annotSize;
  public String		lowText;
  public String		highText;
  public boolean	invisible;
  public boolean	invisibleOld;
  public int		animationCount;
  public int		animationDirection;
  public double         xOrig;
  public double         yOrig;
  public double         x0;
  public double         y0;
  public double         factor = 1;
  public double         xScale = 1;
  public double         yScale = 1;
  public String         instance;
  public double         rotate;
  public String         refObject;

  public GeDyndata() {
  }
  public void setType( int type) {
    this.type = type;
  }
  public int getType() {
    return type;
  }
  public void setColor( int color) {
    this.color = color;
  }
  public int getColor() {
    return color;
  }
  public void setColor2( int color2) {
    this.color2 = color2;
  }
  public int getColor2() {
    return color2;
  }
  public void setAccess( int access) {
    this.access = access;
  }
  public int getAccess() {
    return access;
  }
  public void setFactor( double factor) {
    this.factor = factor;
  }
  public double getFactor() {
    return factor;
  }
  public void setX0( double x0) {
    this.x0 = x0;
  }
  public double getX0() {
    return x0;
  }
  public void setY0( double y0) {
    this.y0 = y0;
  }
  public double getY0() {
    return y0;
  }
  public void setRotate( double rotate) {
    this.rotate = rotate;
  }
  public double getRotate() {
    return rotate;
  }
  public void setData( String data, int idx) {
    this.data[idx] = data;
  }
  public String getData( int idx) {
    return data[idx];
  }
  public void setFormat( String format) {
    this.cFormat = new GeCFormat(format);
  }
  public StringBuffer format( float value, StringBuffer buff) {
    return cFormat.format( value, buff);
  }
  public StringBuffer format( int value, StringBuffer buff) {
    return cFormat.format( value, buff);
  }
  public StringBuffer format( String value, StringBuffer buff) {
    return cFormat.format( value, buff);
  }
  public String getAttrName( String str) {
    if ( instance == null) {
      if ( str.startsWith("!"))
        return str.substring(1);
      else
        return str;
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
    if ( instance == null)
      s = str;
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
  public void setLowText( String lowText) {
    this.lowText = lowText;
  }
  public String getLowText() {
    return lowText;
  }
  public void setHighText( String highText) {
    this.highText = highText;
  }
  public String getHighText() {
    return highText;
  }
  public void setInstance( String instance) {
    this.instance = instance;
  }
  public void setRefObject( String refObject) {
    this.refObject = refObject;
  }
}
