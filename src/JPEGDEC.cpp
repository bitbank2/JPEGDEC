//
// JPEG Decoder
//
// written by Larry Bank
// bitbank@pobox.com
// Arduino port started 8/2/2020
// Original JPEG code written 26+ years ago :)
// The goal of this code is to decode baseline JPEG images
// using no more than 18K of RAM (if sent directly to an LCD display)
//
// Copyright 2020 BitBank Software, Inc. All Rights Reserved.
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//    http://www.apache.org/licenses/LICENSE-2.0
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.
//===========================================================================
//
#include "JPEGDEC.h"

// The portable C code is compiled separately from jpegdec.c

void JPEGDEC::setFramebuffer(void *pFramebuffer)
{
    JPEG_setFramebuffer(&_jpeg, pFramebuffer);
} /* setFramebuffer() */

int JPEGDEC::getPixelType()
{
    return JPEG_getPixelType(&_jpeg);
}
void JPEGDEC::setPixelType(int iType)
{
    JPEG_setPixelType(&_jpeg, iType);
} /* setPixelType() */

void JPEGDEC::setMaxOutputSize(int iMaxMCUs)
{
    JPEG_setMaxOutputSize(&_jpeg, iMaxMCUs);
} /* setMaxOutputSize() */
//
// Memory initialization
//
int JPEGDEC::openRAM(uint8_t *pData, int iDataSize, JPEG_DRAW_CALLBACK *pfnDraw)
{
    return JPEG_openRAM(&_jpeg, pData, iDataSize, pfnDraw);
} /* openRAM() */

int JPEGDEC::openFLASH(const uint8_t *pData, int iDataSize, JPEG_DRAW_CALLBACK *pfnDraw)
{
    return JPEG_openFLASH(&_jpeg, pData, iDataSize, pfnDraw);
} /* openFLASH() */

int JPEGDEC::getOrientation()
{
    return JPEG_getOrientation(&_jpeg);
} /* getOrientation() */

int JPEGDEC::getLastError()
{
    return JPEG_getLastError(&_jpeg);
} /* getLastError() */

int JPEGDEC::getWidth()
{
    return JPEG_getWidth(&_jpeg);
} /* getWidth() */

int JPEGDEC::getHeight()
{
    return JPEG_getHeight(&_jpeg);
} /* getHeight() */

int JPEGDEC::hasThumb()
{
    return JPEG_hasThumb(&_jpeg);
} /* hasThumb() */

int JPEGDEC::getThumbWidth()
{
    return JPEG_getThumbWidth(&_jpeg);
} /* getThumbWidth() */

int JPEGDEC::getThumbHeight()
{
    return JPEG_getThumbHeight(&_jpeg);
} /* getThumbHeight() */

int JPEGDEC::getBpp()
{
    return JPEG_getBpp(&_jpeg);
} /* getBpp() */

int JPEGDEC::getSubSample()
{
    return JPEG_getSubSample(&_jpeg);
} /* getSubSample() */

void JPEGDEC::setCropArea(int x, int y, int w, int h)
{
    JPEG_setCropArea(&_jpeg, x, y, w, h);
} /* setCropArea() */

void JPEGDEC::getCropArea(int *x, int *y, int *w, int *h)
{
    JPEG_getCropArea(&_jpeg, x, y, w, h);
} /* getCropArea() */

int JPEGDEC::getJPEGType()
{
    return JPEG_getJPEGType(&_jpeg);
} /* getJPEGType() */

#ifdef __LINUX__
int JPEGDEC::open(const char *szFilename, JPEG_DRAW_CALLBACK *pfnDraw)
{
    return JPEG_openFile(&_jpeg, szFilename, pfnDraw);
} /* open() */
#endif // __LINUX__
//
// File (SD/MMC) based initialization
//
int JPEGDEC::open(const char *szFilename, JPEG_OPEN_CALLBACK *pfnOpen, JPEG_CLOSE_CALLBACK *pfnClose, JPEG_READ_CALLBACK *pfnRead, JPEG_SEEK_CALLBACK *pfnSeek, JPEG_DRAW_CALLBACK *pfnDraw)
{
    return JPEG_open(&_jpeg, szFilename, pfnOpen, pfnClose, pfnRead, pfnSeek, pfnDraw);

} /* open() */

//
// data stream initialization
//
int JPEGDEC::open(void *fHandle, int iDataSize, JPEG_CLOSE_CALLBACK *pfnClose, JPEG_READ_CALLBACK *pfnRead, JPEG_SEEK_CALLBACK *pfnSeek, JPEG_DRAW_CALLBACK *pfnDraw)
{
    return JPEG_openHandle(&_jpeg, fHandle, iDataSize, pfnClose, pfnRead, pfnSeek, pfnDraw);
} /* open() */

#ifdef FS_H
static int32_t FileRead(JPEGFILE *handle, uint8_t *buffer, int32_t length)
{
    return ((File *)(handle->fHandle))->read(buffer, length);
}
static int32_t FileSeek(JPEGFILE *handle, int32_t position)
{
    return ((File *)(handle->fHandle))->seek(position);
}
static void FileClose(void *handle)
{
    ((File *)handle)->close();
}

int JPEGDEC::open(File &file, JPEG_DRAW_CALLBACK *pfnDraw)
{
    if (!file) return 0;
    return JPEG_openHandle(&_jpeg, &file, file.size(), FileClose, FileRead, FileSeek, pfnDraw);
}
#endif // FS_H

void JPEGDEC::close()
{
    JPEG_close(&_jpeg);
} /* close() */

//
// Decode the image
// returns:
// 1 = good result
// 0 = error
//
int JPEGDEC::decode(int x, int y, int iOptions)
{
    return JPEG_decode(&_jpeg, x, y, iOptions);
} /* decode() */
//
// set draw callback user pointer variable
//
void JPEGDEC::setUserPointer(void *p)
{
    JPEG_setUserPointer(&_jpeg, p);
}

int JPEGDEC::decodeDither(int x, int y, uint8_t *pDither, int iOptions)
{
    return JPEG_decodeDitherAt(&_jpeg, x, y, pDither, iOptions);
}

int JPEGDEC::decodeDither(uint8_t *pDither, int iOptions)
{
    return JPEG_decodeDither(&_jpeg, pDither, iOptions);
}
