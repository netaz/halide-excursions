#ifndef __CV_H
#define __CV_H

#include "Halide.h"

std::pair<Halide::Func, Halide::Func> scharr_3x3(Halide::Func input, bool grayscale = false);
std::pair<Halide::Func, Halide::Func> prewitt_3x3(Halide::Func input, bool grayscale = false);
Halide::Func magnitude(Halide::Func gx, Halide::Func gy);

#endif // __CV_H