#ifndef glow_transform_h
#define glow_transform_h

#include <iostream.h>
#include <fstream.h>

#include "glow.h"

class GlowTransform {
  public:
    GlowTransform() : a11(1),
	a12(0), a13(0), a21(0), a22(1), a23(0), rotation(0), s_a11(1),
	s_a12(0), s_a13(0), s_a21(0), s_a22(1), s_a23(0), s_rotation(0) {};
    GlowTransform operator* (const GlowTransform p);
    void scale( double sx, double sy, double x0, double y0);
    void rotate( double angel, double x0, double y0);
    void move( double x0, double y0);
    void posit( double x0, double y0);
    void reset() { a11=1; a12=0; a21=0; a22=1; a23=0; };
    double x( double x1, double y1);
    double y( double x1, double y1);
    double x( GlowTransform *t, double x1, double y1);
    double y( GlowTransform *t, double x1, double y1);
    bool reverse( double x, double y, double *rx, double *ry);
    void save( ofstream& fp, glow_eSaveMode mode);
    void open( ifstream& fp);
    double rot( GlowTransform *t) { return t->rotation + rotation;};
    double rot() { return rotation;};
    void store() { s_a11=a11;s_a12=a12;s_a13=a13;s_a21=a21;s_a22=a22;s_a23=a23;
	s_rotation=rotation;};
    void scale_from_stored( double sx, double sy, double x0, double y0);
    void rotate_from_stored( double angel, double x0, double y0);
    void move_from_stored( double x0, double y0);
    void set_from_stored( GlowTransform *t);
    double vertical_scale( GlowTransform *t);
    double a11;
    double a12;
    double a13;
    double a21;
    double a22;
    double a23;
    double rotation;
    double s_a11;
    double s_a12;
    double s_a13;
    double s_a21;
    double s_a22;
    double s_a23;
    double s_rotation;
};

#endif
