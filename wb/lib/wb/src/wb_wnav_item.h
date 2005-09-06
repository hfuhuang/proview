/** 
 * Proview   $Id: wb_wnav_item.h,v 1.9 2005-09-06 08:02:04 claes Exp $
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

#ifndef wb_wnav_item_h
#define wb_wnav_item_h

#if defined __cplusplus
extern "C" {
#endif

#ifndef pwr_h
# include "pwr.h"
#endif
#ifndef flow_h
#include "flow.h"
#endif

#ifndef flow_browctx_h
#include "flow_browapi.h"
#endif


typedef enum {
	wnav_eItemType_Object,
	wnav_eItemType_Menu,
	wnav_eItemType_Command,
	wnav_eItemType_Local,
	wnav_eItemType_Header,
	wnav_eItemType_HeaderLarge,
	wnav_eItemType_Volume,
	wnav_eItemType_Attr,
	wnav_eItemType_AttrInput,
	wnav_eItemType_AttrInputF,
	wnav_eItemType_AttrInputInv,
	wnav_eItemType_AttrOutput,
	wnav_eItemType_AttrArray,
	wnav_eItemType_AttrArrayOutput,
	wnav_eItemType_AttrArrayElem,
	wnav_eItemType_AttrObject,
	wnav_eItemType_Enum,
	wnav_eItemType_Mask,
	wnav_eItemType_ObjectName,
	wnav_eItemType_File,
	wnav_eItemType_Text,
	wnav_eItemType_Crossref,
	wnav_eItemType_DocBlock,
	wnav_eItemType_ObjectModTime
	} wnav_eItemType;

typedef enum {
	item_eDisplayType_Attr,
	item_eDisplayType_Path
	} item_eDisplayType;

typedef enum {
	item_eFileType_Unknown,
	item_eFileType_Script,
	item_eFileType_Graph
	} item_eFileType;

class WItem {
  public:
    WItem( pwr_tObjid item_objid, int item_is_root) :
	type( wnav_eItemType_Object), objid(item_objid), is_root(item_is_root),
	node(NULL)
	{};
    int open_attributes( WNav *wnav, double x, double y)
				{ return 1;};
    int	open_children( WNav *wnav, double x, double y)
				{ return 1;};
    int open_trace( WNav *wnav, double x, double y)
				{ return 1;};
    void close( WNav *wnav, double x, double y) {};
    virtual pwr_sAttrRef aref();
    wnav_eItemType	type;
    pwr_tObjid		objid;
    int			is_root;
    brow_tNode		node;
    char	 	name[120];
};

class WItemBaseObject : public WItem {
  public:
    WItemBaseObject( pwr_tObjid item_objid, int item_is_root):
	WItem( item_objid, item_is_root) {};
    int			open_children( WNav *wnav, double x, double y);
    int     		open_attributes( WNav *wnav, double x, double y);
    int     		close( WNav *wnav, double x, double y);
    int 		open_attribute( WNav *wnav, double x, double y, 
				char *attr_name, int element) {return 1;};
    int                 open_crossref( WNav *wnav, double x, double y);
};

class WItemObject : public WItemBaseObject {
  public:
    WItemObject( WNav *wnav, pwr_tObjid item_objid, 
	brow_tNode dest, flow_eDest dest_code, int item_is_root);
};

class WItemMenu : public WItem {
  public:
    WItemMenu( WNav *wnav, char *item_name, 
	brow_tNode dest, flow_eDest dest_code, wnav_sMenu **item_child_list,
	int item_is_root);
    wnav_sMenu		**child_list;
    int			open_children( WNav *wnav, double x, double y);
    int     		close( WNav *wnav, double x, double y);
};

class WItemCommand : public WItem {
  public:
    WItemCommand( WNav *wnav, char *item_name, 
	brow_tNode dest, flow_eDest dest_code, char *item_command, 
	int item_is_root, flow_sAnnotPixmap *pixmap);
    char		command[200];
    int			open_children( WNav *wnav, double x, double y);
};


class WItemLocal : public WItem {
  public:
    WItemLocal( WNav *wnav, char *item_name, char *attr,
	int attr_type, int attr_size, double attr_min_limit,
	double attr_max_limit,
	void *attr_value_p, brow_tNode dest, flow_eDest dest_code);
    void		*value_p;
    char 		old_value[80];
    int 		first_scan;
    int			type_id;
    int			size;
    double		min_limit;
    double		max_limit;
};

class WItemText : public WItem {
  public:
    WItemText( WNav *wnav, char *item_name, char *text,
	brow_tNode dest, flow_eDest dest_code);
};

class WItemHeader : public WItem {
  public:
    WItemHeader( WNav *wnav, char *item_name, char *title,
	brow_tNode dest, flow_eDest dest_code);
};

class WItemHeaderLarge : public WItem {
  public:
    WItemHeaderLarge( WNav *wnav, char *item_name, char *title,
	brow_tNode dest, flow_eDest dest_code);
};

class WItemVolume : public WItem {
  public:
    WItemVolume( WNav *wnav, pwr_tVolumeId item_volid,
	brow_tNode dest, flow_eDest dest_code);
    pwr_tVolumeId	volid;
    ldh_tVolContext	volctx;
    int			open_children( WNav *wnav, double x, double y);
    int			close( WNav *wnav, double x, double y);
};

class WItemObjectName : public WItem {
  public:
    WItemObjectName(
	WNavBrow *item_brow, ldh_tSesContext item_ldhses, 
	pwr_tObjid item_objid, 
	brow_tNode dest, flow_eDest dest_code);
    WNavBrow *brow;
    ldh_tSesContext ldhses;

    int update();
    int get_value( char **value);	// The value should be freed with free
};

class WItemObjectModTime : public WItem {
  public:
    WItemObjectModTime(
	WNavBrow *item_brow, ldh_tSesContext item_ldhses, 
	pwr_tObjid item_objid, 
	brow_tNode dest, flow_eDest dest_code);
    WNavBrow *brow;
    ldh_tSesContext ldhses;

    int update();
};

class WItemDocBlock : public WItem {
  public:
    WItemDocBlock(
	WNavBrow *item_brow, ldh_tSesContext item_ldhses, 
	pwr_tObjid item_objid, char *block, int item_size,
	brow_tNode dest, flow_eDest dest_code);
    WNavBrow *brow;
    ldh_tSesContext ldhses;

    int update();
    int get_value( char **value);	// The value should be freed with free
};

class WItemFile : public WItem {
  public:
    WItemFile( WNav *wnav, char *item_name, char *text, char *file,
	item_eFileType item_filetype, brow_tNode dest, flow_eDest dest_code);
    int			open_children( WNav *wnav, double x, double y);
    char file_name[120];
    item_eFileType file_type;

};

class WItemCrossref : public WItem {
  public:
    WItemCrossref( WNav *wnav, char *item_ref_name, char *item_ref_class, 
	int item_write, brow_tNode dest, flow_eDest dest_code);
    char		ref_name[32];
    char		ref_class[32];
    int			write;
};


class WItemBaseAttr : public WItem {
  public:
    WItemBaseAttr( 
	WNavBrow *item_brow, ldh_tSesContext item_ldhses, 
	pwr_tObjid item_objid,
	char *attr_name, int attr_type_id, pwr_tTid attr_tid,
	int attr_size, int attr_flags,
	char *attr_body);

    WNavBrow *brow;
    ldh_tSesContext ldhses;
    int type_id;
    pwr_tTid tid;
    int size;
    int flags;
    char attr[120];
    char body[20];
    pwr_tClassId classid;

    virtual pwr_sAttrRef aref();
    int get_value( char **value);	// The value should be freed with XtFree
    int update() { return 1;};
};

class WItemAttr : public WItemBaseAttr {
  public:
    WItemAttr( 
	WNavBrow *item_brow, ldh_tSesContext item_ldhses, 
	pwr_tObjid item_objid,
	brow_tNode dest, flow_eDest dest_code,
	char *attr_name, int attr_type_id, pwr_tTid attr_tid,
	int attr_size, int attr_flags,
	char *attr_body, int fullname);
     int open_children( double x, double y);
     int close( double x, double y);
     int update();
};

class WItemAttrInput : public WItemBaseAttr {
  public:
    WItemAttrInput( 
	WNavBrow *item_brow, ldh_tSesContext item_ldhses, 
	pwr_tObjid item_objid,
	brow_tNode dest, flow_eDest dest_code,
	char *attr_name, int attr_type_id, pwr_tTid attr_tid,
	int attr_size, int attr_flags,
	char *attr_body, int attr_input_num);
     int update();
     int set_mask( int radio_button, int value);
     unsigned int mask;
};

class WItemAttrInputF : public WItemBaseAttr {
  public:
    WItemAttrInputF( 
	WNavBrow *item_brow, ldh_tSesContext item_ldhses, 
	pwr_tObjid item_objid,
	brow_tNode dest, flow_eDest dest_code,
	char *attr_name, int attr_type_id, pwr_tTid attr_tid,
	int attr_size, int attr_flags,
	char *attr_body, int attr_input_num);
     int update();
     int set_mask( int radio_button, int value);
     unsigned int mask;
};

class WItemAttrInputInv : public WItemBaseAttr {
  public:
    WItemAttrInputInv( 
	WNavBrow *item_brow, ldh_tSesContext item_ldhses, 
	pwr_tObjid item_objid,
	brow_tNode dest, flow_eDest dest_code,
	char *attr_name, int attr_type_id, pwr_tTid attr_tid,
	int attr_size, int attr_flags,
	char *attr_body, int attr_input_num);
     int update();
     int set_mask( int radio_button, int value);
     unsigned int mask;
};

class WItemAttrOutput : public WItemBaseAttr {
  public:
    WItemAttrOutput( 
	WNavBrow *item_brow, ldh_tSesContext item_ldhses, 
	pwr_tObjid item_objid,
	brow_tNode dest, flow_eDest dest_code,
	char *attr_name, int attr_type_id, pwr_tTid attr_tid,
	int attr_size, int attr_flags,
	char *attr_body, int attr_output_num);
     int update();
     int set_mask( int radio_button, int value);
     unsigned int mask;
};

class WItemAttrArray : public WItemBaseAttr {
  public:
    int elements;
    WItemAttrArray( 
	WNavBrow *item_brow, ldh_tSesContext item_ldhses, 
	pwr_tObjid item_objid,
	brow_tNode dest, flow_eDest dest_code,
	char *attr_name, int attr_elements, int attr_type_id, 
	pwr_tTid attr_tid, int attr_size, int attr_flags, 
	char *attr_body, int fullname);
    int     open_children( double x, double y) {return 1;};
    int     open_attributes( double x, double y);
    int     close( double x, double y);
};

class WItemAttrArrayOutput : public WItemBaseAttr {
  public:
    int elements;
    unsigned int mask;
    WItemAttrArrayOutput( 
	WNavBrow *item_brow, ldh_tSesContext item_ldhses, 
	pwr_tObjid item_objid,
	brow_tNode dest, flow_eDest dest_code,
	char *attr_name, int attr_elements, int attr_type_id, 
	pwr_tTid attr_tid,
	int attr_size, int attr_flags, char *attr_body, int attr_output_num);
    int     open_children( double x, double y) {return 1;};
    int     open_attributes( double x, double y);
    int     close( double x, double y);
    int	    update();
    int     set_mask( int radio_button, int value);
};

class WItemAttrArrayElem : public WItemBaseAttr {
  public:
    int element;
    int offset;
    WItemAttrArrayElem( 
	WNavBrow *item_brow, ldh_tSesContext item_ldhses, 
	pwr_tObjid item_objid,
	brow_tNode dest, flow_eDest dest_code,
	char *attr_name, int attr_element, int attr_type_id,
	pwr_tTid attr_tid, int attr_size, int attr_offset, 
	int attr_flags, char *attr_body);
     int open_children( double x, double y);
     int close( double x, double y);
     int update();
     int get_value( char **value);
};

class WItemAttrObject : public WItemBaseAttr {
  public:
    bool is_elem;
    int idx;
  
    WItemAttrObject( 
	WNavBrow *item_brow, ldh_tSesContext item_ldhses, 
	pwr_tObjid item_objid,
	brow_tNode dest, flow_eDest dest_code,
	char *attr_name, int attr_type_id, 
	int attr_size, bool attr_is_elem, int attr_idx, 
	int attr_flags, char *attr_body, int fullname);
    int     open_children( double x, double y) {return 1;};
    int     open_attributes( double x, double y);
    int     open_crossref( WNav *wnav, double x, double y);
    int     close( double x, double y);
};

class WItemEnum : public WItemBaseAttr {
  public:
    WItemEnum( 
	WNavBrow *item_brow, ldh_tSesContext item_ldhses, 
	pwr_tObjid item_objid, 
	char *attr_enum_name, char *attr_name, 
	int attr_type_id, pwr_tTid attr_tid, int attr_size, 
	int attr_flags, char *attr_body,
	unsigned int item_num, int item_is_element, int item_element,
	brow_tNode dest, flow_eDest dest_code);
    char	 	enum_name[40];
    unsigned int	num;
    int			is_element;
    int			element;

    int update();
    int set();
};

class WItemMask : public WItemBaseAttr {
  public:
    WItemMask( 
	WNavBrow *item_brow, ldh_tSesContext item_ldhses, 
	pwr_tObjid item_objid, 
	char *attr_mask_name, char *attr_name, 
	int attr_type_id, pwr_tTid attr_tid, int attr_size, 
	int attr_flags, char *attr_body,
	unsigned int item_mask, int item_is_element, int item_element,
	brow_tNode dest, flow_eDest dest_code);
    char	 	mask_name[40];
    unsigned int	mask;
    int			is_element;
    int			element;

    int update();
    int set( int set_value);
};

#if defined __cplusplus
}
#endif
#endif
