halide-excursions
=================

A Halide journey taken for pleasure, this repo will hopefully serve as collection of Halide imaging functions that are useful to the community.

The halide-excursions project is an attempt to create a large, open source repository of Halide computer-vision, computational-photography, and image processing functions.  Anyone and everyone is more than welcome to join.

NOTES:
1. This assumes you have Halide "installed" on your system
2. Currently only tests on Linux

To build:
$ make bin/test

Guidelines for new functions (BKMs)
===================================
Functions should not cast the type of its inputs, if possible.  If we allow functions to perform type casting, then it will be hard to give the application developer control over performance issues related to casting.  Also, the developer may need to perform multiple casts when feeding the output of one function to another.  This does, however, transfer some responsibility to the application developer because int8 accumulators may easily overflow.
For example, consider this function:
	
	Halide::Func gaussian_3x3_3(Halide::Func input) {
	    Halide::Func gaussian_x, gaussian_y;
	    Halide::Var x,y,c;
	  
	    gaussian_x(x,y,c) = (input(x-1,y,c) + input(x,y,c) * 2 + input(x+1,y,c))/4;
	    gaussian_y(x,y,c) = (gaussian_x(x,y-1,c)  + gaussian_x(x,y,c) * 2 + gaussian_x(x,y+1,c) )/4;

	    return gaussian_y;
	}

If the input function uses uint8_t or int8_t, then the gaussian_x and gaussian_y will likely overflow.  Halide will not emit an error or warning, but the results will be wrong.  We should document this for all functions that may overflow.  If the code could declare and assert the use of the correct type, it would be even better.
