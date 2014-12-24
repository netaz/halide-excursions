#include <Halide.h>
#include <string>
#include "kernels/invert.h"
#include "kernels/openvx.h"
#include "kernels/cv.h"
#include "kernels/color_convert.h"
#include "utils/utils.h"

using Halide::Image;
#include "utils/image_io.h"

int scale_example(int argc, const char **argv) {
  	Halide::Image<uint8_t> input = load<uint8_t>(argv[0]);
    Halide::RDom r(input);
    Halide::Var x,y,c;
    Halide::Func padded("padded");
    padded(x,y,c) = input(clamp(x, 0, input.width()-1), clamp(y, 0, input.height()-1), c);

    Halide::Func invert_fn = invert<uint8_t>(padded, r);
    Halide::Image<uint8_t> output = invert_fn.realize(input.width(), input.height(), input.channels());
    save(output, "output/invert.png");

    Halide::Func reflect = reflect_vert(padded, 500, input.width());
    reflect.realize(output);
    save(output, "output/reflect.png");

    Halide::Func reflect2 = reflect_vert(padded, 200, input.width());
    reflect2.realize(output);
    save(output, "output/reflect2.png");

    float w_scale = 2, h_scale = 2;
    Halide::Func nn_scale_up = nn_scale(padded, 1/w_scale, 1/h_scale);
    Halide::Image<uint8_t> output_scaled_up = nn_scale_up.realize(input.width()*w_scale, input.height()*h_scale, input.channels());
    save(output_scaled_up, "output/nn_scale_up.png");

    w_scale = 0.5f, h_scale = 0.5f;
    Halide::Func nn_scale_down = nn_scale(padded, 1/w_scale, 1/h_scale);
    Halide::Image<uint8_t> output_scaled_down = nn_scale_down.realize(input.width()*w_scale, input.height()*h_scale, input.channels());
    save(output_scaled_down, "output/nn_scale_down.png");

    Halide::Func bilinear_scale_down = bilinear_scale(padded, 1/w_scale, 1/h_scale);
    bilinear_scale_down.realize(output_scaled_down);
    save(output_scaled_down, "output/bilinear_scale_down.png");   

    w_scale = 2, h_scale = 2;
    Halide::Func bilinear_scale_up = bilinear_scale(padded, 1/w_scale, 1/h_scale);
    bilinear_scale_up.realize(output_scaled_up);
    save(output_scaled_up, "output/bilinear_scale_up.png");   

    float gamma = 5.0f;
    Halide::Func fast_unsharp = fast_unsharp_mask(padded, gamma);
    TO_3D_UINT8_LAMBDA(fast_unsharp).realize(output);
    save(output, "output/fast_unsharp.png");       

    Halide::Image<uint8_t> gray_output(input.width(), input.height());
    Halide::Func luma("lluma");
    luma = rgb_extract_luma(padded);
    Halide::Func unsharp = unsharp_mask(luma, gaussian_3x3(luma, true), gamma, true);
    TO_2D_UINT8_LAMBDA(unsharp).realize(gray_output);
    save(gray_output, "output/unsharp.png");       

    printf("%s DONE\n", __func__);
    return EXIT_SUCCESS;
}
