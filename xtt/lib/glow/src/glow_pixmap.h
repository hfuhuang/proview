#ifndef glow_pixmap_h
#define glow_pixmap_h

#include <iostream.h>
#include "glow.h"
#include "glow_ctx.h"
#include "glow_point.h"
#include "glow_array_elem.h"


class GlowPixmap : public GlowArrayElem {
  public:
    GlowPixmap( GlowCtx *glow_ctx, glow_sPixmapData *pixmap_data,
	 double x = 0, double y = 0,
	glow_eDrawType d_type = glow_eDrawType_Line, int size = 2);
    GlowPixmap( const GlowPixmap& p);
    ~GlowPixmap();
    friend ostream& operator<< ( ostream& o, const GlowPixmap t);
    void zoom();
    void nav_zoom();
    void print_zoom();
    void traverse( int x, int y);
    int	event_handler( void *pos, glow_eEvent event, int x, int y, void *node);
    void conpoint_select( void *pos, int x, int y, double *distance, 
		void **cp) {};
    void print( void *pos, void *node);
    void save( ofstream& fp, glow_eSaveMode mode);
    void open( ifstream& fp);
    void draw( void *pos, int highlight, int hot, void *node);
    void nav_draw( void *pos, int highlight, void *node);
    void draw_inverse( void *pos, int hot, void *node);
    void erase( void *pos, int hot, void *node);
    void nav_erase( void *pos, void *node);
    void move( void *pos, double x, double y, int highlight, int hot);
    void shift( void *pos, double delta_x, double delta_y,
	int highlight, int hot);
    void get_borders( double pos_x, double pos_y, double *x_right, 
		double *x_left, double *y_high, double *y_low, void *node);
    int get_conpoint( int num, double *x, double *y, glow_eDirection *dir) 
		{ return 0;};
    glow_eObjectType type() { return glow_eObjectType_Pixmap;};
    GlowCtx *ctx;
    GlowPoint p;
    void *pixmaps;
    glow_sPixmapData pixmap_data;
    int pixmap_size;
    glow_eDrawType draw_type;
};

#endif
