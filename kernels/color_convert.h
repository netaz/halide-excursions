#ifndef __COLOR_CONVERT_H
#define __COLOR_CONVERT_H

#include "Halide.h"

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
#endif // __COLOR_CONVERT_H