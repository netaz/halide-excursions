#include "openvx.h"

// Per OpenVX
Halide::Func scale(interpolation_type interpolation) {
    Halide::Func scale;
    return scale;
}

// Per OpenVX
// Implements the Sobel Image Filter k.
// This k produces two output planes (one can be omitted) in the x and y plane.
// The Sobel Operators Gx,Gy are defined as:
//     
//          -1  0  1            -1 -2 -1
//     Gx = -2  0  2       Gy =  0  0  0
//           1  0  1             1  2  1
//
// https://www.khronos.org/registry/vx/specs/1.0/html/da/d4b/group__group__vision__function__sobel3x3.html
//
std::pair<Halide::Func, Halide::Func> sobel_3x3(Halide::Func input, bool grayscale) {
    Halide::Func kx("kx"), ky("ky");
    Halide::Func gradient_x("gradient_x"), gradient_y("gradient_y");
    Halide::RDom r(-1,3,-1,3);
    Halide::Var x,y,c;
    
    kx(x,y) = 0;
    kx(-1,-1) = -1;    kx(0,-1) = 0;    kx(1,-1) = 1;
    kx(-1, 0) = -2;    kx(0, 0) = 0;    kx(1, 0) = 2;
    kx(-1, 1) = -1;    kx(0, 1) = 0;    kx(1, 1) = 1;
    if (grayscale)
        gradient_x(x,y) = sum(input(x+r.x, y+r.y) * kx(r.x, r.y));
    else
        gradient_x(x,y,c) = sum(input(x+r.x, y+r.y, c) * kx(r.x, r.y));
        
    ky(x,y) = 0;
    ky(-1,-1) = -1;    ky(0,-1) = -2;    ky(1,-1) = -1;
    ky(-1, 0) =  0;    ky(0, 0) =  0;    ky(1, 0) =  0;
    ky(-1, 1) =  1;    ky(0, 1) =  2;    ky(1, 1) =  1;
    if (grayscale)
        gradient_y(x,y) = sum(input(x+r.x, y+r.y) * ky(r.x, r.y));
    else
        gradient_y(x,y,c) = sum(input(x+r.x, y+r.y, c) * ky(r.x, r.y));

    std::pair<Halide::Func, Halide::Func> ret = std::make_pair(gradient_x, gradient_y);
    return ret;
}

// Per OpenVX
// Computes a Gaussian filter over a window of the input image.
// This filter uses the following convolution matrix:
//         1  2  1
//     K = 2  4  2    *  1/16
//         1  2  1
//
// https://www.khronos.org/registry/vx/specs/1.0/html/d6/d58/group__group__vision__function__gaussian__image.html
Halide::Func gaussian_3x3(Halide::Func input) {
    Halide::Func k, gaussian;
    Halide::RDom r(-1,3,-1,3);
    Halide::Var x,y,c;
    
    k(x,y) = 0;
    k(-1,-1) = 1;    k(0,-1) = 2;    k(1,-1) = 1;
    k(-1, 0) = 2;    k(0, 0) = 4;    k(1, 0) = 2;
    k(-1, 1) = 1;    k(0, 1) = 2;    k(1, 1) = 1;

    gaussian(x,y,c) = sum(input(x+r.x, y+r.y, c) * k(r.x, r.y));
    gaussian(x,y,c) /= 16;

    return gaussian;
}

// Per OpenVX
// https://www.khronos.org/registry/vx/specs/1.0/html/d0/d15/group__group__vision__function__gaussian__pyramid.html
Halide::Func gaussian_5x5(Halide::Func input) {
    Halide::Func k, gaussian;
    Halide::RDom r(-2,5,-2,5);
    Halide::Var x,y,c;

    k(x,y) = 0;
    k(-2,-2) = 1;    k(-1,-2) =  4;   k(0,-2) =  6;   k(1,-2) =  4;   k(2,-2) = 1;
    k(-2,-1) = 4;    k(-1,-1) = 16;   k(0,-1) = 24;   k(1,-1) = 16;   k(2,-1) = 4;
    k(-2, 0) = 6;    k(-1, 0) = 24;   k(0, 0) = 36;   k(1, 0) = 24;   k(2, 0) = 6;
    k(-2, 1) = 4;    k(-1, 1) = 16;   k(0, 1) = 24;   k(1, 1) = 16;   k(2, 1) = 4;
    k(-2, 2) = 1;    k(-1, 2) =  4;   k(0, 2) =  6;   k(1, 2) =  4;   k(2, 2) = 1;

    gaussian(x,y,c) = sum(input(x+r.x, y+r.y, c) * k(r.x, r.y));
    gaussian(x,y,c) /= 256;

    return gaussian;
}


// Per OpenVX 
// Implements Erosion, which shrinks the white space in an image.
// This k uses a 3x3 box around the output pixel used to determine value.
// https://www.khronos.org/registry/vx/specs/1.0/html/dc/dff/group__group__vision__function__erode__image.html
Halide::Func erode_3x3(Halide::Func input) {
    Halide::Func erode("erode");
    Halide::RDom r(-1,3,-1,3);
    Halide::Var x,y,c;

    erode(x,y,c) = Halide::minimum(input(x+r.x, y+r.y, c));
    return erode;
}

// Per OpenVX 
// Implements Dilation, which grows the white space in an image.
// This k uses a 3x3 box around the output pixel used to determine value.
// https://www.khronos.org/registry/vx/specs/1.0/html/dc/d73/group__group__vision__function__dilate__image.html
Halide::Func dilate_3x3(Halide::Func input) {
    Halide::Func dilate("dilate");
    Halide::RDom r(-1,3,-1,3);
    Halide::Var x,y,c;

    dilate(x,y,c) = Halide::maximum(input(x+r.x, y+r.y, c));
    return dilate;
}

// Per OpenVX 
// Computes a Box filter over a window of the input image.
// https://www.khronos.org/registry/vx/specs/1.0/html/da/d7c/group__group__vision__function__box__image.html
Halide::Func box_3x3(Halide::Func input) {
    Halide::Func box("box");
    Halide::RDom r(-1,3,-1,3);
    Halide::Var x,y,c;

    box(x,y,c) = Halide::sum(input(x+r.x, y+r.y, c));
    box(x,y,c) /= 9;
    return box;
}

// Per OpenVX
// https://www.khronos.org/registry/vx/specs/1.0/html/d0/d7b/group__group__vision__function__integral__image.html
// TODO: this is currently incorrect
Halide::Func integral_image(Halide::Func input) {
    Halide::Func integral("integral");
    Halide::RDom r(-1,2,-1,2);
    Halide::Var x,y,c;

    integral(x,y,c) = 0;
    integral(-1,y,c) = 0;
    integral(x,-1,c) = 0;
    //integral(x,y,c) = Halide::sum(Halide::cast<int32_t>(input(x+r.x, y+r.y, c)));
    //integral(x,y,c) = integral(x,y,c) + integral(x-1,y,c) + integral(x,y-1,c) + integral(x-1,y-1,c);
    Halide::Expr e[] = { integral(x,y,c), integral(x-1,y,c), integral(x,y-1,c), integral(x-1,y-1,c) };
    integral(x,y,c) = e[0] + e[1] + e[2] + e[3];
    return integral;
}

