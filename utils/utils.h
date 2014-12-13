#ifndef __UTILS_H
#define __UTILS_H

#include "Halide.h"
#include "stdio.h"

#define AS_UINT8(expr) (Halide::cast<uint8_t>(min(expr, 255)))

// TODO: this function assumes printf is used for user i/o
template <typename T>
void dump_test_img(Halide::Image<T> &img) {
    printf("Dumping image: %s\n", img.name().c_str());
    printf("\tdimensions=%d\n", img.dimensions());
    for (int i=img.min(0); i<img.min(0)+img.extent(0); i++) {
        if (img.dimensions()>1) {
            printf("\t%d: ", i);
            for (int j=img.min(1); j<img.min(1)+img.extent(1); j++) {
                printf("%d ", img(i,j));
            }
            printf("\n");
        }
        else
            printf("%d\n", img(i));
    }
}

// TODO: this assumes a 2-dimensional image
template <typename T>
void randomize(Halide::Image<T> &img) {
    srand((unsigned int)time(NULL));
    for (int i=img.min(0); i<img.min(0)+img.extent(0); i++)
        for (int j=img.min(1); j<img.min(1)+img.extent(1); j++)
            img(i, j) = rand() % 100;
}

#include <limits>       // std::numeric_limits
template <typename T>
T verify_max(Halide::Image<T> &img) {
    T max_val = std::numeric_limits<T>::min();
    for (int i=img.min(0); i<img.min(0)+img.extent(0); i++)
        for (int j=img.min(1); j<img.min(1)+img.extent(1); j++)
            max_val = std::max(max_val, img(i,j));
    return max_val;
}

#endif // __UTILS_H