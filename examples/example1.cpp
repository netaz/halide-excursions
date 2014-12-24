#include <Halide.h>
#include <string>
#include "kernels/invert.h"
#include "kernels/openvx.h"
#include "kernels/cv.h"
#include "kernels/color_convert.h"
#include "utils/utils.h"

using Halide::Image;
#include "utils/image_io.h"

int openvx_example(int argc, const char **argv) {
    Halide::Image<uint8_t> input = load<uint8_t>(argv[0]);

    Halide::Image<uint8_t> output(input.width(), input.height(), input.channels());
    Halide::Image<uint8_t> output2(input.width(), input.height(), input.channels());
    Halide::Var x,y,c;

    Halide::Func padded, padded32;
    padded(x,y,c) = input(clamp(x, 0, input.width()-1),
                          clamp(y, 0, input.height()-1),
                          c);

    padded32(x,y,c) = Halide::cast<int32_t>(padded(x,y,c));
    Halide::Func gaussian_3x3_fn = gaussian_3x3(padded32);
    Halide::Func gaussian_3x3_fn_uint8;
    gaussian_3x3_fn_uint8(x,y,c) = AS_UINT8(gaussian_3x3_fn(x,y,c));
    gaussian_3x3_fn_uint8.realize(output);
    save(output, "output/gaussian_3x3.png");

    TO_3D_UINT8_LAMBDA(gaussian_3x3_2(padded32)).realize(output2);
    save(output2, "output/gaussian_3x3_2.png");
    //assert(output == output2);
    TO_3D_UINT8_LAMBDA(gaussian_3x3_3(padded32)).realize(output);
    save(output, "output/gaussian_3x3_3.png");
    TO_3D_UINT8_LAMBDA(gaussian_3x3_5(padded)).realize(output);
    save(output, "output/gaussian_3x3_5.png");

    Halide::Func erode_3x3_fn = erode_3x3(padded);
    erode_3x3_fn.realize(output);
    save(output, "output/erode_3x3.png");

    Halide::Func dilate_3x3_fn = dilate_3x3(padded);
    dilate_3x3_fn.realize(output);
    save(output, "output/dilate_3x3.png");

    Halide::Func box_3x3_fn = dilate_3x3(padded);
    box_3x3_fn.realize(output);
    save(output, "output/box_3x3.png");

    Halide::Func gaussian_5x5_fn = gaussian_5x5(padded);
    Halide::Func gaussian_5x5_fn_uint8;
    gaussian_5x5_fn_uint8(x,y,c) = AS_UINT8(gaussian_5x5_fn(x,y,c));
    gaussian_5x5_fn_uint8.realize(output);
    save(output, "output/gaussian_5x5.png");

#if 0
    Halide::Func integral_fn = integral_image(padded);
    Halide::Func integral_fn_uint8;
    integral_fn_uint8(x,y,c) = Halide::cast<uint8_t>(integral_fn(x,y,c));
    integral_fn_uint8.realize(output);
    save(output, "output/integral.png");
#endif    

    Halide::Func luma;
    luma = rgb2luma(padded);
    Halide::Func luma_fn_uint8;
    luma_fn_uint8(x,y,c) = AS_UINT8(luma(x,y,c));
    luma_fn_uint8.realize(output);
    save(output, "output/luma.png");
    
    printf("%s DONE\n", __func__);
    return EXIT_SUCCESS;
}
