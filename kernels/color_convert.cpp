#include "color_convert.h"

// Given a 3-channel RGB input, return a function with a single
// channel - the luminance channel (grayscale)
// See http://en.wikipedia.org/wiki/YUV
Halide::Func rgb_extract_luma(Halide::Func rgb) {
    Halide::Var x,y,c;
    Halide::Func luma("luma");
    luma(x, y) = 0.299f * rgb(x, y, RED) + 
                 0.587f * rgb(x, y, GREEN) + 
                 0.114f * rgb(x, y, BLUE);
    return luma;
}

// Given an RGB input, return a function where all 3 channels 
// have the same luminance value
Halide::Func rgb2luma(Halide::Func rgb) {
    Halide::Var x,y,c;
    Halide::Func luma("luma");
    luma(x, y, c) = 0.299f * rgb(x, y, RED) + 
                    0.587f * rgb(x, y, GREEN) + 
                    0.114f * rgb(x, y, BLUE);
    return luma;
}
