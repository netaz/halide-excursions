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

int openvx_example(int argc, const char **argv);
int sobel_example(int argc, const char **argv);
int prewitt_example(int argc, const char **argv);
int canny_example(int argc, const char **argv);
int cv_example(int argc, const char **argv);
int scale_example(int argc, const char **argv);
int sched_example(int argc, const char **argv);

Halide::Func rotate(Halide::Func input, int height) {
    Halide::Func rot;
    Halide::Var x,y;

    rot(x,y) = input(y, (height - 1) - x);
    return rot;
}



struct example {
    const char* name;
    int (*code)(int argc, const char **argv);
    int argc;
    const char *argv[10];
};

example examples[] = {
    {"openvx", openvx_example, 1, {"images/rgb.png"} },
    {"sobel", sobel_example, 1, {"images/bikesgray-wikipedia.png"} },
    {"prewitt", prewitt_example, 1, {"images/bikesgray-wikipedia.png"} },
    {"canny", canny_example, 1, {"images/bikesgray-wikipedia.png"} },
    {"cv", cv_example, 1, {"images/bikesgray-wikipedia.png"} },
    {"scale", scale_example, 1, {"images/rgb.png"} },
    {"sched", sched_example, 1, {"images/rgb.png"} },
};


int main(int argc, char **argv) {
    if (argc != 2) {
        printf("Error: missing example name\n");
        printf("usage: test <example-name>\n");
        return EXIT_FAILURE;
    }

    size_t num_examples = sizeof(examples) / sizeof(examples[0]);
    size_t i = 0;
    for (; i<num_examples; i++) {
        example e = examples[i];
        if (!strcmp(e.name, argv[1])) {
            (*e.code)(e.argc, e.argv);
            break;
        }
    }

    if (i == num_examples) {
        printf("Error: Could not find example '%s'\n", argv[1]);
        return EXIT_FAILURE;
    }

     return EXIT_SUCCESS;
}