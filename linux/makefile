CFLAGS=-c -Wall -O2 -I../src -D__LINUX__
LIBS = -lm -lpthread

all: jpegdec

jpegdec: jpeg.o main.o
	$(CC) main.o jpeg.o $(LIBS) -o jpegdec

jpeg.o: ../src/jpeg.c ../src/JPEGDEC.h makefile
	$(CC) $(CFLAGS) ../src/jpeg.c

main.o: main.c ../src/JPEGDEC.h makefile
	$(CC) $(CFLAGS) main.c

clean:
	rm *.o jpegdec
