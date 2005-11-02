/* 
 * Proview   $Id: GeColor.java,v 1.5 2005-11-02 14:02:18 claes Exp $
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
import java.awt.*;

public class GeColor {

  // Color tones
  public static final int COLOR_TONE_NO 				= 0;
  public static final int COLOR_TONE_GRAY 				= 1;
  public static final int COLOR_TONE_YELLOWGREEN       			= 2;
  public static final int COLOR_TONE_YELLOW 				= 3;
  public static final int COLOR_TONE_ORANGE 				= 4;
  public static final int COLOR_TONE_RED 				= 5;
  public static final int COLOR_TONE_MAGENTA 				= 6;
  public static final int COLOR_TONE_BLUE 				= 7;
  public static final int COLOR_TONE_SEABLUE 				= 8;
  public static final int COLOR_TONE_GREEN 				= 9;
  public static final int COLOR_TONE_DARKGRAY 				= 10;
  public static final int COLOR_TONE_DARKYELLOWGREEN 			= 11;
  public static final int COLOR_TONE_DARKYELLOW        			= 12;
  public static final int COLOR_TONE_DARKORANGE        			= 13;
  public static final int COLOR_TONE_DARKRED 				= 14;
  public static final int COLOR_TONE_DARKMAGENTA       			= 15;
  public static final int COLOR_TONE_DARKBLUE 				= 16;
  public static final int COLOR_TONE_DARKSEABLUE       			= 17;
  public static final int COLOR_TONE_DARKGREEN 				= 18;
  public static final int COLOR_TONE_LIGHTGRAY 				= 19;
  public static final int COLOR_TONE_LIGHTYELLOWGREEN 			= 20;
  public static final int COLOR_TONE_LIGHTYELLOW 			= 21;
  public static final int COLOR_TONE_LIGHTORANGE 			= 22;
  public static final int COLOR_TONE_LIGHTRED 				= 23;
  public static final int COLOR_TONE_LIGHTMAGENTA 			= 24;
  public static final int COLOR_TONE_LIGHTBLUE 				= 25;
  public static final int COLOR_TONE_LIGHTSEABLUE 			= 26;
  public static final int COLOR_TONE_LIGHTGREEN 			= 27;
  public static final int COLOR_TONE_GRAYHIGHSATURATION 		= 28;
  public static final int COLOR_TONE_YELLOWGREENHIGHSATURATION 		= 29;
  public static final int COLOR_TONE_YELLOWHIGHSATURATION 		= 30;
  public static final int COLOR_TONE_ORANGEHIGHSATURATION 		= 31;
  public static final int COLOR_TONE_REDHIGHSATURATION 			= 32;
  public static final int COLOR_TONE_MAGENTAHIGHSATURATION 		= 33;
  public static final int COLOR_TONE_BLUEHIGHSATURATION 		= 34;
  public static final int COLOR_TONE_SEABLUEHIGHSATURATION 		= 35;
  public static final int COLOR_TONE_GREENHIGHSATURATION 		= 36;
  public static final int COLOR_TONE_DARKGRAYHIGHSATURATION 		= 37;
  public static final int COLOR_TONE_DARKYELLOWGREENHIGHSATURATION 	= 38;
  public static final int COLOR_TONE_DARKYELLOWHIGHSATURATION        	= 39;
  public static final int COLOR_TONE_DARKORANGEHIGHSATURATION        	= 40;
  public static final int COLOR_TONE_DARKREDHIGHSATURATION 		= 41;
  public static final int COLOR_TONE_DARKMAGENTAHIGHSATURATION       	= 42;
  public static final int COLOR_TONE_DARKBLUEHIGHSATURATION 		= 43;
  public static final int COLOR_TONE_DARKSEABLUEHIGHSATURATION       	= 44;
  public static final int COLOR_TONE_DARKGREENHIGHSATURATION 		= 45;
  public static final int COLOR_TONE_LIGHTGRAYHIGHSATURATION 		= 46;
  public static final int COLOR_TONE_LIGHTYELLOWGREENHIGHSATURATION 	= 47;
  public static final int COLOR_TONE_LIGHTYELLOWHIGHSATURATION 		= 48;
  public static final int COLOR_TONE_LIGHTORANGEHIGHSATURATION 		= 49;
  public static final int COLOR_TONE_LIGHTREDHIGHSATURATION 		= 50;
  public static final int COLOR_TONE_LIGHTMAGENTAHIGHSATURATION 	= 51;
  public static final int COLOR_TONE_LIGHTBLUEHIGHSATURATION 		= 52;
  public static final int COLOR_TONE_LIGHTSEABLUEHIGHSATURATION 	= 53;
  public static final int COLOR_TONE_LIGHTGREENHIGHSATURATION 		= 54;
  public static final int COLOR_TONE_GRAYLOWSATURATION 			= 55;
  public static final int COLOR_TONE_YELLOWGREENLOWSATURATION 		= 56;
  public static final int COLOR_TONE_YELLOWLOWSATURATION 		= 57;
  public static final int COLOR_TONE_ORANGELOWSATURATION 		= 58;
  public static final int COLOR_TONE_REDLOWSATURATION 			= 59;
  public static final int COLOR_TONE_MAGENTALOWSATURATION 		= 60;
  public static final int COLOR_TONE_BLUELOWSATURATION 			= 61;
  public static final int COLOR_TONE_SEABLUELOWSATURATION 		= 62;
  public static final int COLOR_TONE_GREENLOWSATURATION 		= 63;
  public static final int COLOR_TONE_DARKGRAYLOWSATURATION 		= 64;
  public static final int COLOR_TONE_DARKYELLOWGREENLOWSATURATION 	= 65;
  public static final int COLOR_TONE_DARKYELLOWLOWSATURATION        	= 66;
  public static final int COLOR_TONE_DARKORANGELOWSATURATION        	= 67;
  public static final int COLOR_TONE_DARKREDLOWSATURATION 		= 68;
  public static final int COLOR_TONE_DARKMAGENTALOWSATURATION       	= 69;
  public static final int COLOR_TONE_DARKBLUELOWSATURATION 		= 70;
  public static final int COLOR_TONE_DARKSEABLUELOWSATURATION       	= 71;
  public static final int COLOR_TONE_DARKGREENLOWSATURATION 		= 72;
  public static final int COLOR_TONE_LIGHTGRAYLOWSATURATION 		= 73;
  public static final int COLOR_TONE_LIGHTYELLOWGREENLOWSATURATION 	= 74;
  public static final int COLOR_TONE_LIGHTYELLOWLOWSATURATION 		= 75;
  public static final int COLOR_TONE_LIGHTORANGELOWSATURATION 		= 76;
  public static final int COLOR_TONE_LIGHTREDLOWSATURATION 		= 77;
  public static final int COLOR_TONE_LIGHTMAGENTALOWSATURATION 		= 78;
  public static final int COLOR_TONE_LIGHTBLUELOWSATURATION 		= 79;
  public static final int COLOR_TONE_LIGHTSEABLUELOWSATURATION 		= 80;
  public static final int COLOR_TONE_LIGHTGREENLOWSATURATION 		= 81;
  public static final int COLOR_TONE_MAX = 81;

  public static final int COLOR_BLACK = 0;
  public static final int COLOR_RED = 1;
  public static final int COLOR_GRAY = 2;

  public static final int COLOR_1 = 0;
  public static final int COLOR_2 = 1;
  public static final int COLOR_3 = 2;
  public static final int COLOR_4 = 3;
  public static final int COLOR_5 = 4;
  public static final int COLOR_6 = 5;
  public static final int COLOR_7 = 6;
  public static final int COLOR_8 = 7;
  public static final int COLOR_9 = 8;
  public static final int COLOR_10 = 9;
  public static final int COLOR_11 = 10;
  public static final int COLOR_12 = 11;
  public static final int COLOR_13 = 12;
  public static final int COLOR_14 = 13;
  public static final int COLOR_15 = 14;
  public static final int COLOR_16 = 15;
  public static final int COLOR_17 = 16;
  public static final int COLOR_18 = 17;
  public static final int COLOR_19 = 18;
  public static final int COLOR_20 = 19;
  public static final int COLOR_21 = 20;
  public static final int COLOR_22 = 21;
  public static final int COLOR_23 = 22;
  public static final int COLOR_24 = 23;
  public static final int COLOR_25 = 24;
  public static final int COLOR_26 = 25;
  public static final int COLOR_27 = 26;
  public static final int COLOR_28 = 27;
  public static final int COLOR_29 = 28;
  public static final int COLOR_30 = 29;
  public static final int COLOR_31 = 30;
  public static final int COLOR_32 = 31;
  public static final int COLOR_33 = 32;
  public static final int COLOR_34 = 33;
  public static final int COLOR_35 = 34;
  public static final int COLOR_36 = 35;
  public static final int COLOR_37 = 36;
  public static final int COLOR_38 = 37;
  public static final int COLOR_39 = 38;
  public static final int COLOR_40 = 39;
  public static final int COLOR_41 = 40;
  public static final int COLOR_42 = 41;
  public static final int COLOR_43 = 42;
  public static final int COLOR_44 = 43;
  public static final int COLOR_45 = 44;
  public static final int COLOR_46 = 45;
  public static final int COLOR_47 = 46;
  public static final int COLOR_48 = 47;
  public static final int COLOR_49 = 48;
  public static final int COLOR_50 = 49;
  public static final int COLOR_51 = 50;
  public static final int COLOR_52 = 51;
  public static final int COLOR_53 = 52;
  public static final int COLOR_54 = 53;
  public static final int COLOR_55 = 54;
  public static final int COLOR_56 = 55;
  public static final int COLOR_57 = 56;
  public static final int COLOR_58 = 57;
  public static final int COLOR_59 = 58;
  public static final int COLOR_60 = 59;
  public static final int COLOR_61 = 60;
  public static final int COLOR_62 = 61;
  public static final int COLOR_63 = 62;
  public static final int COLOR_64 = 63;
  public static final int COLOR_65 = 64;
  public static final int COLOR_66 = 65;
  public static final int COLOR_67 = 66;
  public static final int COLOR_68 = 67;
  public static final int COLOR_69 = 68;
  public static final int COLOR_70 = 69;
  public static final int COLOR_71 = 70;
  public static final int COLOR_72 = 71;
  public static final int COLOR_73 = 72;
  public static final int COLOR_74 = 73;
  public static final int COLOR_75 = 74;
  public static final int COLOR_76 = 75;
  public static final int COLOR_77 = 76;
  public static final int COLOR_78 = 77;
  public static final int COLOR_79 = 78;
  public static final int COLOR_80 = 79;
  public static final int COLOR_81 = 80;
  public static final int COLOR_82 = 81;
  public static final int COLOR_83 = 82;
  public static final int COLOR_84 = 83;
  public static final int COLOR_85 = 84;
  public static final int COLOR_86 = 85;
  public static final int COLOR_87 = 86;
  public static final int COLOR_88 = 87;
  public static final int COLOR_89 = 88;
  public static final int COLOR_90 = 89;
  public static final int COLOR_91 = 90;
  public static final int COLOR_92 = 91;
  public static final int COLOR_93 = 92;
  public static final int COLOR_94 = 93;
  public static final int COLOR_95 = 94;
  public static final int COLOR_96 = 95;
  public static final int COLOR_97 = 96;
  public static final int COLOR_98 = 97;
  public static final int COLOR_99 = 98;
  public static final int COLOR_100 = 99;
  public static final int COLOR_101 = 100;
  public static final int COLOR_102 = 101;
  public static final int COLOR_103 = 102;
  public static final int COLOR_104 = 103;
  public static final int COLOR_105 = 104;
  public static final int COLOR_106 = 105;
  public static final int COLOR_107 = 106;
  public static final int COLOR_108 = 107;
  public static final int COLOR_109 = 108;
  public static final int COLOR_110 = 109;
  public static final int COLOR_111 = 110;
  public static final int COLOR_112 = 111;
  public static final int COLOR_113 = 112;
  public static final int COLOR_114 = 113;
  public static final int COLOR_115 = 114;
  public static final int COLOR_116 = 115;
  public static final int COLOR_117 = 116;
  public static final int COLOR_118 = 117;
  public static final int COLOR_119 = 118;
  public static final int COLOR_120 = 119;
  public static final int COLOR_121 = 120;
  public static final int COLOR_122 = 121;
  public static final int COLOR_123 = 122;
  public static final int COLOR_124 = 123;
  public static final int COLOR_125 = 124;
  public static final int COLOR_126 = 125;
  public static final int COLOR_127 = 126;
  public static final int COLOR_128 = 127;
  public static final int COLOR_129 = 128;
  public static final int COLOR_130 = 129;
  public static final int COLOR_131 = 130;
  public static final int COLOR_132 = 131;
  public static final int COLOR_133 = 132;
  public static final int COLOR_134 = 133;
  public static final int COLOR_135 = 134;
  public static final int COLOR_136 = 135;
  public static final int COLOR_137 = 136;
  public static final int COLOR_138 = 137;
  public static final int COLOR_139 = 138;
  public static final int COLOR_140 = 139;
  public static final int COLOR_141 = 140;
  public static final int COLOR_142 = 141;
  public static final int COLOR_143 = 142;
  public static final int COLOR_144 = 143;
  public static final int COLOR_145 = 144;
  public static final int COLOR_146 = 145;
  public static final int COLOR_147 = 146;
  public static final int COLOR_148 = 147;
  public static final int COLOR_149 = 148;
  public static final int COLOR_150 = 149;
  public static final int COLOR_151 = 150;
  public static final int COLOR_152 = 151;
  public static final int COLOR_153 = 152;
  public static final int COLOR_154 = 153;
  public static final int COLOR_155 = 154;
  public static final int COLOR_156 = 155;
  public static final int COLOR_157 = 156;
  public static final int COLOR_158 = 157;
  public static final int COLOR_159 = 158;
  public static final int COLOR_160 = 159;
  public static final int COLOR_161 = 160;
  public static final int COLOR_162 = 161;
  public static final int COLOR_163 = 162;
  public static final int COLOR_164 = 163;
  public static final int COLOR_165 = 164;
  public static final int COLOR_166 = 165;
  public static final int COLOR_167 = 166;
  public static final int COLOR_168 = 167;
  public static final int COLOR_169 = 168;
  public static final int COLOR_170 = 169;
  public static final int COLOR_171 = 170;
  public static final int COLOR_172 = 171;
  public static final int COLOR_173 = 172;
  public static final int COLOR_174 = 173;
  public static final int COLOR_175 = 174;
  public static final int COLOR_176 = 175;
  public static final int COLOR_177 = 176;
  public static final int COLOR_178 = 177;
  public static final int COLOR_179 = 178;
  public static final int COLOR_180 = 179;
  public static final int COLOR_181 = 180;
  public static final int COLOR_182 = 181;
  public static final int COLOR_183 = 182;
  public static final int COLOR_184 = 183;
  public static final int COLOR_185 = 184;
  public static final int COLOR_186 = 185;
  public static final int COLOR_187 = 186;
  public static final int COLOR_188 = 187;
  public static final int COLOR_189 = 188;
  public static final int COLOR_190 = 189;
  public static final int COLOR_191 = 190;
  public static final int COLOR_192 = 191;
  public static final int COLOR_193 = 192;
  public static final int COLOR_194 = 193;
  public static final int COLOR_195 = 194;
  public static final int COLOR_196 = 195;
  public static final int COLOR_197 = 196;
  public static final int COLOR_198 = 197;
  public static final int COLOR_199 = 198;
  public static final int COLOR_200 = 199;
  public static final int COLOR_201 = 200;
  public static final int COLOR_202 = 201;
  public static final int COLOR_203 = 202;
  public static final int COLOR_204 = 203;
  public static final int COLOR_205 = 204;
  public static final int COLOR_206 = 205;
  public static final int COLOR_207 = 206;
  public static final int COLOR_208 = 207;
  public static final int COLOR_209 = 208;
  public static final int COLOR_210 = 209;
  public static final int COLOR_211 = 210;
  public static final int COLOR_212 = 211;
  public static final int COLOR_213 = 212;
  public static final int COLOR_214 = 213;
  public static final int COLOR_215 = 214;
  public static final int COLOR_216 = 215;
  public static final int COLOR_217 = 216;
  public static final int COLOR_218 = 217;
  public static final int COLOR_219 = 218;
  public static final int COLOR_220 = 219;
  public static final int COLOR_221 = 220;
  public static final int COLOR_222 = 221;
  public static final int COLOR_223 = 222;
  public static final int COLOR_224 = 223;
  public static final int COLOR_225 = 224;
  public static final int COLOR_226 = 225;
  public static final int COLOR_227 = 226;
  public static final int COLOR_228 = 227;
  public static final int COLOR_229 = 228;
  public static final int COLOR_230 = 229;
  public static final int COLOR_231 = 230;
  public static final int COLOR_232 = 231;
  public static final int COLOR_233 = 232;
  public static final int COLOR_234 = 233;
  public static final int COLOR_235 = 234;
  public static final int COLOR_236 = 235;
  public static final int COLOR_237 = 236;
  public static final int COLOR_238 = 237;
  public static final int COLOR_239 = 238;
  public static final int COLOR_240 = 239;
  public static final int COLOR_241 = 240;
  public static final int COLOR_242 = 241;
  public static final int COLOR_243 = 242;
  public static final int COLOR_244 = 243;
  public static final int COLOR_245 = 244;
  public static final int COLOR_246 = 245;
  public static final int COLOR_247 = 246;
  public static final int COLOR_248 = 247;
  public static final int COLOR_249 = 248;
  public static final int COLOR_250 = 249;
  public static final int COLOR_251 = 250;
  public static final int COLOR_252 = 251;
  public static final int COLOR_253 = 252;
  public static final int COLOR_254 = 253;
  public static final int COLOR_255 = 254;
  public static final int COLOR_256 = 255;
  public static final int COLOR_257 = 256;
  public static final int COLOR_258 = 257;
  public static final int COLOR_259 = 258;
  public static final int COLOR_260 = 259;
  public static final int COLOR_261 = 260;
  public static final int COLOR_262 = 261;
  public static final int COLOR_263 = 262;
  public static final int COLOR_264 = 263;
  public static final int COLOR_265 = 264;
  public static final int COLOR_266 = 265;
  public static final int COLOR_267 = 266;
  public static final int COLOR_268 = 267;
  public static final int COLOR_269 = 268;
  public static final int COLOR_270 = 269;
  public static final int COLOR_271 = 270;
  public static final int COLOR_272 = 271;
  public static final int COLOR_273 = 272;
  public static final int COLOR_274 = 273;
  public static final int COLOR_275 = 274;
  public static final int COLOR_276 = 275;
  public static final int COLOR_277 = 276;
  public static final int COLOR_278 = 277;
  public static final int COLOR_279 = 278;
  public static final int COLOR_280 = 279;
  public static final int COLOR_281 = 280;
  public static final int COLOR_282 = 281;
  public static final int COLOR_283 = 282;
  public static final int COLOR_284 = 283;
  public static final int COLOR_285 = 284;
  public static final int COLOR_286 = 285;
  public static final int COLOR_287 = 286;
  public static final int COLOR_288 = 287;
  public static final int COLOR_289 = 288;
  public static final int COLOR_290 = 289;
  public static final int COLOR_291 = 290;
  public static final int COLOR_292 = 291;
  public static final int COLOR_293 = 292;
  public static final int COLOR_294 = 293;
  public static final int COLOR_295 = 294;
  public static final int COLOR_296 = 295;
  public static final int COLOR_297 = 296;
  public static final int COLOR_298 = 297;
  public static final int COLOR_299 = 298;
  public static final int COLOR_300 = 299;
  public static final int COLOR_LINEERASE = 300;
  public static final int COLOR_INHERIT = 9999;
  public static final int COLOR_NO = 10000;

  static final double colorValues[] = {
0, 		0, 		0,		// 4 Black
1, 		0.2, 		0.2,		// 4 Red
0.7, 		0.7, 		0.7,		// 4 Gray
1, 		1, 		1};		// 4 White

  static Color[] colors = new Color[301];
  public static final int NO_COLOR = 9999;
  public static final int NO_TONE = 0;
  public static Color getColor( int idx, int default_color) {
    if ( default_color < 300) {
//      System.out.println("Using default color");
      idx = default_color;
    }
//    System.out.println("Using ge color");
    if ( idx < 0 || idx > 299)
      idx = 1;
    if ( colors[idx] == null)
      colors[idx] = rgbColor( idx);
    return colors[idx];
  }

  static public class Rgb {
    public Rgb() {}
    double r;
    double g;
    double b;
  }

  static Rgb hisToRgb( double h, double i, double s) {
    double m1, m2, i1;

    m1 = s * Math.sin( h * Math.PI/180);
    m2 = s * Math.cos( h * Math.PI/180);
    i1 = i / Math.sqrt(3);

    Rgb rgb = new Rgb();

    rgb.r = m1 * 2 / Math.sqrt(6) + i1 / Math.sqrt(3);
    rgb.g = - m1 / Math.sqrt(6)  + m2 / Math.sqrt(2) + i1 / Math.sqrt(3);
    rgb.b = - m1 / Math.sqrt(6) - m2 / Math.sqrt(2) + i1 / Math.sqrt(3);

    rgb.r = rgb.r / 2 + 0.5;
    rgb.g = rgb.g / 2 + 0.5;
    rgb.b = rgb.b / 2 + 0.5;
    if ( rgb.r > 1) rgb.r = 1;
    if ( rgb.r < 0) rgb.r = 0;
    if ( rgb.g > 1) rgb.g = 1;
    if ( rgb.g < 0) rgb.g = 0;
    if ( rgb.b > 1) rgb.b = 1;
    if ( rgb.b < 0) rgb.b = 0;

    return rgb;
  }


  static final double rgbTab[] = {
      0.254, 0.329, 0,  // YellowGreen
      0.357, 0.459, 0,
      0.498, 0.639, 0,
      0.624, 0.800, 0,
      0.764, 0.976, 0,
      0.808, 1.000, 0.129,
      0.847, 1.000, 0.310,
      0.898, 1.000, 0.537,
      0.933, 1.000, 0.710,
      0.949, 1.000, 0.776,
      0., 0., 0,		// Yellow
      0., 0., 0,
      0., 0., 0,
      0., 0., 0,
      0., 0., 0,
      0., 0., 0,
      0., 0., 0,
      0., 0., 0,
      0., 0., 0,
      0., 0., 0,
      0., 0., 0,		// Orange
      0., 0., 0,
      0., 0., 0,
      0., 0., 0,
      0., 0., 0,
      0., 0., 0,
      1.000, 0.725, 0.420,
      1.000, 0.796, 0.569,
      1.000, 0.886, 0.667,
      1.000, 0.937, 0.820};
      

    static final double ctab[] = {
      18,  -20.0, 0.2, -1.4, 2.8, .9, 0.5, -1.4, 3, .9, 1.5, -3.5, 5.0, 1.0,  // YellowGreen
      28,  10.0, 0.2, -1.4, 2.8, .9, 0.5, -1.4, 3, .9, 1.5, -3.5, 5.0, 1.0,  // Yellow
      45,  20.0, 0.2, -1.3, 2.8, .9, 0.5, -1.3, 3, .9, 1.5, -3.5, 4.5, 1.0, // Orange
      90,  0.0, 0.2, -1.1, 2.8, .9, 0.5, -1.1, 3, .9, 1.5, -4.5, 4.5, 1.0, // Red
      150, 0.0, 0.2, -1., 2.8, .9, 0.5, -1., 3, .9, 1.5, -3.5, 5.9, 1.0, // Violet
      240, 0.0, 0.2, -1., 2.8, .9, 0.4, -1., 3, .9, 1.5, -3.5, 5.5, 1.0, // Blue
      280, 0.0, 0.2, -1., 2.8, .9, 0.4, -1., 3, .9, 1.5, -3.5, 5.5, 1.0, // Seablue
      355, -20.0, 0.2, -1., 2.8, .9, 0.4, -1., 3, .9, 1.5, -4.5, 3.5, 1.0};  // Green

  public static Color rgbColor( int idx) {
    double gray_i0 = 0.32;
    double gray_i1 = 0.95;

    double h1, i1, s1;
    double r, g, b;
    int i, j, k;
    Rgb rgb;
    Color color = null;

    if ( idx == 300) {
      // This should be background color TODO...
      return getColor(33);
    }
    else if ( idx < 4)
      color = new Color( (int)(colorValues[3*idx] * 255),
		(int)(colorValues[3*idx + 1] * 255),
		(int)(colorValues[3*idx + 2] * 255));
    else  if ( idx < 20) {
      h1 = 360. * (idx - 4) / 16;
      s1 = 1.5;
      i1 = 1;
      rgb = hisToRgb( h1, i1, s1);
      color = new Color( (int)(rgb.r * 255), (int)(rgb.g * 255),(int)(rgb.b * 255));
    }
    else if ( idx < 60) {
      int val;
      if ( idx < 30 || (50 <= idx && idx < 60))
	gray_i0 = 0.25F;

      val = (int)( (gray_i0 + (gray_i1 - gray_i0) * Math.pow( (double)(9 - idx % 10) / 9, 0.9)) * 255);
      color = new Color( val, val, val);
    }
    else if ( idx < 300) {
      double i_min, i_max, s, a, h, hk;

      i = (idx - 60) / 30;
      j = (idx - 60 - i * 30) / 10;
      k = 9 - (idx - 60 - i * 30 - j * 10 );

      h = ctab[i * 14];
      hk = ctab[i * 14 + 1];
      s = ctab[i*14 + 2 + j*4];
      i_min = ctab[i*14 + 2 + j*4 + 1];
      i_max = ctab[i*14 + 2 + j*4 + 2];
      a = ctab[i*14 + 2 + j*4 + 3];

      s1 = s;
      i1 = i_min + (i_max - i_min) * 
	  Math.pow((double)k/9, a);
      h1 = h + hk * k / 9;
	
      if ( (i == 0 && j == 2) ||
	   (i == 2 && j == 2 && k > 5)) {
	// Formula doesn't work for yellow...
        rgb = new Rgb();
	rgb.r = rgbTab[ (i * 10 + k) * 3];
	rgb.g = rgbTab[ (i * 10 + k) * 3 + 1];
	rgb.b = rgbTab[ (i * 10 + k) * 3 + 2];
      }
      else
	rgb = hisToRgb( h1, i1, s1);

      color = new Color( (int)(rgb.r * 255), (int)(rgb.g * 255),(int)(rgb.b * 255));
    }
    return color;
  }

  public static Color shiftColor( int dt, int shift, int color_inverse) {
    int incr;
    int baseDrawtype;
    int drawtype;
    if ( color_inverse != 0)
      shift = -shift;

    if ( dt >= 20) {
      baseDrawtype = dt / 10 * 10;
      incr = shift + dt - baseDrawtype;
      if ( incr < 0)
	drawtype = COLOR_4; // White
      else if ( incr >= 10)
	drawtype = COLOR_30; // DarkGrey
      else
	drawtype = baseDrawtype + incr;
    }
    else
      drawtype = dt;

    if ( colors[drawtype] == null)
      colors[drawtype] = rgbColor( drawtype);
    return colors[drawtype];
  }

  public static int getDrawtype( int local_drawtype, int color_tone, int color_shift,
	int color_intensity,
	int color_brightness, int color_inverse, int default_color, boolean dimmed) {
    int drawtype;
    int	base_drawtype;
    int	incr;
    int intensity = color_intensity;
    int brightness = color_brightness;

    if ( default_color == NO_COLOR && color_tone != NO_TONE) {
      int tone = color_tone;
      if ( local_drawtype > 30) {
        if ( tone >= COLOR_TONE_GRAYHIGHSATURATION &&
	     tone < COLOR_TONE_GRAYLOWSATURATION) {
	    tone -= 27;
	    intensity += 2;
	}
        else if ( color_tone >= COLOR_TONE_GRAYLOWSATURATION) {
	    tone -= 2 * 27;
	    intensity -= 1;
	}
        if ( tone >= COLOR_TONE_DARKGRAY &&
	     tone < COLOR_TONE_LIGHTGRAY) {
	    tone -= 9;
	    brightness -= 2;
	}
        else if ( color_tone >= COLOR_TONE_LIGHTGRAY) {
	    tone -= 18;
	    brightness += 2;
	}

	drawtype = (local_drawtype - local_drawtype / 30 * 30 +
		30 * tone);
      }
      else
        drawtype = local_drawtype;
    }
    else if ( default_color != NO_COLOR) {
      drawtype = default_color;
    }
    else
      drawtype = local_drawtype;

    if ( brightness != 0) {
      if ( local_drawtype >= 30) {
        base_drawtype = drawtype / 10 * 10;
        incr = -brightness + drawtype - base_drawtype;
        if ( incr < 0)
          drawtype = 3; // White
        else if ( incr >= 10)
          drawtype = 29; // DarkGrey
        else
          drawtype = (base_drawtype + incr);
      }
    }
    if ( intensity != 0 ) {
      if ( drawtype >= 60) {
        base_drawtype = drawtype / 30 * 30;
        incr = drawtype - base_drawtype;
	drawtype += Math.min( 2 - incr / 10, intensity) * 10;
	if ( drawtype < base_drawtype)
	    drawtype = 40 + incr;
      }
    }
    if ( color_shift != 0) {
      if ( drawtype >= 60) {
        incr = color_shift -
		color_shift / 8 * 8;
        if ( incr < 0)
          incr += 8;

        incr = drawtype + incr * 30;
        if ( incr >= 300)
          incr -= 240;
        drawtype = incr;
      }
    }
    if ( color_inverse == 1)
    {
      if ( drawtype >= 10)
        drawtype = drawtype + 10 - 2 * (drawtype % 10) - 1;
    }
    if ( dimmed) {
      if ( drawtype == 0)
	drawtype = 25;
      else if ( 26 <= drawtype && drawtype <= 29)
	drawtype = drawtype - 4;
      else if ( 36 <= drawtype && drawtype <= 39)
	drawtype = drawtype - 4;
      else if ( 46 <= drawtype && drawtype <= 49)
	drawtype = drawtype - 4;
      else if ( 56 <= drawtype && drawtype <= 59)
	drawtype = drawtype - 4;
    }
    if ( drawtype < 0 || drawtype > 300) {
      System.out.println("** Invalid drawtype");
      drawtype = 0;
    }
    return drawtype;
  }

  public static Color getColor( int drawtype) {
    if ( colors[drawtype] == null)
      colors[drawtype] = rgbColor( drawtype);
    return colors[drawtype];
  }

  public static Color getColor( int local_drawtype, int color_tone, int color_shift,
	int color_intensity, int color_brightness, int color_inverse, 
	int default_color, boolean dimmed) {
    int drawtype = getDrawtype( local_drawtype, color_tone, color_shift, color_intensity,
				color_brightness, color_inverse, default_color, dimmed);
    if ( colors[drawtype] == null)
      colors[drawtype] = rgbColor( drawtype);
    return colors[drawtype];
  }
}









