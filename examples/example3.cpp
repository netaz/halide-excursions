#include <Halide.h>
#include <string>
#include "excursions.h"
#include "utils/utils.h"


using Halide::Image;
#include "utils/image_io.h"

int prewitt_example(int argc, const char **argv) {
    Halide::Image<uint8_t> input = load<uint8_t>(argv[0]);
    Halide::Var x,y,c;
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
    return EXIT_SUCCESS;
}


int cv_example(int argc, const char **argv) {
    Halide::Image<uint8_t> input = load<uint8_t>("images/bikesgray-wikipedia.png");
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
    return EXIT_SUCCESS;
}

int canny_example(int argc, const char **argv) {
    Halide::Image<uint8_t> input = load<uint8_t>(argv[0]);
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
    return EXIT_SUCCESS;
}
