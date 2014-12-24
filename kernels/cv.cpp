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


// -------------------------------------------------------------------------------------------------------------
// one-sides (vertical) reflection across the line x=k
Halide::Func reflect_vert(Halide::Func input, int k, int width) {
    Halide::Var x,y,c;
    Halide::Func reflect("reflect");

    if (k>width/2) 
        reflect(x,y,c) = select(x<k, input(x,y,c), input(2*k-x, y, c));
    else
        reflect(x,y,c) = select(x>k, input(x,y,c), input(2*k-x, y, c));
    return reflect;
}

//Halide::Func nn_interpolation()
// https://www.khronos.org/registry/vx/specs/1.0/html/d1/d26/group__group__vision__function__scale__image.html
Halide::Func nn_scale(Halide::Func input, float w_factor, float h_factor) {
    Halide::Func scale("scale");
    Halide::Var x,y,c;
    //scale(x,y,c) = input(Halide::cast<int>(x*w_factor), Halide::cast<int>(y*h_factor),c);
    scale(x,y,c) = input(Halide::cast<int>(((x+0.5f)*w_factor)-0.5f), Halide::cast<int>(((y+0.5f)*h_factor)-0.5f),c);
    return scale;
}

// https://www.khronos.org/registry/vx/specs/1.0/html/d1/d26/group__group__vision__function__scale__image.html
Halide::Func bilinear_scale(Halide::Func input, float w_factor, float h_factor) {
    Halide::Func scale("scale");
    Halide::Var x,y,c;
 
    Halide::Expr x_lower = Halide::cast<int>(x * w_factor);
    Halide::Expr y_lower = Halide::cast<int>(y * h_factor);
    Halide::Expr s = (x * w_factor) - x_lower;
    Halide::Expr t = (y * h_factor) - y_lower;

    scale(x,y,c) =  Halide::cast<uint8_t>(
                    (1-s) * (1-t) * input(x_lower, y_lower+1, c)  +
                    s * (1-t)     * input(x_lower+1, y_lower, c)  +
                    (1-s) * t     * input(x_lower, y_lower+1, c)  + 
                    s * t         * input(x_lower+1, y_lower+1, c)
                    );

    return scale;
}

// average_mask == low frequency component
// (input - average_mask) == high frequency component
// output = gamma * (input - average_mask) + average_mask
//        = gamma * input + (1-gamma) * average_mask
// if gamma>1, then high-frequency component is emphasized
// Handbook of Computer Vision Algorithms in Image Algebra, 2nd Ed - Gerhard X. Ritter, section 2.10
Halide::Func unsharp_mask(Halide::Func input, Halide::Func avg_mask, float gamma, bool grayscale) {
    Halide::Func output("unsharp_mask_output");
    Halide::Var x,y,c;

    if (grayscale)
        output(x,y) = gamma * input(x,y) + (1.0f-gamma) * avg_mask(x,y);
    else
        output(x,y,c) = gamma * input(x,y,c) + (1.0f-gamma) * avg_mask(x,y,c);
    return output;
}

// This implementation uses an equal weight mean function for the average mask and the unsharp 
// mask is applied in one go using a convolution with a 3x3 neighborhood
Halide::Func fast_unsharp_mask(Halide::Func input, float gamma, float grayscale) {
    Halide::Func output("output"),f("convolution");
    Halide::RDom r(-1,3,-1,3);
    Halide::Var x,y,c;

    Halide::Expr v = (1.0f-gamma) / 9.0f;
    Halide::Expr w = (8.0f * gamma + 1.0f) / 9.0f;

    f(x,y) = 0.0f;
    f(-1,-1) = v;    f(0,-1) = v;    f(1,-1) = v;
    f(-1, 0) = v;    f(0, 0) = w;    f(1, 0) = v;
    f(-1, 1) = v;    f(0, 1) = v;    f(1, 1) = v;

    if (grayscale)
        output(x,y) = sum(input(x+r.x, y+r.y) * f(r.x, r.y));
    else
        output(x,y,c) = sum(input(x+r.x, y+r.y, c) * f(r.x, r.y));
    return output;
}
