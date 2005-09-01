/** 
 * Proview   $Id: wb_goen.h,v 1.2 2005-09-01 14:57:58 claes Exp $
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
 **/

#ifndef wb_goen_h
#define wb_goen_h

/* wb_goen.h
   This includefile contains values that describe the grapics
   drawn by the different methods in goen.  */

#ifndef _Xm_h
#include <Xm/Xm.h>
#endif


/* Input connectionpoint inverted or not */
#define GOEN_NOT_INVERTED       0
#define GOEN_INVERTED           1

/* Parmeter index in graphbody */
#define PAR_INPUT       0
#define PAR_INTERN      1
#define PAR_OUTPUT      2
#define PAR_DEV         3

/* The connectionpins direction */
#define CON_CENTER              0
#define CON_RIGHT               1
#define CON_UP                  2
#define CON_LEFT                3
#define CON_DOWN                4

/* Different types off gcc used in gredit */
#define GOEN_GGC_MAX            14
#define GOEN_GGC_LINE           0       /* Usual ggc for drawing        */
#define GOEN_GGC_HIGHLIGHT      1       /* Ggc for highlight            */
#define GOEN_GGC_TEXT           2       /* ggc for text                 */
#define GOEN_GGC_SUBWINDOW      3       /* ggc for subwindow mark       */
#define GOEN_GGC_BACKGROUND     4       /* ggc for rootobject           */
#define GOEN_GGC_PINTEXT        5       /* ggc for pintext              */
#define GOEN_GGC_OBJNAMETEXT    6       /* ggc for instance object name */
#define GOEN_GGC_SELRECT        7       /* ggc for selection rectangle  */
#define GOEN_GGC_GRID           8       /* ggc for gridpoint            */
#define GOEN_GGC_LINEDASHED     9       /* ggc for dashed line          */
#define GOEN_GGC_DISPLAYNODE    10      /* ggc for display node         */
#define GOEN_GGC_SMALLTEXT      11      /* ggc for text                 */
#define GOEN_GGC_MEDIUMTEXT     12      /* ggc for text                 */
#define GOEN_GGC_BIGTEXT        13      /* ggc for text                 */

/* Cursors used in gredit */
#define GOEN_CURSOR_MAX         5
#define GOEN_CURSOR_CLOCK       0
#define GOEN_CURSOR_HAND        1
#define GOEN_CURSOR_CROSS       2

/* Geometrical values */
#define GOEN_F_LINEWIDTH        0.005   /* linewidth for drawing        */
#define GOEN_F_HIGHLIGHT_LINEWIDTH 0.010 /* linewidth for highlight ggc */
#define GOEN_F_OBJNAME_STRWIDTH 0.017   /* Widht of letter used in objname */
#define GOEN_F_OBJNAME_STRHEIGHT 0.023  /* Height of fonts used in objname */
#define GOEN_F_MINWIDTH         8       /* Minimum width in strwidth size */
#define GOEN_F_GRID             0.05    /* Size between two conpoints */
#define GOEN_F_PINLENGTH        0.03    /* Length of a connection pin */
#define GOEN_F_INVERTCIRCLE     0.02    /* Diameter of a invert circle */
#define GOEN_F_GOE_WKSCALE      0.01    /* scalefactor goe - gredit
                                           (worldcoordinates) */
#define GOEN_F_GOE_REALSCALE    1.0     /* Visible scalefactor goe-gredit */
#define GOEN_F_DASHES           0.02    /* Length of dashes in ggc_linedashed */
#define GOEN_F_TEXTSIZE            2       /* Text size for texts and annot */

/* Cursor for hotevent */
#define GOEN_F_HOTCURSOR        (*cursors)[ GOEN_CURSOR_CROSS ]

/* Fonts for different text in the objects */
#define GOEN_F_OBJNAMEFONT      (*fonts)[ GOE_K_FONT_BOLD ][GOE_K_SIZE_12 ]
#define GOEN_F_TEXTFONT         (*fonts)[ GOE_K_FONT_BOLD ][GOE_K_SIZE_12 ]
#define GOEN_F_SMALLTEXTFONT    (*fonts)[ GOE_K_FONT_NORMAL ][GOE_K_SIZE_12 ]
#define GOEN_F_MEDIUMTEXTFONT   (*fonts)[ GOE_K_FONT_BOLD ][GOE_K_SIZE_18 ]
#define GOEN_F_BIGTEXTFONT      (*fonts)[ GOE_K_FONT_BOLD ][GOE_K_SIZE_24 ]
#define GOEN_F_PINNAMEFONT      (*fonts)[ GOE_K_FONT_NORMAL ][ GOE_K_SIZE_12 ]
#define GOEN_F_ANNOTATIONFONT   (*fonts)[ GOE_K_FONT_BOLD ][GOE_K_SIZE_12 ]

#define GOEN_MAX_COLOR          5
#define GOEN_MAX_ARROWS         3
#define GOEN_MAX_FILLPATTERN    6
#define GOEN_MAX_CURVATURE      11
#define GOEN_MAX_GRAPHMETHOD    18
/* Description of connectiontypes */
#define GOEN_ARROW_NONE 0               /* No arrow                     */
#define GOEN_ARROW_END  1               /* One arrow at end of connection */
#define GOEN_ARROW_BOTH 2               /* Two arrows                   */
#define GOEN_ROUTECON   0               /* Routed connection            */
#define GOEN_JAGGED     1               /* Streight line connection     */
#define GOEN_RECTILINEAR 2              /* Rectilinear connectionstyle  */
#define GOEN_STRANSDIV 3                /* Simultaneus transition divergence */
#define GOEN_STRANSCONV 4               /* Simultaneus transition convergence */
#define GOEN_STEPDIV 5                  /* Step divergence */
#define GOEN_STEPCONV 6                 /* Step convergence */
#define GOEN_JAGGED_DARR 7              /* Streight line connection double arrow    */
#define GOEN_RECTILINEAR_DARR 8         /* Rectilinear connectionstyle double arrow */
#define GOEN_JAGGED_INVARR 9            /* Streight line connection inverse arrow    */
#define GOEN_RECTILINEAR_INVARR 10      /* Rectilinear connectionstyle inverse arrow */
#define GOEN_CONDRAW    0               /* Connection drawn */
#define GOEN_CONUSERREF 1               /* Reference nodes at conpoints */
#define GOEN_CONSYSREF  2               /* Reference nodes at conpoints */
#define GOEN_REFNODEWIDTH 0.08          /* Reference rect width */
#define GOEN_REFNODEHEIGHT 0.035        /* Reference rect height */
#define GOEN_DISPLAYNODEWIDTH 0.09      /* Display node rect width */
#define GOEN_DISPLAYNODEHEIGHT 0.030    /* Display node rect height */

/* Connection attributes */
#define GOEN_CON_OBJTOOBJ       1
#define GOEN_CON_OUTPUTTOINPUT  2
#define GOEN_CON_SOURCEDEST     4
#define GOEN_CON_NOSOURCEDEST   8
#define GOEN_CON_EXECUTEORDER   16
#define GOEN_CON_NOEXECUTEORDER 32
#define GOEN_CON_SIGNAL         64
#define GOEN_CON_NOSIGNAL       128

#define GOEN_UNFILL     0               /* No fillpattern               */
#define GOEN_FILL1      1               /* Some fillpattern Grey medium-dark */
#define GOEN_FILL2      2               /* Some fillpattern Grey light */
#define GOEN_FILL3      3               /* Some fillpattern Grey light-medium */
#define GOEN_FILL4      4               /* Some fillpattern Grey medium */
#define GOEN_FILL5      5               /* Some fillpattern Grey dark */

#define GOEN_DISPLAYNODE_ANNOT      9     /* Annot nr for display node */

/* Array for the cursors loaded by goen */
typedef int             goen_t_cursors[GOEN_CURSOR_MAX];

/* Struct for connectionpoint geometric info returned by goen_get_point_info */
typedef struct  {
        unsigned long   type;
        float           x;
        float           y;
        }       goen_conpoint_type;

typedef struct  {
        float           x;
        float           y;
        }       goen_point_type;
                                                                                



#ifndef wb_gre_h
#include "wb_gre.h"
#endif



void goen_create_cursors( 
	Widget		widget,
	goen_t_cursors	*cursors
);

#if 0
void goen_create_ggcs(
	Widget		widget,
	goe_t_colors	*colors,
	goe_t_fonts	*fonts,
	goen_t_cursors	*cursors,
	goen_t_ggcs	*ggcs
);
#endif

int goen_create_contype( 
    flow_tCtx	        ctx,
    pwr_tClassId	conclass,
    ldh_tSesContext	ldhses,
    flow_tConClass      *con_class
);

int goen_create_nodetype( 
    flow_tCtx	         ctx,
    pwr_tClassId	 classid,
    ldh_tSesContext	 ldhses,
    unsigned int 	 *mask,
    unsigned long	 subwindowmark,
    unsigned long	 node_width,
    flow_tNode           *node_class,
    vldh_t_node		 node
);

int	goen_get_parinfo( 
    gre_ctx			grectx,
    pwr_tClassId		classid,
    ldh_tSesContext		ldhses,
    unsigned int		*mask,
    unsigned long		node_width,
    unsigned long		con_point,
    goen_conpoint_type		*graph_pointer,
    unsigned long		*par_type,
    unsigned long		*par_inverted,
    unsigned long		*par_index,
    vldh_t_node			 node
);

int	goen_get_pointinfo( 
    gre_ctx		 	grectx,
    pwr_tClassId		classid,
    ldh_tSesContext		ldhses,
    unsigned int		*mask,
    unsigned long		node_width,
    unsigned long		con_point,
    goen_conpoint_type	*graph_pointer,
    vldh_t_node		 node
);

int	goen_get_parameter( 
    pwr_tClassId		classid,
    ldh_tSesContext		ldhses,
    unsigned int		*mask,
    unsigned long		con_point,
    unsigned long		*par_type,
    unsigned long		*par_inverted,
    unsigned long		*par_index
);

int	goen_get_location_point( 
    gre_ctx		 	grectx,
    pwr_tClassId		classid,
    ldh_tSesContext		ldhses,
    unsigned int		*mask,
    unsigned long		node_width,
    goen_point_type		*location_point,
    vldh_t_node			node
);

int	goen_get_text_width( 
	char		*text,
	XmFontList	fontlist,
	float		*width,
	float		*height,
	int		*text_rows
);









#endif
