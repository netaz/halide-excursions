#include <Halide.h>
#include <string>
#include "kernels/invert.h"
#include "kernels/openvx.h"
#include "kernels/cv.h"
#include "kernels/color_convert.h"
#include "utils/utils.h"
#include "utils/clock.h"

using Halide::Image;
#include "utils/image_io.h"

class Separable2dConvolutionSched : public Scheduler {
public:
    Separable2dConvolutionSched(size_t policy) : policy(policy) {}
    virtual void schedule(Halide::Func f, Halide::Var x, Halide::Var y) const {}
    virtual void schedule(Halide::Func fx, Halide::Func fy, Halide::Var x, Halide::Var y) const {
    //virtual void schedule(Halide::Func fx, Halide::Func fy) const {
        //Halide::Var x,y,xi,yi;
        Halide::Var xi,yi;
        switch(policy) {
        case 1:
            fx.compute_at(fy, y);
            break;
        case 2:
            fx.store_root().compute_at(fy, y);
            break;
        case 3:
            fx.compute_at(fy, x).vectorize(x, 4);
            break;
        case 4:
            fx.compute_at(fy, x).vectorize(x, 4);
            fy.tile(x, y, xi, yi, 8, 8).parallel(y).vectorize(xi, 4);
            break;
        case 5:
            fx.store_root()
                    .compute_at(fy, y)
                    .split(x, x, xi, 8)
                    .vectorize(xi, 4)
                    .parallel(x);
            fy.split(x, x, xi, 8)
                    .vectorize(xi, 4)
                    .parallel(x);
            break;
        case 6:
            fx.store_at(fy, y)
                    .compute_at(fy, yi)
                    .vectorize(x, 4); 
            fy.split(y, y, yi, 8)
                    .parallel(y)
                    .vectorize(x, 4);
            break;
        }
    }

private:
    size_t policy;
};

class ConvolutionSched : public Scheduler {
public:
    ConvolutionSched(size_t policy) : policy(policy) {}
    virtual void schedule(Halide::Func f, Halide::Var x, Halide::Var y) const {
        Halide::Var xi,yi;
        switch(policy) {
        case 1:
            f.compute_root();
            break;
        case 2:
            f.store_root().vectorize(x, 4);
            break;
        case 3:
            f.store_root().vectorize(x, 8);
            break;
        case 4:
            f.store_root().split(x, x, xi, 8).vectorize(xi, 4).parallel(x);
            break;
        case 5:
            f.store_root().split(x, x, xi, 8).vectorize(xi, 4);
            break;
        case 6:
            f.tile(x, y, xi, yi, 256, 32).vectorize(xi, 8).parallel(y);
            break;
        case 7:
            f.tile(x, y, xi, yi, 256, 32).vectorize(xi, 4).parallel(y);
            break;
        }
            
    }    virtual void schedule(Halide::Func fx, Halide::Func fy, Halide::Var x, Halide::Var y) const {}

private:
    size_t policy;
};


timings t;

int sched_example(int argc, const char **argv) {
    Halide::Image<uint8_t> input = load<uint8_t>(argv[0]);

    Halide::Image<uint8_t> output(input.width(), input.height(), input.channels());
    Halide::Image<uint8_t> output2(input.width(), input.height(), input.channels());
    Halide::Var x,y,xi,yi,c;

    Halide::Func padded, padded32;
    padded(x,y,c) = input(clamp(x, 0, input.width()-1),
                          clamp(y, 0, input.height()-1),
                          c);

    padded32(x,y,c) = Halide::cast<int32_t>(padded(x,y,c));

    //Halide::Func test = gaussian_3x3_3(padded32, x,y, c, Separable2dConvolutionSched());
    //Halide::Func test = gaussian_3x3_3(padded32, Separable2dConvolutionSched(4));
    Halide::Func test = gaussian_3x3_2(padded32, ConvolutionSched(6));
    //Halide::Func test = gaussian_3x3_4(padded32, Separable2dConvolutionSched(4));
    
#define PERFORM_EXTERNAL_CAST
#ifdef PERFORM_EXTERNAL_CAST    
    Halide::Func test2("test2");
    //test.store_root();
    test.compute_root();
    test2(x,y,c) = Halide::cast<uint8_t>(test(x,y,c));
    //test2.tile(x, y, xi, yi, 256, 32).vectorize(xi, 4).parallel(y);
    //test2.tile(x, y, xi, yi, 256, 32).vectorize(xi, 8).parallel(y);
    test2.vectorize(x, 4).parallel(y);

    //Halide::Target target = Halide::get_jit_target_from_environment();
    //test2.compile_jit(target);

#endif // PERFORM_EXTERNAL_CAST
    {
        for (int i=0; i<50; i++) {
            interval i(t);
        #ifdef PERFORM_EXTERNAL_CAST
            test2.realize(output);
        #else
            test.realize(output);
        #endif
        }
    }
    t.dump();
//////////////////////////////////////////
    
    printf("%s DONE\n", __func__);
    return EXIT_SUCCESS;
}
