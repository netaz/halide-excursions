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


#endif // __OPENVX_H