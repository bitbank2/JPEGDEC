CFLAGS=-c -Wall -O2 -ggdb -I../../../src -D__LINUX__
LIBS = -lm -lpthread

all: jpegdec

jpegdec: main.o jpegdec.o
	$(CC) main.o jpegdec.o $(LIBS) -g -o jpegdec

main.o: main.c ../../../src/JPEGDEC.h makefile
	$(CC) $(CFLAGS) main.c

jpegdec.o: ../../../src/jpegdec.c ../../../src/jpeg.inl ../../../src/JPEGDEC.h
	$(CC) $(CFLAGS) ../../../src/jpegdec.c -o jpegdec.o

clean:
	rm -f *.o jpegdec
