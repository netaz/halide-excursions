#ifndef __CV_H
#define __CV_H

#include "Halide.h"

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

#endif // __CV_H