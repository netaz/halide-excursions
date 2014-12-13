#include "cv.h"
#include "openvx.h"

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

// Prewitt operator
// Computes x,y gradients
// http://en.wikipedia.org/wiki/Prewitt_operator
std::pair<Halide::Func, Halide::Func> prewitt_3x3(Halide::Func input, bool grayscale) {
    Halide::Func kx("kx"), ky("ky");
    Halide::Func gx("gradient_x"), gy("gradient_y");
    Halide::RDom r(-1,3,-1,3);
    Halide::Var x,y,c;
    
    kx(x,y) = 0;
    kx(-1,-1) = -1;    kx(0,-1) = 0;    kx(1,-1) = 1;
    kx(-1, 0) = -1;    kx(0, 0) = 0;    kx(1, 0) = 1;
    kx(-1, 1) = -1;    kx(0, 1) = 0;    kx(1, 1) = 1;
    if (grayscale)
        gx(x,y) = sum(input(x+r.x, y+r.y) * kx(r.x, r.y));
    else
        gx(x,y,c) = sum(input(x+r.x, y+r.y, c) * kx(r.x, r.y));
        
    ky(x,y) = 0;
    ky(-1,-1) = -1;    ky(0,-1) = -1;    ky(1,-1) = -1;
    ky(-1, 0) =  0;    ky(0, 0) =  0;    ky(1, 0) =  0;
    ky(-1, 1) =  1;    ky(0, 1) =  1;    ky(1, 1) =  1;
    if (grayscale)
        gy(x,y) = sum(input(x+r.x, y+r.y) * ky(r.x, r.y));
    else
        gy(x,y,c) = sum(input(x+r.x, y+r.y, c) * ky(r.x, r.y));

    std::pair<Halide::Func, Halide::Func> ret = std::make_pair(gx, gy);
    return ret;
}

// compute gradient magnitude
Halide::Func grad_magnitude(Halide::Func Gx, Halide::Func Gy) {
    Halide::Func mag("grad_magnitude");
    Halide::Var x,y,c;
    mag(x,y) =  Halide::sqrt( 
                     Halide::pow(Gx(x,y), 2) + Halide::pow(Gy(x,y), 2)
                );
    return mag;
}

// compute gradient angle (radians)
// angle = arctan(Gy(x,y) / Gx(x,y))
// More precisely defined in: http://en.wikipedia.org/wiki/Atan2 
Halide::Func grad_angle(Halide::Func Gx, Halide::Func Gy) {
    Halide::Func angle("angle");
    Halide::Var x,y,c;
    angle(x,y) =  Halide::atan2(Gy(x,y), Gx(x,y));
    return angle;
}

// Based on: http://blog.pkh.me/p/14-fun-and-canny-optim-for-a-canny-edge-detector.html
Halide::Func grad_direction(Halide::Func Gx, Halide::Func Gy)
{
    Halide::Var x,y;
    Halide::Func angle = grad_angle(Gx, Gy);
    Halide::Expr theta = angle(x,y);
    Halide::Expr gx = Gx(x,y);
    Halide::Expr gy = Gy(x,y);
    Halide::Func dir("grad_direction");
 
    const float PI38 =(3*M_PI/8);
    const float PI8 = (M_PI/8);
    dir(x,y) = 
            select ( gx !=0, 
                select( (theta < PI38) && (theta > PI8), 
                    select( ((gx > 0 && gy > 0) || (gx < 0 && gy < 0)), DIRECTION_45DOWN, DIRECTION_45UP),
                    select(theta < PI8, DIRECTION_HORIZONTAL, DIRECTION_VERTICAL)    
                ),
                DIRECTION_VERTICAL
            );
    return dir;
}


// Gaussian 5x5 filter; with delta=1.4
// Used by Canny edge detector
// http://en.wikipedia.org/wiki/Canny_edge_detector
Halide::Func gaussian_5x5_delta14(Halide::Func input, bool grayscale) {
    Halide::Func k, gaussian("gaussian_5x5_delta14");
    Halide::RDom r(-2,5,-2,5);
    Halide::Var x,y,c;

    k(x,y) = 0;
    k(-2,-2) = 2;    k(-1,-2) =  4;   k(0,-2) =  5;   k(1,-2) =  4;   k(2,-2) = 2;
    k(-2,-1) = 4;    k(-1,-1) =  9;   k(0,-1) = 12;   k(1,-1) =  9;   k(2,-1) = 4;
    k(-2, 0) = 5;    k(-1, 0) = 12;   k(0, 0) = 15;   k(1, 0) = 12;   k(2, 0) = 5;
    k(-2, 1) = 4;    k(-1, 1) =  9;   k(0, 1) = 12;   k(1, 1) =  9;   k(2, 1) = 4;
    k(-2, 2) = 2;    k(-1, 2) =  4;   k(0, 2) =  5;   k(1, 2) =  4;   k(2, 2) = 2;

    if (grayscale) {
        gaussian(x,y) = sum(input(x+r.x, y+r.y) * k(r.x, r.y));
        gaussian(x,y) /= 159;
    } else {
        gaussian(x,y,c) = sum(input(x+r.x, y+r.y, c) * k(r.x, r.y));
        gaussian(x,y,c) /= 159;
    }

    return gaussian;
}


// Canny edge detector
// http://docs.opencv.org/doc/tutorials/imgproc/imgtrans/canny_detector/canny_detector.html
// http://dasl.mem.drexel.edu/alumni/bGreen/www.pages.drexel.edu/_weg22/can_tut.html
Halide::Func canny_detector(Halide::Func input, bool grayscale) {
    // 1. noise reduction
    Halide::Func blur = gaussian_5x5_delta14(input, grayscale);
    // 2a. gradient calculation
    std::pair<Halide::Func, Halide::Func> gradients = sobel_3x3(blur, grayscale);
    // 2b. gradient magnitude and direction
    Halide::Func mag = grad_magnitude(gradients.first, gradients.second);
    Halide::Func dir = grad_direction(gradients.first, gradients.second);

    // 3. non-maximum suppression
    // 4. hysteresis
    return mag;
}