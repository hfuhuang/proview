#ifndef flow_browapi_h
#define flow_browapi_h

#if defined __cplusplus
extern "C" {
#endif

#ifndef __cplusplus
typedef void *BrowCtx;
#ifndef flow_api_h
typedef void *FlowCtx;
#endif
#endif

#include "flow.h"

typedef struct {
	double	base_zoom_factor;
	double	indentation;
	double	annotation_space;
} brow_sAttributes;

typedef enum {
	brow_eAttr_base_zoom_factor 	= 1 << 0,
	brow_eAttr_indentation		= 1 << 1,
	brow_eAttr_annotation_space	= 1 << 2
	} brow_eAttribute;


typedef BrowCtx *brow_tCtx;
typedef void *brow_tNode;
typedef void *brow_tCon;
typedef void *brow_tObject;
typedef void *brow_tNodeClass;
typedef void *brow_tConClass;

int brow_Save( brow_tCtx ctx, char *filename);
int brow_Open( brow_tCtx ctx, char *filename);
int brow_SaveTrace( brow_tCtx ctx, char *filename);
int brow_OpenTrace( brow_tCtx ctx, char *filename);
void brow_DeleteAll( brow_tCtx ctx);
void brow_DeleteNode( brow_tCtx ctx, brow_tNode node);
void brow_CloseNode( brow_tCtx ctx, brow_tNode node);
int brow_FindSelectedObject( brow_tCtx ctx, brow_tObject object);
void brow_ResetHighlightAll( brow_tCtx ctx);
void brow_ResetInverseAll( brow_tCtx ctx);
void brow_SetSelectHighlight( brow_tCtx ctx);
void brow_SetSelectInverse( brow_tCtx ctx);
void brow_ResetSelectHighlight( brow_tCtx ctx);
void brow_ResetSelectInverse( brow_tCtx ctx);
void brow_SelectInsert( brow_tCtx ctx, brow_tObject object);
void brow_SelectRemove( brow_tCtx ctx, brow_tObject object);
void brow_SelectClear( brow_tCtx ctx);
void brow_GetSelectedNodes( brow_tCtx ctx, brow_tNode **nodes, int *num);
void brow_SetHighlight( brow_tObject object, int value);
void brow_GetHighlight( brow_tObject object, int *value);
void brow_SetInverse( brow_tObject object, int value);
void brow_CreateNode( brow_tCtx ctx, char *name, brow_tNodeClass nc,
	brow_tNode destination, flow_eDest destination_code, 
	void *user_data, int relative_annot_pos, brow_tNode *node);
void brow_CreatePasteNode( brow_tCtx ctx, char *name, brow_tNodeClass nc,
	double x, double y, void *user_data, brow_tNode *node);
void brow_SetAnnotation( brow_tNode node, int number, char *text, int size);
void brow_GetAnnotation( brow_tNode node, int number, char *text, int size);
void brow_OpenAnnotationInput( brow_tNode node, int number);
int brow_AnnotationInputIsOpen( brow_tNode node, int number);
int brow_GetAnnotationInput( brow_tNode node, int number, char **text);
void brow_CloseAnnotationInput( brow_tNode node, int number);
void brow_SetPasteNodeAnnotation( brow_tNode node, int number, char *text, 
	int size);
void brow_EnableEvent( BrowCtx *ctx, flow_eEvent event, 
		flow_eEventType event_type, 
		int (*event_cb)(FlowCtx *ctx, flow_tEvent event));
void brow_DisableEvent( BrowCtx *ctx, flow_eEvent event);
void brow_DisableEventAll( BrowCtx *ctx);
void brow_Cut( brow_tCtx ctx);
void brow_Copy( brow_tCtx ctx);
void brow_Paste( brow_tCtx ctx);
void brow_PasteClear( brow_tCtx ctx);
void brow_CreateRect( brow_tCtx ctx, double x, double y, 
	double width, double height,
	flow_eDrawType draw_type, int line_width, int fix_line_width, 
	brow_tObject *rect);
void brow_CreateFilledRect( brow_tCtx ctx, double x, double y, 
	double width, double height,
	flow_eDrawType draw_type,
	brow_tObject *rect);
void brow_CreateLine( brow_tCtx ctx, double x1, double y1, 
	double x2, double y2,
	flow_eDrawType draw_type, int line_width, int fix_line_width, 
	brow_tObject *line);
void brow_CreateArc( brow_tCtx ctx, double x1, double y1, 
	double x2, double y2, int angel1, int angel2,
	flow_eDrawType draw_type, int line_width, brow_tObject *arc);
void brow_CreateText( brow_tCtx ctx, char *text_str, double x, double y, 
	flow_eDrawType draw_type, int text_size, brow_tObject *text);
void brow_CreatePixmap( brow_tCtx ctx, flow_sPixmapData *pixmap_data, 
	double x, double y, flow_eDrawType draw_type, int size, 
	brow_tObject *pixmap);
void brow_CreateAnnotPixmap( brow_tCtx ctx, int number, 
	double x, double y, flow_eDrawType draw_type, int size, int relative_pos,
	brow_tObject *annot_pixmap);
void brow_CreateAnnot( brow_tCtx ctx, double x, double y, int number, 
	flow_eDrawType draw_type, int text_size, brow_tObject *annot);
void brow_AddRect( brow_tNodeClass nc, double x, double y, 
	double width, double height,
	flow_eDrawType draw_type, int line_width, int fix_line_width);
void brow_AddFilledRect( brow_tNodeClass nc, double x, double y, 
	double width, double height,
	flow_eDrawType draw_type);
void brow_AddFrame( brow_tNodeClass nc, double x, double y, 
	double width, double height,
	flow_eDrawType draw_type, int line_width, int fix_line_width);
void brow_AddLine( brow_tNodeClass nc, double x1, double y1, 
	double x2, double y2,
	flow_eDrawType draw_type, int line_width);
void brow_AddArc( brow_tNodeClass nc, double x1, double y1, 
	double x2, double y2, int angel1, int angel2,
	flow_eDrawType draw_type, int line_width);
void brow_AddImage( brow_tNodeClass nc, char *imagefile, double x, double y);
void brow_AddText( brow_tNodeClass nc, char *text_str, double x, double y, 
	flow_eDrawType draw_type, int text_size);
void brow_AddAnnot( brow_tNodeClass nc, double x, double y, int number,
	flow_eDrawType draw_type, int text_size, flow_eAnnotType annot_type,
	int relative_pos);
void brow_AddAnnotPixmap( brow_tNodeClass nc, int number,
	double x, double y, flow_eDrawType draw_type, int size, int relative_pos);
void brow_AddRadiobutton( brow_tNodeClass nc, double x, double y, 
	double width, double height, int number,
	flow_eDrawType draw_type, int line_width);
void brow_CreatePushButton( brow_tCtx ctx, char *text, double x, double y, 
	double width, double height, brow_tObject *pushbutton);
void brow_CreateNodeClass( brow_tCtx ctx, char *name, flow_eNodeGroup group,
	brow_tNodeClass *nodeclass);
void brow_NodeClassAdd( brow_tNodeClass nc, brow_tObject object);
void brow_GetSelectList( brow_tCtx ctx, brow_tObject **list, int *cnt);
void brow_GetObjectList( brow_tCtx ctx, brow_tObject **list, int *cnt);
flow_eObjectType brow_GetObjectType( brow_tObject object);
void brow_MeasureNode( brow_tNode node, double *ll_x, double *ll_y,
	double *ur_x, double *ur_y);
int brow_Print( brow_tCtx ctx, char *filename);
void brow_GetUserData( brow_tObject object, void **user_data);
void brow_SetUserData( brow_tObject object, void *user_data);
void brow_GetCtxUserData( brow_tCtx ctx, void **user_data);
void brow_SetCtxUserData( brow_tCtx ctx, void *user_data);
brow_tCtx brow_GetCtx( brow_tObject object);
void brow_SetTraceAttr( brow_tObject object, char *trace_object, 
		char *trace_attribute, flow_eTraceType trace_attr_type);
void brow_GetTraceAttr( brow_tObject object, char *trace_object, 
		char *trace_attribute, flow_eTraceType *trace_attr_type);
void brow_SetTraceData( brow_tObject object, void *trace_data);
int brow_TraceInit( brow_tCtx ctx, int (*trace_connect_func)( brow_tObject, 
	char *, char *, flow_eTraceType, void **), 
	int (*trace_disconnect_func)( brow_tObject),
	int (*trace_scan_func)( brow_tObject, void *));
void brow_TraceClose( brow_tCtx ctx);
void brow_TraceScan( brow_tCtx ctx);
void brow_RemoveTraceObjects( brow_tCtx ctx);
void brow_Zoom( brow_tCtx ctx, double zoom_factor);
void brow_GetZoom( brow_tCtx ctx, double *zoom_factor);
void brow_ZoomAbsolute( brow_tCtx ctx, double zoom_factor);
void brow_SetAttributes( brow_tCtx ctx, brow_sAttributes *attr,
	unsigned long mask);
void brow_PositionToPixel( brow_tCtx ctx, double x, double y, 
		int *pix_x, int *pix_y);
void brow_UnZoom( brow_tCtx ctx);
void brow_CenterObject( brow_tCtx ctx, brow_tObject object, double factor);
void brow_GetNodePosition( brow_tNode node, double *x, double *y);
void brow_MeasureAnnotation( brow_tNodeClass node_class, int number, char *text, 
	double *width, double *height);
void brow_MeasureAnnotText( brow_tCtx ctx, char *text, flow_eDrawType draw_type,
		int text_size, flow_eAnnotType annot_type,
		double *width, double *height, int *rows);
flow_eNodeGroup brow_GetNodeGroup( brow_tNode node);
void brow_GetObjectName( brow_tObject object, char *name);
void brow_Reconfigure( brow_tCtx ctx);
void brow_SetNodraw( brow_tCtx ctx);
void brow_ResetNodraw( brow_tCtx ctx);
void brow_Redraw( brow_tCtx ctx, double y_redraw);
void brow_SetAnnotPixmap( brow_tNode node, int number, 
	flow_sAnnotPixmap *pixmap);
void brow_RemoveAnnotPixmap( brow_tNode node, int number);
void brow_AllocAnnotPixmap( brow_tCtx ctx, flow_sPixmapData *pixmap_data, 
		flow_sAnnotPixmap **pixmap);
void brow_FreeAnnotPixmap( brow_tCtx ctx, flow_sAnnotPixmap *pixmap);
void brow_SetRadiobutton( brow_tNode node, int number, int value);
void brow_GetRadiobutton( brow_tNode node, int number, int *value);
void brow_SetOpen( brow_tNode node, int mask);
void brow_ResetOpen( brow_tNode node, int mask);
int brow_IsOpen( brow_tNode node);
int brow_GetNext( brow_tCtx ctx, brow_tObject object, brow_tObject *next);
int brow_GetPrevious( brow_tCtx ctx, brow_tObject object, brow_tObject *prev);
int brow_GetFirst( brow_tCtx ctx, brow_tObject *first);
int brow_GetLast( brow_tCtx ctx, brow_tObject *last);
int brow_GetParent( brow_tCtx ctx, brow_tObject object, brow_tObject *parent);
int brow_GetChild( brow_tCtx ctx, brow_tObject object, brow_tObject *child);
int brow_GetNextSibling( brow_tCtx ctx, brow_tObject object, 
		brow_tObject *sibling);
int brow_GetPreviousSibling( brow_tCtx ctx, brow_tObject object, 
		brow_tObject *sibling);
int brow_IsVisible( brow_tCtx ctx, brow_tObject object);
int brow_Page( brow_tCtx ctx, double factor);
int brow_CreateSecondaryCtx( brow_tCtx ctx, brow_tCtx *secondary_ctx,
        int (*init_proc)(brow_tCtx ctx, void *client_data),
	void  *client_data, 
	flow_eCtxType type);
void brow_DeleteSecondaryCtx( brow_tCtx ctx);
int brow_ChangeCtx( Widget w, brow_tCtx from_ctx, brow_tCtx to_ctx);
void brow_SetInputFocus( brow_tCtx ctx);
void brow_SetWidgetInputFocus( Widget w);
void brow_SetClickSensitivity( brow_tCtx ctx, int value);
void brow_SetWhiteBackground( brow_tCtx ctx);

#if defined __cplusplus
}
#endif
#endif
