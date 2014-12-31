#include <Halide.h>
#include <string>
#include "excursions.h"
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
        case 8:
            //f.store_root().split(x, x, xi, 32).vectorize(xi, 4).parallel(y);
            // f.store_root().split(x, x, xi, 256).vectorize(xi, 4).parallel(y);  -- increasing the split size doesn't help much
            f.split(x, x, xi, 256).vectorize(xi, 4).parallel(y);
            break;
        }
            
    }    virtual void schedule(Halide::Func fx, Halide::Func fy, Halide::Var x, Halide::Var y) const {}

private:
    size_t policy;
};


static Halide::Func createAndSchedulePipeline(Halide::Image<uint8_t> input) {
    Halide::Var x,y,xi,yi,c;
    Halide::Func padded, padded32;

    padded(x,y,c) = input(clamp(x, 0, input.width()-1),
                          clamp(y, 0, input.height()-1),
                          c);

    padded32(x,y,c) = Halide::cast<int32_t>(padded(x,y,c));

    //Halide::Func test = gaussian_3x3_3(padded32, x,y, c, Separable2dConvolutionSched());
    Halide::Func test = gaussian_3x3_3(padded32, Separable2dConvolutionSched(4));
    //Halide::Func test = gaussian_3x3_2(padded32, ConvolutionSched(8));
    //Halide::Func test = gaussian_3x3_4(padded32, Separable2dConvolutionSched(4));
    
#define PERFORM_EXTERNAL_CAST
#ifdef PERFORM_EXTERNAL_CAST    
    Halide::Func test2("test2");
    test.compute_root();
    test2(x,y,c) = Halide::cast<uint8_t>(test(x,y,c));
    //test2.tile(x, y, xi, yi, 256, 32).vectorize(xi, 4).parallel(y);
    //test2.tile(x, y, xi, yi, 256, 32).vectorize(xi, 8).parallel(y);
    test2.vectorize(x, 4).parallel(y);

#endif // PERFORM_EXTERNAL_CAST

    return test2;
}

 // Phases:
 //    1. Define processing logic and flow graph
 //    2. Overlay schedule policy
 //    3. [optional] compile flow graph
 //    4. Execute

#if (USAGE==USE_HALIDE_AOT)
#include "generated/halide_sched_example.h"

int aot_sched_example(int argc, const char **argv) {
    Halide::Image<uint8_t> input = load<uint8_t>(argv[0]);
      
    {    
        buffer_t output_buf = {0};
        buffer_t * input_buf = input.raw_buffer();

        uint8_t result[input.width() * input.height() * input.channels()];
        output_buf.extent[0] = input.extent(0); // Width.
        output_buf.extent[1] = input.extent(1); // Height.
        output_buf.extent[2] = input.extent(2); // Height.
        output_buf.stride[0] = input.stride(0);  // Spacing in memory between adjacent values of x.
        output_buf.stride[1] = input.stride(1); // Spacing in memory between adjacent values of y;
        output_buf.stride[2] = input.stride(2);
        output_buf.elem_size = sizeof(uint8_t); // Bytes per element.
        output_buf.host = (uint8_t *)result;
        timings t;
        for (int i=0; i<50; i++) {
            interval i(t);
            halide_sched_example(input_buf, &output_buf);
        }
        t.dump();
    }
    printf("%s (pre-compiled) DONE\n", __func__);

    return EXIT_SUCCESS;
}

#elif (USAGE==USE_HALIDE_JIT)
int jit_sched_example(int argc, const char **argv) {
    Halide::Func example;
    Halide::Image<uint8_t> input = load<uint8_t>(argv[0]);
    example = createAndSchedulePipeline(input);
    
    Halide::Image<uint8_t> output(input.width(), input.height(), input.channels());
    // This is redundant because realize() will do an implicit compile_jit().
    // However, it is kept to "warm up" Halide
    Halide::Target target = Halide::get_jit_target_from_environment();
    example.compile_jit(target);

    {    
        timings t(example.name());
        for (int i=0; i<50; i++) {
            interval i(t);
            example.realize(output);
        }
        t.dump();
    }
    printf("%s (jit) DONE\n", __func__);
    return EXIT_SUCCESS;
}
//#endif // USAGE==USE_HALIDE_JIT


#elif (USAGE==GENERATE_AOT_OBJS) 

#include <string>
#include <vector>
// Static compilation (generation)
static int generate_aot_binary(int argc, const char **argv) {
    Halide::Func example;
    Halide::Image<uint8_t> input = load<uint8_t>("images/rgb.png");
    example = createAndSchedulePipeline(input);

    Halide::ImageParam input_param(Halide::type_of<uint8_t>(), input.dimensions());
    Halide::Target target = Halide::get_target_from_environment();

    const std::string function("halide_sched_example");
    std::string object, header;
    if (argc==1) {
        object = argv[0];
        object += "//";
        object += function;
        header = object;
        object += ".o";
        header += ".h";
    } else {
        object = function;
    }
    std::vector<Halide::Argument> args;
    args.push_back(input_param);
    example.compile_to_object(object, args, function, target);
    example.compile_to_header(header, args, function);
        
    printf("\n%s:%s DONE\n\n", __FILE__, __func__);
    return EXIT_SUCCESS;
}

int main(int argc, const char **argv) {
    //const char *input = "images/rgb.png";
    return generate_aot_binary(argc-1, argv+1);
}
#else
#error "USAGE should be one of: USE_HALIDE_AOT | USE_HALIDE_JIT | GENERATE_AOT_OBJS"
#endif // USAGE

#if (USAGE!=GENERATE_AOT_OBJS) 
int sched_example(int argc, const char **argv) {
    #if (USAGE==USE_HALIDE_JIT)
    return jit_sched_example(argc,argv);
    #elif (USAGE==USE_HALIDE_AOT)
    return aot_sched_example(argc,argv);
    #else
    #error "USAGE should be one of: USE_HALIDE_AOT | USE_HALIDE_JIT"
    #endif // USAGE
}
#endif