#ifndef __UTILS_H
#define __UTILS_H

//
//
//  This file contains all sorts of utility functions, most of which are useful for verification
//  purposes.
//
// TODO: the functions in this file assume printf is used for user i/o.  This needs to be paramaterized
// TODO: functions currently ignore strides

#include "Halide.h"
#include <stdio.h>
#include <limits>       // std::numeric_limits

#define USE_HALIDE_JIT     1
#define USE_HALIDE_AOT     2 
#define GENERATE_AOT_OBJS  3

// I can't decide if using these macros makes the code more readable or more obscure
#define AS_UINT8(expr)             (Halide::cast<uint8_t>(min(expr, 255)))
#define TO_2D_UINT8_LAMBDA(func)   (lambda(x,y,Halide::cast<uint8_t>(min(func(x,y), 255))))
#define TO_3D_UINT8_LAMBDA(func)   (lambda(x,y,c,Halide::cast<uint8_t>(min(func(x,y,c), 255))))

namespace excursions {

template <typename T>
T clip(const T& n, const T& lower, const T& upper) {
  return std::max(lower, std::min(n, upper));
}

template <typename T>
struct clammper {
    int y_min, y_max;
    int x_min, x_max;
    Halide::Image<T> img;
    clammper(Halide::Image<T> img, int y_min, int y_max, int x_min, int x_max) :
        y_min(y_min), y_max(y_max),
        x_min(x_min), x_max(x_max),
        img(img) {}
 
    T operator()(int x, int y, int c) const { 
        return img( clip(x, x_min, x_max), 
                    clip(y, y_min, y_max),
                    c );
    }

    T operator()(int x, int y) const { 
        return img( clip(x, x_min, x_max), 
                    clip(y, y_min, y_max) );
    }
};

template <typename T>
void dump_test_img(Halide::Image<T> &img) {
    printf("Dumping image: %s\n", img.name().c_str());
    printf("\tdimensions=%d\n", img.dimensions());
    for (int i=img.min(0); i<img.min(0)+img.extent(0); i++) {
        if (img.dimensions()>1) {
            printf("\t%d: ", i);
            for (int j=img.min(1); j<img.min(1)+img.extent(1); j++) {
                if (img.dimensions()>2) {
                    printf("(");
                    for (int k=img.min(2); k<img.min(2)+img.extent(2); k++) {
                        printf("%3d ", img(i,j,k));
                    }
                    printf(")");
                }
                else
                    printf("%3d ", img(i,j));    
            }
            printf("\n");
        }
        else
            printf("%3d\n", img(i));
    }
}

template <typename T>
void dump_3x3_neghiborhood(Halide::Image<T> &img, int x, int y) {
    printf("\n%3d %3d %3d\n"
             "%3d %3d %3d\n"
             "%3d %3d %3d\n\n", img(x-1,y-1), img(x,y-1), img(x+1,y-1), 
                                img(x-1,y),   img(x,y),   img(x+1,y), 
                                img(x-1,y+1), img(x,y+1), img(x+1,y+1));
}

template <typename T>
void dump_3x3_neghiborhood(const excursions::clammper<T> &cl, int x, int y) {
        printf("\n%3d %3d %3d\n"
             "%3d %3d %3d\n"
             "%3d %3d %3d\n\n", cl(x-1,y-1), (cl(x,y-1)), (cl(x+1,y-1)), 
                                cl(x-1,y),   (cl(x,y)),   (cl(x+1,y)), 
                                cl(x-1,y+1), (cl(x,y+1)), (cl(x+1,y+1)));
}
template <typename T>
void dump_3x3_neghiborhood(Halide::Image<T> &img, int x, int y, int c) {
    printf("\n%3d %3d %3d\n"
             "%3d %3d %3d\n"
             "%3d %3d %3d\n\n", img(x-1,y-1,c), img(x,y-1,c), img(x+1,y-1,c), 
                                img(x-1,y,c),   img(x,y,c),   img(x+1,y,c), 
                                img(x-1,y+1,c), img(x,y+1,c), img(x+1,y+1,c));
}

template <typename T>
void dump_3x3_neghiborhood(Halide::Image<T> &img, int x, int y, int c, const excursions::clammper<T> &cl) {
    printf("\n%3d %3d %3d\n"
             "%3d %3d %3d\n"
             "%3d %3d %3d\n\n", cl(x-1,y-1,c), cl(x,y-1,c), cl(x+1,y-1,c), 
                                cl(x-1,y,c),   cl(x,y,c),   cl(x+1,y,c), 
                                cl(x-1,y+1,c), cl(x,y+1,c), cl(x+1,y+1,c));
}

// This is useful for inializing test images
template <typename T>
void randomize(Halide::Image<T> &img) {
    srand((unsigned int)time(NULL));
    for (int i=img.min(0); i<img.min(0)+img.extent(0); i++) {
        if (img.dimensions()>1) {
            for (int j=img.min(1); j<img.min(1)+img.extent(1); j++) {
                if (img.dimensions()>2) {
                    for (int k=img.min(2); k<img.min(2)+img.extent(2); k++)
                        img(i, j, k) = rand() % 100;
                } else {
                    img(i, j) = rand() % 100;
                }
            }
        } else {
            img(i) = rand() % 100;
        }
    }
}

// This is useful for inializing test images
template <typename T>
void init_monotonic(Halide::Image<T> &img) {
    T counter = 0;
    for (int i=img.min(0); i<img.min(0)+img.extent(0); i++) {
        if (img.dimensions()>1) {
            for (int j=img.min(1); j<img.min(1)+img.extent(1); j++) {
                if (img.dimensions()>2) {
                    for (int k=img.min(2); k<img.min(2)+img.extent(2); k++)
                        img(i, j, k) = counter++;
                } else {
                    img(i, j) = counter++;
                }
            }
        } else {
            img(i) = counter++;
        }
    }
}

// Returns true if images are binary equals
template <typename T>
bool compare_images(Halide::Image<T> im1, Halide::Image<T> im2) {
    if (im1.dimensions() != im2.dimensions())
        return false;
    
    size_t img_size = sizeof(T);  // image size in bytes
    for (int d=0; d<im1.dimensions(); d++) {
        if (im1.extent(d) != im2.extent(d))
            return false;
        img_size *= im1.extent(d);
    }

    return (memcmp(im1.data(), im2.data(), img_size) == 0) ;
}

template <typename T>
T verify_max(Halide::Image<T> &img) {
    T max_val = std::numeric_limits<T>::min();
    for (int i=img.min(0); i<img.min(0)+img.extent(0); i++)
        for (int j=img.min(1); j<img.min(1)+img.extent(1); j++)
            max_val = std::max(max_val, img(i,j));
    return max_val;
}

} // namespace excursions

#endif // __UTILS_H