#include <Halide.h>
#include <string>
#include "excursions.h"
#include "utils/utils.h"

using Halide::Image;
#include "utils/image_io.h"

#include "gtest/gtest.h"

static bool verbose = false;

bool box_3x3__test() {
	Halide::Image<uint8_t> input(10,10,"input");
	Halide::Image<uint32_t> output(10,10,"output");
	Halide::Image<uint32_t> test(10,10,"test");
	excursions::randomize(input);
  //excursions::init_monotonic(input);

	Halide::Func padded("padded"), padded32;
	Halide::Var x,y,c;
  padded(x,y) = input(clamp(x, 0, input.width()-1),
                      clamp(y, 0, input.height()-1));
  padded32(x,y) = Halide::cast<uint32_t>(padded(x,y));

  Halide::Func box_3x3_fn = box_3x3(padded32, true);
  box_3x3_fn.realize(output);

  excursions::clammper<uint8_t> cl(input, 0, input.width()-1, 0, input.height()-1);

  for (int j = 0; j < input.height(); j++) {
      for (int i = 0; i < input.width(); i++) {
        uint32_t sum = cl(i-1,j-1) + cl(i,j-1) + cl(i+1,j-1) + 
                       cl(i-1,j)   + cl(i,j) +   cl(i+1,j)   + 
                       cl(i-1,j+1) + cl(i,j+1) + cl(i+1,j+1);
      	test(i,j) = sum/9;
        // if (i==1 && j==1) {
        //    excursions::dump_3x3_neghiborhood(cl, i, j);
        //    excursions::dump_3x3_neghiborhood(input, i, j);
        //    printf("sum=%d %d\n", sum, test(j,i));
        //  }
      }
  }

  if (verbose) {
    excursions::dump_test_img(input);
    excursions::dump_test_img(output);
    excursions::dump_test_img(test);
  }
  return excursions::compare_images(output, test);
}

bool box_3x3__test_3d() {
  Halide::Image<uint8_t> input(10,10,3,"input");
  Halide::Image<uint32_t> output(10,10,3,"output");
  Halide::Image<uint32_t> test(10,10,3,"test");
  //excursions::init_monotonic(input);
  excursions::randomize(input);

  Halide::Func padded("padded"), padded32;
  Halide::Var x,y,c;
  padded(x,y,c) = input(clamp(x, 0, input.width()-1),
                        clamp(y, 0, input.height()-1),
                        c);
  padded32(x,y,c) = Halide::cast<uint32_t>(padded(x,y,c));

  Halide::Func box_3x3_fn = box_3x3(padded32);
  box_3x3_fn.realize(output);

  excursions::clammper<uint8_t> cl(input, 0, input.height()-1, 0, input.width()-1);

  for (int j = 0; j < input.height(); j++) {
    for (int i = 0; i < input.width(); i++) {
      for (int k = 0; k < input.channels(); k++) {
        uint32_t sum = cl(i-1,j-1,k) + cl(i,j-1,k) + cl(i+1,j-1,k) + 
                       cl(i-1,j,k)   + cl(i,j,k) +   cl(i+1,j,k)   + 
                       cl(i-1,j+1,k) + cl(i,j+1,k) + cl(i+1,j+1,k);
        test(i,j,k) = sum/9;
      }
    }
  }

  if (verbose) {
    excursions::dump_test_img(input);
    excursions::dump_test_img(output);
    excursions::dump_test_img(test);
  }
  return excursions::compare_images(output, test);
}

TEST(box3x3Test, Normal) {
  EXPECT_EQ(true,box_3x3__test_3d());
  EXPECT_EQ(true,box_3x3__test());

}

#include <stdio.h>

GTEST_API_ int main(int argc, char **argv) {
  printf("Running main() from gtest_main.cc\n");
  testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
