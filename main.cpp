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
    Halide::Image<uint8_t> input = load<uint8_t>("images/bikesgray-wikipedia.png");
    Halide::Func padded;
    padded(x,y) = input(clamp(x, 0, input.width()-1), clamp(y, 0, input.height()-1));

    Halide::Image<uint8_t> sobel_output(input.width(), input.height());

    // smooth the image
    // Halide::Func gaussian = gaussian_3x3(padded, true);

    // calculate the horizontal and vertical gradients (GX, Gy)
    std::pair<Halide::Func, Halide::Func> sobel = sobel_3x3(padded, true);
    Halide::Func Gx, Gy;
    Gx(x,y) = AS_UINT8(sobel.first(x,y));
    Gx.realize(sobel_output);
    save(sobel_output, "output/sobel_gx.png");

    Gy(x,y) = AS_UINT8(sobel.second(x,y));
    Gy.realize(sobel_output);
    save(sobel_output, "output/sobel_gy.png");

    // Calculate gradient magnitudes and scale them to image intensity 
    // Described in http://patrick-fuller.com/gradients-image-processing-for-scientists-and-engineers-part-3/
    Halide::RDom r(input);
     
    Halide::Func maxpix, minpix, luminosity, mag, mag_uint8;

    // calculate the {min,max} luminosity values of the original grayscale image
    Halide::Image<int32_t> max_out, min_out;
    maxpix(x) = 0; minpix(x) = 0;
    maxpix(0) = max(input(r.x, r.y), maxpix(0));
    minpix(0) = min(input(r.x, r.y), minpix(0));
    max_out = maxpix.realize(1);
    min_out = minpix.realize(1);

    // calculate the magnitude of Gx, Gy
    mag = grad_magnitude(sobel.first, sobel.second);
    // scale the magnitude by the luminosity range 
    luminosity(x,y) = 255.0f * ((mag(x,y)-min_out(0)) / (max_out(0)-min_out(0)));
    TO_2D_UINT8_LAMBDA(luminosity).realize(sobel_output);
    save(sobel_output, "output/sobel_mag.png");

    printf("%s DONE\n", __func__);
}

void prewitt_example() {
    Halide::Var x,y,c;
    Halide::Image<uint8_t> input = load<uint8_t>("images/bikesgray-wikipedia.png");
    Halide::Func padded;
    padded(x,y) = input(clamp(x, 0, input.width()-1), clamp(y, 0, input.height()-1));

    Halide::Image<uint8_t> output(input.width(), input.height());

    // calculate the horizontal and vertical gradients (GX, Gy)
    std::pair<Halide::Func, Halide::Func> prewitt = prewitt_3x3(padded, true);
    Halide::Func Gx, Gy, mag;
    Gx(x,y) = AS_UINT8(prewitt.first(x,y));
    Gx.realize(output);
    save(output, "output/prewitt_gx.png");

    Gy(x,y) = AS_UINT8(prewitt.second(x,y));
    Gy.realize(output);
    save(output, "output/prewitt_gy.png");

    // calculate the magnitude of Gx, Gy
    mag = grad_magnitude(prewitt.first, prewitt.second);
    TO_2D_UINT8_LAMBDA(mag).realize(output);
    save(output, "output/prewitt_mag.png");

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

void canny_example() {
    Halide::Image<uint8_t> input = load<uint8_t>("images/bikesgray-wikipedia.png");
    Halide::Image<uint8_t> output(input.width(), input.height(), 3);
    Halide::Var x,y,c;
    Halide::Func padded;
    padded(x,y) = input(clamp(x, 0, input.width()-1), clamp(y, 0, input.height()-1));

    Halide::Func canny = canny_detector(padded, true);

    // color-code
    Halide::Func color("color");
    color(x,y,c) = 0;
    //color(x,y,0) = select( canny(x,y) == DIRECTION_45UP || canny(x,y) == DIRECTION_45DOWN, 255, 0);
    //color(x,y,1) = select( canny(x,y) == DIRECTION_VERTICAL, 255, 0);
    color(x,y,2) = select( canny(x,y) == DIRECTION_HORIZONTAL, 255, 0);

    lambda(x,y,c,Halide::cast<uint8_t>(min(color(x,y,c), 255))).realize(output);
    save(output, "output/canny.png");

    printf("%s DONE\n", __func__);
}


int main(int argc, char **argv) {

    Halide::Image<uint8_t> input = load<uint8_t>("images/rgb.png");
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

    double w_scale = 2, h_scale = 2;
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

    // openvx_example(input);

    // Halide::Image<uint8_t> cv_input = load<uint8_t>("images/bikesgray-wikipedia.png");
    // cv_example(cv_input);
    // sobel_example();
    // prewitt_example();
    //canny_example();

    return 0;
}



