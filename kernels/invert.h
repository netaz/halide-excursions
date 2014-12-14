#ifndef __INVERT_H
#define __INVERT_H

#include <Halide.h>

/*
 * Invert input over the specified reduction domain (r)
 * 
 */
template <typename TIMAGE>
Halide::Func invert(Halide::Func input, Halide::RDom r) {
    Halide::Var x,y,c;
    Halide::Func invert("invert");

    Halide::Func img_max;
    img_max(c) = Halide::cast<TIMAGE>(0);
    img_max(c) = max(input(r.x, r.y, c), img_max(c));
    
    Halide::Image<TIMAGE> max_val = img_max.realize(1);

    // TODO: What can I do with this???
    // http://halide-lang.org/docs/_inline_reductions_8h.html
    // Halide::Expr e = Halide::maximum(r, input(r.x, r.y, c));
    // int16_t fff = cast<int16_t>(e[c])
        
    invert(x,y,c) = max_val(0) - input(x,y,c);
    return invert;
}

// one-sides (vertical) reflection across the line x=k
Halide::Func reflect_vert(Halide::Func input, int k, int width) {
    Halide::Var x,y,c;
    Halide::Func reflect("reflect");

    if (k>width/2) 
    	reflect(x,y,c) = select(x<k, input(x,y,c), input(2*k-x, y, c));
    else
    	reflect(x,y,c) = select(x>k, input(x,y,c), input(2*k-x, y, c));
    return reflect;
}

#endif // __INVERT_H