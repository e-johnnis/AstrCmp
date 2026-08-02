CXX:=g++
INSTALL_EXE:=/usr/bin
INSTALL_LIB:=/usr/lib

CFLAGS=-fopenmp 
INCLUDE=-I./include -I/usr/include/opencv4
LIBS=-lopencv_core -lopencv_imgcodecs -lopencv_imgproc -lopencv_features2d -lopencv_calib3d -lopencv_videoio -lraw

.PHONY: build install remove

build:
	mkdir -p build
	$(CXX) $(CFLAGS) $(INCLUDE) -fPIC -shared -o build/libacmp.so processor.cpp $(LIBS)
	$(CXX) $(CFLAGS) $(INCLUDE) -o build/acmp main.cpp -L build -lacmp

install:
	cp -d -v build/libacmp.so $(INSTALL_LIB)/.
	cp -d -v build/acmp $(INSTALL_EXE)/.

remove:
	rm $(INSTALL_LIB)/libacmp.so $(INSTALL_EXE)/acmp