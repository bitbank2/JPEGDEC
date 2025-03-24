//
// JPEG Display helper class
//
// written by Larry Bank
// bitbank@pobox.com
//
// Copyright 2025 BitBank Software, Inc. All Rights Reserved.
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
#ifndef __JPEGDISPLAY__
#define __JPEGDISPLAY__
#include <JPEGDEC.h>
#include <bb_spi_lcd.h>

class JPEGDisplay
{
  public:
    int loadJPEG(BB_SPI_LCD *pLCD, int x, int y, const void *pData, int iDataSize);
    int loadJPEG(BB_SPI_LCD *pLCD, int x, int y, const char *fname);
    int getJPEGInfo(int *width, int *height, int *bpp, const void *pData, int iDataSize);
    int getJPEGInfo(int *width, int *height, int *bpp, const char *fname);
};

#endif // __JPEGDISPLAY__
