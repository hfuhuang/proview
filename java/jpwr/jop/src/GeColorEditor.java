
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

public class GeColorEditor extends PropertyEditorSupport {

  public GeColorEditor() {
  }
  private static String[] tagStrings = {
        /* glow_eDrawType_Line */      	"Black",
	/* glow_eDrawType_LineRed */ 	"Red",
	/* glow_eDrawType_LineGray */ 	"Grey",
	/* glow_eDrawType_Color4 */    	"White",
	/* glow_eDrawType_Color5 */    	"YellowGreen",
	/* glow_eDrawType_Color6 */    	"Yellow",
	/* glow_eDrawType_Color7 */    	"Gold",
	/* glow_eDrawType_Color8 */    	"Orange",
	/* glow_eDrawType_Color9 */ 	"OrangeRed",
	/* glow_eDrawType_Color10 */ 	"Red",
	/* glow_eDrawType_Color11 */ 	"RedViolet",
	/* glow_eDrawType_Color12 */ 	"Violet",
	/* glow_eDrawType_Color13 */ 	"BlueViolet",
	/* glow_eDrawType_Color14 */ 	"BlueBlueViolet",
	/* glow_eDrawType_Color15 */ 	"Blue",
	/* glow_eDrawType_Color16 */ 	"BlueBlueGreen",
	/* glow_eDrawType_Color17 */ 	"BlueGreen",
	/* glow_eDrawType_Color18 */ 	"GreenGreenBlue",
	/* glow_eDrawType_Color19 */ 	"Green",
	/* glow_eDrawType_Color20 */ 	"GreenGreenYellow",
	/* glow_eDrawType_Color21 */ 	"GrayFix1",
	/* glow_eDrawType_Color22 */ 	"GrayFix2",
	/* glow_eDrawType_Color23 */ 	"GrayFix3",
	/* glow_eDrawType_Color24 */ 	"GrayFix4",
	/* glow_eDrawType_Color25 */ 	"GrayFix5",
	/* glow_eDrawType_Color26 */ 	"GrayFix6",
	/* glow_eDrawType_Color27 */ 	"GrayFix7",
	/* glow_eDrawType_Color28 */ 	"GrayFix8",
	/* glow_eDrawType_Color29 */ 	"GrayFix9",
	/* glow_eDrawType_Color30 */ 	"GrayFix10",
	/* glow_eDrawType_Color31 */ 	"GrayLow1",
	/* glow_eDrawType_Color32 */ 	"GrayLow2",
	/* glow_eDrawType_Color33 */ 	"GrayLow3",
	/* glow_eDrawType_Color34 */ 	"GrayLow4",
	/* glow_eDrawType_Color35 */ 	"GrayLow5",
	/* glow_eDrawType_Color36 */ 	"GrayLow6",
	/* glow_eDrawType_Color37 */ 	"GrayLow7",
	/* glow_eDrawType_Color38 */ 	"GrayLow8",
	/* glow_eDrawType_Color39 */ 	"GrayLow9",
	/* glow_eDrawType_Color40 */ 	"GrayLow10",
	/* glow_eDrawType_Color41 */ 	"GrayMedium1",
	/* glow_eDrawType_Color42 */ 	"GrayMedium2",
	/* glow_eDrawType_Color43 */ 	"GrayMedium3",
	/* glow_eDrawType_Color44 */ 	"GrayMedium4",
	/* glow_eDrawType_Color45 */ 	"GrayMedium5",
	/* glow_eDrawType_Color46 */ 	"GrayMedium6",
	/* glow_eDrawType_Color47 */ 	"GrayMedium7",
	/* glow_eDrawType_Color48 */ 	"GrayMedium8",
	/* glow_eDrawType_Color49 */ 	"GrayMedium9",
	/* glow_eDrawType_Color50 */ 	"GrayMedium10",
	/* glow_eDrawType_Color51 */ 	"GrayHigh1",
	/* glow_eDrawType_Color52 */ 	"GrayHigh2",
	/* glow_eDrawType_Color53 */ 	"GrayHigh3",
	/* glow_eDrawType_Color54 */ 	"GrayHigh4",
	/* glow_eDrawType_Color55 */ 	"GrayHigh5",
	/* glow_eDrawType_Color56 */ 	"GrayHigh6",
	/* glow_eDrawType_Color57 */ 	"GrayHigh7",
	/* glow_eDrawType_Color58 */ 	"GrayHigh8",
	/* glow_eDrawType_Color59 */ 	"GrayHigh9",
	/* glow_eDrawType_Color60 */ 	"GrayHigh10",
	/* glow_eDrawType_Color61 */ 	"YellowGreenLow1",
	/* glow_eDrawType_Color62 */ 	"YellowGreenLow2",
	/* glow_eDrawType_Color63 */ 	"YellowGreenLow3",
	/* glow_eDrawType_Color64 */ 	"YellowGreenLow4",
	/* glow_eDrawType_Color65 */ 	"YellowGreenLow5",
	/* glow_eDrawType_Color66 */ 	"YellowGreenLow6",
	/* glow_eDrawType_Color67 */ 	"YellowGreenLow7",
	/* glow_eDrawType_Color68 */ 	"YellowGreenLow8",
	/* glow_eDrawType_Color69 */ 	"YellowGreenLow9",
	/* glow_eDrawType_Color70 */ 	"YellowGreenLow10",
	/* glow_eDrawType_Color71 */ 	"YellowGreenMedium1",
	/* glow_eDrawType_Color72 */ 	"YellowGreenMedium2",
	/* glow_eDrawType_Color73 */ 	"YellowGreenMedium3",
	/* glow_eDrawType_Color74 */ 	"YellowGreenMedium4",
	/* glow_eDrawType_Color75 */ 	"YellowGreenMedium5",
	/* glow_eDrawType_Color76 */ 	"YellowGreenMedium6",
	/* glow_eDrawType_Color77 */ 	"YellowGreenMedium7",
	/* glow_eDrawType_Color78 */ 	"YellowGreenMedium8",
	/* glow_eDrawType_Color79 */ 	"YellowGreenMedium9",
	/* glow_eDrawType_Color80 */ 	"YellowGreenMedium10",
	/* glow_eDrawType_Color81 */ 	"YellowGreenHigh1",
	/* glow_eDrawType_Color82 */ 	"YellowGreenHigh2",
	/* glow_eDrawType_Color83 */ 	"YellowGreenHigh3",
	/* glow_eDrawType_Color84 */ 	"YellowGreenHigh4",
	/* glow_eDrawType_Color85 */ 	"YellowGreenHigh5",
	/* glow_eDrawType_Color86 */ 	"YellowGreenHigh6",
	/* glow_eDrawType_Color87 */ 	"YellowGreenHigh7",
	/* glow_eDrawType_Color88 */ 	"YellowGreenHigh8",
	/* glow_eDrawType_Color89 */ 	"YellowGreenHigh9",
	/* glow_eDrawType_Color90 */ 	"YellowGreenHigh10",
	/* glow_eDrawType_Color91 */ 	"YellowLow1",
	/* glow_eDrawType_Color92 */ 	"YellowLow2",
	/* glow_eDrawType_Color93 */ 	"YellowLow3",
	/* glow_eDrawType_Color94 */ 	"YellowLow4",
	/* glow_eDrawType_Color95 */ 	"YellowLow5",
	/* glow_eDrawType_Color96 */ 	"YellowLow6",
	/* glow_eDrawType_Color97 */ 	"YellowLow7",
	/* glow_eDrawType_Color98 */ 	"YellowLow8",
	/* glow_eDrawType_Color99 */ 	"YellowLow9",
	/* glow_eDrawType_Color100 */ 	"YellowLow10",
	/* glow_eDrawType_Color101 */ 	"YellowMedium1",
	/* glow_eDrawType_Color102 */ 	"YellowMedium2",
	/* glow_eDrawType_Color103 */ 	"YellowMedium3",
	/* glow_eDrawType_Color104 */ 	"YellowMedium4",
	/* glow_eDrawType_Color105 */ 	"YellowMedium5",
	/* glow_eDrawType_Color106 */ 	"YellowMedium6",
	/* glow_eDrawType_Color107 */ 	"YellowMedium7",
	/* glow_eDrawType_Color108 */ 	"YellowMedium8",
	/* glow_eDrawType_Color109 */ 	"YellowMedium9",
	/* glow_eDrawType_Color110 */ 	"YellowMedium10",
	/* glow_eDrawType_Color111 */ 	"YellowHigh1",
	/* glow_eDrawType_Color112 */ 	"YellowHigh2",
	/* glow_eDrawType_Color113 */ 	"YellowHigh3",
	/* glow_eDrawType_Color114 */ 	"YellowHigh4",
	/* glow_eDrawType_Color115 */ 	"YellowHigh5",
	/* glow_eDrawType_Color116 */ 	"YellowHigh6",
	/* glow_eDrawType_Color117 */ 	"YellowHigh7",
	/* glow_eDrawType_Color118 */ 	"YellowHigh8",
	/* glow_eDrawType_Color119 */ 	"YellowHigh9",
	/* glow_eDrawType_Color120 */ 	"YellowHigh10",
	/* glow_eDrawType_Color121 */ 	"OrangeLow1",
	/* glow_eDrawType_Color122 */ 	"OrangeLow2",
	/* glow_eDrawType_Color123 */ 	"OrangeLow3",
	/* glow_eDrawType_Color124 */ 	"OrangeLow4",
	/* glow_eDrawType_Color125 */ 	"OrangeLow5",
	/* glow_eDrawType_Color126 */ 	"OrangeLow6",
	/* glow_eDrawType_Color127 */ 	"OrangeLow7",
	/* glow_eDrawType_Color128 */ 	"OrangeLow8",
	/* glow_eDrawType_Color129 */ 	"OrangeLow9",
	/* glow_eDrawType_Color130 */ 	"OrangeLow10",
	/* glow_eDrawType_Color131 */ 	"OrangeMedium1",
	/* glow_eDrawType_Color132 */ 	"OrangeMedium2",
	/* glow_eDrawType_Color133 */ 	"OrangeMedium3",
	/* glow_eDrawType_Color134 */ 	"OrangeMedium4",
	/* glow_eDrawType_Color135 */ 	"OrangeMedium5",
	/* glow_eDrawType_Color136 */ 	"OrangeMedium6",
	/* glow_eDrawType_Color137 */ 	"OrangeMedium7",
	/* glow_eDrawType_Color138 */ 	"OrangeMedium8",
	/* glow_eDrawType_Color139 */ 	"OrangeMedium9",
	/* glow_eDrawType_Color140 */ 	"OrangeMedium10",
	/* glow_eDrawType_Color141 */ 	"OrangeHigh1",
	/* glow_eDrawType_Color142 */ 	"OrangeHigh2",
	/* glow_eDrawType_Color143 */ 	"OrangeHigh3",
	/* glow_eDrawType_Color144 */ 	"OrangeHigh4",
	/* glow_eDrawType_Color145 */ 	"OrangeHigh5",
	/* glow_eDrawType_Color146 */ 	"OrangeHigh6",
	/* glow_eDrawType_Color147 */ 	"OrangeHigh7",
	/* glow_eDrawType_Color148 */ 	"OrangeHigh8",
	/* glow_eDrawType_Color149 */ 	"OrangeHigh9",
	/* glow_eDrawType_Color150 */ 	"OrangeHigh10",
	/* glow_eDrawType_Color151 */ 	"RedLow1",
	/* glow_eDrawType_Color152 */ 	"RedLow2",
	/* glow_eDrawType_Color153 */ 	"RedLow3",
	/* glow_eDrawType_Color154 */ 	"RedLow4",
	/* glow_eDrawType_Color155 */ 	"RedLow5",
	/* glow_eDrawType_Color156 */ 	"RedLow6",
	/* glow_eDrawType_Color157 */ 	"RedLow7",
	/* glow_eDrawType_Color158 */ 	"RedLow8",
	/* glow_eDrawType_Color159 */ 	"RedLow9",
	/* glow_eDrawType_Color160 */ 	"RedLow10",
	/* glow_eDrawType_Color161 */ 	"RedMedium1",
	/* glow_eDrawType_Color162 */ 	"RedMedium2",
	/* glow_eDrawType_Color163 */ 	"RedMedium3",
	/* glow_eDrawType_Color164 */ 	"RedMedium4",
	/* glow_eDrawType_Color165 */ 	"RedMedium5",
	/* glow_eDrawType_Color166 */ 	"RedMedium6",
	/* glow_eDrawType_Color167 */ 	"RedMedium7",
	/* glow_eDrawType_Color168 */ 	"RedMedium8",
	/* glow_eDrawType_Color169 */ 	"RedMedium9",
	/* glow_eDrawType_Color170 */ 	"RedMedium10",
	/* glow_eDrawType_Color171 */ 	"RedHigh1",
	/* glow_eDrawType_Color172 */ 	"RedHigh2",
	/* glow_eDrawType_Color173 */ 	"RedHigh3",
	/* glow_eDrawType_Color174 */ 	"RedHigh4",
	/* glow_eDrawType_Color175 */ 	"RedHigh5",
	/* glow_eDrawType_Color176 */ 	"RedHigh6",
	/* glow_eDrawType_Color177 */ 	"RedHigh7",
	/* glow_eDrawType_Color178 */ 	"RedHigh8",
	/* glow_eDrawType_Color179 */ 	"RedHigh9",
	/* glow_eDrawType_Color180 */ 	"RedHigh10",
	/* glow_eDrawType_Color181 */ 	"MagentaLow1",
	/* glow_eDrawType_Color182 */ 	"MagentaLow2",
	/* glow_eDrawType_Color183 */ 	"MagentaLow3",
	/* glow_eDrawType_Color184 */ 	"MagentaLow4",
	/* glow_eDrawType_Color185 */ 	"MagentaLow5",
	/* glow_eDrawType_Color186 */ 	"MagentaLow6",
	/* glow_eDrawType_Color187 */ 	"MagentaLow7",
	/* glow_eDrawType_Color188 */ 	"MagentaLow8",
	/* glow_eDrawType_Color189 */ 	"MagentaLow9",
	/* glow_eDrawType_Color190 */ 	"MagentaLow10",
	/* glow_eDrawType_Color191 */ 	"MagentaMedium1",
	/* glow_eDrawType_Color192 */ 	"MagentaMedium2",
	/* glow_eDrawType_Color193 */ 	"MagentaMedium3",
	/* glow_eDrawType_Color194 */ 	"MagentaMedium4",
	/* glow_eDrawType_Color195 */ 	"MagentaMedium5",
	/* glow_eDrawType_Color196 */ 	"MagentaMedium6",
	/* glow_eDrawType_Color197 */ 	"MagentaMedium7",
	/* glow_eDrawType_Color198 */ 	"MagentaMedium8",
	/* glow_eDrawType_Color199 */ 	"MagentaMedium9",
	/* glow_eDrawType_Color200 */ 	"MagentaMedium10",
	/* glow_eDrawType_Color201 */ 	"MagentaHigh1",
	/* glow_eDrawType_Color202 */ 	"MagentaHigh2",
	/* glow_eDrawType_Color203 */ 	"MagentaHigh3",
	/* glow_eDrawType_Color204 */ 	"MagentaHigh4",
	/* glow_eDrawType_Color205 */ 	"MagentaHigh5",
	/* glow_eDrawType_Color206 */ 	"MagentaHigh6",
	/* glow_eDrawType_Color207 */ 	"MagentaHigh7",
	/* glow_eDrawType_Color208 */ 	"MagentaHigh8",
	/* glow_eDrawType_Color209 */ 	"MagentaHigh9",
	/* glow_eDrawType_Color210 */ 	"MagentaHigh10",
	/* glow_eDrawType_Color211 */ 	"BlueLow1",
	/* glow_eDrawType_Color212 */ 	"BlueLow2",
	/* glow_eDrawType_Color213 */ 	"BlueLow3",
	/* glow_eDrawType_Color214 */ 	"BlueLow4",
	/* glow_eDrawType_Color215 */ 	"BlueLow5",
	/* glow_eDrawType_Color216 */ 	"BlueLow6",
	/* glow_eDrawType_Color217 */ 	"BlueLow7",
	/* glow_eDrawType_Color218 */ 	"BlueLow8",
	/* glow_eDrawType_Color219 */ 	"BlueLow9",
	/* glow_eDrawType_Color220 */ 	"BlueLow10",
	/* glow_eDrawType_Color221 */ 	"BlueMedium1",
	/* glow_eDrawType_Color222 */ 	"BlueMedium2",
	/* glow_eDrawType_Color223 */ 	"BlueMedium3",
	/* glow_eDrawType_Color224 */ 	"BlueMedium4",
	/* glow_eDrawType_Color225 */ 	"BlueMedium5",
	/* glow_eDrawType_Color226 */ 	"BlueMedium6",
	/* glow_eDrawType_Color227 */ 	"BlueMedium7",
	/* glow_eDrawType_Color228 */ 	"BlueMedium8",
	/* glow_eDrawType_Color229 */ 	"BlueMedium9",
	/* glow_eDrawType_Color230 */ 	"BlueMedium10",
	/* glow_eDrawType_Color231 */ 	"BlueHigh1",
	/* glow_eDrawType_Color232 */ 	"BlueHigh2",
	/* glow_eDrawType_Color233 */ 	"BlueHigh3",
	/* glow_eDrawType_Color234 */ 	"BlueHigh4",
	/* glow_eDrawType_Color235 */ 	"BlueHigh5",
	/* glow_eDrawType_Color236 */ 	"BlueHigh6",
	/* glow_eDrawType_Color237 */ 	"BlueHigh7",
	/* glow_eDrawType_Color238 */ 	"BlueHigh8",
	/* glow_eDrawType_Color239 */ 	"BlueHigh9",
	/* glow_eDrawType_Color240 */ 	"BlueHigh10",
	/* glow_eDrawType_Color241 */ 	"SeaBlueLow1",
	/* glow_eDrawType_Color242 */ 	"SeaBlueLow2",
	/* glow_eDrawType_Color243 */ 	"SeaBlueLow3",
	/* glow_eDrawType_Color244 */ 	"SeaBlueLow4",
	/* glow_eDrawType_Color245 */ 	"SeaBlueLow5",
	/* glow_eDrawType_Color246 */ 	"SeaBlueLow6",
	/* glow_eDrawType_Color247 */ 	"SeaBlueLow7",
	/* glow_eDrawType_Color248 */ 	"SeaBlueLow8",
	/* glow_eDrawType_Color249 */ 	"SeaBlueLow9",
	/* glow_eDrawType_Color250 */ 	"SeaBlueLow10",
	/* glow_eDrawType_Color251 */ 	"SeaBlueMedium1",
	/* glow_eDrawType_Color252 */ 	"SeaBlueMedium2",
	/* glow_eDrawType_Color253 */ 	"SeaBlueMedium3",
	/* glow_eDrawType_Color224 */ 	"SeaBlueMedium4",
	/* glow_eDrawType_Color255 */ 	"SeaBlueMedium5",
	/* glow_eDrawType_Color256 */ 	"SeaBlueMedium6",
	/* glow_eDrawType_Color257 */ 	"SeaBlueMedium7",
	/* glow_eDrawType_Color258 */ 	"SeaBlueMedium8",
	/* glow_eDrawType_Color259 */ 	"SeaBlueMedium9",
	/* glow_eDrawType_Color260 */ 	"SeaBlueMedium10",
	/* glow_eDrawType_Color261 */ 	"SeaBlueHigh1",
	/* glow_eDrawType_Color262 */ 	"SeaBlueHigh2",
	/* glow_eDrawType_Color263 */ 	"SeaBlueHigh3",
	/* glow_eDrawType_Color264 */ 	"SeaBlueHigh4",
	/* glow_eDrawType_Color265 */ 	"SeaBlueHigh5",
	/* glow_eDrawType_Color266 */ 	"SeaBlueHigh6",
	/* glow_eDrawType_Color267 */ 	"SeaBlueHigh7",
	/* glow_eDrawType_Color268 */ 	"SeaBlueHigh8",
	/* glow_eDrawType_Color269 */ 	"SeaBlueHigh9",
	/* glow_eDrawType_Color270 */ 	"SeaBlueHigh10",
	/* glow_eDrawType_Color271 */ 	"GreenLow1",
	/* glow_eDrawType_Color272 */ 	"GreenLow2",
	/* glow_eDrawType_Color273 */ 	"GreenLow3",
	/* glow_eDrawType_Color274 */ 	"GreenLow4",
	/* glow_eDrawType_Color275 */ 	"GreenLow5",
	/* glow_eDrawType_Color276 */ 	"GreenLow6",
	/* glow_eDrawType_Color277 */ 	"GreenLow7",
	/* glow_eDrawType_Color278 */ 	"GreenLow8",
	/* glow_eDrawType_Color279 */ 	"GreenLow9",
	/* glow_eDrawType_Color280 */ 	"GreenLow10",
	/* glow_eDrawType_Color281 */ 	"GreenMedium1",
	/* glow_eDrawType_Color282 */ 	"GreenMedium2",
	/* glow_eDrawType_Color283 */ 	"GreenMedium3",
	/* glow_eDrawType_Color284 */ 	"GreenMedium4",
	/* glow_eDrawType_Color285 */ 	"GreenMedium5",
	/* glow_eDrawType_Color286 */ 	"GreenMedium6",
	/* glow_eDrawType_Color287 */ 	"GreenMedium7",
	/* glow_eDrawType_Color288 */ 	"GreenMedium8",
	/* glow_eDrawType_Color289 */ 	"GreenMedium9",
	/* glow_eDrawType_Color290 */ 	"GreenMedium10",
	/* glow_eDrawType_Color291 */ 	"GreenHigh1",
	/* glow_eDrawType_Color292 */ 	"GreenHigh2",
	/* glow_eDrawType_Color293 */ 	"GreenHigh3",
	/* glow_eDrawType_Color294 */ 	"GreenHigh4",
	/* glow_eDrawType_Color295 */ 	"GreenHigh5",
	/* glow_eDrawType_Color296 */ 	"GreenHigh6",
	/* glow_eDrawType_Color297 */ 	"GreenHigh7",
	/* glow_eDrawType_Color298 */ 	"GreenHigh8",
	/* glow_eDrawType_Color299 */ 	"GreenHigh9",
        /* glow_eDrawType_Color300 */  	"GreenHigh10",
	"Inherit",
	};
  public String[] getTags() {
    return tagStrings;
  }
  public String getJavaInitializationString() {
    return java.lang.String.valueOf(((Number)getValue()).intValue());
  }
  public void setAsText(String s) throws IllegalArgumentException {
    if (s.equals("Inherit")) setValue( new Integer(GeColor.COLOR_INHERIT));
/* glow_eDrawType_Line */      	else if (s.equals("Black")) setValue( new Integer(GeColor.COLOR_1));
/* glow_eDrawType_LineRed */ 	else if (s.equals("Red")) setValue( new Integer(GeColor.COLOR_2));
/* glow_eDrawType_LineGray */ 	else if (s.equals("Grey")) setValue( new Integer(GeColor.COLOR_3));
/* glow_eDrawType_Color4 */    	else if (s.equals("White")) setValue( new Integer(GeColor.COLOR_4));
/* glow_eDrawType_Color5 */    	else if (s.equals("YellowGreen")) setValue( new Integer(GeColor.COLOR_5));
/* glow_eDrawType_Color6 */    	else if (s.equals("Yellow")) setValue( new Integer(GeColor.COLOR_6));
/* glow_eDrawType_Color7 */    	else if (s.equals("Gold")) setValue( new Integer(GeColor.COLOR_7));
/* glow_eDrawType_Color8 */    	else if (s.equals("Orange")) setValue( new Integer(GeColor.COLOR_8));
/* glow_eDrawType_Color9 */ 	else if (s.equals("OrangeRed")) setValue( new Integer(GeColor.COLOR_9));
/* glow_eDrawType_Color10 */ 	else if (s.equals("Red")) setValue( new Integer(GeColor.COLOR_10));
/* glow_eDrawType_Color11 */ 	else if (s.equals("RedViolet")) setValue( new Integer(GeColor.COLOR_11));
/* glow_eDrawType_Color12 */ 	else if (s.equals("Violet")) setValue( new Integer(GeColor.COLOR_12));
/* glow_eDrawType_Color13 */ 	else if (s.equals("BlueViolet")) setValue( new Integer(GeColor.COLOR_13));
/* glow_eDrawType_Color14 */ 	else if (s.equals("BlueBlueViolet")) setValue( new Integer(GeColor.COLOR_14));
/* glow_eDrawType_Color15 */ 	else if (s.equals("Blue")) setValue( new Integer(GeColor.COLOR_15));
/* glow_eDrawType_Color16 */ 	else if (s.equals("BlueBlueGreen")) setValue( new Integer(GeColor.COLOR_16));
/* glow_eDrawType_Color17 */ 	else if (s.equals("BlueGreen")) setValue( new Integer(GeColor.COLOR_17));
/* glow_eDrawType_Color18 */ 	else if (s.equals("GreenGreenBlue")) setValue( new Integer(GeColor.COLOR_18));
/* glow_eDrawType_Color19 */ 	else if (s.equals("Green")) setValue( new Integer(GeColor.COLOR_19));
/* glow_eDrawType_Color20 */ 	else if (s.equals("GreenGreenYellow")) setValue( new Integer(GeColor.COLOR_20));
/* glow_eDrawType_Color21 */ 	else if (s.equals("GrayFix1")) setValue( new Integer(GeColor.COLOR_21));
/* glow_eDrawType_Color22 */ 	else if (s.equals("GrayFix2")) setValue( new Integer(GeColor.COLOR_22));
/* glow_eDrawType_Color23 */ 	else if (s.equals("GrayFix3")) setValue( new Integer(GeColor.COLOR_23));
/* glow_eDrawType_Color24 */ 	else if (s.equals("GrayFix4")) setValue( new Integer(GeColor.COLOR_24));
/* glow_eDrawType_Color25 */ 	else if (s.equals("GrayFix5")) setValue( new Integer(GeColor.COLOR_25));
/* glow_eDrawType_Color26 */ 	else if (s.equals("GrayFix6")) setValue( new Integer(GeColor.COLOR_26));
/* glow_eDrawType_Color27 */ 	else if (s.equals("GrayFix7")) setValue( new Integer(GeColor.COLOR_27));
/* glow_eDrawType_Color28 */ 	else if (s.equals("GrayFix8")) setValue( new Integer(GeColor.COLOR_28));
/* glow_eDrawType_Color29 */ 	else if (s.equals("GrayFix9")) setValue( new Integer(GeColor.COLOR_29));
/* glow_eDrawType_Color30 */ 	else if (s.equals("GrayFix10")) setValue( new Integer(GeColor.COLOR_30));
/* glow_eDrawType_Color31 */ 	else if (s.equals("GrayLow1")) setValue( new Integer(GeColor.COLOR_31));
/* glow_eDrawType_Color32 */ 	else if (s.equals("GrayLow2")) setValue( new Integer(GeColor.COLOR_32));
/* glow_eDrawType_Color33 */ 	else if (s.equals("GrayLow3")) setValue( new Integer(GeColor.COLOR_33));
/* glow_eDrawType_Color34 */ 	else if (s.equals("GrayLow4")) setValue( new Integer(GeColor.COLOR_34));
/* glow_eDrawType_Color35 */ 	else if (s.equals("GrayLow5")) setValue( new Integer(GeColor.COLOR_35));
/* glow_eDrawType_Color36 */ 	else if (s.equals("GrayLow6")) setValue( new Integer(GeColor.COLOR_36));
/* glow_eDrawType_Color37 */ 	else if (s.equals("GrayLow7")) setValue( new Integer(GeColor.COLOR_37));
/* glow_eDrawType_Color38 */ 	else if (s.equals("GrayLow8")) setValue( new Integer(GeColor.COLOR_38));
/* glow_eDrawType_Color39 */ 	else if (s.equals("GrayLow9")) setValue( new Integer(GeColor.COLOR_39));
/* glow_eDrawType_Color40 */ 	else if (s.equals("GrayLow10")) setValue( new Integer(GeColor.COLOR_40));
/* glow_eDrawType_Color41 */ 	else if (s.equals("GrayMedium1")) setValue( new Integer(GeColor.COLOR_41));
/* glow_eDrawType_Color42 */ 	else if (s.equals("GrayMedium2")) setValue( new Integer(GeColor.COLOR_42));
/* glow_eDrawType_Color43 */ 	else if (s.equals("GrayMedium3")) setValue( new Integer(GeColor.COLOR_43));
/* glow_eDrawType_Color44 */ 	else if (s.equals("GrayMedium4")) setValue( new Integer(GeColor.COLOR_44));
/* glow_eDrawType_Color45 */ 	else if (s.equals("GrayMedium5")) setValue( new Integer(GeColor.COLOR_45));
/* glow_eDrawType_Color46 */ 	else if (s.equals("GrayMedium6")) setValue( new Integer(GeColor.COLOR_46));
/* glow_eDrawType_Color47 */ 	else if (s.equals("GrayMedium7")) setValue( new Integer(GeColor.COLOR_47));
/* glow_eDrawType_Color48 */ 	else if (s.equals("GrayMedium8")) setValue( new Integer(GeColor.COLOR_48));
/* glow_eDrawType_Color49 */ 	else if (s.equals("GrayMedium9")) setValue( new Integer(GeColor.COLOR_49));
/* glow_eDrawType_Color50 */ 	else if (s.equals("GrayMedium10")) setValue( new Integer(GeColor.COLOR_50));
/* glow_eDrawType_Color51 */ 	else if (s.equals("GrayHigh1")) setValue( new Integer(GeColor.COLOR_51));
/* glow_eDrawType_Color52 */ 	else if (s.equals("GrayHigh2")) setValue( new Integer(GeColor.COLOR_52));
/* glow_eDrawType_Color53 */ 	else if (s.equals("GrayHigh3")) setValue( new Integer(GeColor.COLOR_53));
/* glow_eDrawType_Color54 */ 	else if (s.equals("GrayHigh4")) setValue( new Integer(GeColor.COLOR_54));
/* glow_eDrawType_Color55 */ 	else if (s.equals("GrayHigh5")) setValue( new Integer(GeColor.COLOR_55));
/* glow_eDrawType_Color56 */ 	else if (s.equals("GrayHigh6")) setValue( new Integer(GeColor.COLOR_56));
/* glow_eDrawType_Color57 */ 	else if (s.equals("GrayHigh7")) setValue( new Integer(GeColor.COLOR_57));
/* glow_eDrawType_Color58 */ 	else if (s.equals("GrayHigh8")) setValue( new Integer(GeColor.COLOR_58));
/* glow_eDrawType_Color59 */ 	else if (s.equals("GrayHigh9")) setValue( new Integer(GeColor.COLOR_59));
/* glow_eDrawType_Color60 */ 	else if (s.equals("GrayHigh10")) setValue( new Integer(GeColor.COLOR_60));
/* glow_eDrawType_Color61 */ 	else if (s.equals("YellowGreenLow1")) setValue( new Integer(GeColor.COLOR_61));
/* glow_eDrawType_Color62 */ 	else if (s.equals("YellowGreenLow2")) setValue( new Integer(GeColor.COLOR_62));
/* glow_eDrawType_Color63 */ 	else if (s.equals("YellowGreenLow3")) setValue( new Integer(GeColor.COLOR_63));
/* glow_eDrawType_Color64 */ 	else if (s.equals("YellowGreenLow4")) setValue( new Integer(GeColor.COLOR_64));
/* glow_eDrawType_Color65 */ 	else if (s.equals("YellowGreenLow5")) setValue( new Integer(GeColor.COLOR_65));
/* glow_eDrawType_Color66 */ 	else if (s.equals("YellowGreenLow6")) setValue( new Integer(GeColor.COLOR_66));
/* glow_eDrawType_Color67 */ 	else if (s.equals("YellowGreenLow7")) setValue( new Integer(GeColor.COLOR_67));
/* glow_eDrawType_Color68 */ 	else if (s.equals("YellowGreenLow8")) setValue( new Integer(GeColor.COLOR_68));
/* glow_eDrawType_Color69 */ 	else if (s.equals("YellowGreenLow9")) setValue( new Integer(GeColor.COLOR_69));
/* glow_eDrawType_Color70 */ 	else if (s.equals("YellowGreenLow10")) setValue( new Integer(GeColor.COLOR_70));
/* glow_eDrawType_Color71 */ 	else if (s.equals("YellowGreenMedium1")) setValue( new Integer(GeColor.COLOR_71));
/* glow_eDrawType_Color72 */ 	else if (s.equals("YellowGreenMedium2")) setValue( new Integer(GeColor.COLOR_72));
/* glow_eDrawType_Color73 */ 	else if (s.equals("YellowGreenMedium3")) setValue( new Integer(GeColor.COLOR_73));
/* glow_eDrawType_Color74 */ 	else if (s.equals("YellowGreenMedium4")) setValue( new Integer(GeColor.COLOR_74));
/* glow_eDrawType_Color75 */ 	else if (s.equals("YellowGreenMedium5")) setValue( new Integer(GeColor.COLOR_75));
/* glow_eDrawType_Color76 */ 	else if (s.equals("YellowGreenMedium6")) setValue( new Integer(GeColor.COLOR_76));
/* glow_eDrawType_Color77 */ 	else if (s.equals("YellowGreenMedium7")) setValue( new Integer(GeColor.COLOR_77));
/* glow_eDrawType_Color78 */ 	else if (s.equals("YellowGreenMedium8")) setValue( new Integer(GeColor.COLOR_78));
/* glow_eDrawType_Color79 */ 	else if (s.equals("YellowGreenMedium9")) setValue( new Integer(GeColor.COLOR_79));
/* glow_eDrawType_Color80 */ 	else if (s.equals("YellowGreenMedium10")) setValue( new Integer(GeColor.COLOR_80));
/* glow_eDrawType_Color81 */ 	else if (s.equals("YellowGreenHigh1")) setValue( new Integer(GeColor.COLOR_81));
/* glow_eDrawType_Color82 */ 	else if (s.equals("YellowGreenHigh2")) setValue( new Integer(GeColor.COLOR_82));
/* glow_eDrawType_Color83 */ 	else if (s.equals("YellowGreenHigh3")) setValue( new Integer(GeColor.COLOR_83));
/* glow_eDrawType_Color84 */ 	else if (s.equals("YellowGreenHigh4")) setValue( new Integer(GeColor.COLOR_84));
/* glow_eDrawType_Color85 */ 	else if (s.equals("YellowGreenHigh5")) setValue( new Integer(GeColor.COLOR_85));
/* glow_eDrawType_Color86 */ 	else if (s.equals("YellowGreenHigh6")) setValue( new Integer(GeColor.COLOR_86));
/* glow_eDrawType_Color87 */ 	else if (s.equals("YellowGreenHigh7")) setValue( new Integer(GeColor.COLOR_87));
/* glow_eDrawType_Color88 */ 	else if (s.equals("YellowGreenHigh8")) setValue( new Integer(GeColor.COLOR_88));
/* glow_eDrawType_Color89 */ 	else if (s.equals("YellowGreenHigh9")) setValue( new Integer(GeColor.COLOR_89));
/* glow_eDrawType_Color90 */ 	else if (s.equals("YellowGreenHigh10")) setValue( new Integer(GeColor.COLOR_90));
/* glow_eDrawType_Color91 */ 	else if (s.equals("YellowLow1")) setValue( new Integer(GeColor.COLOR_91));
/* glow_eDrawType_Color92 */ 	else if (s.equals("YellowLow2")) setValue( new Integer(GeColor.COLOR_92));
/* glow_eDrawType_Color93 */ 	else if (s.equals("YellowLow3")) setValue( new Integer(GeColor.COLOR_93));
/* glow_eDrawType_Color94 */ 	else if (s.equals("YellowLow4")) setValue( new Integer(GeColor.COLOR_94));
/* glow_eDrawType_Color95 */ 	else if (s.equals("YellowLow5")) setValue( new Integer(GeColor.COLOR_95));
/* glow_eDrawType_Color96 */ 	else if (s.equals("YellowLow6")) setValue( new Integer(GeColor.COLOR_96));
/* glow_eDrawType_Color97 */ 	else if (s.equals("YellowLow7")) setValue( new Integer(GeColor.COLOR_97));
/* glow_eDrawType_Color98 */ 	else if (s.equals("YellowLow8")) setValue( new Integer(GeColor.COLOR_98));
/* glow_eDrawType_Color99 */ 	else if (s.equals("YellowLow9")) setValue( new Integer(GeColor.COLOR_99));
/* glow_eDrawType_Color100 */ 	else if (s.equals("YellowLow10")) setValue( new Integer(GeColor.COLOR_100));
/* glow_eDrawType_Color101 */ 	else if (s.equals("YellowMedium1")) setValue( new Integer(GeColor.COLOR_101));
/* glow_eDrawType_Color102 */ 	else if (s.equals("YellowMedium2")) setValue( new Integer(GeColor.COLOR_102));
/* glow_eDrawType_Color103 */ 	else if (s.equals("YellowMedium3")) setValue( new Integer(GeColor.COLOR_103));
/* glow_eDrawType_Color104 */ 	else if (s.equals("YellowMedium4")) setValue( new Integer(GeColor.COLOR_104));
/* glow_eDrawType_Color105 */ 	else if (s.equals("YellowMedium5")) setValue( new Integer(GeColor.COLOR_105));
/* glow_eDrawType_Color106 */ 	else if (s.equals("YellowMedium6")) setValue( new Integer(GeColor.COLOR_106));
/* glow_eDrawType_Color107 */ 	else if (s.equals("YellowMedium7")) setValue( new Integer(GeColor.COLOR_107));
/* glow_eDrawType_Color108 */ 	else if (s.equals("YellowMedium8")) setValue( new Integer(GeColor.COLOR_108));
/* glow_eDrawType_Color109 */ 	else if (s.equals("YellowMedium9")) setValue( new Integer(GeColor.COLOR_109));
/* glow_eDrawType_Color110 */ 	else if (s.equals("YellowMedium10")) setValue( new Integer(GeColor.COLOR_110));
/* glow_eDrawType_Color111 */ 	else if (s.equals("YellowHigh1")) setValue( new Integer(GeColor.COLOR_111));
/* glow_eDrawType_Color112 */ 	else if (s.equals("YellowHigh2")) setValue( new Integer(GeColor.COLOR_112));
/* glow_eDrawType_Color113 */ 	else if (s.equals("YellowHigh3")) setValue( new Integer(GeColor.COLOR_113));
/* glow_eDrawType_Color114 */ 	else if (s.equals("YellowHigh4")) setValue( new Integer(GeColor.COLOR_114));
/* glow_eDrawType_Color115 */ 	else if (s.equals("YellowHigh5")) setValue( new Integer(GeColor.COLOR_115));
/* glow_eDrawType_Color116 */ 	else if (s.equals("YellowHigh6")) setValue( new Integer(GeColor.COLOR_116));
/* glow_eDrawType_Color117 */ 	else if (s.equals("YellowHigh7")) setValue( new Integer(GeColor.COLOR_117));
/* glow_eDrawType_Color118 */ 	else if (s.equals("YellowHigh8")) setValue( new Integer(GeColor.COLOR_118));
/* glow_eDrawType_Color119 */ 	else if (s.equals("YellowHigh9")) setValue( new Integer(GeColor.COLOR_119));
/* glow_eDrawType_Color120 */ 	else if (s.equals("YellowHigh10")) setValue( new Integer(GeColor.COLOR_120));
/* glow_eDrawType_Color121 */ 	else if (s.equals("OrangeLow1")) setValue( new Integer(GeColor.COLOR_121));
/* glow_eDrawType_Color122 */ 	else if (s.equals("OrangeLow2")) setValue( new Integer(GeColor.COLOR_122));
/* glow_eDrawType_Color123 */ 	else if (s.equals("OrangeLow3")) setValue( new Integer(GeColor.COLOR_123));
/* glow_eDrawType_Color124 */ 	else if (s.equals("OrangeLow4")) setValue( new Integer(GeColor.COLOR_124));
/* glow_eDrawType_Color125 */ 	else if (s.equals("OrangeLow5")) setValue( new Integer(GeColor.COLOR_125));
/* glow_eDrawType_Color126 */ 	else if (s.equals("OrangeLow6")) setValue( new Integer(GeColor.COLOR_126));
/* glow_eDrawType_Color127 */ 	else if (s.equals("OrangeLow7")) setValue( new Integer(GeColor.COLOR_127));
/* glow_eDrawType_Color128 */ 	else if (s.equals("OrangeLow8")) setValue( new Integer(GeColor.COLOR_128));
/* glow_eDrawType_Color129 */ 	else if (s.equals("OrangeLow9")) setValue( new Integer(GeColor.COLOR_129));
/* glow_eDrawType_Color130 */ 	else if (s.equals("OrangeLow10")) setValue( new Integer(GeColor.COLOR_130));
/* glow_eDrawType_Color131 */ 	else if (s.equals("OrangeMedium1")) setValue( new Integer(GeColor.COLOR_131));
/* glow_eDrawType_Color132 */ 	else if (s.equals("OrangeMedium2")) setValue( new Integer(GeColor.COLOR_132));
/* glow_eDrawType_Color133 */ 	else if (s.equals("OrangeMedium3")) setValue( new Integer(GeColor.COLOR_133));
/* glow_eDrawType_Color134 */ 	else if (s.equals("OrangeMedium4")) setValue( new Integer(GeColor.COLOR_134));
/* glow_eDrawType_Color135 */ 	else if (s.equals("OrangeMedium5")) setValue( new Integer(GeColor.COLOR_135));
/* glow_eDrawType_Color136 */ 	else if (s.equals("OrangeMedium6")) setValue( new Integer(GeColor.COLOR_136));
/* glow_eDrawType_Color137 */ 	else if (s.equals("OrangeMedium7")) setValue( new Integer(GeColor.COLOR_137));
/* glow_eDrawType_Color138 */ 	else if (s.equals("OrangeMedium8")) setValue( new Integer(GeColor.COLOR_138));
/* glow_eDrawType_Color139 */ 	else if (s.equals("OrangeMedium9")) setValue( new Integer(GeColor.COLOR_139));
/* glow_eDrawType_Color140 */ 	else if (s.equals("OrangeMedium10")) setValue( new Integer(GeColor.COLOR_140));
/* glow_eDrawType_Color141 */ 	else if (s.equals("OrangeHigh1")) setValue( new Integer(GeColor.COLOR_141));
/* glow_eDrawType_Color142 */ 	else if (s.equals("OrangeHigh2")) setValue( new Integer(GeColor.COLOR_142));
/* glow_eDrawType_Color143 */ 	else if (s.equals("OrangeHigh3")) setValue( new Integer(GeColor.COLOR_143));
/* glow_eDrawType_Color144 */ 	else if (s.equals("OrangeHigh4")) setValue( new Integer(GeColor.COLOR_144));
/* glow_eDrawType_Color145 */ 	else if (s.equals("OrangeHigh5")) setValue( new Integer(GeColor.COLOR_145));
/* glow_eDrawType_Color146 */ 	else if (s.equals("OrangeHigh6")) setValue( new Integer(GeColor.COLOR_146));
/* glow_eDrawType_Color147 */ 	else if (s.equals("OrangeHigh7")) setValue( new Integer(GeColor.COLOR_147));
/* glow_eDrawType_Color148 */ 	else if (s.equals("OrangeHigh8")) setValue( new Integer(GeColor.COLOR_148));
/* glow_eDrawType_Color149 */ 	else if (s.equals("OrangeHigh9")) setValue( new Integer(GeColor.COLOR_149));
/* glow_eDrawType_Color150 */ 	else if (s.equals("OrangeHigh10")) setValue( new Integer(GeColor.COLOR_150));
/* glow_eDrawType_Color151 */ 	else if (s.equals("RedLow1")) setValue( new Integer(GeColor.COLOR_151));
/* glow_eDrawType_Color152 */ 	else if (s.equals("RedLow2")) setValue( new Integer(GeColor.COLOR_152));
/* glow_eDrawType_Color153 */ 	else if (s.equals("RedLow3")) setValue( new Integer(GeColor.COLOR_153));
/* glow_eDrawType_Color154 */ 	else if (s.equals("RedLow4")) setValue( new Integer(GeColor.COLOR_154));
/* glow_eDrawType_Color155 */ 	else if (s.equals("RedLow5")) setValue( new Integer(GeColor.COLOR_155));
/* glow_eDrawType_Color156 */ 	else if (s.equals("RedLow6")) setValue( new Integer(GeColor.COLOR_156));
/* glow_eDrawType_Color157 */ 	else if (s.equals("RedLow7")) setValue( new Integer(GeColor.COLOR_157));
/* glow_eDrawType_Color158 */ 	else if (s.equals("RedLow8")) setValue( new Integer(GeColor.COLOR_158));
/* glow_eDrawType_Color159 */ 	else if (s.equals("RedLow9")) setValue( new Integer(GeColor.COLOR_159));
/* glow_eDrawType_Color160 */ 	else if (s.equals("RedLow10")) setValue( new Integer(GeColor.COLOR_160));
/* glow_eDrawType_Color161 */ 	else if (s.equals("RedMedium1")) setValue( new Integer(GeColor.COLOR_161));
/* glow_eDrawType_Color162 */ 	else if (s.equals("RedMedium2")) setValue( new Integer(GeColor.COLOR_162));
/* glow_eDrawType_Color163 */ 	else if (s.equals("RedMedium3")) setValue( new Integer(GeColor.COLOR_163));
/* glow_eDrawType_Color164 */ 	else if (s.equals("RedMedium4")) setValue( new Integer(GeColor.COLOR_164));
/* glow_eDrawType_Color165 */ 	else if (s.equals("RedMedium5")) setValue( new Integer(GeColor.COLOR_165));
/* glow_eDrawType_Color166 */ 	else if (s.equals("RedMedium6")) setValue( new Integer(GeColor.COLOR_166));
/* glow_eDrawType_Color167 */ 	else if (s.equals("RedMedium7")) setValue( new Integer(GeColor.COLOR_167));
/* glow_eDrawType_Color168 */ 	else if (s.equals("RedMedium8")) setValue( new Integer(GeColor.COLOR_168));
/* glow_eDrawType_Color169 */ 	else if (s.equals("RedMedium9")) setValue( new Integer(GeColor.COLOR_169));
/* glow_eDrawType_Color170 */ 	else if (s.equals("RedMedium10")) setValue( new Integer(GeColor.COLOR_170));
/* glow_eDrawType_Color171 */ 	else if (s.equals("RedHigh1")) setValue( new Integer(GeColor.COLOR_171));
/* glow_eDrawType_Color172 */ 	else if (s.equals("RedHigh2")) setValue( new Integer(GeColor.COLOR_172));
/* glow_eDrawType_Color173 */ 	else if (s.equals("RedHigh3")) setValue( new Integer(GeColor.COLOR_173));
/* glow_eDrawType_Color174 */ 	else if (s.equals("RedHigh4")) setValue( new Integer(GeColor.COLOR_174));
/* glow_eDrawType_Color175 */ 	else if (s.equals("RedHigh5")) setValue( new Integer(GeColor.COLOR_175));
/* glow_eDrawType_Color176 */ 	else if (s.equals("RedHigh6")) setValue( new Integer(GeColor.COLOR_176));
/* glow_eDrawType_Color177 */ 	else if (s.equals("RedHigh7")) setValue( new Integer(GeColor.COLOR_177));
/* glow_eDrawType_Color178 */ 	else if (s.equals("RedHigh8")) setValue( new Integer(GeColor.COLOR_178));
/* glow_eDrawType_Color179 */ 	else if (s.equals("RedHigh9")) setValue( new Integer(GeColor.COLOR_179));
/* glow_eDrawType_Color180 */ 	else if (s.equals("RedHigh10")) setValue( new Integer(GeColor.COLOR_180));
/* glow_eDrawType_Color181 */ 	else if (s.equals("MagentaLow1")) setValue( new Integer(GeColor.COLOR_181));
/* glow_eDrawType_Color182 */ 	else if (s.equals("MagentaLow2")) setValue( new Integer(GeColor.COLOR_182));
/* glow_eDrawType_Color183 */ 	else if (s.equals("MagentaLow3")) setValue( new Integer(GeColor.COLOR_183));
/* glow_eDrawType_Color184 */ 	else if (s.equals("MagentaLow4")) setValue( new Integer(GeColor.COLOR_184));
/* glow_eDrawType_Color185 */ 	else if (s.equals("MagentaLow5")) setValue( new Integer(GeColor.COLOR_185));
/* glow_eDrawType_Color186 */ 	else if (s.equals("MagentaLow6")) setValue( new Integer(GeColor.COLOR_186));
/* glow_eDrawType_Color187 */ 	else if (s.equals("MagentaLow7")) setValue( new Integer(GeColor.COLOR_187));
/* glow_eDrawType_Color188 */ 	else if (s.equals("MagentaLow8")) setValue( new Integer(GeColor.COLOR_188));
/* glow_eDrawType_Color189 */ 	else if (s.equals("MagentaLow9")) setValue( new Integer(GeColor.COLOR_189));
/* glow_eDrawType_Color190 */ 	else if (s.equals("MagentaLow10")) setValue( new Integer(GeColor.COLOR_190));
/* glow_eDrawType_Color191 */ 	else if (s.equals("MagentaMedium1")) setValue( new Integer(GeColor.COLOR_191));
/* glow_eDrawType_Color192 */ 	else if (s.equals("MagentaMedium2")) setValue( new Integer(GeColor.COLOR_192));
/* glow_eDrawType_Color193 */ 	else if (s.equals("MagentaMedium3")) setValue( new Integer(GeColor.COLOR_193));
/* glow_eDrawType_Color194 */ 	else if (s.equals("MagentaMedium4")) setValue( new Integer(GeColor.COLOR_194));
/* glow_eDrawType_Color195 */ 	else if (s.equals("MagentaMedium5")) setValue( new Integer(GeColor.COLOR_195));
/* glow_eDrawType_Color196 */ 	else if (s.equals("MagentaMedium6")) setValue( new Integer(GeColor.COLOR_196));
/* glow_eDrawType_Color197 */ 	else if (s.equals("MagentaMedium7")) setValue( new Integer(GeColor.COLOR_197));
/* glow_eDrawType_Color198 */ 	else if (s.equals("MagentaMedium8")) setValue( new Integer(GeColor.COLOR_198));
/* glow_eDrawType_Color199 */ 	else if (s.equals("MagentaMedium9")) setValue( new Integer(GeColor.COLOR_199));
/* glow_eDrawType_Color200 */ 	else if (s.equals("MagentaMedium10")) setValue( new Integer(GeColor.COLOR_200));
/* glow_eDrawType_Color201 */ 	else if (s.equals("MagentaHigh1")) setValue( new Integer(GeColor.COLOR_201));
/* glow_eDrawType_Color202 */ 	else if (s.equals("MagentaHigh2")) setValue( new Integer(GeColor.COLOR_202));
/* glow_eDrawType_Color203 */ 	else if (s.equals("MagentaHigh3")) setValue( new Integer(GeColor.COLOR_203));
/* glow_eDrawType_Color204 */ 	else if (s.equals("MagentaHigh4")) setValue( new Integer(GeColor.COLOR_204));
/* glow_eDrawType_Color205 */ 	else if (s.equals("MagentaHigh5")) setValue( new Integer(GeColor.COLOR_205));
/* glow_eDrawType_Color206 */ 	else if (s.equals("MagentaHigh6")) setValue( new Integer(GeColor.COLOR_206));
/* glow_eDrawType_Color207 */ 	else if (s.equals("MagentaHigh7")) setValue( new Integer(GeColor.COLOR_207));
/* glow_eDrawType_Color208 */ 	else if (s.equals("MagentaHigh8")) setValue( new Integer(GeColor.COLOR_208));
/* glow_eDrawType_Color209 */ 	else if (s.equals("MagentaHigh9")) setValue( new Integer(GeColor.COLOR_209));
/* glow_eDrawType_Color210 */ 	else if (s.equals("MagentaHigh10")) setValue( new Integer(GeColor.COLOR_210));
/* glow_eDrawType_Color211 */ 	else if (s.equals("BlueLow1")) setValue( new Integer(GeColor.COLOR_211));
/* glow_eDrawType_Color212 */ 	else if (s.equals("BlueLow2")) setValue( new Integer(GeColor.COLOR_212));
/* glow_eDrawType_Color213 */ 	else if (s.equals("BlueLow3")) setValue( new Integer(GeColor.COLOR_213));
/* glow_eDrawType_Color214 */ 	else if (s.equals("BlueLow4")) setValue( new Integer(GeColor.COLOR_214));
/* glow_eDrawType_Color215 */ 	else if (s.equals("BlueLow5")) setValue( new Integer(GeColor.COLOR_215));
/* glow_eDrawType_Color216 */ 	else if (s.equals("BlueLow6")) setValue( new Integer(GeColor.COLOR_216));
/* glow_eDrawType_Color217 */ 	else if (s.equals("BlueLow7")) setValue( new Integer(GeColor.COLOR_217));
/* glow_eDrawType_Color218 */ 	else if (s.equals("BlueLow8")) setValue( new Integer(GeColor.COLOR_218));
/* glow_eDrawType_Color219 */ 	else if (s.equals("BlueLow9")) setValue( new Integer(GeColor.COLOR_219));
/* glow_eDrawType_Color220 */ 	else if (s.equals("BlueLow10")) setValue( new Integer(GeColor.COLOR_220));
/* glow_eDrawType_Color221 */ 	else if (s.equals("BlueMedium1")) setValue( new Integer(GeColor.COLOR_221));
/* glow_eDrawType_Color222 */ 	else if (s.equals("BlueMedium2")) setValue( new Integer(GeColor.COLOR_222));
/* glow_eDrawType_Color223 */ 	else if (s.equals("BlueMedium3")) setValue( new Integer(GeColor.COLOR_223));
/* glow_eDrawType_Color224 */ 	else if (s.equals("BlueMedium4")) setValue( new Integer(GeColor.COLOR_224));
/* glow_eDrawType_Color225 */ 	else if (s.equals("BlueMedium5")) setValue( new Integer(GeColor.COLOR_225));
/* glow_eDrawType_Color226 */ 	else if (s.equals("BlueMedium6")) setValue( new Integer(GeColor.COLOR_226));
/* glow_eDrawType_Color227 */ 	else if (s.equals("BlueMedium7")) setValue( new Integer(GeColor.COLOR_227));
/* glow_eDrawType_Color228 */ 	else if (s.equals("BlueMedium8")) setValue( new Integer(GeColor.COLOR_228));
/* glow_eDrawType_Color229 */ 	else if (s.equals("BlueMedium9")) setValue( new Integer(GeColor.COLOR_229));
/* glow_eDrawType_Color230 */ 	else if (s.equals("BlueMedium10")) setValue( new Integer(GeColor.COLOR_230));
/* glow_eDrawType_Color231 */ 	else if (s.equals("BlueHigh1")) setValue( new Integer(GeColor.COLOR_231));
/* glow_eDrawType_Color232 */ 	else if (s.equals("BlueHigh2")) setValue( new Integer(GeColor.COLOR_232));
/* glow_eDrawType_Color233 */ 	else if (s.equals("BlueHigh3")) setValue( new Integer(GeColor.COLOR_233));
/* glow_eDrawType_Color234 */ 	else if (s.equals("BlueHigh4")) setValue( new Integer(GeColor.COLOR_234));
/* glow_eDrawType_Color235 */ 	else if (s.equals("BlueHigh5")) setValue( new Integer(GeColor.COLOR_235));
/* glow_eDrawType_Color236 */ 	else if (s.equals("BlueHigh6")) setValue( new Integer(GeColor.COLOR_236));
/* glow_eDrawType_Color237 */ 	else if (s.equals("BlueHigh7")) setValue( new Integer(GeColor.COLOR_237));
/* glow_eDrawType_Color238 */ 	else if (s.equals("BlueHigh8")) setValue( new Integer(GeColor.COLOR_238));
/* glow_eDrawType_Color239 */ 	else if (s.equals("BlueHigh9")) setValue( new Integer(GeColor.COLOR_239));
/* glow_eDrawType_Color240 */ 	else if (s.equals("BlueHigh10")) setValue( new Integer(GeColor.COLOR_240));
/* glow_eDrawType_Color241 */ 	else if (s.equals("SeaBlueLow1")) setValue( new Integer(GeColor.COLOR_241));
/* glow_eDrawType_Color242 */ 	else if (s.equals("SeaBlueLow2")) setValue( new Integer(GeColor.COLOR_242));
/* glow_eDrawType_Color243 */ 	else if (s.equals("SeaBlueLow3")) setValue( new Integer(GeColor.COLOR_243));
/* glow_eDrawType_Color244 */ 	else if (s.equals("SeaBlueLow4")) setValue( new Integer(GeColor.COLOR_244));
/* glow_eDrawType_Color245 */ 	else if (s.equals("SeaBlueLow5")) setValue( new Integer(GeColor.COLOR_245));
/* glow_eDrawType_Color246 */ 	else if (s.equals("SeaBlueLow6")) setValue( new Integer(GeColor.COLOR_246));
/* glow_eDrawType_Color247 */ 	else if (s.equals("SeaBlueLow7")) setValue( new Integer(GeColor.COLOR_247));
/* glow_eDrawType_Color248 */ 	else if (s.equals("SeaBlueLow8")) setValue( new Integer(GeColor.COLOR_248));
/* glow_eDrawType_Color249 */ 	else if (s.equals("SeaBlueLow9")) setValue( new Integer(GeColor.COLOR_249));
/* glow_eDrawType_Color250 */ 	else if (s.equals("SeaBlueLow10")) setValue( new Integer(GeColor.COLOR_250));
/* glow_eDrawType_Color251 */ 	else if (s.equals("SeaBlueMedium1")) setValue( new Integer(GeColor.COLOR_251));
/* glow_eDrawType_Color252 */ 	else if (s.equals("SeaBlueMedium2")) setValue( new Integer(GeColor.COLOR_252));
/* glow_eDrawType_Color253 */ 	else if (s.equals("SeaBlueMedium3")) setValue( new Integer(GeColor.COLOR_253));
/* glow_eDrawType_Color224 */ 	else if (s.equals("SeaBlueMedium4")) setValue( new Integer(GeColor.COLOR_254));
/* glow_eDrawType_Color255 */ 	else if (s.equals("SeaBlueMedium5")) setValue( new Integer(GeColor.COLOR_255));
/* glow_eDrawType_Color256 */ 	else if (s.equals("SeaBlueMedium6")) setValue( new Integer(GeColor.COLOR_256));
/* glow_eDrawType_Color257 */ 	else if (s.equals("SeaBlueMedium7")) setValue( new Integer(GeColor.COLOR_257));
/* glow_eDrawType_Color258 */ 	else if (s.equals("SeaBlueMedium8")) setValue( new Integer(GeColor.COLOR_258));
/* glow_eDrawType_Color259 */ 	else if (s.equals("SeaBlueMedium9")) setValue( new Integer(GeColor.COLOR_259));
/* glow_eDrawType_Color260 */ 	else if (s.equals("SeaBlueMedium10")) setValue( new Integer(GeColor.COLOR_260));
/* glow_eDrawType_Color261 */ 	else if (s.equals("SeaBlueHigh1")) setValue( new Integer(GeColor.COLOR_261));
/* glow_eDrawType_Color262 */ 	else if (s.equals("SeaBlueHigh2")) setValue( new Integer(GeColor.COLOR_262));
/* glow_eDrawType_Color263 */ 	else if (s.equals("SeaBlueHigh3")) setValue( new Integer(GeColor.COLOR_263));
/* glow_eDrawType_Color264 */ 	else if (s.equals("SeaBlueHigh4")) setValue( new Integer(GeColor.COLOR_264));
/* glow_eDrawType_Color265 */ 	else if (s.equals("SeaBlueHigh5")) setValue( new Integer(GeColor.COLOR_265));
/* glow_eDrawType_Color266 */ 	else if (s.equals("SeaBlueHigh6")) setValue( new Integer(GeColor.COLOR_266));
/* glow_eDrawType_Color267 */ 	else if (s.equals("SeaBlueHigh7")) setValue( new Integer(GeColor.COLOR_267));
/* glow_eDrawType_Color268 */ 	else if (s.equals("SeaBlueHigh8")) setValue( new Integer(GeColor.COLOR_268));
/* glow_eDrawType_Color269 */ 	else if (s.equals("SeaBlueHigh9")) setValue( new Integer(GeColor.COLOR_269));
/* glow_eDrawType_Color270 */ 	else if (s.equals("SeaBlueHigh10")) setValue( new Integer(GeColor.COLOR_270));
/* glow_eDrawType_Color271 */ 	else if (s.equals("GreenLow1")) setValue( new Integer(GeColor.COLOR_271));
/* glow_eDrawType_Color272 */ 	else if (s.equals("GreenLow2")) setValue( new Integer(GeColor.COLOR_272));
/* glow_eDrawType_Color273 */ 	else if (s.equals("GreenLow3")) setValue( new Integer(GeColor.COLOR_273));
/* glow_eDrawType_Color274 */ 	else if (s.equals("GreenLow4")) setValue( new Integer(GeColor.COLOR_274));
/* glow_eDrawType_Color275 */ 	else if (s.equals("GreenLow5")) setValue( new Integer(GeColor.COLOR_275));
/* glow_eDrawType_Color276 */ 	else if (s.equals("GreenLow6")) setValue( new Integer(GeColor.COLOR_276));
/* glow_eDrawType_Color277 */ 	else if (s.equals("GreenLow7")) setValue( new Integer(GeColor.COLOR_277));
/* glow_eDrawType_Color278 */ 	else if (s.equals("GreenLow8")) setValue( new Integer(GeColor.COLOR_278));
/* glow_eDrawType_Color279 */ 	else if (s.equals("GreenLow9")) setValue( new Integer(GeColor.COLOR_279));
/* glow_eDrawType_Color280 */ 	else if (s.equals("GreenLow10")) setValue( new Integer(GeColor.COLOR_280));
/* glow_eDrawType_Color281 */ 	else if (s.equals("GreenMedium1")) setValue( new Integer(GeColor.COLOR_281));
/* glow_eDrawType_Color282 */ 	else if (s.equals("GreenMedium2")) setValue( new Integer(GeColor.COLOR_282));
/* glow_eDrawType_Color283 */ 	else if (s.equals("GreenMedium3")) setValue( new Integer(GeColor.COLOR_283));
/* glow_eDrawType_Color284 */ 	else if (s.equals("GreenMedium4")) setValue( new Integer(GeColor.COLOR_284));
/* glow_eDrawType_Color285 */ 	else if (s.equals("GreenMedium5")) setValue( new Integer(GeColor.COLOR_285));
/* glow_eDrawType_Color286 */ 	else if (s.equals("GreenMedium6")) setValue( new Integer(GeColor.COLOR_286));
/* glow_eDrawType_Color287 */ 	else if (s.equals("GreenMedium7")) setValue( new Integer(GeColor.COLOR_287));
/* glow_eDrawType_Color288 */ 	else if (s.equals("GreenMedium8")) setValue( new Integer(GeColor.COLOR_288));
/* glow_eDrawType_Color289 */ 	else if (s.equals("GreenMedium9")) setValue( new Integer(GeColor.COLOR_289));
/* glow_eDrawType_Color290 */ 	else if (s.equals("GreenMedium10")) setValue( new Integer(GeColor.COLOR_290));
/* glow_eDrawType_Color291 */ 	else if (s.equals("GreenHigh1")) setValue( new Integer(GeColor.COLOR_291));
/* glow_eDrawType_Color292 */ 	else if (s.equals("GreenHigh2")) setValue( new Integer(GeColor.COLOR_292));
/* glow_eDrawType_Color293 */ 	else if (s.equals("GreenHigh3")) setValue( new Integer(GeColor.COLOR_293));
/* glow_eDrawType_Color294 */ 	else if (s.equals("GreenHigh4")) setValue( new Integer(GeColor.COLOR_294));
/* glow_eDrawType_Color295 */ 	else if (s.equals("GreenHigh5")) setValue( new Integer(GeColor.COLOR_295));
/* glow_eDrawType_Color296 */ 	else if (s.equals("GreenHigh6")) setValue( new Integer(GeColor.COLOR_296));
/* glow_eDrawType_Color297 */ 	else if (s.equals("GreenHigh7")) setValue( new Integer(GeColor.COLOR_297));
/* glow_eDrawType_Color298 */ 	else if (s.equals("GreenHigh8")) setValue( new Integer(GeColor.COLOR_298));
/* glow_eDrawType_Color299 */ 	else if (s.equals("GreenHigh9")) setValue( new Integer(GeColor.COLOR_299));
/* glow_eDrawType_Color300 */  	else if (s.equals("GreenHigh10")) setValue( new Integer(GeColor.COLOR_300));
    else throw new IllegalArgumentException(s);
  }
}












