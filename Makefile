FPGA_OCL_CFLAGS := $(shell aocl compile-config)
FPGA_OCL_LFLAGS := $(shell aocl link-config)

CC=g++
LD_FLAGS=-lm -lrt $(FPGA_OCL_LFLAGS)
# CC_FLAGS=-Wall -g $(FPGA_OCL_CFLAGS) -O2 -I /opt/xilinx/xrt/include/
CC_FLAGS=-Wall -g $(FPGA_OCL_CFLAGS) -O2 
#CC_FLAGS=-Wall -g $(FPGA_OCL_CFLAGS) -O2 -D USE_OPENCL_SVM


all: filter-test

clean:
	rm -f *.o
	rm -f filter-test


SOURCES= filter-test.cpp PrealignmentFilter.cpp PerformanceLap.cpp OpenCLUtils.cpp TextUtils.cpp edlib.cpp SWVersions.cpp

filter-test: $(SOURCES)
	g++ $(CC_FLAGS) $(LD_FLAGS) $(SOURCES)  -o filter-test -lOpenCL


