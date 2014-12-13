// export HALIDE_HOME=~/Dev/Halide
// g++ main.cpp kernels/openvx.cpp kernels/cv.cpp kernels/color_convert.cpp -g -I $HALIDE_HOME/include -L $HALIDE_HOME/bin -lHalide `libpng-config --cflags --ldflags` -lpthread -ldl -o test
// LD_LIBRARY_PATH=$HALIDE_HOME/bin ./test

#include <Halide.h>
#include <string>
#include "kernels/invert.h"
#include "kernels/openvx.h"
#include "kernels/cv.h"
#include "kernels/color_convert.h"
#include "utils/utils.h"

using Halide::Image;
#include "utils/image_io.h"

Halide::Func rotate(Halide::Func input, int height) {
    Halide::Func rot;
    Halide::Var x,y;

    rot(x,y) = input(y, (height - 1) - x);
}

void sobel_example() {
    Halide::Var x,y,c;
    // Image from: 
    Halide::Image<uint8_t> sobel_input = load<uint8_t>("images/bikesgray-wikipedia.png");
    Halide::Func sobel_padded;
    sobel_padded(x,y) = sobel_input(clamp(x, 0, sobel_input.width()-1),
                                    clamp(y, 0, sobel_input.height()-1));

    Halide::Image<uint8_t> sobel_output(sobel_input.width(), sobel_input.height());

    // calculate the horizontal and vertical gradients (GX, Gy)
    std::pair<Halide::Func, Halide::Func> sobel = sobel_3x3(sobel_padded, true);
    Halide::Func Gx, Gy;
    Gx(x,y) = AS_UINT8(sobel.first(x,y));
    Gx.realize(sobel_output);
    save(sobel_output, "output/sobel_gx.png");

    Gy(x,y) = AS_UINT8(sobel.second(x,y));
    Gy.realize(sobel_output);
    save(sobel_output, "output/sobel_gy.png");

    // Calculate gradient magnitudes and scale them to image intensity 
    // Described in http://patrick-fuller.com/gradients-image-processing-for-scientists-and-engineers-part-3/
    Halide::RDom r(sobel_input);
     
    Halide::Func maxpix, minpix, luminosity, mag, luminosity_fn_uint8, mag_uint8;

    // calculate the {min,max} luminosity values of the original grayscale image
    Halide::Image<int32_t> max_out, min_out;
    maxpix(x) = 0; minpix(x) = 0;
    maxpix(0) = max(sobel_input(r.x, r.y), maxpix(0));
    minpix(0) = min(sobel_input(r.x, r.y), minpix(0));
    max_out = maxpix.realize(1);
    min_out = minpix.realize(1);
    
    // calculate the magnitude of Gx, Gy
    mag = magnitude(sobel.first, sobel.second);
    // scale the magnitude by the luminosity range 
    luminosity(x,y) = 255.0f * ((mag(x,y)-min_out(0)) / (max_out(0)-min_out(0)));
    luminosity_fn_uint8(x,y) = AS_UINT8(luminosity(x,y));
    luminosity_fn_uint8.realize(sobel_output);
    save(sobel_output, "output/sobel_mag.png");

    printf("%s DONE\n", __func__);
}

const char* const TEST_OUTPUT = "output";
void openvx_example(Halide::Image<uint8_t> input) {
    Halide::Image<uint8_t> output(input.width(), input.height(), input.channels());
    Halide::Var x,y,c;

    Halide::Func padded;
    padded(x,y,c) = input(clamp(x, 0, input.width()-1),
                          clamp(y, 0, input.height()-1),
                          c);

    Halide::Func gaussian_3x3_fn = gaussian_3x3(padded);
    Halide::Func gaussian_3x3_fn_uint8;
    gaussian_3x3_fn_uint8(x,y,c) = AS_UINT8(gaussian_3x3_fn(x,y,c));
    gaussian_3x3_fn_uint8.realize(output);
    save(output, "output/gaussian_3x3.png");

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
}

void cv_example(Halide::Image<uint8_t> input) {
    Halide::Image<uint8_t> output(input.width(), input.height());
    Halide::Var x,y,c;

    Halide::Func padded;
    padded(x,y) = input(clamp(x, 0, input.width()-1), clamp(y, 0, input.height()-1));

    std::pair<Halide::Func, Halide::Func> scharr = scharr_3x3(padded, true);
    Halide::Func gx, gy;
    gx(x,y) = AS_UINT8(scharr.first(x,y));
    gy(x,y) = AS_UINT8(scharr.second(x,y));
    gx.realize(output);
    save(output, "output/scharr_gx.png");
    gy.realize(output);
    save(output, "output/scharr_gy.png");

    printf("%s DONE\n", __func__);
}

int main(int argc, char **argv) {

    Halide::Image<uint8_t> input = load<uint8_t>("images/rgb.png");
    Halide::RDom r(input);
    Halide::Func producer;
    Halide::Var x,y,c;
    producer(x,y,c) = input(x,y,c);

    Halide::Func invert_fn = invert<uint8_t>(producer, r);
    Halide::Image<uint8_t> output = invert_fn.realize(input.width(), input.height(), input.channels());
    save(output, "output/invert.png");

    openvx_example(input);

    Halide::Image<uint8_t> cv_input = load<uint8_t>("images/bikesgray-wikipedia.png");
    cv_example(cv_input);
    sobel_example();

    return 0;
}

