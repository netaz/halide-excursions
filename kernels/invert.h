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

//Halide::Func nn_interpolation()
// https://www.khronos.org/registry/vx/specs/1.0/html/d1/d26/group__group__vision__function__scale__image.html
Halide::Func nn_scale(Halide::Func input, float w_factor, float h_factor) {
	Halide::Func scale("scale");
    Halide::Var x,y,c;
	//scale(x,y,c) = input(Halide::cast<int>(x*w_factor), Halide::cast<int>(y*h_factor),c);
    scale(x,y,c) = input(Halide::cast<int>(((x+0.5f)*w_factor)-0.5f), Halide::cast<int>(((y+0.5f)*h_factor)-0.5f),c);
	return scale;
}

// https://www.khronos.org/registry/vx/specs/1.0/html/d1/d26/group__group__vision__function__scale__image.html
Halide::Func bilinear_scale(Halide::Func input, float w_factor, float h_factor) {
    Halide::Func scale("scale");
    Halide::Var x,y,c;
 
    Halide::Expr x_lower = Halide::cast<int>(x * w_factor);
    Halide::Expr y_lower = Halide::cast<int>(y * h_factor);
    Halide::Expr s = (x * w_factor) - x_lower;
    Halide::Expr t = (y * h_factor) - y_lower;

    scale(x,y,c) =  Halide::cast<uint8_t>(
                    (1-s) * (1-t) * input(x_lower, y_lower+1, c)  +
                    s * (1-t)     * input(x_lower+1, y_lower, c)  +
                    (1-s) * t     * input(x_lower, y_lower+1, c)  + 
                    s * t         * input(x_lower+1, y_lower+1, c)
                    );

    return scale;
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