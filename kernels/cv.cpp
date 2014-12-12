#include "cv.h"

// Scharr operator
// Computes x,y gradients
// http://patrick-fuller.com/gradients-image-processing-for-scientists-and-engineers-part-3/
std::pair<Halide::Func, Halide::Func> scharr_3x3(Halide::Func input, bool grayscale) {
    Halide::Func kx("kx"), ky("ky");
    Halide::Func gx("gradient_x"), gy("gradient_y");
    Halide::RDom r(-1,3,-1,3);
    Halide::Var x,y,c;
    
    kx(x,y) = 0;
    kx(-1,-1) =  -3;    kx(0,-1) = 0;    kx(1,-1) =  3;
    kx(-1, 0) = -10;    kx(0, 0) = 0;    kx(1, 0) = 10;
    kx(-1, 1) =  -3;    kx(0, 1) = 0;    kx(1, 1) =  3;
    if (grayscale)
        gx(x,y) = sum(input(x+r.x, y+r.y) * kx(r.x, r.y));
    else
        gx(x,y,c) = sum(input(x+r.x, y+r.y, c) * kx(r.x, r.y));
        
    ky(x,y) = 0;
    ky(-1,-1) = -3;    ky(0,-1) = -10;    ky(1,-1) = -3;
    ky(-1, 0) =  0;    ky(0, 0) =   0;    ky(1, 0) =  0;
    ky(-1, 1) =  3;    ky(0, 1) =  10;    ky(1, 1) =  3;
    if (grayscale)
        gy(x,y) = sum(input(x+r.x, y+r.y) * ky(r.x, r.y));
    else
        gy(x,y,c) = sum(input(x+r.x, y+r.y, c) * ky(r.x, r.y));

    std::pair<Halide::Func, Halide::Func> ret = std::make_pair(gx, gy);
    return ret;
}

Halide::Func magnitude(Halide::Func gx, Halide::Func gy) {
    Halide::Func mag;
    Halide::Var x,y,c;
    mag(x,y) =  Halide::sqrt( 
                     Halide::pow(gx(x,y), 2) + Halide::pow(gy(x,y), 2)
                );
    return mag;
}