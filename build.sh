export HALIDE_HOME=~/Dev/Halide
g++ main.cpp kernels/openvx.cpp kernels/cv.cpp -g -I $HALIDE_HOME/include -L $HALIDE_HOME/bin -lHalide `libpng-config --cflags --ldflags` -lpthread -ldl -o test
LD_LIBRARY_PATH=$HALIDE_HOME/bin ./invert
