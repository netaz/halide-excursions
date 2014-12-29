#ifndef __OPENVX_H
#define __OPENVX_H

#include "Halide.h"
#include "sched_policy.h"
//
// OpenVX Kernels 
// For OpenVX specification, see: https://www.khronos.org/registry/vx/specs/1.0/html
//

enum interpolation_type {
    NEAREST_NEIGHBOR,
    AREA,
    BILINEAR
};

Halide::Func scale(interpolation_type interpolation);
std::pair<Halide::Func, Halide::Func> sobel_3x3(Halide::Func input, bool grayscale = false);
Halide::Func gaussian_3x3(Halide::Func input, bool grayscale = false, const Scheduler &s = NoPSched());
Halide::Func gaussian_5x5(Halide::Func input);
Halide::Func erode_3x3(Halide::Func input);
Halide::Func dilate_3x3(Halide::Func input);
Halide::Func box_3x3(Halide::Func input);
Halide::Func integral_image(Halide::Func input);

// Alternative implementations of Gaussian 3x3 kernel.  Not useful except for testing if 
// the algorithm implementation has bearings on the performance
Halide::Func gaussian_3x3_2(Halide::Func input, const Scheduler &s = NoPSched());
Halide::Func gaussian_3x3_3(Halide::Func input, const Scheduler &s = NoPSched());
Halide::Func gaussian_3x3_4(Halide::Func input, const Scheduler &s = NoPSched());
Halide::Func gaussian_3x3_5(Halide::Func input, const Scheduler &s = NoPSched());


//
// Color conversion functions
//

enum {
    RED   = 0,
    GREEN = 1,
    BLUE  = 2
};

Halide::Func rgb_extract_luma(Halide::Func rgb);
Halide::Func rgb2luma(Halide::Func rgb);

// Gradient direction
enum {
    DIRECTION_45UP,
    DIRECTION_45DOWN,
    DIRECTION_HORIZONTAL,
    DIRECTION_VERTICAL,
    DIRECTION_UNKNWON,
};

std::pair<Halide::Func, Halide::Func> scharr_3x3(Halide::Func input, bool grayscale = false);
std::pair<Halide::Func, Halide::Func> prewitt_3x3(Halide::Func input, bool grayscale = false);
Halide::Func gaussian_5x5_delta14(Halide::Func input, bool grayscale = false);
Halide::Func grad_magnitude(Halide::Func gx, Halide::Func gy);
Halide::Func grad_angle(Halide::Func Gx, Halide::Func Gy);
Halide::Func grad_direction(Halide::Func Gx, Halide::Func Gy);
Halide::Func canny_detector(Halide::Func input, bool grayscale = false);

Halide::Func fast_unsharp_mask(Halide::Func input, float gamma, float grayscale=false);
Halide::Func unsharp_mask(Halide::Func input, Halide::Func avg_mask, float gamma, bool grayscale=false);
Halide::Func bilinear_scale(Halide::Func input, float w_factor, float h_factor);
Halide::Func nn_scale(Halide::Func input, float w_factor, float h_factor);
Halide::Func reflect_vert(Halide::Func input, int k, int width);

/*
 * Invert input over the specified reduction domain (r)
 * 
 */
template <typename TIMAGE>
Halide::Func invert(Halide::Func input, Halide::RDom r) {
    Halide::Var x,y,c;
    Halide::Func invert("invert");

    Halide::Func img_max;
    img_max(c) = Halide::cast<TIMAGE>(0);
    img_max(c) = max(input(r.x, r.y, c), img_max(c));
    
    Halide::Image<TIMAGE> max_val = img_max.realize(1);

    // TODO: What can I do with this???
    // http://halide-lang.org/docs/_inline_reductions_8h.html
    // Halide::Expr e = Halide::maximum(r, input(r.x, r.y, c));
    // int16_t fff = cast<int16_t>(e[c])
        
    invert(x,y,c) = max_val(0) - input(x,y,c);
    return invert;
}


/*
https://staff.fnwi.uva.nl/r.vandenboomgaard/multimedia/labRotation.pdf

function r = pv( im, x, y, method )
% get a pixel value using interpolation
[M,N] = size(im);
if (x>=1) && (x<=M) && (y>=1) && (y<=N)
% OK point (x,y) is within image
if method==’linear’
% do the bilinear interpolation
else
% do the nearest neighbor interpolation
end
else
% outside the image --> return a sensible value
r = 0;
end

x = x 0 cos(φ) + y 0 sin(φ) 
y = − x 0 sin(φ) + y 0 cos(φ)

Note than angles in Matlab have to be given in radians
*/


#endif // __OPENVX_H