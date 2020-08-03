#ifndef __JPEGDEC__
#define __JPEGDEC__
#ifdef __MACH__
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <stdio.h>
#define memcpy_P memcpy
#define PROGMEM
#else
#include <Arduino.h>
#endif
//
// JPEG Decoder
// Written by Larry Bank
// Copyright (c) 2020 BitBank Software, Inc.
// 
// Designed to decode baseline JPEG images (8 or 24-bpp)
// using less than 22K of RAM
//

/* Defines and variables */
#define VLC_BUF_SIZE 2048
#define VLC_HIGHWATER 1536
#define FILE_BUF_SIZE 2048
#define HUFF_TABLEN  273
#define DCTSIZE 64

// RGB565 pixel byte order
#define BIG_ENDIAN_PIXELS 0
#define LITTLE_ENDIAN_PIXELS 1

typedef struct jpeg_file_tag
{
  int32_t iPos; // current file position
  int32_t iSize; // file size
  uint8_t *pData; // memory file pointer
  void * fHandle; // class pointer to File/SdFat or whatever you want
} JPEGFILE;

typedef struct jpeg_draw_tag
{
    int x, y; // upper left corner of current MCU
    int iWidth, iHeight; // size of this MCU
    int iBpp; // bit depth of the pixels (8 or 16)
    uint16_t *pPixels; // 16-bit pixels
} JPEGDRAW;

// Callback function prototypes
typedef int32_t (JPEG_READ_CALLBACK)(JPEGFILE *pFile, uint8_t *pBuf, int32_t iLen);
typedef int32_t (JPEG_SEEK_CALLBACK)(JPEGFILE *pFile, int32_t iPosition);
typedef void (JPEG_DRAW_CALLBACK)(JPEGDRAW *pDraw);
typedef void * (JPEG_OPEN_CALLBACK)(char *szFilename, int32_t *pFileSize);
typedef void (JPEG_CLOSE_CALLBACK)(void *pHandle);

//
// our private structure to hold a GIF image decode state
//
typedef struct jpeg_image_tag
{
    int iWidth, iHeight;
    uint8_t ucBpp, ucSubSample, ucLittleEndian, ucHuffTableUsed;
    uint8_t ucMode;
    int iVLCOff; // current VLC data offset
    int iVLCSize; // current quantity of data in the VLC buffer
    int iResInterval;
    JPEG_READ_CALLBACK *pfnRead;
    JPEG_SEEK_CALLBACK *pfnSeek;
    JPEG_DRAW_CALLBACK *pfnDraw;
    JPEG_OPEN_CALLBACK *pfnOpen;
    JPEG_CLOSE_CALLBACK *pfnClose;
    JPEGFILE JPEGFile;
    uint16_t usMCUBuf[16*16]; // current MCU
    int16_t sQuantTable[DCTSIZE*2]; // quantization tables
    uint8_t ucHuffVals[HUFF_TABLEN*8];
    uint8_t ucFileBuf[FILE_BUF_SIZE]; // holds temp data and pixel stack
    uint8_t ucVLC[VLC_BUF_SIZE];
    uint8_t ucHuffDC[4096];
    uint8_t ucHuffAC[4096];
    unsigned short usHuffTable[4096];
} JPEGIMAGE;

//
// The JPEGDEC class wraps portable C code which does the actual work
//
class JPEGDEC
{
  public:
    int open(uint8_t *pData, int iDataSize, JPEG_DRAW_CALLBACK *pfnDraw);
    int open(char *szFilename, JPEG_OPEN_CALLBACK *pfnOpen, JPEG_CLOSE_CALLBACK *pfnClose, JPEG_READ_CALLBACK *pfnRead, JPEG_SEEK_CALLBACK *pfnSeek, JPEG_DRAW_CALLBACK *pfnDraw);
    void close();
    void begin(int iEndian);
    int decode();
    int getWidth();
    int getHeight();
    int getBpp();
    int getSubSample();

  private:
    JPEGIMAGE _jpeg;
};

// Due to unaligned memory causing an exception, we have to do these macros the slow way
#define INTELSHORT(p) ((*p) + (*(p+1)<<8))
#define INTELLONG(p) ((*p) + (*(p+1)<<8) + (*(p+2)<<16) + (*(p+3)<<24))
#define MOTOSHORT(p) (((*(p))<<8) + (*(p+1)))
#define MOTOLONG(p) (((*p)<<24) + ((*(p+1))<<16) + ((*(p+2))<<8) + (*(p+3)))

// Must be a 32-bit target processor
#define REGISTER_WIDTH 32

#endif // __JPEGDEC__
