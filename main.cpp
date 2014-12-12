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

bool random_musings() {
    Halide::Var x,y;
    const int WIDTH = 10;
    const int LENGTH = 10;

    Image<int32_t> input(WIDTH, LENGTH);
    randomize(input);
    dump_test_img(input);

    Halide::Image<int32_t> output;
    
    Halide::Func hist("hist");
    Halide::RDom r(input);
    hist(x) = 0;
    hist(clamp(input(r.x, r.y)/4, 0, 24)) += 1;
    
    output = hist.realize(25);
    dump_test_img(output);

    Halide::Func maxxx("maxxx");
    maxxx(x) = 0;
    maxxx(0) = max(input(r.x, r.y), maxxx(0));
    output = maxxx.realize(1);
    printf("%d\n", output(0));

    assert(output(0) == verify_max(input));
    return true;
}

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

    std::pair<Halide::Func, Halide::Func> sobel = sobel_3x3(sobel_padded, true);
    Halide::Func gx, gy;
    gx(x,y) = Halide::cast<uint8_t>(sobel.first(x,y));
    gx.realize(sobel_output);
    save(sobel_output, "output/sobel_gx.png");

    gy(x,y) = Halide::cast<uint8_t>(sobel.second(x,y));
    gy.realize(sobel_output);
    save(sobel_output, "output/sobel_gy.png");

    Halide::RDom r(sobel_input);
    // Halide::Expr e = Halide::maximum(r, sobel_input(r.x, r.y));
    // int16_t max_input = Halide::cast<int16_t>(e(0,0));

    // Follow algo described in http://patrick-fuller.com/gradients-image-processing-for-scientists-and-engineers-part-3/
    Halide::Func maxpix, minpix, luminosity, mag, luminosity_fn_uint8, mag_uint8;
    Halide::Image<int32_t> max_out, min_out;
    maxpix(x) = 0;
    minpix(x) = 0;
    maxpix(0) = max(sobel_input(r.x, r.y), maxpix(0));
    minpix(0) = min(sobel_input(r.x, r.y), minpix(0));
    max_out = maxpix.realize(1);
    min_out = minpix.realize(1);
    printf("%d %d\n", max_out(0), min_out(0));
    mag = magnitude(sobel.first, sobel.second);
    //mag_uint8(x,y) = Halide::cast<uint8_t>(mag(x,y));
    luminosity(x,y) = 255.0f * ((mag(x,y)-min_out(0)) / (max_out(0)-min_out(0)));
    luminosity_fn_uint8(x,y) = Halide::cast<uint8_t>(luminosity(x,y));
    luminosity_fn_uint8.realize(sobel_output);
    save(sobel_output, "output/sobel_mag.png");
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
    gaussian_3x3_fn_uint8(x,y,c) = Halide::cast<uint8_t>(gaussian_3x3_fn(x,y,c));
    gaussian_3x3_fn_uint8.realize(output);
    save(output, "output/gaussian_3x3.png");
#if 0
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
    gaussian_5x5_fn_uint8(x,y,c) = Halide::cast<uint8_t>(gaussian_5x5_fn(x,y,c));
    gaussian_5x5_fn_uint8.realize(output);
    save(output, "output/gaussian_5x5.png");
#endif 

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
    luma_fn_uint8(x,y,c) = Halide::cast<uint8_t>(luma(x,y,c));
    luma_fn_uint8.realize(output);
    save(output, "output/luma.png");
    
    printf("openvx_example DONE\n");
}

void cv_example(Halide::Image<uint8_t> input) {
    Halide::Image<uint8_t> output(input.width(), input.height());
    Halide::Var x,y,c;

    Halide::Func padded;
    padded(x,y) = input(clamp(x, 0, input.width()-1), clamp(y, 0, input.height()-1));

    std::pair<Halide::Func, Halide::Func> scharr = scharr_3x3(padded, true);
    Halide::Func gx, gy;
    gx(x,y) = Halide::cast<uint8_t>(scharr.first(x,y));
    gy(x,y) = Halide::cast<uint8_t>(scharr.second(x,y));
    gx.realize(output);
    save(output, "output/scharr_gx.png");
    gy.realize(output);
    save(output, "output/scharr_gy.png");
}

int main(int argc, char **argv) {

    random_musings();

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

    return 0;
}

