#ifndef glow_conpoint_h
#define glow_conpoint_h

#include <iostream.h>
#include <fstream.h>
#include "glow.h"
#include "glow_ctx.h"
#include "glow_point.h"
#include "glow_array_elem.h"


class GlowConPoint : public GlowArrayElem {
  public:
    GlowConPoint( GlowCtx *glow_ctx, double x = 0, double y = 0, 
	int cp_num = 0, glow_eDirection d = glow_eDirection_Center) : 
	ctx(glow_ctx), number(cp_num), p(glow_ctx,x,y), direction(d),
	trace_attr_type(glow_eTraceType_Boolean)
	{ strcpy( trace_attribute, ""); };
    friend ostream& operator<< ( ostream& o, const GlowConPoint cp);
    void zoom();
    void nav_zoom();
    void print_zoom();
    void traverse( int x, int y);
    int	event_handler( void *pos, glow_eEvent event, int x, int y, void *node);
    void conpoint_select( void *pos, int x, int y, double *distance, 
		void **cp);
    void conpoint_select( GlowTransform *t, int x, int y, 
	double *distance, void **cp, int *pix_x, int *pix_y);
    void print( void *pos, void *node) {};
    void save( ofstream& fp, glow_eSaveMode mode);
    void open( ifstream& fp);
    void draw( void *pos, int highlight, int hot, void *node) {};
    void nav_draw( void *pos, int highlight, void *node) {};
    void erase( void *pos, int hot, void *node) {};
    void nav_erase( void *pos, void *node) {};
    void get_borders( double pos_x, double pos_y, double *x_right, 
	double *x_left, double *y_high, double *y_low, void *node) {};
    int get_conpoint( int num, double *x, double *y, glow_eDirection *dir);
    int get_conpoint( GlowTransform *t, int num, bool flip_horizontal, 
		      bool flip_vertical, double *x, double *y, glow_eDirection *dir);
    glow_eObjectType type() { return glow_eObjectType_ConPoint;};
    GlowCtx *ctx;
    int	number;
    GlowPoint p;
    glow_eDirection direction;
    char trace_attribute[32];
    glow_eTraceType trace_attr_type;
    GlowTransform trf;
};

#endif
