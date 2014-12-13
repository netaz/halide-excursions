#ifndef __OPENVX_H
#define __OPENVX_H

#include "Halide.h"

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
Halide::Func gaussian_3x3(Halide::Func input, bool grayscale = false);
Halide::Func gaussian_5x5(Halide::Func input);
Halide::Func erode_3x3(Halide::Func input);
Halide::Func dilate_3x3(Halide::Func input);
Halide::Func box_3x3(Halide::Func input);
Halide::Func integral_image(Halide::Func input);

#endif // __OPENVX_H