//
// PIL - portable imaging library
// Copyright (c) 2000 BitBank Software, Inc.
// Written by Larry Bank
// Project started 12/9/2000
// A highly optimized imaging library designed for resource-constrained
// environments such as mobile/embedded devices
//

//#ifndef __cplusplus
//#include <stdio.h>
//#include <stdlib.h>
//#include <math.h>
//#ifdef _WIN32
//#include <windows.h>
//#include <tchar.h>
//#else
//#include "my_windows.h"
//#include <stdint.h>
//#include <stdlib.h>
//#include <string.h>
//#include <ctype.h>
//#include <zlib.h>
//#endif
//#endif // __cplusplus

#include <SD.h>
extern File file;

//#define USE_MISC_CODEC
//#define USE_LZW_CODEC
//#define USE_FLATE_CODEC
#define USE_JPEG_CODEC
//#define USE_JBIG_CODEC
//#define USE_VIDEO_CODECS
//#define USE_COLOR_SIMD

#ifdef __cplusplus
extern "C" {
#endif // __cplusplus

#if __SIZEOF_LONG__ > 4
#define _64BITS
#endif

#include "pil.h"
#include "pil_io.h"


void PILDrawRuns(signed int *flips, unsigned char *pDest, int iWidth);

#ifdef _WIN32_WCE
#include <Cmnintrin.h>
#endif

#define JPEG_PROGRESSIVE // allow progressive support
// Maximum valid tags we accept for a TIFF page
#define MAX_TIFF_TAGS 128

// External ARM assembly language code to speed things up
typedef void (*PFNSCALE2GRAY)(unsigned char *pSrc, unsigned char *pDest, int lsize, unsigned char ucBpp);
void ARMScale2Gray(unsigned char *pSrc, unsigned char *pDest, int lsize, unsigned char ucBpp);
void ARMFastCopy(void *pSrc, void *pDest, int iLen);
void ARMDrawScaled16_0(void *pSrc, void *pDest, int iWidth, int iScaleX);
void ARMDrawScaled32_0(void *pSrc, void *pDest, int iWidth, int iScaleX);
void ARMDrawMCU21(signed short *pMCU, unsigned char *pDest, int lsize);
void ARMDrawMCU21Half(signed short *pMCU, unsigned char *pDest, int lsize);
void ARMDrawMCU11(signed short *pMCU, unsigned char *pDest, int lsize);
void ARMDrawMCU11Half(signed short *pMCU, unsigned char *pDest, int lsize);
//void ARMDrawMCU22FAST(signed short *pMCU, unsigned char *pDest, int lsize, PILTABLES *pTables);
void ARMDrawMCU22(signed short *pMCU, unsigned char *pDest, int lsize);
void ARMDrawMCU22Half(signed short *pMCU, unsigned char *pDest, int lsize);
void ARMJPEGIDCT(JPEGDATA *, signed short *, int);
void ARMJPEGFDCT(signed short *);
int ARMDecodeMCU(BUFFERED_BITS *bb, signed short *pMCU, JPEGDATA *pJPEG, int *iDCPredictor);
int X64MAPMCU(BUFFERED_BITS *bb, JPEGDATA *pJPEG, int iDCPredictor);
int X64DECODEMCU(BUFFERED_BITS *bb, signed short *pMCU, JPEGDATA *pJPEG, int *iDCPredictor);
int X86DECODEMCU(BUFFERED_BITS *bb, signed short *pMCU, JPEGDATA *pJPEG, int *iDCPredictor);
int X64LOSSLESSDECODE(unsigned short *pOut, BUFFERED_BITS *bb, int iWidth, unsigned char *pFast);
int ARMDecodeMCUFast(BUFFERED_BITS *bb, JPEGDATA *pJPEG, int *iDCPredictor);
int X64DECODEMCUFAST(BUFFERED_BITS *bb, JPEGDATA *pJPEG, int *iDCPredictor);
int X86DECODEMCUFAST(BUFFERED_BITS *bb, JPEGDATA *pJPEG, int *iDCPredictor);
void X86InsertCode(BUFFERED_BITS *bb, uint32_t ulCode, int iLen);
void X64InsertCode(BUFFERED_BITS *bb, uint32_t ulCode, int iLen);
void X86DrawScaled32_0(void *pSrc, void *pDest, int iWidth, int iScaleX);
int ARMEncodeMCU(JPEGDATA *pJPEG, signed short *pMCUData, PIL_CODE *pPC, int iDCPred);
void ARMDraw1Scaled(unsigned char *irlcptr, unsigned char *bitoff, int iStart, int xright, int iScaleFactor);
uint32_t ARMJPEGGet16Bits(unsigned char *pBuf, int *iOff);
void JPEGPutMCUGray(PIL_PAGE *inpage, unsigned char *pSrc, unsigned char *cOutput, JPEGDATA *pJPEG, int x, int y, int lsize);
void JPEGPutMCUGray12(PIL_PAGE *inpage, signed short *pSrc, unsigned char *cOutput, int x, int y, int lsize);
void PILStoreCode(PIL_CODE *pPC, uint32_t ulCode, int iLen);
void PILStoreCodeH263(PIL_CODE *pPC, uint32_t ulCode, int iLen);
void PILFlushCode(PIL_CODE *pPC, int bJPEG);
void PILCountGIFPages(PIL_FILE *pFile);
int PILReadGIF(PIL_PAGE *pPage, PIL_FILE *pFile, int iRequestedPage);
int PILAWDStrips(PIL_PAGE *pPage);
void PILIOCAInfo(unsigned char *cBuf, PIL_PAGE *pPage, signed int *iDataOff, signed int *iDataLen, signed int iMaxLen);
int PILReadCALS2Page(PIL_FILE *pFile, PIL_PAGE *pPage, int iRequestedPage);
int PILReadFLC(PIL_PAGE *pPage, PIL_FILE *pFile);
int PILReadRLE(PIL_PAGE *pInPage, PIL_PAGE *pOutPage);
int PILReadCinepak(PIL_PAGE *inpage, PIL_PAGE *outpage, int iOptions);
int PILReadMSVC(PIL_PAGE *inpage, PIL_PAGE *outpage, int iOptions);
int PILReadH263(PIL_PAGE *inpage, PIL_PAGE *outpage, int iOptions);
int PILReadMPEG4(PIL_PAGE *inpage, PIL_PAGE *outpage);
int PILReadMPEG(PIL_PAGE *inpage, PIL_PAGE *outpage, int iOptions);
int PILCountMAGPages(PIL_FILE *pFile);
void PILIOCAPages(PIL_FILE *pFile);
int PILGetPDFInfo(PIL_FILE *pFile);
int PILReadEXIFThumb(PIL_FILE *pFile, PIL_PAGE *pPage, int iOptions);
int PILReadPPMAXThumb(PIL_FILE *pFile, PIL_PAGE *pPage, int iRequestedPage);
int PILReadFITSPage(PIL_FILE *pFile, PIL_PAGE *pPage);
int PILReadOpenEXR(PIL_FILE *pFile, PIL_PAGE *pPage);
int PILReadDDS(PIL_FILE *pFile, PIL_PAGE *pPage);
int PILReadKTX(PIL_FILE *pFile, PIL_PAGE *pPage);
int PILReadPPMAX(PIL_FILE *pFile, PIL_PAGE *pPage, int iRequestedPage);
int PILReadDICOM(PIL_FILE *pFile, PIL_PAGE *pPage, int iRequestedPage, int iDataSize);
int PILPreLoadPNG(PIL_PAGE *pPage, PIL_FILE *pFile, int iRequestedPage);
int PILLoadJEDMICS(PIL_PAGE *pPage);
int PILReadPDFPage(PIL_FILE *pFile, PIL_PAGE *pPage, unsigned char *pData, int iRequestedPage);
int PILGetTargaMap(PIL_PAGE *pPage, unsigned char *pData);
int PILGetTarga(PIL_PAGE *pPage);
int PILGetTargaC(PIL_PAGE *pPage, PIL_FILE *pFile);
int PILLoadFax(unsigned char cFileType, PIL_PAGE *pPage);
int PILReadPSEG(PIL_FILE *pFile, PIL_PAGE *pPage);
int PILAdjustColors(PIL_PAGE *inpage, int iDestBpp, int iOptions, int bAllocate);
int PILAdjustClr2(PIL_PAGE *inpage, int iDestBpp);
void JPEGGetMCU22(unsigned char *pImage, PIL_PAGE *pPage, int lsize, int x, int y, signed short *pMCUData, int bSIMD);
void H263GetMCU22(unsigned char *pImage, PIL_PAGE *pPage, int lsize, int x, int y, signed short *pMCUData);
//void MPEGReadData(MPEGDATA *pMPEG, int bVideo);
int PILGetSFFPages(PIL_FILE *pFile, int iPageOffset);
int PILGetDICOMInfo(PIL_FILE *pFile);
void PILCountMAXPages(PIL_FILE *pFile);
void PILCountPNGFrames(PIL_FILE *pFile);
void PILCountAVMPages(PIL_FILE *pFile);
void PILCountAWDPages(PIL_FILE *pFile);
//void MPEGSwapFrames(MPEGDATA *pMPEG);
int ASCII85Decode(unsigned char *buf, int iLen);
//MPEGDATA * PILPrepMPEGStruct(void);
int PILReadBMP(PIL_PAGE *pInPage, PIL_PAGE *pOutPage);
int PILReadJBIG(PIL_PAGE *pInPage, PIL_PAGE *pOutPage);
int PILMakeJBIG(PIL_PAGE *pInPage, PIL_PAGE *pOutPage);
int PILGetFITSInfo(PIL_FILE *pFile);
int PILSecurity(TCHAR *szCompany, uint32_t ulKey);
void PILTIFFHoriz(PIL_PAGE *InPage, int bDecode);
int PILTIFFMiniInfo(PIL_FILE *pFile, int bMotorola, int iOffset, int bJPEG);
void PILCalcDICOMRange(PIL_PAGE *pPage);
int PILMakeTIFFHeader(PIL_PAGE *pPage, unsigned char *pHeader, int *iHeaderLen, int iIFD, int iOptions);
void PILMultiWrite(void * iHandle, unsigned char *pOut, int *Offset, unsigned char *pBuf, int iLen);
void PILConvertCMYK(PIL_PAGE *pPage);
//DWORD WINAPI PILReadThread(LPVOID pInPage);
int X64LZWCopyBytes(unsigned char *buf, int iOffset, int iUncompressedLen, uint32_t *pSymbols);
unsigned char PILPAETH(unsigned char a, unsigned char b, unsigned char c);
void PILDecodeJPEGSlice(void *pInStruct);
void PILDecodeJPEGThumbSlice(void *pInStruct);
extern int PILFixYCbCr(PIL_PAGE *pPage);
extern void JPEGPixel2_A(JPEGDATA *pJPEG, unsigned char *pDest, int x, int iY1, int iY2, int iCb, int iCr);
extern void PILGetEXIFInfo(unsigned char *buf, int iEXIF1, int iEXIF2, int bMotorola, PIL_PAGE *outpage);
extern void PILFreeHuffTables(JPEGDATA *pJPEG);
extern int PILMakeJPEG(PIL_PAGE *pPage, PIL_PAGE *pOutPage, int iQFactor);
extern int PILReadFlate(PIL_PAGE *pInPage, PIL_PAGE *pOutPage, int iOptions);
    extern int PILMakeFlate(PIL_PAGE *pInPage, PIL_PAGE *pOutPage, int iOptions);
    extern int PILMakePCX(PIL_PAGE *pInPage, PIL_PAGE *pOutPage);
    extern int PILMakePackBits(PIL_PAGE *pInPage, PIL_PAGE *pOutPage);
    extern int PILMakePCL(PIL_PAGE *pInPage, PIL_PAGE *pOutPage);
    extern int PILMakeRLE(PIL_PAGE *pInPage, PIL_PAGE *pOutPage);
    extern int PILReadThunderscan(PIL_PAGE *pInPage, PIL_PAGE *pOutPage, int iOptions);
    extern int PILReadDICOMRLE(PIL_PAGE *pInPage, PIL_PAGE *pOutPage, int iOptions);
    extern int PILReadPCX(PIL_PAGE *pInPage, PIL_PAGE *pOutPage);
    extern int PILMakeLZW(PIL_PAGE *pInPage, PIL_PAGE *pOutPage, int iOptions);
    extern int PILMakeGIFLZW(PIL_PAGE *pInPage, PIL_PAGE *pOutPage, int iOptions);
    extern int PILReadLZW(PIL_PAGE *pInPage, PIL_PAGE *pOutPage, int bGIF, int iOptions);
    extern int PILMedianCut(PIL_PAGE *pInPage, PIL_PAGE *pOutPage, int iDestBpp);
// External SIMD versions of functions
extern int JPEGSimpleFilter_SIMD(PIL_PAGE *inpage, int iOffset, JPEGDATA *pJPEG, JPEG_SLICE *pJPEGSlices, int iNumThreads, int *iNumSlices, unsigned char *pOut, int bThumbnail);
extern void JPEGPutMCU22_SIMD(PIL_PAGE *inpage, int x, int y, int lsize, signed short *pMCU, unsigned char *cOutput, JPEGDATA *pJPEG);
extern int PNGDeFilter_SIMD(int i, unsigned char *pOut, int *piOffset, int *piOut, int iIn, int iPitch);
extern void Scale2Gray_SIMD(unsigned char *source, unsigned char *dest, int width, unsigned char ucBpp);
extern void PILFixTIFFRGB_SIMD(unsigned char *p, PIL_PAGE *pPage);
extern void PILTIFFHoriz_SIMD(PIL_PAGE *InPage, int bDecode);
extern void PIL24To16SIMD(unsigned char *pSrc, unsigned short *pDest, int iWidth);
extern void PIL24To32SIMD(unsigned char *pSrc, unsigned char *pDest, int iWidth);
extern void PIL32To16SIMD(unsigned char *pSrc, unsigned short *pDest, int iWidth);

static const char szCopyright[] = "Copyright 2001-2015 BitBank Software, Inc.";
static const char szPILName [] = "PIL - Portable Imaging Libary 1.1";

uint32_t ulTraceCount = 0;

//
// VGA default palette
//
unsigned char vga_pal[48] = {0,0,0, 0,0,128, 0,128,0, 0,128,128, 128,0,0, 128,0,128,
                   128,128,0, 128,128,128, 192,192,192, 0,0,255, 0,255,0,
                   0,255,255, 255,0,0, 255,0,255, 255,255,0, 255,255,255};
// 3-3-2 palette
// Blue 8-bit palette values
unsigned char b_pal[4] = {00,0x55,0xaa,0xff};
// Red and Green 8-bit palette values
unsigned char rg_pal[8] = {00,0x24,0x49,0x6d,0x92,0xb6,0xdb,0xff};

#ifdef USE_VIDEO_CODECS
// fractional values stored * 1000
int iMPEGFPS[16] = {0,23976,24000,25000,29970,30000,50000,59940,60000,0,0,0,0,0,0,0};
#endif // USE_VIDEO_CODECS

// Scale to gray conversion table
// Groups of 2x2 pixels are converted to a gray level from the default VGA
// palette.  The quantity of black pixels determines the level
// for this purpose, black=0, gray1=4, gray2=8, gray3=7, gray4=F
// The byte translated is for two adjacent pixels merged from two lines to one.
//
// The input byte is the first 4 bw bits of the first line (upper nibble)
// followed by the 4 bw bits of the line below it (lower nibble)
//
uint32_t ulGrays32[16] = {0xff000000, 0xff444444, 0xff444444, 0xff888888,0xff444444, 0xff888888,0xff888888,0xffcccccc,
                              0xff444444, 0xff888888, 0xff888888, 0xffcccccc,0xf8888888, 0xffcccccc,0xffcccccc,0xffffffff};
#ifdef GTK_INVERTED
    unsigned char ucGrays[256]={0xff,0xfb,0xfb,0xf7,0xbf,0xbb,0xbb,0xb7,0xbf,0xbb,0xbb,0xb7,0x7f,0x7b,0x7b,0x77,              // 0-15
        0xfb,0xf7,0xf7,0xf8,0xbb,0xb7,0xb7,0xb8,0xbb,0xb7,0xb7,0xb8,0x7b,0x77,0x77,0x78,              // 16-31
        0xfb,0xf7,0xf7,0xf8,0xbb,0xb7,0xb7,0xb8,0xbb,0xb7,0xb7,0xb8,0x7b,0x77,0x77,0x78,              // 32-47
        0xf7,0xf8,0xf8,0xf0,0xb7,0xb8,0xb8,0xb0,0xb7,0xb8,0xb8,0xb0,0x77,0x78,0x78,0x70,           // 48-63
        0xbf,0xbb,0xbb,0xb7,0x7f,0x7b,0x7b,0x77,0x7f,0x7b,0x7b,0x77,0x8f,0x8b,0x8b,0x87,  // 64-79
        0xbb,0xb7,0xb7,0xb8,0x7b,0x77,0x77,0x78,0x7b,0x77,0x77,0x78,0x8b,0x87,0x87,0x88,  // 80-95
        0xbb,0xb7,0xb7,0xb8,0x7b,0x77,0x77,0x78,0x7b,0x77,0x77,0x78,0x8b,0x87,0x87,0x88,  // 96-111
        0xb7,0xb8,0xb8,0xb0,0x77,0x78,0x78,0x70,0x77,0x78,0x78,0x70,0x87,0x88,0x88,0x80,  // 112-127
        0xbf,0xbb,0xbb,0xb7,0x7f,0x7b,0x7b,0x77,0x7f,0x7b,0x7b,0x77,0x8f,0x8b,0x8b,0x87,  // 128-143
        0xbb,0xb7,0xb7,0xb8,0x7b,0x77,0x77,0x78,0x7b,0x77,0x77,0x78,0x8b,0x87,0x87,0x88,  // 144-159
        0xbb,0xb7,0xb7,0xb8,0x7b,0x77,0x77,0x78,0x7b,0x77,0x77,0x78,0x8b,0x87,0x87,0x88,  // 160-175
        0xb7,0xb8,0xb8,0xb0,0x77,0x78,0x78,0x70,0x77,0x78,0x78,0x70,0x87,0x88,0x88,0x80,  // 176-191
        0x7f,0x7b,0x7b,0x77,0x8f,0x8b,0x8b,0x87,0x8f,0x8b,0x8b,0x87,0x0f,0x0b,0x0b,0x07,  // 192-207
        0x7b,0x77,0x77,0x78,0x8b,0x87,0x87,0x88,0x8b,0x87,0x87,0x88,0x0b,0x07,0x07,0x08,  // 208-223
        0x7b,0x77,0x77,0x78,0x8b,0x87,0x87,0x88,0x8b,0x87,0x87,0x88,0x0b,0x07,0x07,0x08,  // 224-239
        0x77,0x78,0x78,0x70,0x87,0x88,0x88,0x80,0x87,0x88,0x88,0x80,0x07,0x08,0x08,0x00}; // 240-255
#else
unsigned char ucGrays[256]={0,4,4,8,0x40,0x44,0x44,0x48,0x40,0x44,0x44,0x48,0x80,0x84,0x84,0x88,              // 0-15
                            4,8,8,7,0x44,0x48,0x48,0x47,0x44,0x48,0x48,0x47,0x84,0x88,0x88,0x87,              // 16-31
                            4,8,8,7,0x44,0x48,0x48,0x47,0x44,0x48,0x48,0x47,0x84,0x88,0x88,0x87,              // 32-47
                            8,7,7,0x0f,0x48,0x47,0x47,0x4f,0x48,0x47,0x47,0x4f,0x88,0x87,0x87,0x8f,           // 48-63
                            0x40,0x44,0x44,0x48,0x80,0x84,0x84,0x88,0x80,0x84,0x84,0x88,0x70,0x74,0x74,0x78,  // 64-79
                            0x44,0x48,0x48,0x47,0x84,0x88,0x88,0x87,0x84,0x88,0x88,0x87,0x74,0x78,0x78,0x77,  // 80-95
                            0x44,0x48,0x48,0x47,0x84,0x88,0x88,0x87,0x84,0x88,0x88,0x87,0x74,0x78,0x78,0x77,  // 96-111
                            0x48,0x47,0x47,0x4f,0x88,0x87,0x87,0x8f,0x88,0x87,0x87,0x8f,0x78,0x77,0x77,0x7f,  // 112-127
                            0x40,0x44,0x44,0x48,0x80,0x84,0x84,0x88,0x80,0x84,0x84,0x88,0x70,0x74,0x74,0x78,  // 128-143
                            0x44,0x48,0x48,0x47,0x84,0x88,0x88,0x87,0x84,0x88,0x88,0x87,0x74,0x78,0x78,0x77,  // 144-159
                            0x44,0x48,0x48,0x47,0x84,0x88,0x88,0x87,0x84,0x88,0x88,0x87,0x74,0x78,0x78,0x77,  // 160-175
                            0x48,0x47,0x47,0x4f,0x88,0x87,0x87,0x8f,0x88,0x87,0x87,0x8f,0x78,0x77,0x77,0x7f,  // 176-191
                            0x80,0x84,0x84,0x88,0x70,0x74,0x74,0x78,0x70,0x74,0x74,0x78,0xf0,0xf4,0xf4,0xf8,  // 192-207
                            0x84,0x88,0x88,0x87,0x74,0x78,0x78,0x77,0x74,0x78,0x78,0x77,0xf4,0xf8,0xf8,0xf7,  // 208-223
                            0x84,0x88,0x88,0x87,0x74,0x78,0x78,0x77,0x74,0x78,0x78,0x77,0xf4,0xf8,0xf8,0xf7,  // 224-239
                            0x88,0x87,0x87,0x8f,0x78,0x77,0x77,0x7f,0x78,0x77,0x77,0x7f,0xf8,0xf7,0xf7,0xff}; // 240-255
#endif // GTK_INVERTED
uint32_t ulGrays[256]={0x00000000,0x42080000,0x42080000,0x84100000,0x00004208,0x42084208,0x42084208,0x84104208,
       	  	 					 0x00004208,0x42084208,0x42084208,0x84104208,0x00008410,0x42088410,0x42088410,0x84108410,              // 0-15
                            0x42080000,0x84100000,0x84100000,0xc6180000,0x42084208,0x84104208,0x84104208,0xc6184208,
                            0x42084208,0x84104208,0x84104208,0xc6184208,0x42088410,0x84108410,0x84108410,0xc6188410,              // 16-31
                            0x42080000,0x84100000,0x84100000,0xc6180000,0x42084208,0x84104208,0x84104208,0xc6184208,
                            0x42084208,0x84104208,0x84104208,0xc6184208,0x42088410,0x84108410,0x84108410,0xc6188410,              // 32-47
                            0x84100000,0xc6180000,0xc6180000,0xffff0000,0x84104208,0xc6184208,0xc6184208,0xffff4208,
                            0x84104208,0xc6184208,0xc6184208,0xffff4208,0x84108410,0xc6188410,0xc6188410,0xffff8410,           // 48-63
                            0x00004208,0x42084208,0x42084208,0x84104208,0x00008410,0x42088410,0x42088410,0x84108410,
                            0x00008410,0x42088410,0x42088410,0x84108410,0x0000c618,0x4208c618,0x4208c618,0x8410c618,  // 64-79
                            0x42084208,0x84104208,0x84104208,0xc6184208,0x42088410,0x84108410,0x84108410,0xc6188410,
                            0x42088410,0x84108410,0x84108410,0xc6188410,0x4208c618,0x8410c618,0x8410c618,0xc618c618,  // 80-95
                            0x42084208,0x84104208,0x84104208,0xc6184208,0x42088410,0x84108410,0x84108410,0xc6188410,
                            0x42088410,0x84108410,0x84108410,0xc6188410,0x4208c618,0x8410c618,0x8410c618,0xc618c618,  // 96-111
                            0x84104208,0xc6184208,0xc6184208,0xffff4208,0x84108410,0xc6188410,0xc6188410,0xffff8410,
                            0x84108410,0xc6188410,0xc6188410,0xffff8410,0x8410c618,0xc618c618,0xc618c618,0xffffc618,  // 112-127
                            0x00004208,0x42084208,0x42084208,0x84104208,0x00008410,0x42088410,0x42088410,0x84108410,
                            0x00008410,0x42088410,0x42088410,0x84108410,0x0000c618,0x4208c618,0x4208c618,0x8410c618,  // 128-143
                            0x42084208,0x84104208,0x84104208,0xc6184208,0x42088410,0x84108410,0x84108410,0xc6188410,
                            0x42088410,0x84108410,0x84108410,0xc6188410,0x4208c618,0x8410c618,0x8410c618,0xc618c618,  // 144-159
                            0x42084208,0x84104208,0x84104208,0xc6184208,0x42088410,0x84108410,0x84108410,0xc6188410,
                            0x42088410,0x84108410,0x84108410,0xc6188410,0x4208c618,0x8410c618,0x8410c618,0xc618c618,  // 160-175
                            0x84104208,0xc6184208,0xc6184208,0xffff4208,0x84108410,0xc6188410,0xc6188410,0xffff8410,
                            0x84108410,0xc6188410,0xc6188410,0xffff8410,0x8410c618,0xc618c618,0xc618c618,0xffffc618,  // 176-191
                            0x00008410,0x42088410,0x42088410,0x84108410,0x0000c618,0x4208c618,0x4208c618,0x8410c618,
                            0x0000c618,0x4208c618,0x4208c618,0x8410c618,0x0000ffff,0x4208ffff,0x4208ffff,0x8410ffff,  // 192-207
                            0x42088410,0x84108410,0x84108410,0xc6188410,0x4208c618,0x8410c618,0x8410c618,0xc618c618,
                            0x4208c618,0x8410c618,0x8410c618,0xc618c618,0x4208ffff,0x8410ffff,0x8410ffff,0xc618ffff,  // 208-223
                            0x42088410,0x84108410,0x84108410,0xc6188410,0x4208c618,0x8410c618,0x8410c618,0xc618c618,
                            0x4208c618,0x8410c618,0x8410c618,0xc618c618,0x4208ffff,0x8410ffff,0x8410ffff,0xc618ffff,  // 224-239
                            0x84108410,0xc6188410,0xc6188410,0xffff8410,0x8410c618,0xc618c618,0xc618c618,0xffffc618,
                            0x8410c618,0xc618c618,0xc618c618,0xffffc618,0x8410ffff,0xc618ffff,0xc618ffff,0xffffffff}; // 240-255

/* Table of byte flip values to mirror-image incoming CCITT data */
unsigned char ucMirror[256]=
     {0, 128, 64, 192, 32, 160, 96, 224, 16, 144, 80, 208, 48, 176, 112, 240,
      8, 136, 72, 200, 40, 168, 104, 232, 24, 152, 88, 216, 56, 184, 120, 248,
      4, 132, 68, 196, 36, 164, 100, 228, 20, 148, 84, 212, 52, 180, 116, 244,
      12, 140, 76, 204, 44, 172, 108, 236, 28, 156, 92, 220, 60, 188, 124, 252,
      2, 130, 66, 194, 34, 162, 98, 226, 18, 146, 82, 210, 50, 178, 114, 242,
      10, 138, 74, 202, 42, 170, 106, 234, 26, 154, 90, 218, 58, 186, 122, 250,
      6, 134, 70, 198, 38, 166, 102, 230, 22, 150, 86, 214, 54, 182, 118, 246,
      14, 142, 78, 206, 46, 174, 110, 238, 30, 158, 94, 222, 62, 190, 126, 254,
      1, 129, 65, 193, 33, 161, 97, 225, 17, 145, 81, 209, 49, 177, 113, 241,
      9, 137, 73, 201, 41, 169, 105, 233, 25, 153, 89, 217, 57, 185, 121, 249,
      5, 133, 69, 197, 37, 165, 101, 229, 21, 149, 85, 213, 53, 181, 117, 245,
      13, 141, 77, 205, 45, 173, 109, 237, 29, 157, 93, 221, 61, 189, 125, 253,
      3, 131, 67, 195, 35, 163, 99, 227, 19, 147, 83, 211, 51, 179, 115, 243,
      11, 139, 75, 203, 43, 171, 107, 235, 27, 155, 91, 219, 59, 187, 123, 251,
      7, 135, 71, 199, 39, 167, 103, 231, 23, 151, 87, 215, 55, 183, 119, 247,
      15, 143, 79, 207, 47, 175, 111, 239, 31, 159, 95, 223, 63, 191, 127, 255};


/* Number of consecutive 1 bits in a byte from MSB to LSB */
char bitcount[256] =
        {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,  /* 0-15 */
         0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,  /* 16-31 */
         0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,  /* 32-47 */
         0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,  /* 48-63 */
         0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,  /* 64-79 */
         0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,  /* 80-95 */
         0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,  /* 96-111 */
         0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,  /* 112-127 */
         1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,  /* 128-143 */
         1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,  /* 144-159 */
         1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,  /* 160-175 */
         1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,  /* 176-191 */
         2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,  /* 192-207 */
         2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,  /* 208-223 */
         3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,  /* 224-239 */
         4,4,4,4,4,4,4,4,5,5,5,5,6,6,7,8}; /* 240-255 */
/*
 The code tree that follows has: bit_length, decode routine
 These codes are for Group 4 (MMR) decoding

 01 = vertneg1, 11h = vert1, 20h = horiz, 30h = pass, 12h = vert2
 02 = vertneg2, 13h = vert3, 03 = vertneg3, 90h = trash
*/

unsigned char code_table[128] =
        {0x90, 0, 0x40, 0,       /* trash, uncompr mode - codes 0 and 1 */
         3, 7,                   /* V(-3) pos = 2 */
         0x13, 7,                /* V(3)  pos = 3 */
         2, 6, 2, 6,             /* V(-2) pos = 4,5 */
         0x12, 6, 0x12, 6,       /* V(2)  pos = 6,7 */
         0x30, 4, 0x30, 4, 0x30, 4, 0x30, 4,    /* pass  pos = 8->F */
         0x30, 4, 0x30, 4, 0x30, 4, 0x30, 4,
         0x20, 3, 0x20, 3, 0x20, 3, 0x20, 3,    /* horiz pos = 10->1F */
         0x20, 3, 0x20, 3, 0x20, 3, 0x20, 3,
         0x20, 3, 0x20, 3, 0x20, 3, 0x20, 3,
         0x20, 3, 0x20, 3, 0x20, 3, 0x20, 3,
/* V(-1) pos = 20->2F */
         1, 3, 1, 3, 1, 3, 1, 3,
         1, 3, 1, 3, 1, 3, 1, 3,
         1, 3, 1, 3, 1, 3, 1, 3,
         1, 3, 1, 3, 1, 3, 1, 3,
         0x11, 3, 0x11, 3, 0x11, 3, 0x11, 3,   /* V(1)   pos = 30->3F */
         0x11, 3, 0x11, 3, 0x11, 3, 0x11, 3,
         0x11, 3, 0x11, 3, 0x11, 3, 0x11, 3,
         0x11, 3, 0x11, 3, 0x11, 3, 0x11, 3};

/*
 Here are the Huffman address codes for run lengths
 first the short white codes (first 4 bits != 0)
*/
short white_s[1024] =
        {-1,-1,-1,-1,-1,-1,-1,-1,8,29,8,29,8,30,8,30,
        8,45,8,45,8,46,8,46,7,22,7,22,7,22,7,22,
        7,23,7,23,7,23,7,23,8,47,8,47,8,48,8,48,
        6,13,6,13,6,13,6,13,6,13,6,13,6,13,6,13,
        7,20,7,20,7,20,7,20,8,33,8,33,8,34,8,34,
        8,35,8,35,8,36,8,36,8,37,8,37,8,38,8,38,
        7,19,7,19,7,19,7,19,8,31,8,31,8,32,8,32,
        6,1,6,1,6,1,6,1,6,1,6,1,6,1,6,1,
        6,12,6,12,6,12,6,12,6,12,6,12,6,12,6,12,
        8,53,8,53,8,54,8,54,7,26,7,26,7,26,7,26,
        8,39,8,39,8,40,8,40,8,41,8,41,8,42,8,42,
        8,43,8,43,8,44,8,44,7,21,7,21,7,21,7,21,
        7,28,7,28,7,28,7,28,8,61,8,61,8,62,8,62,
        8,63,8,63,8,0,8,0,8,320,8,320,8,384,8,384,
        5,10,5,10,5,10,5,10,5,10,5,10,5,10,5,10,
        5,10,5,10,5,10,5,10,5,10,5,10,5,10,5,10,
        5,11,5,11,5,11,5,11,5,11,5,11,5,11,5,11,
        5,11,5,11,5,11,5,11,5,11,5,11,5,11,5,11,
        7,27,7,27,7,27,7,27,8,59,8,59,8,60,8,60,
        9,1472,9,1536,9,1600,9,1728,7,18,7,18,7,18,7,18,
        7,24,7,24,7,24,7,24,8,49,8,49,8,50,8,50,
        8,51,8,51,8,52,8,52,7,25,7,25,7,25,7,25,
        8,55,8,55,8,56,8,56,8,57,8,57,8,58,8,58,
        6,192,6,192,6,192,6,192,6,192,6,192,6,192,6,192,
        6,1664,6,1664,6,1664,6,1664,6,1664,6,1664,6,1664,6,1664,
        8,448,8,448,8,512,8,512,9,704,9,768,8,640,8,640,
        8,576,8,576,9,832,9,896,9,960,9,1024,9,1088,9,1152,
        9,1216,9,1280,9,1344,9,1408,7,256,7,256,7,256,7,256,
        4,2,4,2,4,2,4,2,4,2,4,2,4,2,4,2,
        4,2,4,2,4,2,4,2,4,2,4,2,4,2,4,2,
        4,2,4,2,4,2,4,2,4,2,4,2,4,2,4,2,
        4,2,4,2,4,2,4,2,4,2,4,2,4,2,4,2,
        4,3,4,3,4,3,4,3,4,3,4,3,4,3,4,3,
        4,3,4,3,4,3,4,3,4,3,4,3,4,3,4,3,
        4,3,4,3,4,3,4,3,4,3,4,3,4,3,4,3,
        4,3,4,3,4,3,4,3,4,3,4,3,4,3,4,3,
        5,128,5,128,5,128,5,128,5,128,5,128,5,128,5,128,
        5,128,5,128,5,128,5,128,5,128,5,128,5,128,5,128,
        5,8,5,8,5,8,5,8,5,8,5,8,5,8,5,8,
        5,8,5,8,5,8,5,8,5,8,5,8,5,8,5,8,
        5,9,5,9,5,9,5,9,5,9,5,9,5,9,5,9,
        5,9,5,9,5,9,5,9,5,9,5,9,5,9,5,9,
        6,16,6,16,6,16,6,16,6,16,6,16,6,16,6,16,
        6,17,6,17,6,17,6,17,6,17,6,17,6,17,6,17,
        4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,
        4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,
        4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,
        4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,
        4,5,4,5,4,5,4,5,4,5,4,5,4,5,4,5,
        4,5,4,5,4,5,4,5,4,5,4,5,4,5,4,5,
        4,5,4,5,4,5,4,5,4,5,4,5,4,5,4,5,
        4,5,4,5,4,5,4,5,4,5,4,5,4,5,4,5,
        6,14,6,14,6,14,6,14,6,14,6,14,6,14,6,14,
        6,15,6,15,6,15,6,15,6,15,6,15,6,15,6,15,
        5,64,5,64,5,64,5,64,5,64,5,64,5,64,5,64,
        5,64,5,64,5,64,5,64,5,64,5,64,5,64,5,64,
        4,6,4,6,4,6,4,6,4,6,4,6,4,6,4,6,
        4,6,4,6,4,6,4,6,4,6,4,6,4,6,4,6,
        4,6,4,6,4,6,4,6,4,6,4,6,4,6,4,6,
        4,6,4,6,4,6,4,6,4,6,4,6,4,6,4,6,
        4,7,4,7,4,7,4,7,4,7,4,7,4,7,4,7,
        4,7,4,7,4,7,4,7,4,7,4,7,4,7,4,7,
        4,7,4,7,4,7,4,7,4,7,4,7,4,7,4,7,
        4,7,4,7,4,7,4,7,4,7,4,7,4,7,4,7};
uint32_t *pWhite_S_32 = (uint32_t *) &white_s[0];

/* the short black codes (first 4 bits != 0) */
short black_s[128] =
       {-1,-1,-1,-1,-1,-1,-1,-1,6,9,6,8,5,7,5,7,
        4,6,4,6,4,6,4,6,4,5,4,5,4,5,4,5,
        3,1,3,1,3,1,3,1,3,1,3,1,3,1,3,1,
        3,4,3,4,3,4,3,4,3,4,3,4,3,4,3,4,
        2,3,2,3,2,3,2,3,2,3,2,3,2,3,2,3,
        2,3,2,3,2,3,2,3,2,3,2,3,2,3,2,3,
        2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,
        2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2};
uint32_t *pBlack_S_32 = (uint32_t *)&black_s[0];

/* The long black codes (first 4 bits == 0) */
#define EOL -9999   /* End of line */
#define EO1D -9998  /* End of 1D coding */
short black_l[1024] =
    {1,0,1,0,12,EOL,12,EOL,1,-1,1,-1,1,-1,1,-1,
     1,-1,1,-1,1,-1,1,-1,1,-1,1,-1,1,-1,1,-1,
     1,-1,1,-1,1,-1,1,-1,1,-1,1,-1,12,EO1D,12,EO1D,
     1,-1,1,-1,1,-1,1,-1,1,-1,1,-1,1,-1,1,-1,
     11,1792,11,1792,11,1792,11,1792,12,1984,12,1984,12,2048,12,2048,
     12,2112,12,2112,12,2176,12,2176,12,2240,12,2240,12,2304,12,2304,
     11,1856,11,1856,11,1856,11,1856,11,1920,11,1920,11,1920,11,1920,
     12,2368,12,2368,12,2432,12,2432,12,2496,12,2496,12,2560,12,2560,
     10,18,10,18,10,18,10,18,10,18,10,18,10,18,10,18,
     12,52,12,52,13,640,13,704,13,768,13,832,12,55,12,55,
     12,56,12,56,13,1280,13,1344,13,1408,13,1472,12,59,12,59,
     12,60,12,60,13,1536,13,1600,11,24,11,24,11,24,11,24,
     11,25,11,25,11,25,11,25,13,1664,13,1728,12,320,12,320,
     12,384,12,384,12,448,12,448,13,512,13,576,12,53,12,53,
     12,54,12,54,13,896,13,960,13,1024,13,1088,13,1152,13,1216,
     10,64,10,64,10,64,10,64,10,64,10,64,10,64,10,64,
     8,13,8,13,8,13,8,13,8,13,8,13,8,13,8,13,
     8,13,8,13,8,13,8,13,8,13,8,13,8,13,8,13,
     8,13,8,13,8,13,8,13,8,13,8,13,8,13,8,13,
     8,13,8,13,8,13,8,13,8,13,8,13,8,13,8,13,
     11,23,11,23,11,23,11,23,12,50,12,50,12,51,12,51,
     12,44,12,44,12,45,12,45,12,46,12,46,12,47,12,47,
     12,57,12,57,12,58,12,58,12,61,12,61,12,256,12,256,
     10,16,10,16,10,16,10,16,10,16,10,16,10,16,10,16,
     10,17,10,17,10,17,10,17,10,17,10,17,10,17,10,17,
     12,48,12,48,12,49,12,49,12,62,12,62,12,63,12,63,
     12,30,12,30,12,31,12,31,12,32,12,32,12,33,12,33,
     12,40,12,40,12,41,12,41,11,22,11,22,11,22,11,22,
     8,14,8,14,8,14,8,14,8,14,8,14,8,14,8,14,
     8,14,8,14,8,14,8,14,8,14,8,14,8,14,8,14,
     8,14,8,14,8,14,8,14,8,14,8,14,8,14,8,14,
     8,14,8,14,8,14,8,14,8,14,8,14,8,14,8,14,
     7,10,7,10,7,10,7,10,7,10,7,10,7,10,7,10,
     7,10,7,10,7,10,7,10,7,10,7,10,7,10,7,10,
     7,10,7,10,7,10,7,10,7,10,7,10,7,10,7,10,
     7,10,7,10,7,10,7,10,7,10,7,10,7,10,7,10,
     7,10,7,10,7,10,7,10,7,10,7,10,7,10,7,10,
     7,10,7,10,7,10,7,10,7,10,7,10,7,10,7,10,
     7,10,7,10,7,10,7,10,7,10,7,10,7,10,7,10,
     7,10,7,10,7,10,7,10,7,10,7,10,7,10,7,10,
     7,11,7,11,7,11,7,11,7,11,7,11,7,11,7,11,
     7,11,7,11,7,11,7,11,7,11,7,11,7,11,7,11,
     7,11,7,11,7,11,7,11,7,11,7,11,7,11,7,11,
     7,11,7,11,7,11,7,11,7,11,7,11,7,11,7,11,
     7,11,7,11,7,11,7,11,7,11,7,11,7,11,7,11,
     7,11,7,11,7,11,7,11,7,11,7,11,7,11,7,11,
     7,11,7,11,7,11,7,11,7,11,7,11,7,11,7,11,
     7,11,7,11,7,11,7,11,7,11,7,11,7,11,7,11,
     9,15,9,15,9,15,9,15,9,15,9,15,9,15,9,15,
     9,15,9,15,9,15,9,15,9,15,9,15,9,15,9,15,
     12,128,12,128,12,192,12,192,12,26,12,26,12,27,12,27,
     12,28,12,28,12,29,12,29,11,19,11,19,11,19,11,19,
     11,20,11,20,11,20,11,20,12,34,12,34,12,35,12,35,
     12,36,12,36,12,37,12,37,12,38,12,38,12,39,12,39,
     11,21,11,21,11,21,11,21,12,42,12,42,12,43,12,43,
     10,0,10,0,10,0,10,0,10,0,10,0,10,0,10,0,
     7,12,7,12,7,12,7,12,7,12,7,12,7,12,7,12,
     7,12,7,12,7,12,7,12,7,12,7,12,7,12,7,12,
     7,12,7,12,7,12,7,12,7,12,7,12,7,12,7,12,
     7,12,7,12,7,12,7,12,7,12,7,12,7,12,7,12,
     7,12,7,12,7,12,7,12,7,12,7,12,7,12,7,12,
     7,12,7,12,7,12,7,12,7,12,7,12,7,12,7,12,
     7,12,7,12,7,12,7,12,7,12,7,12,7,12,7,12,
     7,12,7,12,7,12,7,12,7,12,7,12,7,12,7,12};
uint32_t *pBlack_L_32 = (uint32_t *)&black_l[0];


/* Table of vertical codes for G4 encoding */
/* code followed by length, starting with v(-3) */
int vtable[14] =
        {3,7,     /* V(-3) = 0000011 */
         3,6,     /* V(-2) = 000011  */
         3,3,     /* V(-1) = 011     */
         1,1,     /* V(0)  = 1       */
         2,3,     /* V(1)  = 010     */
         2,6,     /* V(2)  = 000010  */
         2,7};    /* V(3)  = 0000010 */


/* Group 3 Huffman codes ordered for MH encoding */
/* first, the terminating codes for white (code, length) */
short huff_white[128] =
        {0x35,8,7,6,7,4,8,4,0xb,4, /* 0,1,2,3,4 */
         0xc,4,0xe,4,0xf,4,0x13,5,0x14,5,7,5,8,5, /* 5,6,7,8,9,10,11 */
         8,6,3,6,0x34,6,0x35,6,0x2a,6,0x2b,6,0x27,7, /* 12,13,14,15,16,17,18 */
         0xc,7,8,7,0x17,7,3,7,4,7,0x28,7,0x2b,7, /* 19,20,21,22,23,24,25 */
         0x13,7,0x24,7,0x18,7,2,8,3,8,0x1a,8,0x1b,8, /* 26,27,28,29,30,31,32 */
         0x12,8,0x13,8,0x14,8,0x15,8,0x16,8,0x17,8,0x28,8, /* 33,34,35,36,37,38,39 */
         0x29,8,0x2a,8,0x2b,8,0x2c,8,0x2d,8,4,8,5,8, /* 40,41,42,43,44,45,46 */
         0xa,8,0xb,8,0x52,8,0x53,8,0x54,8,0x55,8,0x24,8, /* 47,48,49,50,51,52,53 */
         0x25,8,0x58,8,0x59,8,0x5a,8,0x5b,8,0x4a,8,0x4b,8, /* 54,55,56,57,58,59,60 */
         0x32,8,0x33,8,0x34,8};                        /* 61,62,63 */

/* now the white make-up codes */
short huff_wmuc[82] =
       {0,0,0x1b,5,0x12,5,0x17,6,0x37,7,0x36,8,   /* null,64,128,192,256,320 */
        0x37,8,0x64,8,0x65,8,0x68,8,0x67,8,0xcc,9, /* 384,448,512,576,640,704 */
        0xcd,9,0xd2,9,0xd3,9,0xd4,9,0xd5,9,    /* 768,832,896,960,1024 */
        0xd6,9,0xd7,9,0xd8,9,0xd9,9,0xda,9,    /* 1088,1152,1216,1280,1344 */
        0xdb,9,0x98,9,0x99,9,0x9a,9,0x18,6,    /* 1408,1472,1536,1600,1664 */
        0x9b,9,8,11,0xc,11,0xd,11,0x12,12,     /* 1728,1792,1856,1920,1984 */
        0x13,12,0x14,12,0x15,12,0x16,12,0x17,12, /* 2048,2112,2176,2240,2304 */
        0x1c,12,0x1d,12,0x1e,12,0x1f,12};       /* 2368,2432,2496,2560 */

/* black terminating codes */
short huff_black[128] =
      {0x37,10,2,3,3,2,2,2,3,3,                         /* 0,1,2,3,4 */
       3,4,2,4,3,5,5,6,4,6,4,7,5,7,                     /* 5,6,7,8,9,10,11 */
       7,7,4,8,7,8,0x18,9,0x17,10,0x18,10,8,10,         /* 12,13,14,15,16,17,18 */
       0x67,11,0x68,11,0x6c,11,0x37,11,0x28,11,0x17,11, /* 19,20,21,22,23,24 */
       0x18,11,0xca,12,0xcb,12,0xcc,12,0xcd,12,0x68,12, /* 25,26,27,28,29,30 */
       0x69,12,0x6a,12,0x6b,12,0xd2,12,0xd3,12,0xd4,12, /* 31,32,33,34,35,36 */
       0xd5,12,0xd6,12,0xd7,12,0x6c,12,0x6d,12,0xda,12, /* 37,38,39,40,41,42 */
       0xdb,12,0x54,12,0x55,12,0x56,12,0x57,12,0x64,12, /* 43,44,45,46,47,48 */
       0x65,12,0x52,12,0x53,12,0x24,12,0x37,12,0x38,12, /* 49,50,51,52,53,54 */
       0x27,12,0x28,12,0x58,12,0x59,12,0x2b,12,0x2c,12, /* 55,56,57,58,59,60 */
       0x5a,12,0x66,12,0x67,12};                        /* 61,62,63 */
/* black make up codes */
short huff_bmuc[82] =
       {0,0,0xf,10,0xc8,12,0xc9,12,0x5b,12,0x33,12, /* null,64,128,192,256,320 */
        0x34,12,0x35,12,0x6c,13,0x6d,13,0x4a,13,0x4b,13,   /* 384,448,512,576,640,704 */
        0x4c,13,0x4d,13,0x72,13,0x73,13,0x74,13,0x75,13,   /* 768,832,896,960,1024,1088 */
        0x76,13,0x77,13,0x52,13,0x53,13,0x54,13,0x55,13,   /* 1152,1216,1280,1344,1408,1472 */
        0x5a,13,0x5b,13,0x64,13,0x65,13,8,11,0xc,11,       /* 1536,1600,1664,1728,1792,1856 */
        0xd,11,0x12,12,0x13,12,0x14,12,0x15,12,0x16,12,    /* 1920,1984,2048,2112,2176,2240 */
        0x17,12,0x1c,12,0x1d,12,0x1e,12,0x1f,12};          /* 2304,2368,2432,2496,2560 */


// a 4 entry palette used for 2bpp windows bitmaps "2BP"
unsigned char c2bpppal[12] = {0,0,0,85,85,85,170,170,170,255,255,255};

#define BMP_HEADER_BITFIELDS 54
/* Windows BMP header info (62 bytes) */
unsigned char winbmp[62] =
        {0x42,0x4d,
         0,0,0,0,         /* File size */
         0,0,0,0,0x3e,0,0,0,0x28,0,0,0,
         0,0,0,0, /* Xsize */
         0,0,0,0, /* Ysize */
         1,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,       /* number of planes, bits per pel */
         0,0,0,0,0,0,0,0,0,0xff,0xff,0xff};     /* color palette */

/* OS/2 BMP header info (32 bytes) */
unsigned char os2bmp[32] =
        {0x42,0x4d,
         0,0,0,0,         /* file size */
         0,0,0,0,0x20,0,0,0,   /* xhotspot, yhotspot, offset to data */
         0xc,0,0,0,       /* bitmapinfoheader cbfix */
         0,0,           /* xsize */
         0,0,           /* ysize */
         1,0,1,0,        /* bppel, bpplane */
         0,0,0,0xff,0xff,0xff};    /* rgb0, rgb1 */

// Number of 1s in a byte (popcount)
char setbitcount[256] = {
    0, 1, 1, 2, 1, 2, 2, 3, 1, 2, 2, 3, 2, 3, 3, 4,
    1, 2, 2, 3, 2, 3, 3, 4, 2, 3, 3, 4, 3, 4, 4, 5,
    1, 2, 2, 3, 2, 3, 3, 4, 2, 3, 3, 4, 3, 4, 4, 5,
    2, 3, 3, 4, 3, 4, 4, 5, 3, 4, 4, 5, 4, 5, 5, 6,
    
    1, 2, 2, 3, 2, 3, 3, 4, 2, 3, 3, 4, 3, 4, 4, 5,
    2, 3, 3, 4, 3, 4, 4, 5, 3, 4, 4, 5, 4, 5, 5, 6,
    2, 3, 3, 4, 3, 4, 4, 5, 3, 4, 4, 5, 4, 5, 5, 6,
    3, 4, 4, 5, 4, 5, 5, 6, 4, 5, 5, 6, 5, 6, 6, 7,
    
    1, 2, 2, 3, 2, 3, 3, 4, 2, 3, 3, 4, 3, 4, 4, 5,
    2, 3, 3, 4, 3, 4, 4, 5, 3, 4, 4, 5, 4, 5, 5, 6,
    2, 3, 3, 4, 3, 4, 4, 5, 3, 4, 4, 5, 4, 5, 5, 6,
    3, 4, 4, 5, 4, 5, 5, 6, 4, 5, 5, 6, 5, 6, 6, 7,
    
    2, 3, 3, 4, 3, 4, 4, 5, 3, 4, 4, 5, 4, 5, 5, 6,
    3, 4, 4, 5, 4, 5, 5, 6, 4, 5, 5, 6, 5, 6, 6, 7,
    3, 4, 4, 5, 4, 5, 5, 6, 4, 5, 5, 6, 5, 6, 6, 7,
    4, 5, 5, 6, 5, 6, 6, 7, 5, 6, 6, 7, 6, 7, 7, 8 };

/* CALS header strings */
char *calstrings[] = {"srcdocid: NONE","dstdocid: NONE","txtfilid: NONE",
                     "figid: NONE","srcgph: NONE","doccls: NONE",
                     "rtype: 1","rorient: 000,270","rpelcnt:","rdensity:",
                     "notes: NONE"};

// Conversion of PIL compression types to TIFF
int iTiffComp[] = {0,1,3,3,4,0,0,32773,2,5,0,7,0,0,0,0,0,0,8,0,0,0,0,0,0,0,0,0,0,0,0,0,9};
signed int iSignMasks[15] = {0,1,2,4,8,0x10,0x20,0x40,0x80,0x100,0x200,0x400,0x800,0x1000,0x2000};
int PILCalcBSize(int x, int bpp);
int PILFlipv(PIL_PAGE *pPage);
int PILInvert(PIL_PAGE *pPage);
unsigned char * PILEncodeLine(int xsize, unsigned char *irlcptr, unsigned char *buf);
void PILEncodeLine2(unsigned char *buf, int xsize, int32_t *pDest);
void PILFixTIFFRGB(unsigned char *p, PIL_PAGE *);
void PILFixAlpha(PIL_PAGE *pPage);
unsigned short PILTIFFSHORT(unsigned char *p, int bMotorola);
uint32_t PILTIFFLONG(unsigned char *p, int bMotorola);
int PILFixBitDir(PIL_PAGE *);
void PILDraw1Line(unsigned char *irlcptr, unsigned char *pDest, int Start, int iWidth);
void PILDraw1LineScaled(unsigned char *irlcptr, unsigned char *pDest, int Start, int iWidth, int iScale);
int ParseNumber(unsigned char *buf, int *iOff, int iLength);
int PILReadPNG(PIL_PAGE *pInPage, PIL_PAGE *TempPage, int iOptions);
int PILMakePNG(PIL_PAGE *pInPage, PIL_PAGE *pOutPage, int iOptions);
void PILReadBlock(PIL_PAGE *pPage, int *iOff);
int PILTIFFVALUE(unsigned char *p, int bMotorola);
int PILReadAtOffset(PIL_PAGE *pp, int iOffset, int iLen, int bFilter);

/****************************************************************************
 *                                                                          *
 *  FUNCTION   : PILReadBlock(PIL_PAGE *, int *)                            *
 *                                                                          *
 *  PURPOSE    : Re-align and read the next chunk of image data.            *
 *                                                                          *
 ****************************************************************************/
void PILReadBlock(PIL_PAGE *pPage, int *iOff)
{
//#ifdef _WIN32_WCE // assume WinCE is single threaded
int i;

   i = *iOff & 0xfffff000; // round to nearest page size
   pPage->iFilePos -= (PIL_BUFFER_SIZE - i); // re-align
   PILIOSeek(pPage->file, (PILOffset) pPage->iFilePos, 0);
   i = PILIORead(pPage->file, pPage->pData, PIL_BUFFER_SIZE);
   pPage->iFilePos += i;
   if (i < PIL_BUFFER_SIZE) // read less than a whole block, we are at the end of the file
      pPage->iHighWater = i+4; // set highwater mark to actual end of data plus some slop over
   *iOff = *iOff & 0xfff; // get offset within the page
//#else
//   *iOff |= 0; // supress compiler warning
//   WaitForSingleObject((HANDLE)pPage->hEvent, 5000);
//   ResetEvent((HANDLE)pPage->hEvent);
//#endif
} /* PILReadBlock() */

/****************************************************************************
 *                                                                          *
 *  FUNCTION   : PILMirrorBits(uchar *, uchar *, int)                       *
 *                                                                          *
 *  PURPOSE    : Flip the bit direction a buffer.                           *
 *                                                                          *
 ****************************************************************************/
void PILMirrorBits(unsigned char *pSrc, unsigned char *pDest, int iLen)
{
int i;

   for (i=0; i < iLen; i++)
      *pDest++ = ucMirror[*pSrc++];

} /* PILMirrorBits() */

/****************************************************************************
 *                                                                          *
 *  FUNCTION   : PILTIFFSHORT(char *, ini)                                 *
 *                                                                          *
 *  PURPOSE    : Retrieve a short value from a TIFF tag.                    *
 *                                                                          *
 ****************************************************************************/
unsigned short PILTIFFSHORT(unsigned char *p, int bMotorola)
{
unsigned short s;

if (bMotorola)
   s = *p * 0x100 + *(p+1);
else
   s = *p + *(p+1)*0x100;

return s;
} /* PILTIFFSHORT() */

/****************************************************************************
 *                                                                          *
 *  FUNCTION   : PILTIFFLONG(char *, int)                                  *
 *                                                                          *
 *  PURPOSE    : Retrieve a long value from a TIFF tag.                     *
 *                                                                          *
 ****************************************************************************/
uint32_t PILTIFFLONG(unsigned char *p, int bMotorola)
{
uint32_t l;

if (bMotorola)
   l = *p * 0x1000000 + *(p+1) * 0x10000 + *(p+2) * 0x100 + *(p+3);
else
   l = *p + *(p+1) * 0x100 + *(p+2) * 0x10000 + *(p+3) * 0x1000000;

return l;
} /* PILTIFFLONG() */

/****************************************************************************
 *                                                                          *
 *  FUNCTION   : PILTIFFVALUE(char *, int)                                 *
 *                                                                          *
 *  PURPOSE    : Retrieve the value from a TIFF tag.                        *
 *                                                                          *
 ****************************************************************************/
int PILTIFFVALUE(unsigned char *p, int bMotorola)
{
int i, iType;

   iType = PILTIFFSHORT(p+2, bMotorola); // type is a short
   /* If pointer to a list of items (count > 1), then data type must be a long (offset to the list) */
   if (PILTIFFLONG(p+4, bMotorola) > 1) // the count is a long
      iType = 4;
   switch (iType)
      {
      case 3: /* Short */
         i = PILTIFFSHORT(p+8, bMotorola);
         break;
      case 4: /* Long */
      case 7: // undefined (treat it as a long since it's usually a multibyte buffer)
         i = PILTIFFLONG(p+8, bMotorola);
         break;
      case 6: // signed byte
         i = (signed char)p[8];
         break;
      case 2: /* ASCII */
      case 5: /* Unsigned Rational */
      case 10: /* Signed Rational */
         i = PILTIFFLONG(p+8, bMotorola);
         break;
      default: /* to suppress compiler warning */
         i = 0;
         break;
      }
   return i;

} /* PILTIFFVALUE() */

/****************************************************************************
 *                                                                          *
 *  FUNCTION   : PILTIFFCountPages()                                        *
 *                                                                          *
 *  PURPOSE    : Count the pages in a TIFF file and get the offsets+sizes.  *
 *                                                                          *
 ****************************************************************************/
int PILTIFFCountPages(PIL_FILE *pFile, int bMotorola)
{
int i, iNumBytes, iTags, iTag, IFD;
unsigned char *cBuf, *p; // ucType;
#define TAG_BUF_SIZE (MAX_TIFF_TAGS*12+6)

   if (pFile->cState == PIL_FILE_STATE_LOADED) // already in memory
      cBuf = &pFile->pData[4];
   else
      {
      cBuf = (unsigned char *)PILIOAlloc(TAG_BUF_SIZE);
      if (cBuf == NULL)
         return PIL_ERROR_MEMORY;
//	  PILReadAtOffset(pFile, 4, cBuf, 4); /* Read the first IFD pointer */
      }
   pFile->pPageList = (int *)PILIOAlloc(MAX_PAGES * sizeof(int));
   pFile->pPageLens = (int *)PILIOAlloc(MAX_PAGES * sizeof(int));
   if (pFile->pPageList == NULL || pFile->pPageLens == NULL)
   {
	   PILIOFree(pFile->pPageList);
	   pFile->pPageList = NULL;
	   PILIOFree(pFile->pPageLens);
	   pFile->pPageLens = NULL;
	   return PIL_ERROR_MEMORY;
   }
   IFD = PILTIFFLONG(cBuf, bMotorola);
   pFile->iPageTotal = 0;

   while(IFD > 0 && IFD < pFile->iFileSize) /* count the number of pages */
      {
      if (pFile->cState == PIL_FILE_STATE_LOADED) // alread in memory
         {
         cBuf = &pFile->pData[IFD]; // start at this offset
         if ((pFile->iFileSize - IFD) < TAG_BUF_SIZE)
            iNumBytes = pFile->iFileSize - IFD;
         else
            iNumBytes = TAG_BUF_SIZE;
         }
      else
         {
//		 iNumBytes = PILReadAtOffset(pFile, IFD, cBuf, TAG_BUF_SIZE); /* Read max sized block we accept (12 x 256) */
         if (iNumBytes < 60)
            break; /* done, stop looking */
         }
      iTags = PILTIFFSHORT(cBuf, bMotorola);  /* Number of tags in this dir */
      i = iTags * 12 + 2; /* Offset to next IFD in chain */
      if (i < iNumBytes && iTags >= 2 && iTags < MAX_TIFF_TAGS) /* Valid header */
      {
		  int iSizePtr, iSizeType, iStripCount;
          pFile->pPageList[pFile->iPageTotal++] = IFD; // store it in the list (skip invalid pages)
          IFD = PILTIFFLONG(cBuf+i, bMotorola); // next IFD offset
		  // look through the tags and figure out the size of the data
		  iSizePtr = iSizeType = iStripCount = 0;
		  for (i = 0; i < iTags; i++)
		  {
			  p = &cBuf[i * 12 + 2];
//			  ucType = p[2];
			  iTag = PILTIFFSHORT(p, bMotorola);
			  switch (iTag)
			  {
			  case 279: // strip size
				  iSizePtr = PILTIFFVALUE(p, bMotorola);
				  iSizeType = PILTIFFSHORT(p + 2, bMotorola);
				  iStripCount = PILTIFFLONG(p + 4, bMotorola);
				  break;
			  case 514: // JPEGInterchangeFormatLength - length of JPEG data
				  iSizePtr = PILTIFFVALUE(p, bMotorola);
				  iStripCount = 1;
				  break;
			  case 325: // Tile byte counts
				  iSizeType = PILTIFFSHORT(p + 2, bMotorola);
				  iStripCount = PILTIFFLONG(p + 4, bMotorola);
				  iSizePtr = PILTIFFLONG(p + 8, bMotorola);
				  break;
			  } // switch on tag
		  } // for each tag
		  if (iStripCount < 0 || iStripCount > 65535) // unreasonable number, must be bad data
			  return PIL_ERROR_BADHEADER;
		  if (iStripCount > 1)// need to add up the size of the strips
		  {
			  unsigned char *pTemp = (unsigned char *) PILIOAlloc(iStripCount * sizeof(uint32_t));
			  int k;
			  if (pTemp == NULL)
				  return PIL_ERROR_MEMORY;
			  k = (iSizeType == 3) ? 2 : 4; // short or long?
		//	  PILReadAtOffset(pFile, iSizePtr, pTemp, iStripCount*k);
			  iSizePtr = 0;
			  for (k = 0; k < iStripCount; k++)
			  { // add up the strip/tile sizes
				  if (iSizeType == 3)
					  iSizePtr += PILTIFFSHORT(pTemp + k * 2, bMotorola);
				  else
					  iSizePtr += PILTIFFLONG(pTemp + k * 4, bMotorola);
			  }
			  PILIOFree(pTemp);
		  }
		  pFile->pPageLens[pFile->iPageTotal - 1] = iSizePtr;
	  }
      else
         break; // invalid data, stop
      }
   if (pFile->cState != PIL_FILE_STATE_LOADED)
      PILIOFree(cBuf);
   if (pFile->iPageTotal == 1) // no need for a pagelist
      {
      PILIOFree(pFile->pPageList);
      pFile->pPageList = NULL;
	  PILIOFree(pFile->pPageLens);
	  pFile->pPageLens = NULL;
      }
   else if (pFile->iPageTotal == 0) // no pages
      return PIL_ERROR_PAGENF;
   else // when more than one page, set last offset = file size
      {
      pFile->pPageList[pFile->iPageTotal] = pFile->iFileSize;
      }
   return 0;

} /* PILTIFFCountPages() */

/****************************************************************************
 *                                                                          *
 *  FUNCTION   : PILTIFFMiniInfo()                                          *
 *                                                                          *
 *  PURPOSE    : Gather a minimal set of info about the first page.         *
 *                                                                          *
 ****************************************************************************/
int PILTIFFMiniInfo(PIL_FILE *pFile, int bMotorola, int iOffset, int bJPEG)
{
int iTag, iTags, i, j, iOff, iLen;
unsigned char cTemp[1536]; // up to 128 tags fit
int iID;
int iStripPtr = 0;
int iJPEGStart = 0;
int iMarker;

//   PILReadAtOffset(pFile, iOffset, cTemp, 1536);
   iTags = PILTIFFSHORT(cTemp, bMotorola);  /* Number of tags in this dir */
   if (iTags < 4 || iTags > MAX_TIFF_TAGS)
	   return PIL_ERROR_BADHEADER; /* Bad header info */
   if (bJPEG)
      { // we want the second IFD (image)
      iJPEGStart = iOffset - 8; // save the EXIF image offset into the larger file
      i = PILTIFFLONG(&cTemp[(iTags*12)+2], bMotorola); /* Get the offset */
      iOffset = i + (iOffset-8); // new offset is relative to old
	  memset(cTemp, 0xff, 1536); // in case the read fails, we will bail out
//	  PILReadAtOffset(pFile, iOffset, cTemp, 1536);
      iTags = PILTIFFSHORT(cTemp, bMotorola);  /* Number of tags in this dir */
	  if (iTags < 4 || iTags > MAX_TIFF_TAGS)
		  return PIL_ERROR_BADHEADER; /* Bad header info */
	  }
   /*--- Process the TIFF tags ---*/
   for (iTag=0; iTag<iTags; iTag++)
      {
      unsigned char *p = &cTemp[iTag*12+2];
      j = PILTIFFSHORT(p, bMotorola);  /* current tag value */
      switch (j)
         {
         case 256: /* Image width */
            if (bJPEG)
               pFile->iThumbCX = PILTIFFVALUE(p, bMotorola);
            else
               pFile->iX = PILTIFFVALUE(p, bMotorola);
            break;
         case 257: /* Image Length */
            if (bJPEG)
               pFile->iThumbCY = PILTIFFVALUE(p, bMotorola);
            else
               pFile->iY = PILTIFFVALUE(p, bMotorola);
            break;
         case 513: // JPEGInterchangeFormat - offset to JPEG data
            iStripPtr = iJPEGStart + PILTIFFVALUE(p, bMotorola);
            // Need to read the JPEG data to find the thumbnail size
//			PILReadAtOffset(pFile, iStripPtr, cTemp, 1536);
            iMarker = 0; /* Search for SOF (start of frame) marker */
            i = 2;
            while (i < 1536)
               {
               iMarker = MOTOSHORT(&cTemp[i]) & 0xfffc;
               i += 2;
               if (iMarker < 0xff00) // invalid marker, could be generated by "Arles Image Web Page Creator" or Accusoft
                  continue; // skip 2 bytes and try to resync
               if (iMarker == 0xffc0)
                  break;
               i += MOTOSHORT(&cTemp[i]); /* Skip to next marker */
               }
            if (i >= 1536 || iMarker != 0xffc0)
               {
               return PIL_ERROR_BADHEADER;
               }
            else
               {
               pFile->iThumbCY = MOTOSHORT(&cTemp[i+3]);
               pFile->iThumbCX = MOTOSHORT(&cTemp[i+5]);
               }
            break;
         case 0x8649: // Private Photoshop tag for embedded thumbnail
            iOff = PILTIFFVALUE(p, bMotorola); /* Get the offset */
            iLen = PILTIFFLONG((p+4), bMotorola); /* Get the count/len */
            // read enough of the thumbnail get the size
	//		PILReadAtOffset(pFile, iOff, cTemp, 1536);
            iOff = 0;
            iID = 0;
            while (iOff < iLen-4 && iID != 0x40c && iID != 0x409 && MOTOLONG(&cTemp[iOff]) == 0x3842494d /*'8BIM'*/) // valid image block
               {
               iOff += 4;
               if (iOff <= 1536-4)
                  iID = MOTOSHORT(&cTemp[iOff]); // resource ID
               iOff += 2;
               if (iOff <= 1536-2)
                  j = cTemp[iOff] + 1; // pascal string length
               if (j & 1) // must be even
                  j++;
               iOff += j;
               if (iOff <= 1536-4)
                  j = MOTOLONG(&cTemp[iOff]); // length of resource
               iOff += 4;
               if (j & 1) // must be even length
                  j++;
               if (iID == 0x040c || iID == 0x0409) // thumbnail image
                  {
                  pFile->cBpp = 24; // always 24bpp
                  pFile->iThumbCX = MOTOLONG(&cTemp[iOff+4]);
                  pFile->iThumbCY = MOTOLONG(&cTemp[iOff+8]);
                  }
               iOff += j; // point to next resource
               }
            break;
         }
      }
   return 0;
} /* PILTIFFMiniInfo() */

/****************************************************************************
 *                                                                          *
 *  FUNCTION   : PILTIFFInfo()                                              *
 *                                                                          *
 *  PURPOSE    : Gather info about the specific page.                       *
 *                                                                          *
 ****************************************************************************/
int PILTIFFInfo(PIL_FILE *pFile, PIL_PAGE *pPage, int iOptions)
{
	int i, j, k, iLen, iTag, iTagCount;
	int bIBM = FALSE;
	int iResPtr, iPredictor, iColorTab, iT4Options, iPlanar;
// int iResUnit;
	int iStripPtr, iOffType, iSizePtr, iJPEGSize, iJPEGPtr, iSizeType;
	int iAnnotationOff, iAnnotationSize;
	unsigned char temp[8], *cBuf, *p, ucType;
	int bMotorola;
	int iGPSIFD = 0;
	unsigned char cTemp2[256];
	unsigned char cTemp[1536]; // up to 128 tags fit
	int iTileWidth, iTileHeight, iTileCount, iTileOffsets, iTileSizes, iTileSizeType;
	// Canon CR2 RAW support
	int iNumSlices;

	cTemp[0] = 0; // suppress compiler warning
	if (pPage->iOffset < 0 || pPage->iOffset > pFile->iFileSize) // data offset is invalid
		return PIL_ERROR_BADHEADER;

	iTileSizeType = iTileSizes = 0;
	if (pFile->cState == PIL_FILE_STATE_LOADED) // we have all of the data in memory
	{
		if (pFile->pData[0] != 'I' && pFile->pData[0] != 'M') // we are starting at the IFD offset already
		{
			bMotorola = (pFile->pData[0] == 0); // we assume that there are fewer than 256 tags
		}
		else
		{
			bMotorola = (pFile->pData[0] == 'M');
			if (pPage->iOffset == 0) // need to get the IFD offset
			{
				pPage->iOffset = PILTIFFLONG(&pFile->pData[4], bMotorola);
			}
		}
		cBuf = &pFile->pData[pPage->iOffset]; // start at beginning of IFD for this page
	}
	else // need to read the IFD from the file
	{
//		PILReadAtOffset(pFile, 0, cTemp, 8); // get the first byte to know if M or I
		bMotorola = (cTemp[0] == 'M');
		if (pPage->iOffset == 0) // single page file, get first IFD
		{
			pPage->iOffset = PILTIFFLONG(&cTemp[4], bMotorola);
		}
//		PILReadAtOffset(pFile, pPage->iOffset, cTemp, 1536);
		cBuf = &cTemp[0];
	}
	if (bMotorola)
		pPage->cFlags |= PIL_PAGEFLAGS_MOTOROLA;
	iT4Options = 0;
	iPredictor = 0;
	iAnnotationOff = iAnnotationSize = 0;
	iPlanar = 1; // assume chunky, planar=2
	iColorTab = -1;
	iNumSlices = 0; // assume not Canon RAW
	pPage->cBitDir = PIL_BITDIR_MSB_FIRST; /* Set up default values */
	pPage->cPhotometric = PIL_PHOTOMETRIC_NONE;
	pPage->cBitsperpixel = 1;
	pPage->cJPEGSubSample = 0x11; // assume no YCbCr color subsampling
	pPage->cCompression = PIL_COMP_NONE; /* Default value is uncompressed */
//	pPage->iStripCount = 0; // assume no specific strip information (1 strip)
	iSizePtr = 0;
	iJPEGSize = iJPEGPtr = 0;
	iStripPtr = iOffType = iSizeType = 0;
	iTileWidth = iTileHeight = iTileOffsets = iTileCount = 0;

	iTagCount = PILTIFFSHORT(cBuf, bMotorola);  /* Number of tags in this dir */
	if (iTagCount > MAX_TIFF_TAGS)
		return PIL_ERROR_BADHEADER; /* Bad header info */
	if (pFile->pData == NULL)
		pFile->pData = pPage->pData;
	/*--- Process the TIFF tags ---*/
	for (i = 0; i<iTagCount; i++)
	{
		unsigned char *p = &cBuf[i * 12 + 2];
		ucType = p[2];
		iTag = PILTIFFSHORT(p, bMotorola);  /* current tag value */
		switch (iTag)
		{
		case 999:  /* IBM specific tag */
			bIBM = TRUE;
			break;
		case 254: /* NewSubfileType - not currently used */
			break;
		case 256: /* Image width */
            pPage->iOriginalWidth = pPage->iWidth = PILTIFFVALUE(p, bMotorola);
			break;
		case 257: /* Image Length */
            pPage->iOriginalHeight = pPage->iHeight = PILTIFFVALUE(p, bMotorola);
			break;
		case 258: /* Bits per sample */
			iResPtr = PILTIFFLONG(p + 8, bMotorola); // get the offset
			k = PILTIFFLONG((p + 4), bMotorola); /* Get the count */
			if (k == 1)
			{
				pPage->cBitsperpixel = (char) PILTIFFVALUE(p, bMotorola);
			}
			else if (k == 2)
			{
				pPage->cBitsperpixel = (char) (PILTIFFVALUE(p, bMotorola) + PILTIFFVALUE(&p[2], bMotorola));
			}
			else if (k == 3 || k == 4)
			{
	//			PILReadAtOffset(pFile, iResPtr, temp, 8);
				// assume all are equal
				pPage->cBitsperpixel = (unsigned char) (k * PILTIFFSHORT(temp, bMotorola));
			}
			else
			{
				pPage->cBitsperpixel = 99; // we don't support other numbers of color components
			}
			break;
		case 259: /* Compression type */
			k = PILTIFFVALUE(p, bMotorola);
			if (k == 1)
				pPage->cCompression = PIL_COMP_NONE;
			else if (k == 2)
				pPage->cCompression = PIL_COMP_TIFFHUFFMAN;
			else if (k == 3)
				pPage->cCompression = PIL_COMP_G31D;
			else if (k == 4)
				pPage->cCompression = PIL_COMP_G4;
			else if (k == 5)
				pPage->cCompression = PIL_COMP_LZW;
			else if (k == 6 || k == 7)
				pPage->cCompression = PIL_COMP_JPEG;
			else if (k == 8 || k == 32946)
				pPage->cCompression = PIL_COMP_FLATE;
			else if (k == 9) // T.85 - JBIG1 for bitonal only
				pPage->cCompression = PIL_COMP_JBIG;
			else if (k == 32773)
				pPage->cCompression = PIL_COMP_TIFFPACKBITS;
			else if (k == 32809)
				pPage->cCompression = PIL_COMP_THUNDERSCAN;
			else if (k == 0x80b2) // deflate
				pPage->cCompression = PIL_COMP_FLATE;
			else
				pPage->cCompression = PIL_COMP_UNKNOWN;
			break;
		case 262: /* Photometric value */
			pPage->cPhotometric = (char) PILTIFFVALUE(p, bMotorola);
            pFile->cPhotometric = pPage->cPhotometric; // keep it here to know what it was later
			break;
		case 263: /* Threshholding method */
			break;
		case 266: /* Fill order */
			pPage->cBitDir = (char) PILTIFFVALUE(p, bMotorola) - 1;
			break;
		case 270: // Description
			j = PILTIFFVALUE(p, bMotorola); /* Get the offset */
			k = PILTIFFLONG((p + 4), bMotorola); /* Get the count */
			if (k > 127)
				k = 127; // max we can handle
			if (k < 0) // bogus value would cause an exception
				k = 0;
			if (k)
			{
				memset(pPage->szComment, 0, 128);
				if (k <= 4) // value is stored in the tag
					memcpy(pPage->szComment, &p[8], k);
				else
				{
//					PILReadAtOffset(pFile, j, (unsigned char *) pPage->szComment, k); // read the comment text
				}
			}
			break;
		case 271: // Make
			j = PILTIFFVALUE(p, bMotorola); /* Get the offset */
			k = PILTIFFLONG((p + 4), bMotorola); /* Get the count */
			if (k > 127)
				k = 127; // max we can handle
			if (k < 0) // bogus value would cause an exception
				k = 0;
//			if (k)
//			{
//				memset(pPage->szMake, 0, 128);
//				if (k <= 4)
//					memcpy(pPage->szMake, &p[8], k);
//				else
//				{
//					PILReadAtOffset(pFile, j, (unsigned char *) pPage->szMake, k); // read the make text
//				}
//			}
			break;
		case 272: // Model
//			j = PILTIFFVALUE(p, bMotorola); /* Get the offset */
//			k = PILTIFFLONG((p + 4), bMotorola); /* Get the count */
//			if (k > 127)
//				k = 127; // max we can handle
//			if (k < 0) // bogus value would cause an exception
//				k = 0;
//			if (k)
//			{
//				memset(pPage->szModel, 0, 128);
//				if (k <= 4)
//					memcpy(pPage->szModel, &p[8], k);
//				else
//				{
//					PILReadAtOffset(pFile, j, (unsigned char *) pPage->szModel, k); // read the model text
//				}
//			}
			break;
		case 273: /* Strip info */
			iStripPtr = PILTIFFVALUE(p, bMotorola);
			iOffType = PILTIFFSHORT(p + 2, bMotorola);
//			pPage->iStripCount = PILTIFFLONG(p + 4, bMotorola);
			break;
		case 274: // Orientation
			pFile->iOrientation = pPage->iOrientation = PILTIFFVALUE(p, bMotorola);
			break;
		case 277: /* Samples per pixel */
			break;
		case 278: /* Rows per strip */
//			pPage->iRowCount = PILTIFFVALUE(p, bMotorola);
			break;
		case 279: /* Strip size */
			iSizePtr = PILTIFFVALUE(p, bMotorola);
			iSizeType = PILTIFFSHORT(p + 2, bMotorola);
			break;
		case 282: /* X resolution */
			iResPtr = PILTIFFVALUE(p, bMotorola);
//			PILReadAtOffset(pFile, iResPtr, temp, 8);
			j = PILTIFFLONG(temp, bMotorola);
			k = PILTIFFLONG(&temp[4], bMotorola);
			if (k != 0)
				pPage->iXres = j / k;
			else
				pPage->iXres = 0;
			break;
		case 283: /* Y resolution */
			iResPtr = PILTIFFVALUE(p, bMotorola);
	//		PILReadAtOffset(pFile, iResPtr, temp, 8);
			j = PILTIFFLONG(temp, bMotorola);
			k = PILTIFFLONG(&temp[4], bMotorola);
			if (k != 0)
				pPage->iYres = j / k;
			else
				pPage->iYres = 0;
			break;
		case 284: // planar/chunky
			iPlanar = PILTIFFVALUE(p, bMotorola);
			break;
		case 292: /* T4 Options flags */
			iT4Options = PILTIFFVALUE(p, bMotorola);
			break;
		case 293: /* T6 Options flags */
			break; // indicates if uncompressed mode is used
//		case 296: /* Resolution unit */
//			iResUnit = PILTIFFVALUE(p, bMotorola);
//			break;
		case 305: // Software
//			j = PILTIFFVALUE(p, bMotorola); /* Get the offset */
//			k = PILTIFFLONG((p + 4), bMotorola); /* Get the count */
//			if (k > 127)
//				k = 127; // max we can handle
//			if (k < 0) // bogus value would cause an exception
//				k = 0;
//			if (k)
//			{
//				memset(pPage->szSoftware, 0, 128);
//				if (k <= 4)
//					memcpy(pPage->szSoftware, &p[8], k);
//				else
//				{
//					PILReadAtOffset(pFile, j, (unsigned char *) pPage->szSoftware, k); // read the software text
//				}
//			}
			break;
		case 315: // Artist
//			j = PILTIFFVALUE(p, bMotorola); /* Get the offset */
//			k = PILTIFFLONG((p + 4), bMotorola); /* Get the count */
//			if (k > 127)
//				k = 127; // max we can handle
//			if (k < 0) // bogus value would cause an exception
//				k = 0;
//			if (k)
//			{
//				memset(pPage->szArtist, 0, 128);
//				if (k <= 4)
//					memcpy(pPage->szArtist, &p[8], k);
//				else
//				{
//					PILReadAtOffset(pFile, j, (unsigned char *) pPage->szArtist, k); // read the artist text
//				}
//			}
			break;
		case 317: /* LZW/Flate predictor */
			iPredictor = PILTIFFVALUE(p, bMotorola);
			break;
		case 320: /* TIFF Color Table */
			iColorTab = PILTIFFVALUE(p, bMotorola);
			break;
		case 322: // Tile width
			iTileWidth = PILTIFFVALUE(p, bMotorola);
			break;
		case 323: // Tile length
			iTileHeight = PILTIFFVALUE(p, bMotorola);
			break;
		case 324: // Tile offsets
			iTileCount = PILTIFFLONG(p + 4, bMotorola);
			iTileOffsets = PILTIFFLONG(p + 8, bMotorola);
			break;
		case 325: // Tile byte counts
			iTileSizeType = PILTIFFSHORT(p + 2, bMotorola);
			iTileSizes = PILTIFFLONG(p + 8, bMotorola);
			break;
		case 347: // JPEGTables for compression type 7
//			j = PILTIFFVALUE(p, bMotorola); /* Get the offset */
//			k = PILTIFFLONG((p + 4), bMotorola); /* Get the count */
//			if (k > 0 && k < 2048) // reasonable size range
//			{
//				if (k < 764)
//					pPage->pLocalPalette = (unsigned char *) PILIOAlloc(768); // allocate at least 768 so that it can pass through PILConvert() without crashing
//				else
//					pPage->pLocalPalette = (unsigned char *) PILIOAlloc(k + 4);
//				if (pPage->pLocalPalette == NULL)
//					return PIL_ERROR_MEMORY;
//				PILReadAtOffset(pFile, j, pPage->pLocalPalette, k); // read the JPEG data tables
//				pPage->pLocalPalette[k] = 0xff;
//				pPage->pLocalPalette[k + 1] = 0xd9; // add an EOI to make sure parsing stops at the end
//				pPage->iAnnotationSize = k + 2; // store table size here
//			}
			break;
		case 513: // JPEGInterchangeFormat - offset to JPEG data
			iJPEGPtr = PILTIFFVALUE(p, bMotorola);
			pPage->cCompression = PIL_COMP_JPEG; // some JPEG's don't have the compression tag in the EXIF TIFF header
			break;
		case 514: // JPEGInterchangeFormatLength - length of JPEG data
			iJPEGSize = PILTIFFVALUE(p, bMotorola);
			pPage->cCompression = PIL_COMP_JPEG; // some JPEG's don't have the compression tag in the EXIF TIFF header
			break;
		case 515: // JPEGRestartInterval
			j = PILTIFFVALUE(p, bMotorola); /* Get the restart interval */
			break;
		case 519: // JPEGQTables
			j = PILTIFFVALUE(p, bMotorola); /* Get the offset */
			k = PILTIFFLONG((p + 4), bMotorola); /* Get the count */
			break;
		case 520: // JPEGDCTables
			j = PILTIFFVALUE(p, bMotorola); /* Get the offset */
			k = PILTIFFLONG((p + 4), bMotorola); /* Get the count */
			break;
		case 521: // JPEGACTables
			j = PILTIFFVALUE(p, bMotorola); /* Get the offset */
			k = PILTIFFLONG((p + 4), bMotorola); /* Get the count */
			break;
		case 530: // JPEGYCbCrSubsampling
		{
			int x, y;
			x = PILTIFFSHORT(p + 8, bMotorola); // two shorts fit in the 12-byte tag
			y = PILTIFFSHORT(p + 10, bMotorola);
			if (x > 2 || y > 2) // we only support 1:1:, 2:1, and 2:2
				return PIL_ERROR_UNSUPPORTED;
			pPage->cJPEGSubSample = (unsigned char) ((x << 4) | y);
		}
			break;
		case 33432: // Copyright
//			j = PILTIFFVALUE(p, bMotorola); /* Get the offset */
//			k = PILTIFFLONG((p + 4), bMotorola); /* Get the count */
//			if (k > 127)
//				k = 127; // max we can handle
//			if (k < 0) // bogus value would cause an exception
//				k = 0;
//			if (k)
//			{
//				memset(pPage->szInfo2, 0, 128);
//				if (k <= 4)
//					memcpy(pPage->szInfo2, &p[8], k);
//				else
//				{
//					PILReadAtOffset(pFile, j, (unsigned char *) pPage->szInfo2, k); // read the software text
//				}
//			}
			break;

		case 0x8649: // Photoshop image resource blocks (embedded thumbnail)
			break;
		case 0x8769: // EXIF subIFD offset (EXIF info)
			pPage->iFrameDelay = PILTIFFVALUE(p, bMotorola); /* Get the offset */
			if (pPage->iFrameDelay < 0 || pPage->iFrameDelay > pFile->iFileSize) // invalid offset
				pPage->iFrameDelay = 0;
			break;
		case 0x8825: // GPS IFD
			iGPSIFD = PILTIFFVALUE(p, bMotorola); /* Get the offset */
			break;
		case 0x80a4: // Wang/Eastman annotations
			iAnnotationOff = PILTIFFVALUE(p, bMotorola); /* Get the offset */
			iAnnotationSize = PILTIFFLONG((p + 4), bMotorola); /* Get the count */
			break;
		case 37686: // Microsoft Document Imaging Annotations
			iAnnotationOff = PILTIFFVALUE(p, bMotorola); /* Get the offset */
			iAnnotationSize = PILTIFFLONG((p + 4), bMotorola); /* Get the count */
			break;

		default: // store it in the metadata structure
			if (iOptions & PIL_READFLAGS_READMETA) // caller wants the unrecognized metadata
			{
				j = PILTIFFVALUE(p, bMotorola); /* Get the offset */
				k = PILTIFFLONG((p + 4), bMotorola); /* Get the count */
				if (k < 0) // bogus value would cause an exception
					k = 0;
				switch (ucType)
				{
				case 1: // BYTE
				case 2: // ASCII
					iLen = k;
					break;
				case 3: // SHORT
					iLen = k * sizeof(short);
					break;
				case 4: // LONG
					iLen = k * sizeof(uint32_t);
					break;
				case 5: // RATIONAL
					iLen = k * 2 * sizeof(uint32_t);
					break;
				default: // unknown type
					iLen = k;
					break;
				}
				if (iLen <= 4) // it's in the tag data
				{
					p += 8; // point to the data
				}
				else // need to read it
				{
					p = NULL;
					if (iLen < 256)
					{
			//			PILReadAtOffset(pFile, j, cTemp2, iLen); // get the first byte to know if M or I
						p = cTemp2;
					}
				}
//				if (p)
//					PILWriteMeta(pPage, iTag, ucType, p, iLen);
			}
			break;
		} /* endswitch */
	}
	if (iNumSlices != 0) // Canon RAW
	{
		pFile->cFileType = PIL_FILE_CANONRAW;
	}
//	if (iGPSIFD != 0) // GPS info is present
//	{
//		PILGPSInfo(pFile, pPage, iGPSIFD);
//	}
	// We were called to gather info, but not work with image data; time to go
	if ((iOptions & PIL_CONVERT_NOALLOC) && pPage->pData == NULL)
		return 0;
	// We're trying to read a thumbnail image, but the reported image size is huge, abort
	if (pFile->iFileSize == 0xffff && (pPage->iWidth > 500 || pPage->iHeight > 500))
		return PIL_ERROR_INVPARAM;
    pFile->cCompression = pPage->cCompression; // preserve original compression type for info display
	if (pPage->cPhotometric == PIL_PHOTOMETRIC_NONE) // photometric interpretation not specified; we need to guess
	{
		if (pPage->cBitsperpixel == 1)
		{
			if (pPage->cCompression == PIL_COMP_G4 || pPage->cCompression == PIL_COMP_G31D || pPage->cCompression == PIL_COMP_G32D ||
				pPage->cCompression == PIL_COMP_MMR || pPage->cCompression == PIL_COMP_TIFFHUFFMAN)
				pPage->cPhotometric = PIL_PHOTOMETRIC_WHITEISZERO;
			else
				pPage->cPhotometric = PIL_PHOTOMETRIC_BLACKISZERO;
		}
	}
	else if (pPage->cCompression == PIL_COMP_TIFFPACKBITS && pPage->cBitsperpixel == 1) // photometric is reversed for packbits 1bpp
	{
		if (pPage->cPhotometric == PIL_PHOTOMETRIC_WHITEISZERO || pPage->cPhotometric == PIL_PHOTOMETRIC_BLACKISZERO)
			pPage->cPhotometric = 1 - pPage->cPhotometric;
	}
//	if (pPage->iRowCount > pPage->iHeight) // problem
//		pPage->iRowCount = pPage->iHeight; // correct it
	// JPEG type 6 with external tables
	if (iJPEGSize != 0 && iJPEGPtr != 0)
	{
//		if (iJPEGSize < 2048) // jpeg tables only
//		{
//			pPage->pLocalPalette = (unsigned char *) PILIOAlloc(iJPEGSize + 4);
//			if (pPage->pLocalPalette == NULL)
//				return PIL_ERROR_MEMORY;
//			PILReadAtOffset(pFile, iJPEGPtr, pPage->pLocalPalette, iJPEGSize); // read the JPEG data tables
//			pPage->pLocalPalette[iJPEGSize] = 0xff;
//			pPage->pLocalPalette[iJPEGSize + 1] = 0xd9; // add an EOI to make sure parsing stops at the end
//			pPage->iAnnotationSize = iJPEGSize + 2; // store table size here
//		}
//		else // JPEG image is stored here
//		{
//			if (iSizePtr == 0) // JPEG data specified by old Type 6 tags
//			{
//				pPage->iStripCount = 1;
//				iSizePtr = iJPEGSize;
//				iStripPtr = iJPEGPtr;
//			}
//		}
	}
	if (iAnnotationSize != 0 && iAnnotationOff != 0) // read annotation data
	{
//		pPage->iAnnotationSize = iAnnotationSize;
//		pPage->pAnnotations = (void *) PILIOAlloc(iAnnotationSize + 4); // leave a little extra room to read the last bytes
//		if (pPage->pAnnotations == NULL)
//			return PIL_ERROR_MEMORY;
//		PILReadAtOffset(pFile, iAnnotationOff, (unsigned char *) pPage->pAnnotations, iAnnotationSize);
	}
	if (iSizePtr == 0 && iStripPtr == 0 && iOffType == 0 && iSizeType == 0 && iTileWidth == 0 && iTileHeight == 0 && iTileCount == 0 && iTileOffsets == 0) // no image info, leave
		return 0;
	if (pPage->cBitsperpixel == 1 && (pPage->iWidth > 0x3ffff || pPage->iHeight > 0x3ffff)) // too big (arbitrary limits of 256k x 256k pixels)
	{
		return PIL_ERROR_UNSUPPORTED;
	}
	if (pPage->cBitsperpixel != 1 && (pPage->iWidth > 0x7fff || pPage->iHeight > 0x7fff)) // too big (arbitrary limits)
	{
		return PIL_ERROR_UNSUPPORTED;
	}
	if (iTileWidth & 15 || iTileHeight & 15) // tile size must be a multiple of 16
		return PIL_ERROR_BADHEADER;

	if (pPage->cBitsperpixel > 1 && ((pPage->iWidth * pPage->iHeight) > 1000000000)) // too big to load (would require too much RAM)
	{
		return PIL_ERROR_UNSUPPORTED;
	}
	if (pPage->cBitsperpixel != 1 && (pPage->cCompression == PIL_COMP_G4 || pPage->cCompression == PIL_COMP_G31D || pPage->cCompression == PIL_COMP_G32D || pPage->cCompression == PIL_COMP_TIFFHUFFMAN))
	{
		return PIL_ERROR_UNSUPPORTED; // these compression types must be 1bpp
	}
	if (pPage->cBitsperpixel != 1 && pPage->cBitsperpixel != 2 && pPage->cBitsperpixel != 4 && pPage->cBitsperpixel != 8
		&& pPage->cBitsperpixel != 16 && pPage->cBitsperpixel != 24 && pPage->cBitsperpixel != 32
		&& pPage->cBitsperpixel != 48 && pPage->cBitsperpixel != 64)
	{
		return PIL_ERROR_UNSUPPORTED;
	}
//	if (pPage->iStripCount <= 1 && iTileWidth == 0) /* Strip size is potentially screwed up, fix it */
	{
//		pPage->iRowCount = pPage->iHeight;
//		pPage->iStripCount = 0; // to tell code later that there is only 1 strip
		if (iSizePtr <= 1 || iSizePtr > pFile->iFileSize) // odd case where this tag is missing or incorrectly specified; assume single page and all of the data
		{
			iSizePtr = pFile->iFileSize;
		}
		if (iStripPtr < 0 || iStripPtr > pFile->iFileSize) // invalid data, don't try to read it
			return PIL_ERROR_BADHEADER;
		pPage->iDataSize = iSizePtr; /* Assume the file size */
		if (pPage->cState == PIL_PAGE_STATE_LOADED) // we have all of the data in memory
		{
			pPage->iOffset = iStripPtr; // point to the one strip of data
		}
		else // we need to allocate and read in the 1 strip
		{
			int iAllocSize = iSizePtr;
			if (pPage->cCompression == PIL_COMP_NONE) // deal with a "short" page (missing data)
			{
//				i = PILCalcSize(pPage->iWidth, pPage->cBitsperpixel) * pPage->iHeight;
				if (i > iAllocSize) iAllocSize = i;
			}
			pPage->pData = (unsigned char *) PILIOAlloc(iAllocSize + 4); // leave a little extra room to read the last bytes
			if (pPage->pData == NULL)
				return PIL_ERROR_MEMORY;
	//		PILReadAtOffset(pFile, iStripPtr, pPage->pData, iSizePtr);
			pPage->iOffset = 0;
			pPage->cState = PIL_PAGE_STATE_LOADED; // now it's loaded
		}
	}
//	else
//	{
//		pPage->iOffset = 0; // otherwise use this + strip offset to find data
//	}
	if (pPage->iWidth == 0 && pPage->cCompression == PIL_COMP_JPEG && iJPEGPtr == 0) // Canon RAW
	{
		iJPEGPtr = iStripPtr;
		pPage->cPhotometric = PIL_PHOTOMETRIC_CFA;
	}
	if (PIL_PAGE_STATE_LOADED == pPage->cState && NULL == pPage->pData) // something very wrong happened; can't be in LOADED state with no data
		return PIL_ERROR_UNKNOWN;

	if (iJPEGPtr && (pPage->iWidth == 0 || pPage->cBitsperpixel < 8)) // JPEG type 6 with no size info or no bpp info
	{ // get size and bpp info from the jpeg data
		if ((MOTOLONG(&pPage->pData[pPage->iOffset]) & 0xffffff00) == 0xffd8ff00)
		{
			int iMarker;
			i = pPage->iOffset + 2; /* Start at offset of first marker */
			iMarker = 0; /* Search for SOF (start of frame) marker */
			while (iMarker != 0xffc0 && i < pPage->iDataSize)
			{
				iMarker = MOTOSHORT(&pPage->pData[i]) & 0xfffc;
				if (iMarker < 0xff00) // invalid marker, could be generated by "Arles Image Web Page Creator" or Accusoft
				{
					i += 2;
					continue; // skip 2 bytes and try to resync
				}
				if (iMarker == 0xffc0)
					break;
				i += 2 + MOTOSHORT(&pPage->pData[i + 2]); /* Skip to next marker */
			} // while
			if (iMarker == 0xffc0)
			{
				pPage->iWidth = MOTOSHORT(&pPage->pData[i + 5]);
				pPage->iHeight = MOTOSHORT(&pPage->pData[i + 7]);
				if (pPage->pData[i + 9] == 1) /* number of components */
				{
					pPage->cBitsperpixel = 8;
				}
				else
				{
					pPage->cBitsperpixel = 24;
				}
			}
		}
	}
	if (pPage->iDataSize == 0) /* No size information */
		pPage->iDataSize = pFile->iFileSize;
//	pPage->iPitch = PILCalcBSize(pPage->iWidth, pPage->cBitsperpixel); // for uncompressed data
	if (iPlanar == 2)
	{
		pPage->cFlags |= PIL_PAGEFLAGS_PLANAR; // we will need to undo it later
	}
	//
	// For now, all of the TIFF Tile handling in PIL occurs here. Tiles are gathered together to form
	// the entire image in PILRead() instead of trying to gather compressed data in small parts
	// and having to rewrite PILConvert() or rewrite each codec
	//
	/*--- Construct the image from tiles ---*/
//	if (iTileWidth > 0 && iTileHeight > 0 && iTileWidth < pPage->iWidth && iTileHeight < pPage->iHeight && (iTileCount > 0 || pPage->iStripCount > 0))
	{
		// Since PIL expects the image to be complete, we need to decode each tile and construct the image here
		signed int iSize;
		unsigned char *pBigTileBuf = NULL; // for bilevel data
		int iBigTilePitch = 0;
		unsigned char **indexbuf = NULL;
		unsigned char *irlcptr = NULL;
		unsigned int iLargest = 0;
		int iError = 0;
		int iTileCX, iTileCY; // horizontal and vertical tile counts
		unsigned char *pSizes, *pStrips;
		unsigned char *pTemp = NULL;
		unsigned char *pData = NULL;
		uint32_t *plTiles = NULL;
		uint32_t *plTileSize = NULL;
		// Some images define a tile size, but store the tile offsets in the strip offsets tag
//		if (iTileCount == 0 && pPage->iStripCount > 0)
		{ // use the strip info as tile info
//			iTileCount = pPage->iStripCount;
			iTileOffsets = iStripPtr;
			iTileSizes = iSizePtr;
			iTileSizeType = iSizeType;
		}
		iSize = 0;
		plTiles = (uint32_t *) PILIOAlloc(iTileCount * sizeof(uint32_t));
		if (plTiles == NULL)
		{
			iError = PIL_ERROR_MEMORY;
			goto tile_end;
		}
		plTileSize = (uint32_t *) PILIOAlloc(iTileCount * sizeof(uint32_t));
		if (plTileSize == NULL)
		{
			iError = PIL_ERROR_MEMORY;
			goto tile_end;
		}
		if (pPage->cBitsperpixel == 1) // special case for 1bpp images
		{
//			iBigTilePitch = PILCalcSize(pPage->iWidth, 1);
			pBigTileBuf = (unsigned char *) PILIOAlloc(iBigTilePitch * iTileHeight);
			if (pBigTileBuf == NULL)
			{
				iError = PIL_ERROR_MEMORY;
				goto tile_end;
			}
		}
		if (iTileCount == 1) // a valid case where the file is a single tile
		{ // the tile sizes and offsets are the actual values for the single tile
			plTileSize[0] = iLargest = iTileSizes;
			plTiles[0] = iTileOffsets;
		}
		else
		{
			if (pFile->cState == PIL_FILE_STATE_LOADED) // we have all of the data in memory
			{
				pSizes = &pFile->pData[iTileSizes];
				pStrips = &pFile->pData[iTileOffsets];
				for (i = 0; i<iTileCount; i++)
				{
					if (iTileSizeType == 3)
						plTileSize[i] = PILTIFFSHORT(pSizes + i * 2, bMotorola);
					else
						plTileSize[i] = PILTIFFLONG(pSizes + i * 4, bMotorola);
					if (plTileSize[i] > iLargest)
						iLargest = plTileSize[i];
					iSize += plTileSize[i];
				}
				if (iSize <= 0) /* Some strange data */
				{
					iError = PIL_ERROR_BADHEADER;
					goto tile_end;
				}
				// get the strip offsets
				for (i = 0; i<iTileCount; i++)
				{
					plTiles[i] = PILTIFFLONG(pStrips + i * 4, bMotorola);
				}
			}
			else // we need to read the data from the file
			{
				pTemp = (unsigned char *) PILIOAlloc(iTileCount * sizeof(int));
				if (pTemp == NULL)
				{
					iError = PIL_ERROR_MEMORY;
					goto tile_end;
				}
				// first get the size information
				i = (iTileSizeType == 3) ? 2 : 4; // short or long?
		//		PILReadAtOffset(pFile, iTileSizes, pTemp, iTileCount*i);
				for (i = 0; i<iTileCount; i++)
				{
					if (iTileSizeType == 3)
						plTileSize[i] = PILTIFFSHORT(pTemp + i * 2, bMotorola);
					else
						plTileSize[i] = PILTIFFLONG(pTemp + i * 4, bMotorola);
					if (plTileSize[i] > iLargest)
						iLargest = plTileSize[i];
					iSize += plTileSize[i];
				}
				if (iSize <= 0) /* Some strange data */
				{
					iError = PIL_ERROR_BADHEADER;
					goto tile_end;
				}
				// now get the tile offsets
				i = (iTileSizeType == 3) ? 2 : 4; // short or long?
		//		PILReadAtOffset(pFile, iTileOffsets, pTemp, iTileCount*i);
				for (i = 0; i<iTileCount; i++)
				{
					plTiles[i] = PILTIFFLONG(pTemp + i * 4, bMotorola);
				}
				PILIOFree(pTemp);
			} // strips must be read from a file
		} // multiple tiles
		if (pPage->cBitsperpixel == 1) // allocate the IRLC buffer for 1bpp tiles
		{
			i = iSize * 13; // reasonable estimate
			if (i < pPage->iHeight * 64) // must be a strange file
				i = pPage->iHeight * 64;
			indexbuf = (unsigned char **) PILIOAlloc((pPage->iHeight * sizeof(char *)) + i); // RLC is never more than 12x G4 size
			if (!indexbuf) /* Allocate the irlc buffer structure */
			{
				iError = PIL_ERROR_MEMORY;
				goto tile_end;
			}
			irlcptr = (unsigned char *) &indexbuf[pPage->iHeight];
		}
		// We now have the tile offsets and sizes; read+decode each tile and construct the full image
		// Allocate the memory for the full image
//		pPage->iPitch = PILCalcSize(pPage->iWidth, pPage->cBitsperpixel);
		pTemp = (unsigned char *) PILIOAlloc(iLargest + 16); // largest tile compressed size
		pData = (unsigned char *) PILIOAlloc(pPage->iPitch * pPage->iHeight);
		if (pData == NULL)
		{
			iError = PIL_ERROR_MEMORY;
			goto tile_end;
		}
		// Each tile must be loaded individually and copied into the main image
		// since the right and bottom most tiles can extend way past the edge of the real image
		iTileCX = (pPage->iWidth + iTileWidth - 1) / iTileWidth;
		iTileCY = (pPage->iHeight + iTileHeight - 1) / iTileHeight;
		if (pPage->cBitsperpixel == 1) // special handling of 1bpp
		{
			int x, y, cy;
			unsigned char *pDest, *pSrc;
			PIL_PAGE pp1, pp2;
			for (y = 0; y<iTileCY; y++) // tile rows
			{
				for (x = 0; x<iTileCX; x++) // tile columns
				{
					i = y * iTileCX + x; // current tile number
			//		PILReadAtOffset(pFile, plTiles[i], pTemp, plTileSize[i]);
					// Decode the tile
					iError = 0;
					if (pPage->cCompression != PIL_COMP_NONE)
					{
						memset(&pp1, 0, sizeof(PIL_PAGE));
						memset(&pp2, 0, sizeof(PIL_PAGE));
						pp1.pData = pTemp;
						pp1.cState = PIL_PAGE_STATE_LOADED;
						pp1.iDataSize = plTileSize[i];
						pp1.iWidth = iTileWidth;
						pp1.iHeight = iTileHeight;
//						pp1.iPitch = PILCalcSize(iTileWidth, pPage->cBitsperpixel);
						pp1.cCompression = pPage->cCompression;
						pp1.cBitsperpixel = pPage->cBitsperpixel;
						pp1.cBitDir = pPage->cBitDir;
						pp2.cCompression = PIL_COMP_RLE;
				//		iError = PILConvert(&pp1, &pp2, 0, NULL, NULL);
						if (iError != 0)
							goto tile_end;
					}
					// Copy it onto the big tile buffer
					if (iError == 0)
					{
						pDest = pBigTileBuf + ((x*iTileWidth) >> 3);
						if (pPage->cCompression == PIL_COMP_NONE)
						{
							pSrc = pTemp;
							for (j = 0; j<iTileHeight; j++) // copy each scanline of the tile into the destination image
							{
								memcpy(pDest, pSrc, (iTileWidth >> 3));
								pDest += iBigTilePitch;
								pSrc += (iTileWidth >> 3);
							}
						}
						else // need to draw the tile into the big tile buffer
						{
							PIL_VIEW pv;
							memset(&pv, 0, sizeof(pv));
							pv.iPitch = iBigTilePitch;
							pv.iScaleX = pv.iScaleY = 256;
							pv.iWidth = iTileWidth;
							pv.iHeight = iTileHeight;
							pv.pBitmap = pDest;
//							PILDraw(&pp2, &pv, TRUE, NULL);
							if (pPage->cCompression != PIL_COMP_NONE)
								PILFree(&pp2); // free the decoded tile
						}
					}
				} // for x
				// capture this width block of tiles into the destination bitmap as IRLC data
				cy = iTileHeight;
				if (y*iTileHeight + cy > pPage->iHeight) // last row is a partial tile
					cy = pPage->iHeight - (y*iTileHeight);
				for (j = 0; j<cy; j++)
				{
					indexbuf[y*iTileHeight + j] = irlcptr;
			//		irlcptr = PILEncodeLine(pPage->iWidth, irlcptr, &pBigTileBuf[j*iBigTilePitch]); /* Encode this line */
				}
			} // for y
			PILIOFree(pData); // we no longer need the uncompressed image data
			pData = NULL;
		} // if 1bpp
		else // color or grayscale bitmaps
		{
			for (i = 0; i<iTileCount; i++)
			{
				PIL_PAGE pp1, pp2;
				unsigned char *pDest, *pSrc;
				int iFlags, cy, cx;
				int x = (i % iTileCX) * iTileWidth; // document X offset of current tile
				int y = (i / iTileCX) * iTileHeight; // document Y offset of current tile
		//		PILReadAtOffset(pFile, plTiles[i], pTemp, plTileSize[i]); // read the tile data
				// Decode the tile
				iError = 0;
				memset(&pp2, 0, sizeof(pp2)); // suppress compiler warning
				if (pPage->cCompression != PIL_COMP_NONE)
				{
					memset(&pp1, 0, sizeof(PIL_PAGE));
					memset(&pp2, 0, sizeof(PIL_PAGE));
					pp1.pData = pTemp;
					pp1.iDataSize = plTileSize[i];
					pp1.iWidth = iTileWidth;
					pp1.iHeight = iTileHeight;
					pp1.cState = PIL_PAGE_STATE_LOADED; // we have all of the data loaded
//					pp1.iPitch = PILCalcSize(iTileWidth, pPage->cBitsperpixel);
					pp1.cCompression = pPage->cCompression;
					pp1.cBitsperpixel = pPage->cBitsperpixel;
//					pp1.pLocalPalette = pPage->pLocalPalette; // in case of JPEG data tables
//					pp1.iAnnotationSize = pPage->iAnnotationSize; // JPEG data table size
					pp2.cCompression = PIL_COMP_NONE;
					iFlags = 0;
					if (pPage->cPhotometric == PIL_PHOTOMETRIC_RGB && pPage->cCompression == PIL_COMP_JPEG) // special case of RGB data compressed as JPEG
						iFlags |= PIL_CONVERT_JPEG_RGB;
					if (iOptions & PIL_CONVERT_SIMD)
						iFlags |= PIL_CONVERT_SIMD;
		//			iError = PILConvert(&pp1, &pp2, iFlags, NULL, NULL);
					if (iError != 0)
						goto tile_end;
				}
				// Copy it onto the main image
				if (iError == 0)
				{
					pDest = pData + (y * pPage->iPitch) + ((x*pPage->cBitsperpixel) >> 3);
					if (pPage->cCompression == PIL_COMP_NONE)
					{
						pSrc = pTemp;
						pp2.iPitch = ((iTileWidth*pPage->cBitsperpixel) >> 3);
					}
					else
						pSrc = pp2.pData;
					cy = iTileHeight;
					if (y + cy > pPage->iHeight) // last tile past bottom
						cy = pPage->iHeight - y;
					cx = iTileWidth;
					if (x + cx > pPage->iWidth) // last tile past right
						cx = pPage->iWidth - x;
					for (j = 0; j<cy; j++) // copy each scanline of the tile into the destination image
					{
						memcpy(pDest, pSrc, ((cx*pPage->cBitsperpixel) >> 3));
						pDest += pPage->iPitch;
						pSrc += pp2.iPitch;
					}
				}
				if (pPage->cCompression != PIL_COMP_NONE)
					PILFree(&pp2); // free the decoded tile
			}
		} // if grayscale/color
		PILIOFree(pPage->pData); // replace the compressed page with the uncompressed page
		if (pPage->cBitsperpixel == 1)
		{
			pPage->pData = (unsigned char *) indexbuf;
			pPage->cCompression = PIL_COMP_RLE;
			pPage->iDataSize = (int) (irlcptr - (unsigned char *) indexbuf);
			indexbuf = NULL; // don't let it get freed since we're keeping it as pPage->pData;
		}
		else
		{
			pPage->pData = pData;
			if (pPage->cCompression == PIL_COMP_JPEG)
			{
//				if (pPage->cPhotometric == PIL_PHOTOMETRIC_RGB) // special case of JPEG RGB - we've already fixed the RGB order
					pPage->cPhotometric = PIL_PHOTOMETRIC_NONE; // reset photometric value so that we don't "fix" this TIFF image again
			}
			pPage->cCompression = PIL_COMP_NONE;
			pPage->iDataSize = pPage->iPitch * pPage->iHeight;
		}
		pPage->cState = PIL_PAGE_STATE_LOADED; // mark the page as being loaded in memory
//		pPage->iStripCount = 0; // don't let it try to load the non-existant strips
	tile_end:
		PILIOFree(pTemp);
		PILIOFree(plTileSize);
		PILIOFree(plTiles);
		PILIOFree(pBigTileBuf);
		PILIOFree(indexbuf);
		if (iError != 0) // something went wrong, free all of the memory and leave
		{
			PILIOFree(pData);
			return iError;
		}
		else
		{
			pPage->cState = PIL_PAGE_STATE_LOADED; // the page is now loaded
		}
	} // read tiled image
	/*--- Gather and homogenize the strip information ---*/
	if (0) //pPage->iStripCount > 1)
	{
		int iSize;
		unsigned char *pSizes, *pStrips, *pTemp;

		iSize = 0;
		if ((pPage->cFlags & PIL_PAGEFLAGS_PLANAR) != 0)
		{
//			if (pPage->iStripCount > pPage->iHeight*3) // something is very wrong
//				return PIL_ERROR_BADHEADER;
		}
		else
		{
//			if (pPage->iStripCount > pPage->iHeight) // something is very wrong
//				return PIL_ERROR_BADHEADER;
		}
//		pPage->plStrips = (uint32_t *) PILIOAlloc(pPage->iStripCount * sizeof(uint32_t));
//		if (pPage->plStrips == NULL)
//			return PIL_ERROR_MEMORY;
//		pPage->plStripSize = (uint32_t *) PILIOAlloc(pPage->iStripCount * sizeof(uint32_t));
//		if (pPage->plStripSize == NULL)
//			return PIL_ERROR_MEMORY;
		if (pFile->cState == PIL_FILE_STATE_LOADED) // we have all of the data in memory
		{
			if (iSizePtr < 0 || iSizePtr > pFile->iFileSize) // bad header info
				return PIL_ERROR_BADHEADER;
			if (iStripPtr < 0 || iStripPtr > pFile->iFileSize)
				return PIL_ERROR_BADHEADER;
			pSizes = &pFile->pData[iSizePtr];
			pStrips = &pFile->pData[iStripPtr];
  i = 0;
//			for (i = 0; i<pPage->iStripCount; i++)
//			{
//				if (iSizeType == 3)
//					pPage->plStripSize[i] = PILTIFFSHORT(pSizes + i * 2, bMotorola);
//				else
//					pPage->plStripSize[i] = PILTIFFLONG(pSizes + i * 4, bMotorola);
//				iSize += pPage->plStripSize[i];
//			}
			pPage->iDataSize = iSize;
			if (iSize <= 0 || iSize > pFile->iFileSize) /* Some strange data */
			{
				return PIL_ERROR_BADHEADER;
			}
			// get the strip offsets
      i = 0;
//			for (i = 0; i<pPage->iStripCount; i++)
//			{
//				if (iOffType == 3) // shorts
//					pPage->plStrips[i] = PILTIFFSHORT(pStrips + i * 2, bMotorola);
//				else
//					pPage->plStrips[i] = PILTIFFLONG(pStrips + i * 4, bMotorola);
//			}
		}
		else // we need to read the data from the file
		{
//			pTemp = (unsigned char *) PILIOAlloc(pPage->iStripCount * sizeof(int));
			if (pTemp == NULL)
				return PIL_ERROR_MEMORY;
			// first get the size information
			i = (iSizeType == 3) ? 2 : 4; // short or long?
//			PILReadAtOffset(pFile, iSizePtr, pTemp, 1/*pPage->iStripCount*/ *i);
      i = 0;
//			for (i = 0; i<pPage->iStripCount; i++)
			{
//				if (iSizeType == 3)
//					pPage->plStripSize[i] = PILTIFFSHORT(pTemp + i * 2, bMotorola);
//				else
//					pPage->plStripSize[i] = PILTIFFLONG(pTemp + i * 4, bMotorola);
//				iSize += pPage->plStripSize[i];
			}
			pPage->iDataSize = iSize;
			if (iSize <= 0 || iSize > pFile->iFileSize) /* Some strange data */
			{
				PILIOFree(pTemp);
				return PIL_ERROR_BADHEADER;
			}
			i = (iOffType == 3) ? 2 : 4; // short or long?
		//	PILReadAtOffset(pFile, iStripPtr, pTemp, 1/*pPage->iStripCount*/ *i);
     i = 0;
//			for (i = 0; i<pPage->iStripCount; i++)
//			{
//				if (iOffType == 3)
//					pPage->plStrips[i] = PILTIFFSHORT(pTemp + i * 2, bMotorola);
//				else
//					pPage->plStrips[i] = PILTIFFLONG(pTemp + i * 4, bMotorola);
//			}
			PILIOFree(pTemp);
		} // strips must be read from a file
	} // multiple strips
	// If we got here without loading the page data, load it now
	if (pPage->cState != PIL_PAGE_STATE_LOADED)
	{
		int iCurrentSize;
		cBuf = (unsigned char *) PILIOAlloc(pPage->iDataSize);
		if (cBuf == NULL)
		{
			if (0) //pPage->iStripCount > 1)
			{
//				PILIOFree(pPage->plStrips);
//				PILIOFree(pPage->plStripSize);
//				pPage->plStrips = NULL;
//				pPage->plStripSize = NULL;
			}
			return PIL_ERROR_MEMORY; // out of memory error
		}
		if (0) //(pPage->iStripCount > 1)
		{
			p = cBuf;
			iCurrentSize = 0;
     i = 0;
//			for (i = 0; i<pPage->iStripCount; i++)
			{ // if source data is available and destination buffer can fit it
//				int iNewSize = pPage->plStripSize[i];
//				if ((int)(iCurrentSize + iNewSize) <= pPage->iDataSize && (int)(pPage->plStrips[i] + iNewSize) <= pFile->iFileSize)
//				{
			//		PILReadAtOffset(pFile, pPage->plStrips[i], p, iNewSize); // source and dest fit
//				}
//				else // data will go past end of old or new buffer; copy less than specified
				{
//					if ((iCurrentSize + iNewSize) > pPage->iDataSize)
//						iNewSize = (pPage->iDataSize - iCurrentSize); // destination can't fit it
//					if ((int)(pPage->plStrips[i] + iNewSize) > pFile->iFileSize)
//						iNewSize = (pFile->iFileSize - pPage->plStrips[i]); // source requirements too large
			//		if (iNewSize > 0)
				//		PILReadAtOffset(pFile, pPage->plStrips[i], p, iNewSize);
				}
//				p += iNewSize;
//				pPage->plStrips[i] = iCurrentSize; // new offset to each strip of data (could be compressed or not)
//				iCurrentSize += pPage->plStripSize[i];
			} // for each strip
		} // if more than 1 strip
//		else
//			PILReadAtOffset(pFile, pPage->iOffset, cBuf, pPage->iDataSize);
		pPage->cState = PIL_PAGE_STATE_LOADED;
		pPage->pData = cBuf;
		pPage->iOffset = 0;
	} // page data is now loaded into memory
	// Special case for uncompressed data in strips - the line size and strip size may be different
//	if (pPage->pData != NULL && pPage->iStripCount > 1 && pPage->cPhotometric != PIL_PHOTOMETRIC_YCBCR && pPage->cCompression == PIL_COMP_NONE && (pPage->cFlags & PIL_PAGEFLAGS_PLANAR) == 0) // only do it for "chunky" images
	{
		int y = 0;
    i =0;
//		for (i = 0; i < pPage->iStripCount; i++)
//		{
//			memcpy(&pPage->pData[y * pPage->iPitch], &pPage->pData[pPage->plStrips[i]], pPage->plStripSize[i]);
//			y += pPage->iRowCount;
//		}
		// "remove" the strips since we don't need them any more
//		pPage->iStripCount = 1;
//		PILIOFree(pPage->plStrips);
//		pPage->plStrips = NULL;
//		PILIOFree(pPage->plStripSize);
//		pPage->plStripSize = NULL;
	}
#ifdef USE_JPEG_CODEC
	// Special case where there are strips of JPEG data
//	if (pPage->pData != NULL && pPage->iStripCount > 1 && pPage->cCompression == PIL_COMP_JPEG)
	{
#ifdef FUTURE
		i = -1;
		if ((pPage->iRowCount & 0xf) == 0) // We can only glue the pieces together if they are exactly an integral number of MCUs high, otherwise there will be gaps
			i = PILRepackJPEG(pPage); // try the new way
		if (i == 0)
		{
			return i;
		}
		else // it failed, so do it the "old" way
#endif // FUTURE
		{
			PIL_PAGE pp1, pp2; // temp page to decode the whole image
			int iRowsLeft;
			int iFlags;
			unsigned char *pTemp;
			memcpy(&pp1, pPage, sizeof(PIL_PAGE)); // get size info etc
			// fix the pitch to ensure the slop-over parts of MCU don't corrupt the next lines
			pPage->iPitch = ((pPage->iWidth + 15) & 0xfff0) * 3;
			i = (pPage->iHeight + 32) & 0xfff0; // make sure we leave space for "slop over" lines in JPEG
			pTemp = (unsigned char *) PILIOAlloc(pPage->iPitch * i); // allocate the main page
			if (pTemp == NULL)
			{
				PILIOFree(pPage->pData); // free the compressed data
				pPage->pData = NULL;
				return PIL_ERROR_MEMORY;
			}
			memset(&pp2, 0, sizeof(pp2));
			pp1.pJPEG = PILPrepJPEGStruct(); // need to allocate jpeg tables
			iRowsLeft = pPage->iHeight;
			iFlags = PIL_CONVERT_NOALLOC | PIL_CONVERT_IGNORE_ERRORS;
			if (pPage->cPhotometric == PIL_PHOTOMETRIC_RGB) // RGB photometric
				iFlags |= PIL_CONVERT_JPEG_RGB;
			iFlags |= (iOptions & PIL_CONVERT_SIMD);
//			for (i = 0; i<pPage->iStripCount; i++) // loop through the strips
 i = 0;
			{
//				pp1.pData = &pPage->pData[pPage->plStrips[i]]; // point to start of strip
//				pp1.iDataSize = pPage->plStripSize[i];
				pp1.iOffset = 0;
				pp1.cState = PIL_PAGE_STATE_LOADED; // don't let it try to read data
//				pp1.cSpecial = 0; // reset "filtered" flag for each strip
//				if (iRowsLeft > pPage->iRowCount)
//					pp1.iHeight = pPage->iRowCount; // tell it the image height = strip height
//				else
//					pp1.iHeight = iRowsLeft; // last strip may have fewer rows
//				iRowsLeft -= pPage->iRowCount;
				pp2.cCompression = PIL_COMP_NONE;
				pp2.iPitch = pPage->iPitch;
//				pp2.pData = &pTemp[i*pPage->iRowCount*pPage->iPitch]; // point to the starting line of the strip
				if (i != 0 && (MOTOSHORT(pp1.pData) & 0xffc0) != 0xffc0) // Type 6 files can have strips of byte-aligned data with no SOS
				{
					if (pp1.pJPEG)
					{
						pp1.pJPEG->bStrip = TRUE;
					}
				}
//				j = PILReadJPEG(&pp1, &pp2, NULL, iFlags, FALSE, NULL, NULL);
// debug
j = 0;
				if (j != PIL_ERROR_SUCCESS)
				{
					PILIOFree(pTemp); // free uncompressed image
					PILIOFree(pPage->pData); // free compressed image
					pPage->pData = NULL;
					PILIOFree(pp1.pJPEG);
					return PIL_ERROR_DECOMP;
				}
			} // for each strip
			// we succeeded; move the data back into the main page and free the compressed data
			PILIOFree(pp1.pJPEG);
			PILIOFree(pPage->pData); // free the compressed data
			pPage->pData = pTemp; // uncompressed image
//			pPage->pPalette = pp2.pPalette; // for 8bpp grayscale, we need the palette that was allocated by PILReadJPEG()
			if (pPage->cBitsperpixel == 8 && pPage->cPhotometric == PIL_PHOTOMETRIC_WHITEISZERO) // need to invert the palette
			{
//				for (j = 0; j < 768; j++)
//					pPage->pPalette[j] = 255 - pPage->pPalette[j];
			}
//			PILIOFree(pPage->pLocalPalette); // free TIFF data tables
//			pPage->pLocalPalette = NULL;
			pPage->cCompression = PIL_COMP_NONE;
			pPage->iDataSize = pPage->iPitch * pPage->iHeight;
//			pPage->iStripCount = 0; // no more strips
//			PILIOFree(pPage->plStrips);
//			pPage->plStrips = NULL;
//			PILIOFree(pPage->plStripSize);
//			pPage->plStripSize = NULL;
			return 0; // we're done
		} // decode JPEG strips (old way)
	} // JPEG strips

	if (pPage->cPhotometric == PIL_PHOTOMETRIC_YCBCR && pPage->cCompression == PIL_COMP_NONE) // Convert/fix YCbCr colorspace uncompressed images
	{
//		return PILFixYCbCr(pPage);
	}
#endif
    
//	if ((pPage->cPhotometric == PIL_PHOTOMETRIC_CIELAB1 || pPage->cPhotometric == PIL_PHOTOMETRIC_CIELAB2) && pPage->cCompression == PIL_COMP_NONE) // Convert/fix CIE L*a*b* colorspace uncompressed images
//	{
//		return PILFixCIELAB(pPage);
//	}
	// Need to do this so we don't read past the end of memory-mapped files
	// only needed if we haven't had to decompress it already (e.g. tiled JPEG)
	if (pPage->cCompression != PIL_COMP_NONE && pPage->iDataSize > (signed) (pPage->iOffset + pFile->iFileSize)) // bad data size, fix it
	{
		pPage->iDataSize = pFile->iFileSize - pPage->iOffset; // assume one page of data
		if (pPage->iDataSize < 0)
			pPage->iDataSize = 0; // no way to fix it :(
	}
	if (iPredictor == 2) /* Horizontal differencing */
		pPage->cFlags |= PIL_PAGEFLAGS_PREDICTOR;
	/* Uncompressed bitmap data gets inverted by GetBMP() */
	//      if (pPage->cBitsperpixel == 1 && (pPage->cCompression == PIL_COMP_NONE || pPage->cCompression == PIL_COMP_TIFFPACKBITS))
	//         pPage->cPhotometric = 1 - pPage->cPhotometric;
	if (pPage->cPhotometric == PIL_PHOTOMETRIC_RGB)
	{
		if (pPage->cBitsperpixel != 24 && pPage->cBitsperpixel != 32 && pPage->cBitsperpixel != 48 && pPage->cBitsperpixel != 64)
		{
			pPage->cBitsperpixel = 24; /* Full color RGB */
			pPage->cPhotometric = PIL_PHOTOMETRIC_NONE;
		}
	}
	if (pPage->pData != NULL && (pPage->cPhotometric == PIL_PHOTOMETRIC_WHITEISZERO || pPage->cPhotometric == PIL_PHOTOMETRIC_BLACKISZERO) && (pPage->cBitsperpixel > 1 && pPage->cBitsperpixel <= 16)) /* Need to create a grayscale palette */
	{
//		pPage->pPalette = PILGrayPalette(pPage->cBitsperpixel > 8 ? 8 : pPage->cBitsperpixel);
		if (pPage->cPhotometric == PIL_PHOTOMETRIC_WHITEISZERO)
		{
			if (pPage->cCompression == PIL_COMP_NONE)
			{
				// invert the image
				for (i = 0; i<pPage->iDataSize; i++)
					pPage->pData[i] = 255 - pPage->pData[i];
				pPage->cPhotometric = PIL_PHOTOMETRIC_BLACKISZERO;
			}
			else // for compressed images, need to set photometric to inverted to force inversion after decompression
				pPage->cPhotometric = PIL_PHOTOMETRIC_WHITEISZERO;
		}
	}
	if (pPage->cPhotometric == PIL_PHOTOMETRIC_PALETTECOLOR && pPage->cBitsperpixel == 16 && pPage->pData != NULL) // strange case that we don't natively support; need to convert it
	{
		// Q&D - convert it to RGB565 so that we don't have to allocate more memory
		unsigned char *puc = (unsigned char *) &pPage->pData[pPage->iOffset];
		unsigned char *pucPalette;
		unsigned short us, usR, usG, usB;
		// translate all of the pixels through the color table
		if (pFile->cState == PIL_FILE_STATE_LOADED)
		{
			pucPalette = (unsigned char *) &pFile->pData[iColorTab];
		}
		else
		{
			pucPalette = (unsigned char *) PILIOAlloc(65536 * 3 * 2); // 384K just for the palette!
			if (pucPalette == NULL)
			{
				PILIOFree(pPage->pData); // free compressed image
				pPage->pData = NULL;
				return PIL_ERROR_MEMORY;
			}
		//	PILReadAtOffset(pFile, iColorTab, pucPalette, 65536 * 3 * 2);
		}
		for (i = 0; i<pPage->iWidth * pPage->iHeight; i++) // loop through every pixel
		{
			if (bMotorola)
			{
				us = MOTOSHORT(&puc[0]);
				usR = MOTOSHORT(&pucPalette[us * 2]);
				usG = MOTOSHORT(&pucPalette[us * 2 + 0x20000]);
				usB = MOTOSHORT(&pucPalette[us * 2 + 0x40000]);
			}
			else
			{
				us = INTELSHORT(&puc[0]);
				usR = INTELSHORT(&pucPalette[us * 2]);
				usG = INTELSHORT(&pucPalette[us * 2 + 0x20000]);
				usB = INTELSHORT(&pucPalette[us * 2 + 0x40000]);
			}
			*(unsigned short *) puc = (usB >> 11) | ((usG >> 10) << 5) | ((usR >> 11) << 11); // store back the RGB565 pixel
			puc += 2;
		} // for
		if (pFile->cState != PIL_FILE_STATE_LOADED)
		{
			PILIOFree(pucPalette);
		}
		pPage->cPhotometric = PIL_PHOTOMETRIC_RGB; // reset to non-palette color image
		pPage->cState = PIL_PAGE_STATE_LOADED; // the page is now loaded
		return 0;
	}
	if (pPage->cPhotometric == PIL_PHOTOMETRIC_PALETTECOLOR && pPage->cBitsperpixel <= 8) /* Palette color */
	{
		unsigned char *p;
		unsigned short us;
//		pPage->cPhotometric = PIL_PHOTOMETRIC_NONE;
		if (iColorTab == -1) /* Create grayscale palette */
  {
	//		pPage->pPalette = PILGrayPalette(pPage->cBitsperpixel);
  }
		else
		{
			int iShift;
			if (iColorTab < pFile->iFileSize)
			{
//				pPage->pPalette = (unsigned char *) PILIOAlloc(768);
//				if (pPage->pPalette == NULL)
//					return PIL_ERROR_MEMORY;
				if (pFile->cState == PIL_FILE_STATE_LOADED)
				{
					cBuf = &pFile->pData[iColorTab]; /* point to color table */
				}
				else
				{
	//				PILReadAtOffset(pFile, iColorTab, &cTemp[0], 1536);
					cBuf = &cTemp[0];
				}
				// The TIFF ColorMap is defined as SHORTs, but some programs write 8-bit values taking up 16-bits of space
				// check if the color values are written as 8 or 16-bits
				p = cBuf;
				if (!bMotorola)
					p++; // point to high byte
				iShift = 0; // assume 8-bit palette entries
				for (i = 0; i<1536; i += 2)
				{
					if (p[0] != 0) // the high byte is defined
					{
						iShift = 8; // 16-bit palette entries
						break;
					}
					p += 2;
				}
//				p = pPage->pPalette + 2; // Red is first
				j = 0;
				for (i = 0; i<(1 << pPage->cBitsperpixel); i++) /* Get the blue values */
				{
					if (bMotorola)
						us = MOTOSHORT(&cBuf[j]);
					else
						us = INTELSHORT(&cBuf[j]);
					us >>= iShift;
//					*p = (unsigned char) us;
//					p += 3;
					j += 2;
				}
				j = (int) (1L << pPage->cBitsperpixel)*sizeof(short);
//				p = pPage->pPalette + 1; // Green comes next
				for (i = 0; i<(1 << pPage->cBitsperpixel); i++) /* Get the green values */
				{
					if (bMotorola)
						us = MOTOSHORT(&cBuf[j]);
					else
						us = INTELSHORT(&cBuf[j]);
					us >>= iShift;
//					*p = (unsigned char) us;
//					p += 3;
					j += 2;
				}
//				p = pPage->pPalette; // followed by Blue
				j = 2 * (1 << pPage->cBitsperpixel)*sizeof(short);
				for (i = 0; i<(1 << pPage->cBitsperpixel); i++) /* Get the red values */
				{
					if (bMotorola)
						us = MOTOSHORT(&cBuf[j]);
					else
						us = INTELSHORT(&cBuf[j]);
					us >>= iShift;
					*p = (unsigned char) us;
					p += 3;
					j += 2;
				}
			}
		}
	}
//	if (pPage->cBitsperpixel == 2 && pPage->cCompression == PIL_COMP_NONE)
//		PILAdjustClr2(pPage, 4); // we don't support 2bpp, convert to 4
	if (bIBM) /* KLUDGE!! IBM does not know how to create valid TIFFs! */
	{
//		pPage->iStripCount = 0; // only 1 strip
//		pPage->iRowCount = pPage->iHeight;
	}
//	if (iPlanar == 2 && (pPage->cBitsperpixel == 24 || pPage->cBitsperpixel == 32) && pPage->cCompression == PIL_COMP_NONE)
//	{
//		int iErr;
//		iErr = PILFixPlanar(pPage); // fix a planar image
//		if (iErr != 0)
//			return iErr;
//		goto tiffinfo_exit;
//	}
	/* Check for 2D option with G3 data */
	if (pPage->cCompression == PIL_COMP_G31D && (iT4Options & 1))
		pPage->cCompression = PIL_COMP_G32D;
	/* Check for 24bpp uncompressed - need to fix the RGB order */
	else if (pPage->cCompression == PIL_COMP_NONE && pPage->cBitsperpixel >= 16 && pPage->pData != NULL)
	{
		if (pPage->cBitsperpixel == 32 && (pPage->cPhotometric == PIL_PHOTOMETRIC_WHITEISZERO || pPage->cPhotometric == PIL_PHOTOMETRIC_BLACKISZERO)) // 32-bit grayscale (rare)
		{
			unsigned char *p;
			int newpitch;
			pPage->cBitsperpixel >>= 2; // 32->8
//			if (pPage->pPalette == NULL)
//				pPage->pPalette = PILGrayPalette(8);
//			newpitch = PILCalcBSize(pPage->iWidth, pPage->cBitsperpixel);
			for (i = 0; i<pPage->iHeight; i++)
			{
				p = pPage->pData + (i * pPage->iPitch);
				if (!bMotorola)
					p += 3; // point to high byte
				cBuf = pPage->pData + (i * newpitch);
				for (j = 0; j<pPage->iWidth; j++)
				{
					*cBuf++ = *p;
					p += 4;
				}
			}
			pPage->iPitch = newpitch;
			pPage->iDataSize = pPage->iPitch * pPage->iHeight;
		}
		if ((pPage->cBitsperpixel == 16 || pPage->cBitsperpixel > 32) && pPage->pData != NULL) // 16bpp gray or 48 or 64bpp color
		{
			unsigned char *p;
			int newpitch, iHCount;
			pPage->cBitsperpixel >>= 1; // 48->24, 64->32, 16->8
//			newpitch = PILCalcBSize(pPage->iWidth, pPage->cBitsperpixel);
			iHCount = pPage->iWidth;
			iHCount *= (pPage->cBitsperpixel >> 3);
			for (i = 0; i<pPage->iHeight; i++)
			{
				p = pPage->pData + (i * pPage->iPitch);
				if (!bMotorola)
					p++; // point to high byte
				cBuf = pPage->pData + (i * newpitch);
				for (j = 0; j<iHCount; j++)
				{
					*cBuf++ = *p;
					p += 2;
				}
			}
			pPage->iPitch = newpitch;
			pPage->iDataSize = pPage->iPitch * pPage->iHeight;
		}
		if (pPage->cPhotometric == PIL_PHOTOMETRIC_RGB && pPage->cBitsperpixel == 24) // don't do it for CMYK or grayscale
		{
			cBuf = &pPage->pData[pPage->iOffset];
			for (i = 0; i<pPage->iHeight; i++)
			{
//				PILFixTIFFRGB(cBuf, pPage);
				cBuf += pPage->iPitch;
			}
		}
	}
//tiffinfo_exit:
//	if (pPage->iStripCount > 1 && pPage->cCompression == PIL_COMP_NONE)
	{
		// if not compressed, we no longer need the strip info since we read in the whole file
//		PILIOFree(pPage->plStrips);
//		PILIOFree(pPage->plStripSize);
//		pPage->plStrips = NULL;
//		pPage->plStripSize = NULL;
//		pPage->iStripCount = 1;
	}
	if (pPage->cPhotometric == PIL_PHOTOMETRIC_SEPARATED && pPage->cCompression == PIL_COMP_NONE && pPage->cBitsperpixel == 32) // CMYK
	{
		// this works for images without color profiles, but nothing else
//		PILConvertCMYK(pPage);
	}
	pPage->cState = PIL_PAGE_STATE_LOADED; // the page is now loaded
	pPage->iOffset = 0;
	return 0;

} /* PILTIFFInfo() */

/****************************************************************************
 *                                                                          *
 *  FUNCTION   : PILClose()                                                 *
 *                                                                          *
 *  PURPOSE    : Close an image file.                                       *
 *                                                                          *
 ****************************************************************************/
int PILClose(PIL_FILE *pFile)
{

   if (pFile->cState == PIL_FILE_STATE_OPEN)
      {
      PILIOClose(pFile->iFile);
      pFile->iFile = (void *)-1;
      }
#ifdef USE_JPEG_CODEC
   if (pFile->pJPEG)
      {
      PILFreeHuffTables(pFile->pJPEG);
      PILIOFree(pFile->pJPEG);
      pFile->pJPEG = NULL;
      }
#endif
   if (pFile->pSoundList)
      {
      PILIOFree(pFile->pSoundList);
      pFile->pSoundList = NULL;
      }
   if (pFile->pKeyFlags)
      {
      PILIOFree(pFile->pKeyFlags);
      pFile->pKeyFlags = NULL;
      }
   if (pFile->pSoundLens)
      {
      PILIOFree(pFile->pSoundLens);
      pFile->pSoundLens = NULL;
      }
   if (pFile->pPageList)
      {
      PILIOFree(pFile->pPageList);
      pFile->pPageList = NULL;
      }
   if (pFile->pPageLens)
      {
      PILIOFree(pFile->pPageLens);
      pFile->pPageLens = NULL;
      }
   if (pFile->lUser != NULL && pFile->lUser != (void *)-1)
      {
	  PILIOFree((void *) pFile->lUser);
	  pFile->lUser = NULL;
      }
   pFile->cState = PIL_FILE_STATE_CLOSED; // reset state flags
   return 0;
} /* PILClose() */

/****************************************************************************
 *                                                                          *
 *  FUNCTION   : PILCheckHighWater(PIL_PAGE *, void *)                      *
 *                                                                          *
 *  PURPOSE    : Adjust output buffer as data grows                         *
 *                                                                          *
 ****************************************************************************/
int PILCheckHighWater(PIL_PAGE *pPage, void *p)
{
int iErr = 0;

//    if ((unsigned char *)p >= pPage->pHighWater) // need to adjust the buffer
    {
    void *pTemp = NULL;
//        pPage->iCurrentBufferSize += pPage->iBufferIncrement;
//        pPage->pHighWater += pPage->iBufferIncrement;
//        pTemp = PILIOReAlloc(pPage->pData, pPage->iCurrentBufferSize);
        if (pTemp != pPage->pData) // buffer got moved; we need to re-arrange things
        {
            if (pTemp == NULL) // failed to allocate; everything is now toast
            {
                iErr = PIL_ERROR_MEMORY;
            }
            else // re-arrange the buffer contents
            {
            }
        }
    }
    return iErr;
} /* PILCheckHighWater() */

/****************************************************************************
 *                                                                          *
 *  FUNCTION   : PILFree()                                                  *
 *                                                                          *
 *  PURPOSE    : Free the page                                              *
 *                                                                          *
 ****************************************************************************/
int PILFree(PIL_PAGE *pPage)
{
int iErr = 0;
int i;

   if (pPage == NULL)
      return PIL_ERROR_INVPARAM;

   i = 0; // total memory blocks freed
   if (pPage->pData)
      {
      PILIOFree(pPage->pData);
      pPage->pData = NULL;
      i++;
      }
   if (i == 0) // nothing to free
      iErr = PIL_ERROR_INVPARAM;
   return iErr;

} /* PILFree() */

#ifdef __cplusplus
}
#endif // __cplusplus
