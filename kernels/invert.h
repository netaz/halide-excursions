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


/*
https://staff.fnwi.uva.nl/r.vandenboomgaard/multimedia/labRotation.pdf

function r = pv( im, x, y, method )
% get a pixel value using interpolation
[M,N] = size(im);
if (x>=1) && (x<=M) && (y>=1) && (y<=N)
% OK point (x,y) is within image
if method==’linear’
% do the bilinear interpolation
else
% do the nearest neighbor interpolation
end
else
% outside the image --> return a sensible value
r = 0;
end

x = x 0 cos(φ) + y 0 sin(φ) 
y = − x 0 sin(φ) + y 0 cos(φ)

Note than angles in Matlab have to be given in radians
*/
#endif // __INVERT_H