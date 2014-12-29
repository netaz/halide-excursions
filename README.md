halide-excursions
=================

A Halide journey taken for pleasure, this repo will hopefully serve as collection of Halide imaging functions that are useful to the community.

The halide-excursions project is an attempt to create a large, open source repository of Halide computer-vision, computational-photography, and image processing functions.  Anyone and everyone is more than welcome to join.

NOTES:
1. This assumes you have Halide "installed" on your system and that the environment variable HALIDE_HOME points to this directory.
2. Currently only tests on Linux


To build examples using JIT compilation:
	$ make bin/test
	$ LD_LIBRARY_PATH=$HALIDE_HOME/bin bin/test <test-name>

To build examples using AOT (ahead-of-time) compilation:
	$ make bin/test_aot
	$ LD_LIBRARY_PATH=$HALIDE_HOME/bin bin/aot_test <test-name>

To generate the Halide functions object files (AoT objects):
	$  make bin/generate_aot


Guidelines for new functions (BKMs)
===================================
Functions should not cast the types of their inputs, if possible.  There are several motivations for this guideline:
- Casting from one type to another can have a negative effect on the performance.
- When an Excursions function decides to perform type casting it effectively makes a performance decision on behalf of the developer and robs him/her of the opportunity to make decisions related to the entire pipeline.
- If an Excursions function outputs a different type from its input, then this may confuse the pipeline developer.  Moreover, the pipeline developer may need to perform multiple casts when feeding the output of one Excurstion function to another, if each function casts to a different type.  

This does, however, transfer some responsibility to the application developer because int8 accumulators may easily overflow.
For example, consider this function:
	
	Halide::Func gaussian_3x3_3(Halide::Func input) {
	    Halide::Func gaussian_x, gaussian_y;
	    Halide::Var x,y,c;
	  
	    gaussian_x(x,y,c) = (input(x-1,y,c) + input(x,y,c) * 2 + input(x+1,y,c))/4;
	    gaussian_y(x,y,c) = (gaussian_x(x,y-1,c)  + gaussian_x(x,y,c) * 2 + gaussian_x(x,y+1,c) )/4;

	    return gaussian_y;
	}

If the input function uses uint8_t or int8_t, then the gaussian_x and gaussian_y will likely overflow and truncate the accumulated value.  Halide will not emit an error or warning, but the results will be wrong.  
We should explicitly document all functions that may overflow so that a pipeline developer will be aware that certain input types will produce incorrect results.  If the code could declare and assert the use of the correct type, it would be even better.
