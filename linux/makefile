CFLAGS=-D__LINUX__ -Wall -O2
PORTABLE_FLAGS=-DJPEGDEC_NO_ARDUINO -DNO_SIMD -Wall -Wextra -Werror -O2
ASAN_FLAGS=-fsanitize=address -fno-omit-frame-pointer -O1 -g -Werror=vla
LIBS=-lm -lpthread

all: libJPEGDEC.a

libJPEGDEC.a: JPEGDEC.o jpegdec.o
	ar -rc libJPEGDEC.a JPEGDEC.o jpegdec.o

c_api_test: c_api_test.o jpegdec_asan.o
	$(CC) $(ASAN_FLAGS) c_api_test.o jpegdec_asan.o $(LIBS) -o c_api_test

cpp_api_test: cpp_api_test.o JPEGDEC_asan.o jpegdec_asan.o
	$(CXX) $(ASAN_FLAGS) cpp_api_test.o JPEGDEC_asan.o jpegdec_asan.o $(LIBS) -o cpp_api_test

progressive_test: progressive_test.o jpegdec_asan.o
	$(CC) $(ASAN_FLAGS) progressive_test.o jpegdec_asan.o $(LIBS) -o progressive_test

yuv420p_test: yuv420p_test.o jpegdec_asan.o
	$(CC) $(ASAN_FLAGS) yuv420p_test.o jpegdec_asan.o $(LIBS) -o yuv420p_test

nv12_test: nv12_test.o jpegdec_asan.o
	$(CC) $(ASAN_FLAGS) nv12_test.o jpegdec_asan.o $(LIBS) -o nv12_test

c_api_test.o: c_api_test.c ../src/JPEGDEC.h ../test_images/tulips.h
	$(CC) $(CFLAGS) $(ASAN_FLAGS) -c c_api_test.c

cpp_api_test.o: cpp_api_test.cpp ../src/JPEGDEC.h ../test_images/tulips.h
	$(CXX) $(CFLAGS) $(ASAN_FLAGS) -c cpp_api_test.cpp

progressive_test.o: progressive_test.c progressive_test_image.h ../src/JPEGDEC.h
	$(CC) $(CFLAGS) $(ASAN_FLAGS) -c progressive_test.c

yuv420p_test.o: yuv420p_test.c ../src/JPEGDEC.h ../test_images/tulips.h
	$(CC) $(CFLAGS) $(ASAN_FLAGS) -c yuv420p_test.c

nv12_test.o: nv12_test.c ../src/JPEGDEC.h ../test_images/tulips.h
	$(CC) $(CFLAGS) $(ASAN_FLAGS) -c nv12_test.c

JPEGDEC.o: ../src/JPEGDEC.cpp ../src/JPEGDEC.h
	$(CXX) $(CFLAGS) -c ../src/JPEGDEC.cpp

jpegdec.o: ../src/jpegdec.c ../src/jpeg.inl ../src/JPEGDEC.h
	$(CC) $(CFLAGS) -c ../src/jpegdec.c

JPEGDEC_asan.o: ../src/JPEGDEC.cpp ../src/JPEGDEC.h
	$(CXX) $(CFLAGS) $(ASAN_FLAGS) -c ../src/JPEGDEC.cpp -o JPEGDEC_asan.o

jpegdec_asan.o: ../src/jpegdec.c ../src/jpeg.inl ../src/JPEGDEC.h
	$(CC) $(CFLAGS) $(ASAN_FLAGS) -c ../src/jpegdec.c -o jpegdec_asan.o

jpegdec_no_arduino.o: ../src/jpegdec.c ../src/jpeg.inl ../src/JPEGDEC.h
	$(CC) $(PORTABLE_FLAGS) -c ../src/jpegdec.c -o jpegdec_no_arduino.o

JPEGDEC_no_arduino.o: ../src/JPEGDEC.cpp ../src/JPEGDEC.h
	$(CXX) $(PORTABLE_FLAGS) -c ../src/JPEGDEC.cpp -o JPEGDEC_no_arduino.o

test: c_api_test cpp_api_test progressive_test yuv420p_test nv12_test jpegdec_no_arduino.o JPEGDEC_no_arduino.o
	ASAN_OPTIONS=detect_leaks=1:halt_on_error=1 ./c_api_test
	ASAN_OPTIONS=detect_leaks=1:halt_on_error=1 ./cpp_api_test
	ASAN_OPTIONS=detect_leaks=1:halt_on_error=1 ./progressive_test
	ASAN_OPTIONS=detect_leaks=1:halt_on_error=1 ./yuv420p_test
	ASAN_OPTIONS=detect_leaks=1:halt_on_error=1 ./nv12_test

clean:
	rm -rf *.o libJPEGDEC.a c_api_test cpp_api_test progressive_test yuv420p_test nv12_test
