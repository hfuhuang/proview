
package jpwr.jop;
import java.beans.*;

/**
 * Title:
 * Description:
 * Copyright:    Copyright (c) 2000
 * Company:
 * @author 	 CS
 * @version 1.0
 */

public class GeColorToneEditor extends PropertyEditorSupport {

  public GeColorToneEditor() {
  }
  private static String[] tagStrings = {
	/* glow_eDrawTone_No */ 				"NoTone",
	/* glow_eDrawTone_Gray */ 				"ToneGray",
	/* glow_eDrawTone_YellowGreen */    		        "ToneYellowGreen",
	/* glow_eDrawTone_Yellow */ 				"ToneYellow",
	/* glow_eDrawTone_Orange */ 				"ToneOrange",
	/* glow_eDrawTone_Red */ 				"ToneRed",
	/* glow_eDrawTone_Magenta */ 				"ToneMagenta",
	/* glow_eDrawTone_Blue */ 				"ToneBlue",
	/* glow_eDrawTone_Seablue */ 				"ToneSeablue",
	/* glow_eDrawTone_Green */ 				"ToneGreen",
	/* glow_eDrawTone_DarkGray */				"ToneDarkGray",
	/* glow_eDrawTone_LightGray */				"ToneLightGray",
	/* glow_eDrawTone_DarkYellowGreen */			"ToneDarkYellowGreen",
	/* glow_eDrawTone_LightYellowGreen */	       		"ToneLightYellowGreen",
	/* glow_eDrawTone_YellowGreenHighSaturation */      	"ToneYellowGreenHighSaturation",
	/* glow_eDrawTone_YellowGreenLowSaturation */       	"ToneYellowGreenLowSaturation",
	/* glow_eDrawTone_DarkYellowGreenHighSaturation */	"ToneDarkYellowGreenHighSaturation",
	/* glow_eDrawTone_DarkYellowGreenLowSaturation */   	"ToneDarkYellowGreenLowSaturation",
	/* glow_eDrawTone_LightYellowGreenHighSaturation */	"ToneLightYellowGreenHighSaturation",
	/* glow_eDrawTone_LightYellowGreenLowSaturation */	"ToneLightYellowGreenLowSaturation",
	/* glow_eDrawTone_DarkYellow */				"ToneDarkYellow",
	/* glow_eDrawTone_LightYellow */			"ToneLightYellow",
	/* glow_eDrawTone_YellowHighSaturation */		"ToneYellowHighSaturation",
	/* glow_eDrawTone_YellowLowSaturation */		"ToneYellowLowSaturation",
	/* glow_eDrawTone_DarkYellowHighSaturation */       	"ToneDarkYellowHighSaturation",
	/* glow_eDrawTone_DarkYellowLowSaturation */		"ToneDarkYellowLowSaturation",
	/* glow_eDrawTone_LightYellowHighSaturation */      	"ToneLightYellowHighSaturation",
	/* glow_eDrawTone_LightYellowLowSaturation */       	"ToneLightYellowLowSaturation",
	/* glow_eDrawTone_DarkOrange */				"ToneDarkOrange",
	/* glow_eDrawTone_LightOrange */			"ToneLightOrange",
	/* glow_eDrawTone_OrangeHighSaturation */		"ToneOrangeHighSaturation",
	/* glow_eDrawTone_OrangeLowSaturation */		"ToneOrangeLowSaturation",
	/* glow_eDrawTone_DarkOrangeHighSaturation */		"ToneDarkOrangeHighSaturation",
	/* glow_eDrawTone_DarkOrangeLowSaturation */		"ToneDarkOrangeLowSaturation",
	/* glow_eDrawTone_LightOrangeHighSaturation */		"ToneLightOrangeHighSaturation",
	/* glow_eDrawTone_LightOrangeLowSaturation */		"ToneLightOrangeLowSaturation",
	/* glow_eDrawTone_DarkRed */				"ToneDarkRed",
	/* glow_eDrawTone_LightRed */				"ToneLightRed",
	/* glow_eDrawTone_RedHighSaturation */			"ToneRedHighSaturation",
	/* glow_eDrawTone_RedLowSaturation */			"ToneRedLowSaturation",
	/* glow_eDrawTone_DarkRedHighSaturation */		"ToneDarkRedHighSaturation",
	/* glow_eDrawTone_DarkRedLowSaturation */		"ToneDarkRedLowSaturation",
	/* glow_eDrawTone_LightRedHighSaturation */		"ToneLightRedHighSaturation",
	/* glow_eDrawTone_LightRedLowSaturation */		"ToneLightRedLowSaturation",
	/* glow_eDrawTone_DarkMagenta */			"ToneDarkMagenta",
	/* glow_eDrawTone_LightMagenta */			"ToneLightMagenta",
	/* glow_eDrawTone_MagentaHighSaturation */		"ToneMagentaHighSaturation",
	/* glow_eDrawTone_MagentaLowSaturation */		"ToneMagentaLowSaturation",
	/* glow_eDrawTone_DarkMagentaHighSaturation */		"ToneDarkMagentaHighSaturation",
	/* glow_eDrawTone_DarkMagentaLowSaturation */		"ToneDarkMagentaLowSaturation",
	/* glow_eDrawTone_LightMagentaHighSaturation */		"ToneLightMagentaHighSaturation",
	/* glow_eDrawTone_LightMagentaLowSaturation */		"ToneLightMagentaLowSaturation",
	/* glow_eDrawTone_DarkBlue */				"ToneDarkBlue",
	/* glow_eDrawTone_LightBlue */				"ToneLightBlue",
	/* glow_eDrawTone_BlueHighSaturation */			"ToneBlueHighSaturation",
	/* glow_eDrawTone_BlueLowSaturation */			"ToneBlueLowSaturation",
	/* glow_eDrawTone_DarkBlueHighSaturation */		"ToneDarkBlueHighSaturation",
	/* glow_eDrawTone_DarkBlueLowSaturation */		"ToneDarkBlueLowSaturation",
	/* glow_eDrawTone_LightBlueHighSaturation */		"ToneLightBlueHighSaturation",
	/* glow_eDrawTone_LightBlueLowSaturation */		"ToneLightBlueLowSaturation",
	/* glow_eDrawTone_DarkSeablue */			"ToneDarkSeablue",
	/* glow_eDrawTone_LightSeablue */			"ToneLightSeablue",
	/* glow_eDrawTone_SeablueHighSaturation */		"ToneSeablueHighSaturation",
	/* glow_eDrawTone_SeablueLowSaturation */		"ToneSeablueLowSaturation",
	/* glow_eDrawTone_DarkSeablueHighSaturation */		"ToneDarkSeablueHighSaturation",
	/* glow_eDrawTone_DarkSeablueLowSaturation */	 	"ToneDarkSeablueLowSaturation",
	/* glow_eDrawTone_LightSeablueHighSaturation */		"ToneLightSeablueHighSaturation",
	/* glow_eDrawTone_LightSeablueLowSaturation */		"ToneLightSeablueLowSaturation",
	/* glow_eDrawTone_DarkGreen */				"ToneDarkGreen",
	/* glow_eDrawTone_LightGreen */				"ToneLightGreen",
	/* glow_eDrawTone_GreenHighSaturation */		"ToneGreenHighSaturation",
	/* glow_eDrawTone_GreenLowSaturation */			"ToneGreenLowSaturation",
	/* glow_eDrawTone_DarkGreenHighSaturation */		"ToneDarkGreenHighSaturation",
	/* glow_eDrawTone_DarkGreenLowSaturation */		"ToneDarkGreenLowSaturation",
	/* glow_eDrawTone_LightGreenHighSaturation */		"ToneLightGreenHighSaturation",
	/* glow_eDrawTone_LightGreenLowSaturation */		"ToneLightGreenLowSaturation",
	};
  public String[] getTags() {
    return tagStrings;
  }
  public String getJavaInitializationString() {
    return java.lang.String.valueOf(((Number)getValue()).intValue());
  }
  public void setAsText(String s) throws IllegalArgumentException {
    if (s.equals("NoTone")) setValue( new Integer(GeColor.COLOR_TONE_NO));
/* glow_eDrawTone_Gray */ 				else if (s.equals("ToneGray")) setValue( new Integer(GeColor.COLOR_TONE_GRAY));
/* glow_eDrawTone_YellowGreen */    		        else if (s.equals("ToneYellowGreen")) setValue( new Integer(GeColor.COLOR_TONE_YELLOWGREEN));
/* glow_eDrawTone_Yellow */ 				else if (s.equals("ToneYellow")) setValue( new Integer(GeColor.COLOR_TONE_YELLOW));
/* glow_eDrawTone_Orange */ 				else if (s.equals("ToneOrange")) setValue( new Integer(GeColor.COLOR_TONE_ORANGE));
/* glow_eDrawTone_Red */ 				else if (s.equals("ToneRed")) setValue( new Integer(GeColor.COLOR_TONE_RED));
/* glow_eDrawTone_Magenta */ 				else if (s.equals("ToneMagenta")) setValue( new Integer(GeColor.COLOR_TONE_MAGENTA));
/* glow_eDrawTone_Blue */ 				else if (s.equals("ToneBlue")) setValue( new Integer(GeColor.COLOR_TONE_BLUE));
/* glow_eDrawTone_Seablue */ 				else if (s.equals("ToneSeablue")) setValue( new Integer(GeColor.COLOR_TONE_SEABLUE));
/* glow_eDrawTone_Green */ 				else if (s.equals("ToneGreen")) setValue( new Integer(GeColor.COLOR_TONE_GREEN));
/* glow_eDrawTone_DarkGray */				else if (s.equals("ToneDarkGray")) setValue( new Integer(GeColor.COLOR_TONE_DARKGRAY));
/* glow_eDrawTone_LightGray */				else if (s.equals("ToneLightGray")) setValue( new Integer(GeColor.COLOR_TONE_LIGHTGRAY));
/* glow_eDrawTone_DarkYellowGreen */			else if (s.equals("ToneDarkYellowGreen")) setValue( new Integer(GeColor.COLOR_TONE_DARKYELLOWGREEN));
/* glow_eDrawTone_LightYellowGreen */	       		else if (s.equals("ToneLightYellowGreen")) setValue( new Integer(GeColor.COLOR_TONE_LIGHTYELLOWGREEN));
/* glow_eDrawTone_YellowGreenHighSaturation */      	else if (s.equals("ToneYellowGreenHighSaturation")) setValue( new Integer(GeColor.COLOR_TONE_YELLOWGREEN));
/* glow_eDrawTone_YellowGreenLowSaturation */       	else if (s.equals("ToneYellowGreenLowSaturation")) setValue( new Integer(GeColor.COLOR_TONE_YELLOWGREENLOWSATURATION));
/* glow_eDrawTone_DarkYellowGreenHighSaturation */	else if (s.equals("ToneDarkYellowGreenHighSaturation")) setValue( new Integer(GeColor.COLOR_TONE_DARKYELLOWGREENHIGHSATURATION));
/* glow_eDrawTone_DarkYellowGreenLowSaturation */   	else if (s.equals("ToneDarkYellowGreenLowSaturation")) setValue( new Integer(GeColor.COLOR_TONE_DARKYELLOWGREENLOWSATURATION));
/* glow_eDrawTone_LightYellowGreenHighSaturation */	else if (s.equals("ToneLightYellowGreenHighSaturation")) setValue( new Integer(GeColor.COLOR_TONE_LIGHTYELLOWGREENHIGHSATURATION));
/* glow_eDrawTone_LightYellowGreenLowSaturation */	else if (s.equals("ToneLightYellowGreenLowSaturation")) setValue( new Integer(GeColor.COLOR_TONE_LIGHTYELLOWGREENLOWSATURATION));
/* glow_eDrawTone_DarkYellow */				else if (s.equals("ToneDarkYellow")) setValue( new Integer(GeColor.COLOR_TONE_DARKYELLOW));
/* glow_eDrawTone_LightYellow */			else if (s.equals("ToneLightYellow")) setValue( new Integer(GeColor.COLOR_TONE_LIGHTYELLOW));
/* glow_eDrawTone_YellowHighSaturation */		else if (s.equals("ToneYellowHighSaturation")) setValue( new Integer(GeColor.COLOR_TONE_YELLOWHIGHSATURATION));
/* glow_eDrawTone_YellowLowSaturation */		else if (s.equals("ToneYellowLowSaturation")) setValue( new Integer(GeColor.COLOR_TONE_YELLOWLOWSATURATION));
/* glow_eDrawTone_DarkYellowHighSaturation */       	else if (s.equals("ToneDarkYellowHighSaturation")) setValue( new Integer(GeColor.COLOR_TONE_DARKYELLOWHIGHSATURATION));
/* glow_eDrawTone_DarkYellowLowSaturation */		else if (s.equals("ToneDarkYellowLowSaturation")) setValue( new Integer(GeColor.COLOR_TONE_DARKYELLOWLOWSATURATION));
/* glow_eDrawTone_LightYellowHighSaturation */      	else if (s.equals("ToneLightYellowHighSaturation")) setValue( new Integer(GeColor.COLOR_TONE_LIGHTYELLOWHIGHSATURATION));
/* glow_eDrawTone_LightYellowLowSaturation */       	else if (s.equals("ToneLightYellowLowSaturation")) setValue( new Integer(GeColor.COLOR_TONE_LIGHTYELLOWLOWSATURATION));
/* glow_eDrawTone_DarkOrange */				else if (s.equals("ToneDarkOrange")) setValue( new Integer(GeColor.COLOR_TONE_DARKORANGE));
/* glow_eDrawTone_LightOrange */			else if (s.equals("ToneLightOrange")) setValue( new Integer(GeColor.COLOR_TONE_LIGHTORANGE));
/* glow_eDrawTone_OrangeHighSaturation */		else if (s.equals("ToneOrangeHighSaturation")) setValue( new Integer(GeColor.COLOR_TONE_ORANGEHIGHSATURATION));
/* glow_eDrawTone_OrangeLowSaturation */		else if (s.equals("ToneOrangeLowSaturation")) setValue( new Integer(GeColor.COLOR_TONE_ORANGELOWSATURATION));
/* glow_eDrawTone_DarkOrangeHighSaturation */		else if (s.equals("ToneDarkOrangeHighSaturation")) setValue( new Integer(GeColor.COLOR_TONE_DARKORANGEHIGHSATURATION));
/* glow_eDrawTone_DarkOrangeLowSaturation */		else if (s.equals("ToneDarkOrangeLowSaturation")) setValue( new Integer(GeColor.COLOR_TONE_DARKORANGELOWSATURATION));
/* glow_eDrawTone_LightOrangeHighSaturation */		else if (s.equals("ToneLightOrangeHighSaturation")) setValue( new Integer(GeColor.COLOR_TONE_LIGHTORANGEHIGHSATURATION));
/* glow_eDrawTone_LightOrangeLowSaturation */		else if (s.equals("ToneLightOrangeLowSaturation")) setValue( new Integer(GeColor.COLOR_TONE_LIGHTORANGELOWSATURATION));
/* glow_eDrawTone_DarkRed */				else if (s.equals("ToneDarkRed")) setValue( new Integer(GeColor.COLOR_TONE_DARKRED));
/* glow_eDrawTone_LightRed */				else if (s.equals("ToneLightRed")) setValue( new Integer(GeColor.COLOR_TONE_LIGHTRED));
/* glow_eDrawTone_RedHighSaturation */			else if (s.equals("ToneRedHighSaturation")) setValue( new Integer(GeColor.COLOR_TONE_REDHIGHSATURATION));
/* glow_eDrawTone_RedLowSaturation */			else if (s.equals("ToneRedLowSaturation")) setValue( new Integer(GeColor.COLOR_TONE_REDLOWSATURATION));
/* glow_eDrawTone_DarkRedHighSaturation */		else if (s.equals("ToneDarkRedHighSaturation")) setValue( new Integer(GeColor.COLOR_TONE_DARKREDHIGHSATURATION));
/* glow_eDrawTone_DarkRedLowSaturation */		else if (s.equals("ToneDarkRedLowSaturation")) setValue( new Integer(GeColor.COLOR_TONE_DARKREDLOWSATURATION));
/* glow_eDrawTone_LightRedHighSaturation */		else if (s.equals("ToneLightRedHighSaturation")) setValue( new Integer(GeColor.COLOR_TONE_LIGHTREDHIGHSATURATION));
/* glow_eDrawTone_LightRedLowSaturation */		else if (s.equals("ToneLightRedLowSaturation")) setValue( new Integer(GeColor.COLOR_TONE_LIGHTREDLOWSATURATION));
/* glow_eDrawTone_DarkMagenta */			else if (s.equals("ToneDarkMagenta")) setValue( new Integer(GeColor.COLOR_TONE_DARKMAGENTA));
/* glow_eDrawTone_LightMagenta */			else if (s.equals("ToneLightMagenta")) setValue( new Integer(GeColor.COLOR_TONE_LIGHTMAGENTA));
/* glow_eDrawTone_MagentaHighSaturation */		else if (s.equals("ToneMagentaHighSaturation")) setValue( new Integer(GeColor.COLOR_TONE_MAGENTAHIGHSATURATION));
/* glow_eDrawTone_MagentaLowSaturation */		else if (s.equals("ToneMagentaLowSaturation")) setValue( new Integer(GeColor.COLOR_TONE_MAGENTALOWSATURATION));
/* glow_eDrawTone_DarkMagentaHighSaturation */		else if (s.equals("ToneDarkMagentaHighSaturation")) setValue( new Integer(GeColor.COLOR_TONE_DARKMAGENTAHIGHSATURATION));
/* glow_eDrawTone_DarkMagentaLowSaturation */		else if (s.equals("ToneDarkMagentaLowSaturation")) setValue( new Integer(GeColor.COLOR_TONE_DARKMAGENTALOWSATURATION));
/* glow_eDrawTone_LightMagentaHighSaturation */		else if (s.equals("ToneLightMagentaHighSaturation")) setValue( new Integer(GeColor.COLOR_TONE_LIGHTMAGENTAHIGHSATURATION));
/* glow_eDrawTone_LightMagentaLowSaturation */		else if (s.equals("ToneLightMagentaLowSaturation")) setValue( new Integer(GeColor.COLOR_TONE_LIGHTMAGENTALOWSATURATION));
/* glow_eDrawTone_DarkBlue */				else if (s.equals("ToneDarkBlue")) setValue( new Integer(GeColor.COLOR_TONE_DARKBLUE));
/* glow_eDrawTone_LightBlue */				else if (s.equals("ToneLightBlue")) setValue( new Integer(GeColor.COLOR_TONE_LIGHTBLUE));
/* glow_eDrawTone_BlueHighSaturation */			else if (s.equals("ToneBlueHighSaturation")) setValue( new Integer(GeColor.COLOR_TONE_BLUEHIGHSATURATION));
/* glow_eDrawTone_BlueLowSaturation */			else if (s.equals("ToneBlueLowSaturation")) setValue( new Integer(GeColor.COLOR_TONE_BLUELOWSATURATION));
/* glow_eDrawTone_DarkBlueHighSaturation */		else if (s.equals("ToneDarkBlueHighSaturation")) setValue( new Integer(GeColor.COLOR_TONE_DARKBLUEHIGHSATURATION));
/* glow_eDrawTone_DarkBlueLowSaturation */		else if (s.equals("ToneDarkBlueLowSaturation")) setValue( new Integer(GeColor.COLOR_TONE_DARKBLUELOWSATURATION));
/* glow_eDrawTone_LightBlueHighSaturation */		else if (s.equals("ToneLightBlueHighSaturation")) setValue( new Integer(GeColor.COLOR_TONE_LIGHTBLUEHIGHSATURATION));
/* glow_eDrawTone_LightBlueLowSaturation */		else if (s.equals("ToneLightBlueLowSaturation")) setValue( new Integer(GeColor.COLOR_TONE_LIGHTBLUELOWSATURATION));
/* glow_eDrawTone_DarkSeablue */			else if (s.equals("ToneDarkSeablue")) setValue( new Integer(GeColor.COLOR_TONE_DARKSEABLUE));
/* glow_eDrawTone_LightSeablue */			else if (s.equals("ToneLightSeablue")) setValue( new Integer(GeColor.COLOR_TONE_LIGHTSEABLUE));
/* glow_eDrawTone_SeablueHighSaturation */		else if (s.equals("ToneSeablueHighSaturation")) setValue( new Integer(GeColor.COLOR_TONE_SEABLUEHIGHSATURATION));
/* glow_eDrawTone_SeablueLowSaturation */		else if (s.equals("ToneSeablueLowSaturation")) setValue( new Integer(GeColor.COLOR_TONE_SEABLUELOWSATURATION));
/* glow_eDrawTone_DarkSeablueHighSaturation */		else if (s.equals("ToneDarkSeablueHighSaturation")) setValue( new Integer(GeColor.COLOR_TONE_DARKSEABLUEHIGHSATURATION));
/* glow_eDrawTone_DarkSeablueLowSaturation */	 	else if (s.equals("ToneDarkSeablueLowSaturation")) setValue( new Integer(GeColor.COLOR_TONE_DARKSEABLUELOWSATURATION));
/* glow_eDrawTone_LightSeablueHighSaturation */		else if (s.equals("ToneLightSeablueHighSaturation")) setValue( new Integer(GeColor.COLOR_TONE_LIGHTSEABLUEHIGHSATURATION));
/* glow_eDrawTone_LightSeablueLowSaturation */		else if (s.equals("ToneLightSeablueLowSaturation")) setValue( new Integer(GeColor.COLOR_TONE_LIGHTSEABLUELOWSATURATION));
/* glow_eDrawTone_DarkGreen */				else if (s.equals("ToneDarkGreen")) setValue( new Integer(GeColor.COLOR_TONE_DARKGREEN));
/* glow_eDrawTone_LightGreen */				else if (s.equals("ToneLightGreen")) setValue( new Integer(GeColor.COLOR_TONE_LIGHTGREEN));
/* glow_eDrawTone_GreenHighSaturation */		else if (s.equals("ToneGreenHighSaturation")) setValue( new Integer(GeColor.COLOR_TONE_GREENHIGHSATURATION));
/* glow_eDrawTone_GreenLowSaturation */			else if (s.equals("ToneGreenLowSaturation")) setValue( new Integer(GeColor.COLOR_TONE_GREENLOWSATURATION));
/* glow_eDrawTone_DarkGreenHighSaturation */		else if (s.equals("ToneDarkGreenHighSaturation")) setValue( new Integer(GeColor.COLOR_TONE_DARKGREENHIGHSATURATION));
/* glow_eDrawTone_DarkGreenLowSaturation */		else if (s.equals("ToneDarkGreenLowSaturation")) setValue( new Integer(GeColor.COLOR_TONE_DARKGREENLOWSATURATION));
/* glow_eDrawTone_LightGreenHighSaturation */		else if (s.equals("ToneLightGreenHighSaturation")) setValue( new Integer(GeColor.COLOR_TONE_LIGHTGREENHIGHSATURATION));
/* glow_eDrawTone_LightGreenLowSaturation */		else if (s.equals("ToneLightGreenLowSaturation")) setValue( new Integer(GeColor.COLOR_TONE_LIGHTGREENLOWSATURATION));
    else throw new IllegalArgumentException(s);
  }
}


