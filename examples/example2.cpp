#include <Halide.h>
#include <string>
#include "excursions.h"
#include "utils/utils.h"

using Halide::Image;
#include "utils/image_io.h"

int sobel_example(int argc, const char **argv) {
    Halide::Image<uint8_t> input = load<uint8_t>(argv[0]);
    Halide::Var x,y,c;
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
    return EXIT_SUCCESS;
}
