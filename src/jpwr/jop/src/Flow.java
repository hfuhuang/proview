/* 
 * Proview   $Id: Flow.java,v 1.2 2005-09-01 14:57:50 claes Exp $
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


public class Flow {
    public static final int DRAWOFFSET                          = 2;

    public static final int eConType_Straight                   = 0;
    public static final int eConType_Fixed                      = 1;
    public static final int eConType_AllFixed                   = 2;
    public static final int eConType_Routed                     = 3;
    public static final int eConType_StepDiv                    = 4;
    public static final int eConType_StepConv                   = 5;
    public static final int eConType_TransDiv                   = 6;
    public static final int eConType_TransConv                  = 7;
    public static final int eConType_StraightOneArrow           = 8;
    public static final int eConType_Reference                  = 9;

    public static final int eDrawType_Line                      = 0;
    public static final int eDrawType_LineRed                   = 1;
    public static final int eDrawType_LineGray                  = 2;
    public static final int eDrawType_LineErase                 = 3;
    public static final int eDrawType_LineDashed                = 4;
    public static final int eDrawType_LineDashedRed             = 5;
    public static final int eDrawType_TextHelvetica             = 6;
    public static final int eDrawType_TextHelveticaBold         = 7;
    public static final int eDrawType_TextHelveticaErase        = 8;
    public static final int eDrawType_TextHelveticaEraseBold    = 9;

    public static final int mDisplayLevel_1		        = 1 << 0;
    public static final int mDisplayLevel_2		        = 1 << 2;
    public static final int mDisplayLevel_3		        = 1 << 3;
    public static final int mDisplayLevel_4		        = 1 << 4;
    public static final int mDisplayLevel_5		        = 1 << 5;
    public static final int mDisplayLevel_6		        = 1 << 6;

    public static final int eNodeGroup_Common                   = 0;
    public static final int eNodeGroup_Document                 = 1;
    public static final int eNodeGroup_Trace                    = 2;

    public static final int eTraceType_Boolean                  = 0;
    public static final int eTraceType_Int32                    = 1;
    public static final int eTraceType_Float32                  = 2;
    public static final int eTraceType_User                     = 3;

    public static final int eSave_Ctx			        = 1;
    public static final int eSave_Array			        = 2;
    public static final int eSave_NodeClass			= 3;
    public static final int eSave_ConClass			= 4;
    public static final int eSave_Rect			        = 5;
    public static final int eSave_Line			        = 6;
    public static final int eSave_Point			        = 7;
    public static final int eSave_Arc			        = 8;
    public static final int eSave_Text			        = 9;
    public static final int eSave_Node			        = 10;
    public static final int eSave_Con			        = 11;
    public static final int eSave_ConPoint			= 12;
    public static final int eSave_Annot			        = 13;
    public static final int eSave_Arrow			        = 14;
    public static final int eSave_Pixmap			= 15;
    public static final int eSave_AnnotPixmap		        = 16;
    public static final int eSave_Radiobutton		        = 17;
    public static final int eSave_Frame			        = 18;
    public static final int eSave_End			        = 99;
    public static final int eSave_Ctx_zoom_factor		= 100;
    public static final int eSave_Ctx_base_zoom_factor	        = 101;
    public static final int eSave_Ctx_offset_x		        = 102;
    public static final int eSave_Ctx_offset_y		        = 103;
    public static final int eSave_Ctx_nav_zoom_factor	        = 104;
    public static final int eSave_Ctx_print_zoom_factor	        = 105;
    public static final int eSave_Ctx_nav_offset_x		= 106;
    public static final int eSave_Ctx_nav_offset_y		= 107;
    public static final int eSave_Ctx_x_right		        = 108;
    public static final int eSave_Ctx_x_left		        = 109;
    public static final int eSave_Ctx_y_high	                = 110;
    public static final int eSave_Ctx_y_low			= 111;
    public static final int eSave_Ctx_window_width		= 112;
    public static final int eSave_Ctx_window_height		= 113;
    public static final int eSave_Ctx_nav_window_width	        = 114;
    public static final int eSave_Ctx_nav_window_height	        = 115;
    public static final int eSave_Ctx_nav_rect_ll_x		= 116;
    public static final int eSave_Ctx_nav_rect_ll_y		= 117;
    public static final int eSave_Ctx_nav_rect_ur_x		= 118;
    public static final int eSave_Ctx_nav_rect_ur_y		= 119;
    public static final int eSave_Ctx_nav_rect_hot		= 120;
    public static final int eSave_Ctx_name			= 121;
    public static final int eSave_Ctx_user_highlight	        = 122;
    public static final int eSave_Ctx_a_nc			= 123;
    public static final int eSave_Ctx_a_cc			= 124;
    public static final int eSave_Ctx_a			        = 125;
    public static final int eSave_Ctx_grid_size_x		= 126;
    public static final int eSave_Ctx_grid_size_y		= 127;
    public static final int eSave_Ctx_grid_on		        = 128;
    public static final int eSave_Ctx_draw_delta		= 129;
    public static final int eSave_Ctx_refcon_width		= 130;
    public static final int eSave_Ctx_refcon_height		= 131;
    public static final int eSave_Ctx_refcon_textsize	        = 132;
    public static final int eSave_Ctx_refcon_linewidth          = 133;
    public static final int eSave_Array_a			= 200;
    public static final int eSave_NodeClass_nc_name		= 300;
    public static final int eSave_NodeClass_a		        = 301;
    public static final int eSave_NodeClass_group		= 302;
    public static final int eSave_ConClass_cc_name		= 400;
    public static final int eSave_ConClass_con_type		= 401;
    public static final int eSave_ConClass_corner		= 402;
    public static final int eSave_ConClass_draw_type	        = 403;
    public static final int eSave_ConClass_line_width	        = 404;
    public static final int eSave_ConClass_arrow_width          = 405;
    public static final int eSave_ConClass_arrow_length	        = 406;
    public static final int eSave_ConClass_round_corner_amount  = 407;
    public static final int eSave_ConClass_group		= 408;
    public static final int eSave_Rect_draw_type		= 500;
    public static final int eSave_Rect_line_width		= 501;
    public static final int eSave_Rect_ll			= 502;
    public static final int eSave_Rect_ur			= 503;
    public static final int eSave_Rect_display_level	        = 504;
    public static final int eSave_Line_draw_type		= 600;
    public static final int eSave_Line_line_width		= 601;
    public static final int eSave_Line_p1			= 602;
    public static final int eSave_Line_p2			= 603;
    public static final int eSave_Point_x			= 700;
    public static final int eSave_Point_y			= 701;
    public static final int eSave_Arc_angel1		        = 800;
    public static final int eSave_Arc_angel2		        = 801;
    public static final int eSave_Arc_draw_type		        = 802;
    public static final int eSave_Arc_line_width		= 803;
    public static final int eSave_Arc_ll			= 804;
    public static final int eSave_Arc_ur			= 805;
    public static final int eSave_Text_text_size		= 900;
    public static final int eSave_Text_draw_type		= 901;
    public static final int eSave_Text_text			= 902;
    public static final int eSave_Text_p			= 903;
    public static final int eSave_Node_nc			= 1000;
    public static final int eSave_Node_pos			= 1001;
    public static final int eSave_Node_n_name		        = 1002;
    public static final int eSave_Node_annotsize		= 1003;
    public static final int eSave_Node_annotv		        = 1004;
    public static final int eSave_Node_refcon_cnt		= 1005;
    public static final int eSave_Node_x_right		        = 1006;
    public static final int eSave_Node_x_left		        = 1007;
    public static final int eSave_Node_y_high		        = 1008;
    public static final int eSave_Node_y_low		        = 1009;
    public static final int eSave_Node_trace_object		= 1010;
    public static final int eSave_Node_trace_attribute	        = 1011;
    public static final int eSave_Node_trace_attr_type	        = 1012;
    public static final int eSave_Node_obst_x_right		= 1013;
    public static final int eSave_Node_obst_x_left		= 1014;
    public static final int eSave_Node_obst_y_high		= 1015;
    public static final int eSave_Node_obst_y_low		= 1016;
    public static final int eSave_Con_x_right		        = 1100;
    public static final int eSave_Con_x_left		        = 1101;
    public static final int eSave_Con_y_high		        = 1102;
    public static final int eSave_Con_y_low			= 1103;
    public static final int eSave_Con_cc			= 1104;
    public static final int eSave_Con_dest_node		        = 1105;
    public static final int eSave_Con_source_node		= 1106;
    public static final int eSave_Con_dest_conpoint		= 1107;
    public static final int eSave_Con_source_conpoint	        = 1108;
    public static final int eSave_Con_dest_direction	        = 1109;
    public static final int eSave_Con_source_direction          = 1110;
    public static final int eSave_Con_line_a		        = 1111;
    public static final int eSave_Con_arc_a			= 1112;
    public static final int eSave_Con_arrow_a		        = 1113;
    public static final int eSave_Con_ref_a			= 1114;
    public static final int eSave_Con_p_num			= 1115;
    public static final int eSave_Con_l_num			= 1116;
    public static final int eSave_Con_a_num			= 1117;
    public static final int eSave_Con_arrow_num		        = 1118;
    public static final int eSave_Con_ref_num	                = 1119;
    public static final int eSave_Con_point_x		        = 1120;
    public static final int eSave_Con_point_y		        = 1121;
    public static final int eSave_Con_source_ref_cnt            = 1122;
    public static final int eSave_Con_dest_ref_cnt		= 1123;
    public static final int eSave_Con_c_name		        = 1124;
    public static final int eSave_Con_trace_object		= 1125;
    public static final int eSave_Con_trace_attribute	        = 1126;
    public static final int eSave_Con_trace_attr_type	        = 1127;
    public static final int eSave_Con_temporary_ref		= 1128;
    public static final int eSave_ConPoint_number		= 1200;
    public static final int eSave_ConPoint_direction	        = 1201;
    public static final int eSave_ConPoint_p		        = 1202;
    public static final int eSave_ConPoint_trace_attribute	= 1203;
    public static final int eSave_ConPoint_trace_attr_type	= 1204;
    public static final int eSave_Annot_number		        = 1300;
    public static final int eSave_Annot_draw_type		= 1301;
    public static final int eSave_Annot_text_size		= 1302;
    public static final int eSave_Annot_p			= 1303;
    public static final int eSave_Annot_annot_type		= 1304;
    public static final int eSave_Annot_display_level	        = 1305;
    public static final int eSave_Arrow_arrow_width		= 1400;
    public static final int eSave_Arrow_arrow_length	        = 1401;
    public static final int eSave_Arrow_draw_type		= 1402;
    public static final int eSave_Arrow_line_width		= 1403;
    public static final int eSave_Arrow_p_dest		        = 1404;
    public static final int eSave_Arrow_p1			= 1405;
    public static final int eSave_Arrow_p2			= 1406;
}
