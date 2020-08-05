//
// JPEG Decoder
// written by Larry Bank
// bitbank@pobox.com
// Arduino port started 8/2/2020
// Original JPEG code written 26+ years ago :)
// The goal of this code is to decode baseline JPEG images
// using no more than 22K of RAM (if sent directly to an LCD display)
//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program.  If not, see <http://www.gnu.org/licenses/>.
//
#include "JPEGDEC.h"

// forward references
static int JPEGInit(JPEGIMAGE *pJPEG);
static int JPEGParseInfo(JPEGIMAGE *pPage);
static void JPEGGetMoreData(JPEGIMAGE *pPage);
static int DecodeJPEG(JPEGIMAGE *pImage, int iOptions);

/* JPEG tables */
// zigzag ordering of DCT coefficients
static const unsigned char cZigZag[64] = {0,1,5,6,14,15,27,28,
    2,4,7,13,16,26,29,42,
    3,8,12,17,25,30,41,43,
    9,11,18,24,31,40,44,53,
    10,19,23,32,39,45,52,54,
    20,22,33,38,46,51,55,60,
    21,34,37,47,50,56,59,61,
    35,36,48,49,57,58,62,63};

// un-zigzag ordering
static const unsigned char cZigZag2[64] = {0,1,8,16,9,2,3,10,
    17,24,32,25,18,11,4,5,
    12,19,26,33,40,48,41,34,
    27,20,13,6,7,14,21,28,
    35,42,49,56,57,50,43,36,
    29,22,15,23,30,37,44,51,
    58,59,52,45,38,31,39,46,
    53,60,61,54,47,55,62,63};

// For AA&N IDCT method, multipliers are equal to quantization
// coefficients scaled by scalefactor[row]*scalefactor[col], where
// scalefactor[0] = 1
// scalefactor[k] = cos(k*PI/16) * sqrt(2)    for k=1..7
// For integer operation, the multiplier table is to be scaled by
// IFAST_SCALE_BITS.
static const int iScaleBits[64] = {16384, 22725, 21407, 19266, 16384, 12873,  8867,  4520,
    22725, 31521, 29692, 26722, 22725, 17855, 12299,  6270,
    21407, 29692, 27969, 25172, 21407, 16819, 11585,  5906,
    19266, 26722, 25172, 22654, 19266, 15137, 10426,  5315,
    16384, 22725, 21407, 19266, 16384, 12873,  8867,  4520,
    12873, 17855, 16819, 15137, 12873, 10114,  6967,  3552,
    8867, 12299, 11585, 10426,  8867,  6967,  4799,  2446,
    4520,  6270,  5906,  5315,  4520,  3552,  2446,  1247};
//
// Range clip and shift for RGB565 output
// input value is 0 to 255, then another 256 for overflow to FF, then 512 more for negative values wrapping around
// Trims a few instructions off the final output stage
//
static const uint16_t usRangeTabR[] = {0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000, // 0
    0x0800,0x0800,0x0800,0x0800,0x0800,0x0800,0x0800,0x0800,
    0x1000,0x1000,0x1000,0x1000,0x1000,0x1000,0x1000,0x1000,
    0x1800,0x1800,0x1800,0x1800,0x1800,0x1800,0x1800,0x1800,
    0x2000,0x2000,0x2000,0x2000,0x2000,0x2000,0x2000,0x2000,
    0x2800,0x2800,0x2800,0x2800,0x2800,0x2800,0x2800,0x2800,
    0x3000,0x3000,0x3000,0x3000,0x3000,0x3000,0x3000,0x3000,
    0x3800,0x3800,0x3800,0x3800,0x3800,0x3800,0x3800,0x3800,
    0x4000,0x4000,0x4000,0x4000,0x4000,0x4000,0x4000,0x4000,
    0x4800,0x4800,0x4800,0x4800,0x4800,0x4800,0x4800,0x4800,
    0x5000,0x5000,0x5000,0x5000,0x5000,0x5000,0x5000,0x5000,
    0x5800,0x5800,0x5800,0x5800,0x5800,0x5800,0x5800,0x5800,
    0x6000,0x6000,0x6000,0x6000,0x6000,0x6000,0x6000,0x6000,
    0x6800,0x6800,0x6800,0x6800,0x6800,0x6800,0x6800,0x6800,
    0x7000,0x7000,0x7000,0x7000,0x7000,0x7000,0x7000,0x7000,
    0x7800,0x7800,0x7800,0x7800,0x7800,0x7800,0x7800,0x7800,
    0x8000,0x8000,0x8000,0x8000,0x8000,0x8000,0x8000,0x8000,
    0x8800,0x8800,0x8800,0x8800,0x8800,0x8800,0x8800,0x8800,
    0x9000,0x9000,0x9000,0x9000,0x9000,0x9000,0x9000,0x9000,
    0x9800,0x9800,0x9800,0x9800,0x9800,0x9800,0x9800,0x9800,
    0xa000,0xa000,0xa000,0xa000,0xa000,0xa000,0xa000,0xa000,
    0xa800,0xa800,0xa800,0xa800,0xa800,0xa800,0xa800,0xa800,
    0xb000,0xb000,0xb000,0xb000,0xb000,0xb000,0xb000,0xb000,
    0xb800,0xb800,0xb800,0xb800,0xb800,0xb800,0xb800,0xb800,
    0xc000,0xc000,0xc000,0xc000,0xc000,0xc000,0xc000,0xc000,
    0xc800,0xc800,0xc800,0xc800,0xc800,0xc800,0xc800,0xc800,
    0xd000,0xd000,0xd000,0xd000,0xd000,0xd000,0xd000,0xd000,
    0xd800,0xd800,0xd800,0xd800,0xd800,0xd800,0xd800,0xd800,
    0xe000,0xe000,0xe000,0xe000,0xe000,0xe000,0xe000,0xe000,
    0xe800,0xe800,0xe800,0xe800,0xe800,0xe800,0xe800,0xe800,
    0xf000,0xf000,0xf000,0xf000,0xf000,0xf000,0xf000,0xf000,
    0xf800,0xf800,0xf800,0xf800,0xf800,0xf800,0xf800,0xf800,
    0xf800,0xf800,0xf800,0xf800,0xf800,0xf800,0xf800,0xf800, // 256
    0xf800,0xf800,0xf800,0xf800,0xf800,0xf800,0xf800,0xf800,
    0xf800,0xf800,0xf800,0xf800,0xf800,0xf800,0xf800,0xf800,
    0xf800,0xf800,0xf800,0xf800,0xf800,0xf800,0xf800,0xf800,
    0xf800,0xf800,0xf800,0xf800,0xf800,0xf800,0xf800,0xf800,
    0xf800,0xf800,0xf800,0xf800,0xf800,0xf800,0xf800,0xf800,
    0xf800,0xf800,0xf800,0xf800,0xf800,0xf800,0xf800,0xf800,
    0xf800,0xf800,0xf800,0xf800,0xf800,0xf800,0xf800,0xf800,
    0xf800,0xf800,0xf800,0xf800,0xf800,0xf800,0xf800,0xf800,
    0xf800,0xf800,0xf800,0xf800,0xf800,0xf800,0xf800,0xf800,
    0xf800,0xf800,0xf800,0xf800,0xf800,0xf800,0xf800,0xf800,
    0xf800,0xf800,0xf800,0xf800,0xf800,0xf800,0xf800,0xf800,
    0xf800,0xf800,0xf800,0xf800,0xf800,0xf800,0xf800,0xf800,
    0xf800,0xf800,0xf800,0xf800,0xf800,0xf800,0xf800,0xf800,
    0xf800,0xf800,0xf800,0xf800,0xf800,0xf800,0xf800,0xf800,
    0xf800,0xf800,0xf800,0xf800,0xf800,0xf800,0xf800,0xf800,
    0xf800,0xf800,0xf800,0xf800,0xf800,0xf800,0xf800,0xf800,
    0xf800,0xf800,0xf800,0xf800,0xf800,0xf800,0xf800,0xf800,
    0xf800,0xf800,0xf800,0xf800,0xf800,0xf800,0xf800,0xf800,
    0xf800,0xf800,0xf800,0xf800,0xf800,0xf800,0xf800,0xf800,
    0xf800,0xf800,0xf800,0xf800,0xf800,0xf800,0xf800,0xf800,
    0xf800,0xf800,0xf800,0xf800,0xf800,0xf800,0xf800,0xf800,
    0xf800,0xf800,0xf800,0xf800,0xf800,0xf800,0xf800,0xf800,
    0xf800,0xf800,0xf800,0xf800,0xf800,0xf800,0xf800,0xf800,
    0xf800,0xf800,0xf800,0xf800,0xf800,0xf800,0xf800,0xf800,
    0xf800,0xf800,0xf800,0xf800,0xf800,0xf800,0xf800,0xf800,
    0xf800,0xf800,0xf800,0xf800,0xf800,0xf800,0xf800,0xf800,
    0xf800,0xf800,0xf800,0xf800,0xf800,0xf800,0xf800,0xf800,
    0xf800,0xf800,0xf800,0xf800,0xf800,0xf800,0xf800,0xf800,
    0xf800,0xf800,0xf800,0xf800,0xf800,0xf800,0xf800,0xf800,
    0xf800,0xf800,0xf800,0xf800,0xf800,0xf800,0xf800,0xf800,
    0xf800,0xf800,0xf800,0xf800,0xf800,0xf800,0xf800,0xf800,
    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0, // 512
    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0, // 768
    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};
static const uint16_t usRangeTabG[] = {0x0000,0x0000,0x0000,0x0000,0x0020,0x0020,0x0020,0x0020, // 0
    0x0040,0x0040,0x0040,0x0040,0x0060,0x0060,0x0060,0x0060,
    0x0080,0x0080,0x0080,0x0080,0x00a0,0x00a0,0x00a0,0x00a0,
    0x00c0,0x00c0,0x00c0,0x00c0,0x00e0,0x00e0,0x00e0,0x00e0,
    0x0100,0x0100,0x0100,0x0100,0x0120,0x0120,0x0120,0x0120,
    0x0140,0x0140,0x0140,0x0140,0x0160,0x0160,0x0160,0x0160,
    0x0180,0x0180,0x0180,0x0180,0x01a0,0x01a0,0x01a0,0x01a0,
    0x01c0,0x01c0,0x01c0,0x01c0,0x01e0,0x01e0,0x01e0,0x01e0,
    0x0200,0x0200,0x0200,0x0200,0x0220,0x0220,0x0220,0x0220,
    0x0240,0x0240,0x0240,0x0240,0x0260,0x0260,0x0260,0x0260,
    0x0280,0x0280,0x0280,0x0280,0x02a0,0x02a0,0x02a0,0x02a0,
    0x02c0,0x02c0,0x02c0,0x02c0,0x02e0,0x02e0,0x02e0,0x02e0,
    0x0300,0x0300,0x0300,0x0300,0x0320,0x0320,0x0320,0x0320,
    0x0340,0x0340,0x0340,0x0340,0x0360,0x0360,0x0360,0x0360,
    0x0380,0x0380,0x0380,0x0380,0x03a0,0x03a0,0x03a0,0x03a0,
    0x03c0,0x03c0,0x03c0,0x03c0,0x03e0,0x03e0,0x03e0,0x03e0,
    0x0400,0x0400,0x0400,0x0400,0x0420,0x0420,0x0420,0x0420,
    0x0440,0x0440,0x0440,0x0440,0x0460,0x0460,0x0460,0x0460,
    0x0480,0x0480,0x0480,0x0480,0x04a0,0x04a0,0x04a0,0x04a0,
    0x04c0,0x04c0,0x04c0,0x04c0,0x04e0,0x04e0,0x04e0,0x04e0,
    0x0500,0x0500,0x0500,0x0500,0x0520,0x0520,0x0520,0x0520,
    0x0540,0x0540,0x0540,0x0540,0x0560,0x0560,0x0560,0x0560,
    0x0580,0x0580,0x0580,0x0580,0x05a0,0x05a0,0x05a0,0x05a0,
    0x05c0,0x05c0,0x05c0,0x05c0,0x05e0,0x05e0,0x05e0,0x05e0,
    0x0600,0x0600,0x0600,0x0600,0x0620,0x0620,0x0620,0x0620,
    0x0640,0x0640,0x0640,0x0640,0x0660,0x0660,0x0660,0x0660,
    0x0680,0x0680,0x0680,0x0680,0x06a0,0x06a0,0x06a0,0x06a0,
    0x06c0,0x06c0,0x06c0,0x06c0,0x06e0,0x06e0,0x06e0,0x06e0,
    0x0700,0x0700,0x0700,0x0700,0x0720,0x0720,0x0720,0x0720,
    0x0740,0x0740,0x0740,0x0740,0x0760,0x0760,0x0760,0x0760,
    0x0780,0x0780,0x0780,0x0780,0x07a0,0x07a0,0x07a0,0x07a0,
    0x07c0,0x07c0,0x07c0,0x07c0,0x07e0,0x07e0,0x07e0,0x07e0,
    0x07e0,0x07e0,0x07e0,0x07e0,0x07e0,0x07e0,0x07e0,0x07e0, // 256
    0x07e0,0x07e0,0x07e0,0x07e0,0x07e0,0x07e0,0x07e0,0x07e0,
    0x07e0,0x07e0,0x07e0,0x07e0,0x07e0,0x07e0,0x07e0,0x07e0,
    0x07e0,0x07e0,0x07e0,0x07e0,0x07e0,0x07e0,0x07e0,0x07e0,
    0x07e0,0x07e0,0x07e0,0x07e0,0x07e0,0x07e0,0x07e0,0x07e0,
    0x07e0,0x07e0,0x07e0,0x07e0,0x07e0,0x07e0,0x07e0,0x07e0,
    0x07e0,0x07e0,0x07e0,0x07e0,0x07e0,0x07e0,0x07e0,0x07e0,
    0x07e0,0x07e0,0x07e0,0x07e0,0x07e0,0x07e0,0x07e0,0x07e0,
    0x07e0,0x07e0,0x07e0,0x07e0,0x07e0,0x07e0,0x07e0,0x07e0,
    0x07e0,0x07e0,0x07e0,0x07e0,0x07e0,0x07e0,0x07e0,0x07e0,
    0x07e0,0x07e0,0x07e0,0x07e0,0x07e0,0x07e0,0x07e0,0x07e0,
    0x07e0,0x07e0,0x07e0,0x07e0,0x07e0,0x07e0,0x07e0,0x07e0,
    0x07e0,0x07e0,0x07e0,0x07e0,0x07e0,0x07e0,0x07e0,0x07e0,
    0x07e0,0x07e0,0x07e0,0x07e0,0x07e0,0x07e0,0x07e0,0x07e0,
    0x07e0,0x07e0,0x07e0,0x07e0,0x07e0,0x07e0,0x07e0,0x07e0,
    0x07e0,0x07e0,0x07e0,0x07e0,0x07e0,0x07e0,0x07e0,0x07e0,
    0x07e0,0x07e0,0x07e0,0x07e0,0x07e0,0x07e0,0x07e0,0x07e0,
    0x07e0,0x07e0,0x07e0,0x07e0,0x07e0,0x07e0,0x07e0,0x07e0,
    0x07e0,0x07e0,0x07e0,0x07e0,0x07e0,0x07e0,0x07e0,0x07e0,
    0x07e0,0x07e0,0x07e0,0x07e0,0x07e0,0x07e0,0x07e0,0x07e0,
    0x07e0,0x07e0,0x07e0,0x07e0,0x07e0,0x07e0,0x07e0,0x07e0,
    0x07e0,0x07e0,0x07e0,0x07e0,0x07e0,0x07e0,0x07e0,0x07e0,
    0x07e0,0x07e0,0x07e0,0x07e0,0x07e0,0x07e0,0x07e0,0x07e0,
    0x07e0,0x07e0,0x07e0,0x07e0,0x07e0,0x07e0,0x07e0,0x07e0,
    0x07e0,0x07e0,0x07e0,0x07e0,0x07e0,0x07e0,0x07e0,0x07e0,
    0x07e0,0x07e0,0x07e0,0x07e0,0x07e0,0x07e0,0x07e0,0x07e0,
    0x07e0,0x07e0,0x07e0,0x07e0,0x07e0,0x07e0,0x07e0,0x07e0,
    0x07e0,0x07e0,0x07e0,0x07e0,0x07e0,0x07e0,0x07e0,0x07e0,
    0x07e0,0x07e0,0x07e0,0x07e0,0x07e0,0x07e0,0x07e0,0x07e0,
    0x07e0,0x07e0,0x07e0,0x07e0,0x07e0,0x07e0,0x07e0,0x07e0,
    0x07e0,0x07e0,0x07e0,0x07e0,0x07e0,0x07e0,0x07e0,0x07e0,
    0x07e0,0x07e0,0x07e0,0x07e0,0x07e0,0x07e0,0x07e0,0x07e0,
    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0, // 512
    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0, // 768
    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};
static const uint16_t usRangeTabB[] = {0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000, // 0
    0x0001,0x0001,0x0001,0x0001,0x0001,0x0001,0x0001,0x0001,
    0x0002,0x0002,0x0002,0x0002,0x0002,0x0002,0x0002,0x0002,
    0x0003,0x0003,0x0003,0x0003,0x0003,0x0003,0x0003,0x0003,
    0x0004,0x0004,0x0004,0x0004,0x0004,0x0004,0x0004,0x0004,
    0x0005,0x0005,0x0005,0x0005,0x0005,0x0005,0x0005,0x0005,
    0x0006,0x0006,0x0006,0x0006,0x0006,0x0006,0x0006,0x0006,
    0x0007,0x0007,0x0007,0x0007,0x0007,0x0007,0x0007,0x0007,
    0x0008,0x0008,0x0008,0x0008,0x0008,0x0008,0x0008,0x0008,
    0x0009,0x0009,0x0009,0x0009,0x0009,0x0009,0x0009,0x0009,
    0x000a,0x000a,0x000a,0x000a,0x000a,0x000a,0x000a,0x000a,
    0x000b,0x000b,0x000b,0x000b,0x000b,0x000b,0x000b,0x000b,
    0x000c,0x000c,0x000c,0x000c,0x000c,0x000c,0x000c,0x000c,
    0x000d,0x000d,0x000d,0x000d,0x000d,0x000d,0x000d,0x000d,
    0x000e,0x000e,0x000e,0x000e,0x000e,0x000e,0x000e,0x000e,
    0x000f,0x000f,0x000f,0x000f,0x000f,0x000f,0x000f,0x000f,
    0x0010,0x0010,0x0010,0x0010,0x0010,0x0010,0x0010,0x0010,
    0x0011,0x0011,0x0011,0x0011,0x0011,0x0011,0x0011,0x0011,
    0x0012,0x0012,0x0012,0x0012,0x0012,0x0012,0x0012,0x0012,
    0x0013,0x0013,0x0013,0x0013,0x0013,0x0013,0x0013,0x0013,
    0x0014,0x0014,0x0014,0x0014,0x0014,0x0014,0x0014,0x0014,
    0x0015,0x0015,0x0015,0x0015,0x0015,0x0015,0x0015,0x0015,
    0x0016,0x0016,0x0016,0x0016,0x0016,0x0016,0x0016,0x0016,
    0x0017,0x0017,0x0017,0x0017,0x0017,0x0017,0x0017,0x0017,
    0x0018,0x0018,0x0018,0x0018,0x0018,0x0018,0x0018,0x0018,
    0x0019,0x0019,0x0019,0x0019,0x0019,0x0019,0x0019,0x0019,
    0x001a,0x001a,0x001a,0x001a,0x001a,0x001a,0x001a,0x001a,
    0x001b,0x001b,0x001b,0x001b,0x001b,0x001b,0x001b,0x001b,
    0x001c,0x001c,0x001c,0x001c,0x001c,0x001c,0x001c,0x001c,
    0x001d,0x001d,0x001d,0x001d,0x001d,0x001d,0x001d,0x001d,
    0x001e,0x001e,0x001e,0x001e,0x001e,0x001e,0x001e,0x001e,
    0x001f,0x001f,0x001f,0x001f,0x001f,0x001f,0x001f,0x001f,
    0x001f,0x001f,0x001f,0x001f,0x001f,0x001f,0x001f,0x001f, // 256
    0x001f,0x001f,0x001f,0x001f,0x001f,0x001f,0x001f,0x001f,
    0x001f,0x001f,0x001f,0x001f,0x001f,0x001f,0x001f,0x001f,
    0x001f,0x001f,0x001f,0x001f,0x001f,0x001f,0x001f,0x001f,
    0x001f,0x001f,0x001f,0x001f,0x001f,0x001f,0x001f,0x001f,
    0x001f,0x001f,0x001f,0x001f,0x001f,0x001f,0x001f,0x001f,
    0x001f,0x001f,0x001f,0x001f,0x001f,0x001f,0x001f,0x001f,
    0x001f,0x001f,0x001f,0x001f,0x001f,0x001f,0x001f,0x001f,
    0x001f,0x001f,0x001f,0x001f,0x001f,0x001f,0x001f,0x001f,
    0x001f,0x001f,0x001f,0x001f,0x001f,0x001f,0x001f,0x001f,
    0x001f,0x001f,0x001f,0x001f,0x001f,0x001f,0x001f,0x001f,
    0x001f,0x001f,0x001f,0x001f,0x001f,0x001f,0x001f,0x001f,
    0x001f,0x001f,0x001f,0x001f,0x001f,0x001f,0x001f,0x001f,
    0x001f,0x001f,0x001f,0x001f,0x001f,0x001f,0x001f,0x001f,
    0x001f,0x001f,0x001f,0x001f,0x001f,0x001f,0x001f,0x001f,
    0x001f,0x001f,0x001f,0x001f,0x001f,0x001f,0x001f,0x001f,
    0x001f,0x001f,0x001f,0x001f,0x001f,0x001f,0x001f,0x001f,
    0x001f,0x001f,0x001f,0x001f,0x001f,0x001f,0x001f,0x001f,
    0x001f,0x001f,0x001f,0x001f,0x001f,0x001f,0x001f,0x001f,
    0x001f,0x001f,0x001f,0x001f,0x001f,0x001f,0x001f,0x001f,
    0x001f,0x001f,0x001f,0x001f,0x001f,0x001f,0x001f,0x001f,
    0x001f,0x001f,0x001f,0x001f,0x001f,0x001f,0x001f,0x001f,
    0x001f,0x001f,0x001f,0x001f,0x001f,0x001f,0x001f,0x001f,
    0x001f,0x001f,0x001f,0x001f,0x001f,0x001f,0x001f,0x001f,
    0x001f,0x001f,0x001f,0x001f,0x001f,0x001f,0x001f,0x001f,
    0x001f,0x001f,0x001f,0x001f,0x001f,0x001f,0x001f,0x001f,
    0x001f,0x001f,0x001f,0x001f,0x001f,0x001f,0x001f,0x001f,
    0x001f,0x001f,0x001f,0x001f,0x001f,0x001f,0x001f,0x001f,
    0x001f,0x001f,0x001f,0x001f,0x001f,0x001f,0x001f,0x001f,
    0x001f,0x001f,0x001f,0x001f,0x001f,0x001f,0x001f,0x001f,
    0x001f,0x001f,0x001f,0x001f,0x001f,0x001f,0x001f,0x001f,
    0x001f,0x001f,0x001f,0x001f,0x001f,0x001f,0x001f,0x001f,
    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0, // 512
    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0, // 768
    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};
//
// Helper functions for memory based images
//
static int32_t readMem(JPEGFILE *pFile, uint8_t *pBuf, int32_t iLen)
{
    int32_t iBytesRead;

    iBytesRead = iLen;
    if ((pFile->iSize - pFile->iPos) < iLen)
       iBytesRead = pFile->iSize - pFile->iPos;
    if (iBytesRead <= 0)
       return 0;
    memcpy_P(pBuf, &pFile->pData[pFile->iPos], iBytesRead);
    pFile->iPos += iBytesRead;
    return iBytesRead;
} /* readMem() */

static int32_t seekMem(JPEGFILE *pFile, int32_t iPosition)
{
    if (iPosition < 0) iPosition = 0;
    else if (iPosition >= pFile->iSize) iPosition = pFile->iSize-1;
    pFile->iPos = iPosition;
    return iPosition;
} /* seekMem() */

//
// Memory initialization
//
int JPEGDEC::open(uint8_t *pData, int iDataSize, JPEG_DRAW_CALLBACK *pfnDraw)
{
    memset(&_jpeg, 0, sizeof(JPEGIMAGE));
    _jpeg.pfnRead = readMem;
    _jpeg.pfnSeek = seekMem;
    _jpeg.pfnDraw = pfnDraw;
    _jpeg.pfnOpen = NULL;
    _jpeg.pfnClose = NULL;
    _jpeg.JPEGFile.iSize = iDataSize;
    _jpeg.JPEGFile.pData = pData;
    return JPEGInit(&_jpeg);
} /* open() */

int JPEGDEC::getOrientation()
{
    return (int)_jpeg.ucOrientation;
} /* getOrientation() */

int JPEGDEC::getWidth()
{
    return _jpeg.iWidth;
} /* getWidth() */

int JPEGDEC::getHeight()
{
    return _jpeg.iHeight;
} /* getHeight() */

int JPEGDEC::hasThumb()
{
    return (int)_jpeg.ucHasThumb;
} /* hasThumb() */

int JPEGDEC::getBpp()
{
    return (int)_jpeg.ucBpp;
} /* getBpp() */

int JPEGDEC::getSubSample()
{
    return (int)_jpeg.ucSubSample;
} /* getSubSample() */

//
// File (SD/MMC) based initialization
//
int JPEGDEC::open(char *szFilename, JPEG_OPEN_CALLBACK *pfnOpen, JPEG_CLOSE_CALLBACK *pfnClose, JPEG_READ_CALLBACK *pfnRead, JPEG_SEEK_CALLBACK *pfnSeek, JPEG_DRAW_CALLBACK *pfnDraw)
{
    memset(&_jpeg, 0, sizeof(JPEGIMAGE));
    _jpeg.pfnRead = pfnRead;
    _jpeg.pfnSeek = pfnSeek;
    _jpeg.pfnDraw = pfnDraw;
    _jpeg.pfnOpen = pfnOpen;
    _jpeg.pfnClose = pfnClose;
    _jpeg.JPEGFile.fHandle = (*pfnOpen)(szFilename, &_jpeg.JPEGFile.iSize);
    if (_jpeg.JPEGFile.fHandle == NULL)
       return 0;
    return JPEGInit(&_jpeg);

} /* open() */

void JPEGDEC::close()
{
    if (_jpeg.pfnClose)
        (*_jpeg.pfnClose)(_jpeg.JPEGFile.fHandle);
} /* close() */

//
// Play a single frame
// returns:
// 1 = good result and more frames exist
// 0 = good result and no more frames exist
// -1 = error
int JPEGDEC::decode(int iOptions)
{
    return DecodeJPEG(&_jpeg, iOptions);
} /* decode() */
//
// The following functions are written in plain C and have no
// 3rd party dependencies, not even the C runtime library
//
//
// Initialize a GIF file and callback access from a file on SD or memory
// returns 1 for success, 0 for failure
// Fills in the canvas size of the GIFIMAGE structure
//
static int JPEGInit(JPEGIMAGE *pJPEG)
{
    return JPEGParseInfo(pJPEG); // gather info for image
} /* GIFInit() */
//
// Unpack the Huffman tables
//
static int JPEGGetHuffTables(uint8_t *pBuf, int iLen, JPEGIMAGE *pJPEG)
{
    int i, j, iOffset, iTableOffset;
    unsigned char ucTable;
    
    iOffset = 0;
    while (iLen > 17)  // while there are tables to copy (we may have combined more than 1 table together)
    {
        ucTable = pBuf[iOffset++]; // get table index
        if (ucTable & 0x10) // convert AC offset of 0x10 into offset of 4
            ucTable ^= 0x14;
        pJPEG->ucHuffTableUsed |= (1 << ucTable); // mark this table as being defined
        if (ucTable <= 7) // tables are 0-3, AC+DC
        {
            iTableOffset = ucTable * HUFF_TABLEN;
            j = 0; // total bits
            for (i=0; i<16; i++)
            {
                j += pBuf[iOffset];
                pJPEG->ucHuffVals[iTableOffset+i] = pBuf[iOffset++];
            }
            iLen -= 17; // subtract length of bit lengths
            if (j == 0 || j > 256 || j > iLen) // bogus bit lengths
            {
                return -1;
            }
            iTableOffset += 16;
            for (i=0; i<j; i++)
            {  // copy huffman table
                pJPEG->ucHuffVals[iTableOffset+i] = pBuf[iOffset++];
            }
            iLen -= j;
        }
    }
    return 0;
} /* JPEGGetHuffTables() */
//
// Create 11-bit lookup tables for some images where it doesn't work
// for 10-bit tables
//
int JPEGMakeHuffTables_Slow(JPEGIMAGE *pJPEG, int bThumbnail)
{
    int code, repeat, count, codestart;
    int j;
    int iLen, iTable;
    unsigned short *pTable, *pShort, *pLong;
    unsigned char *pucTable, *pucShort, *pucLong;
    uint32_t ul, *pLongTable;
    int iBitNum; // current code bit length
    int cc; // code
    unsigned char *p, *pBits, ucCode;
    int iMaxLength, iMaxMask;

    pJPEG->b11Bit = 1; // indicate we're using the bigger A/C decode tables
    // first do DC components (up to 4 tables of 12-bit codes)
    // we can save time and memory for the DC codes by knowing that there exist short codes (<= 6 bits)
    // and long codes (>6 bits, but the first 5 bits are 1's).  This allows us to create 2 tables: a 6-bit and 7 or 8-bit
    // to handle any DC codes
    iMaxLength = 12; // assume DC codes can be 12-bits
    iMaxMask = 0x7f; // lower 7 bits after truncate 5 leading 1's
    if (pJPEG->ucMode == 0xc3) // create 13-bit tables for lossless mode
    {
        iMaxLength = 13;
        iMaxMask = 0xff;
    }
    for (iTable = 0; iTable < 2; iTable++)
    {
        if (pJPEG->ucHuffTableUsed & (1<<iTable))
        {
            //         pJPEG->huffdcFast[iTable] = (int *)PILIOAlloc(0x180); // short table = 128 bytes, long table = 256 bytes
            pucShort = (unsigned char *)&pJPEG->ucHuffDCShort[iTable*DCTSIZE*2];
            //         pJPEG->huffdc[iTable] = pJPEG->huffdcFast[iTable] + 0x20; // 0x20 longs = 128 bytes
            pucLong = (unsigned char *)&pJPEG->ucHuffDCLong[iTable*HUFF11SIZE*2];
            pBits = &pJPEG->ucHuffVals[iTable * HUFF_TABLEN];
            p = pBits;
            p += 16; // point to bit data
            cc = 0; // start with a code of 0
            for (iBitNum = 1; iBitNum <= 16; iBitNum++)
            {
                iLen = *pBits++; // get number of codes for this bit length
                if (iBitNum > iMaxLength && iLen > 0) // we can't handle codes longer a certain length
                {
                    return -1;
                }
                while (iLen)
                {
                    //               if (iBitNum > 6) // do long table
                    if ((cc >> (iBitNum-5)) == 0x1f) // first 5 bits are 1 - use long table
                    {
                        count = iMaxLength - iBitNum;
                        codestart = cc << count;
                        pucTable = &pucLong[codestart & iMaxMask]; // use lower 7/8 bits of code
                    }
                    else // do short table
                    {
                        count = 6 - iBitNum;
                        if (count < 0)
                            return -1; // DEBUG - something went wrong
                        codestart = cc << count;
                        pucTable = &pucShort[codestart];
                    }
                    ucCode = *p++;  // get actual huffman code
                    if (ucCode == 16 && pJPEG->ucMode == 0xc3) // lossless mode
                    {
                        // in lossless mode, this code won't fit in 4 bits, so save it's length in the next slot
                        ucCode = 255;
                        pucLong[256] = (unsigned char)iBitNum;
                    }
                    // does precalculating the DC value save time on ARM?
#ifndef USE_ARM_ASM
                    if (ucCode != 0 && (ucCode + iBitNum) <= 6 && pJPEG->ucMode != 0xc2) // we can fit the magnitude value in the code lookup (not for progressive)
                    {
                        int k, iLoop;
                        unsigned char ucCoeff;
                        unsigned char *d = &pucTable[512];
                        unsigned char ucMag = ucCode;
                        ucCode |= ((iBitNum+ucCode) << 4); // add magnitude bits to length
                        repeat = 1<<ucMag;
                        iLoop = 1<<(count-ucMag);
                        for (j=0; j<repeat; j++)
                        { // calcuate the magnitude coeff already
                            if (j & 1<<(ucMag-1)) // positive number
                                ucCoeff = (unsigned char)j;
                            else // negative number
                                ucCoeff = (unsigned char)(j - ((1<<ucMag)-1));
                            for (k=0; k<iLoop; k++)
                            {
                                *d++ = ucCoeff;
                            } // for k
                        } // for j
                    }
#endif
                    else
                    {
                        ucCode |= (iBitNum << 4);
                    }
                    if (count) // do it as dwords to save time
                    {
                        repeat = (1<<count);
                        memset(pucTable, ucCode, repeat);
                        //                  pLongTable = (uint32_t *)pTable;
                        //                  repeat = 1 << (count-2); // store as dwords (/4)
                        //                  ul = code | (code << 16);
                        //                  for (j=0; j<repeat; j++)
                        //                     *pLongTable++ = ul;
                    }
                    else
                    {
                        pucTable[0] = ucCode;
                    }
                    cc++;
                    iLen--;
                }
                cc <<= 1;
            }
        } // if table defined
    }
    // now do AC components (up to 2 tables of 16-bit codes)
    // We split the codes into a short table (9 bits or less) and a long table (first 5 bits are 1)
    for (iTable = 0; iTable < 2; iTable++)
    {
        if (pJPEG->ucHuffTableUsed & (1<<(iTable+4)))  // if this table is defined
        {
            pBits = &pJPEG->ucHuffVals[(iTable+4) * HUFF_TABLEN];
            p = pBits;
            p += 16; // point to bit data
            pShort = &pJPEG->usHuffACShort[iTable*DCTSIZE];
            pLong = &pJPEG->usHuffACLong[iTable*HUFF11SIZE];
            cc = 0; // start with a code of 0
            // construct the decode table
            for (iBitNum = 1; iBitNum <= 16; iBitNum++)
            {
                iLen = *pBits++; // get number of codes for this bit length
                while (iLen)
                {
                    if ((cc >> (iBitNum-4)) == 0xf) // first 4 bits are 1 - use long table
                    {
                        count = 16 - iBitNum;
                        codestart = cc << count;
                        pTable = &pLong[codestart & 0xfff]; // use lower 12 bits of code
                    }
                    else
                    {
                        count = 12 - iBitNum;
                        if (count < 0) // a 13-bit? code - that doesn't fit our optimized scheme, see if we can do a bigger table version
                        {
                            return -1; // DEBUG - fatal error, we currently don't support it
                        }
                        codestart = cc << count;
                        pTable = &pShort[codestart]; // 11 bits or shorter
                    }
                    code = *p++;  // get actual huffman code
                    if (bThumbnail && code != 0) // add "extra" bits to code length since we skip these codes
                    {
                        // get rid of extra bits in code and add increment (1) for AC index
                        code = ((iBitNum+(code & 0xf)) << 8) | ((code >> 4)+1);
                    }
                    else
                    {
                        code |= (iBitNum << 8);
                    }
                    if (count) // do it as dwords to save time
                    {
                        repeat = 1 << (count-1); // store as dwords (/2)
                        ul = code | (code << 16);
                        pLongTable = (uint32_t *)pTable;
                        for (j=0; j<repeat; j++)
                            *pLongTable++ = ul;
                    }
                    else
                    {
                        pTable[0] = (unsigned short)code;
                    }
                    cc++;
                    iLen--;
                }
                cc <<= 1;
            } // for each bit length
        } // if table defined
    }
    return 0;
} /* JPEGMakeHuffTables_Slow() */

//
// Expand the Huffman tables for fast decoding
//
int JPEGMakeHuffTables(JPEGIMAGE *pJPEG, int bThumbnail)
{
    int code, repeat, count, codestart;
    int j;
    int iLen, iTable;
    unsigned short *pTable, *pShort, *pLong;
    unsigned char *pucTable, *pucShort, *pucLong;
    uint32_t ul, *pLongTable;
    int iBitNum; // current code bit length
    int cc; // code
    unsigned char *p, *pBits, ucCode;
    int iMaxLength, iMaxMask;
    int iTablesUsed;
    
    iTablesUsed = 0;
    for (j=0; j<4; j++)
    {
        if (pJPEG->ucHuffTableUsed & (1 << j))
            iTablesUsed++;
    }
    // first do DC components (up to 4 tables of 12-bit codes)
    // we can save time and memory for the DC codes by knowing that there exist short codes (<= 6 bits)
    // and long codes (>6 bits, but the first 5 bits are 1's).  This allows us to create 2 tables: a 6-bit and 7 or 8-bit
    // to handle any DC codes
    iMaxLength = 12; // assume DC codes can be 12-bits
    iMaxMask = 0x7f; // lower 7 bits after truncate 5 leading 1's
    for (iTable = 0; iTable < 4; iTable++)
    {
        if (pJPEG->ucHuffTableUsed && (1 << iTable))
        {
            //         pJPEG->huffdcFast[iTable] = (int *)PILIOAlloc(0x180); // short table = 128 bytes, long table = 256 bytes
            pucShort = (unsigned char *)&pJPEG->ucHuffDCShort[iTable*DCTSIZE*2];
            //         pJPEG->huffdc[iTable] = pJPEG->huffdcFast[iTable] + 0x20; // 0x20 longs = 128 bytes
            pucLong = (unsigned char *)&pJPEG->ucHuffDCLong[iTable*HUFF11SIZE*2];
            pBits = &pJPEG->ucHuffVals[iTable * HUFF_TABLEN];
            p = pBits;
            p += 16; // point to bit data
            cc = 0; // start with a code of 0
            for (iBitNum = 1; iBitNum <= 16; iBitNum++)
            {
                iLen = *pBits++; // get number of codes for this bit length
                if (iBitNum > iMaxLength && iLen > 0) // we can't handle codes longer a certain length
                {
                    return -1;
                }
                while (iLen)
                {
                    //               if (iBitNum > 6) // do long table
                    if ((cc >> (iBitNum-5)) == 0x1f) // first 5 bits are 1 - use long table
                    {
                        count = iMaxLength - iBitNum;
                        codestart = cc << count;
                        pucTable = &pucLong[codestart & iMaxMask]; // use lower 7/8 bits of code
                    }
                    else // do short table
                    {
                        count = 6 - iBitNum;
                        if (count < 0)
                            return -1; // DEBUG - something went wrong
                        codestart = cc << count;
                        pucTable = &pucShort[codestart];
                    }
                    ucCode = *p++;  // get actual huffman code
                    // does precalculating the DC value save time on ARM?
#ifndef USE_ARM_ASM
                    if (ucCode != 0 && (ucCode + iBitNum) <= 6 && pJPEG->ucMode != 0xc2) // we can fit the magnitude value in the code lookup (not for progressive)
                    {
                        int k, iLoop;
                        unsigned char ucCoeff;
                        unsigned char *d = &pucTable[512];
                        unsigned char ucMag = ucCode;
                        ucCode |= ((iBitNum+ucCode) << 4); // add magnitude bits to length
                        repeat = 1<<ucMag;
                        iLoop = 1<<(count-ucMag);
                        for (j=0; j<repeat; j++)
                        { // calcuate the magnitude coeff already
                            if (j & 1<<(ucMag-1)) // positive number
                                ucCoeff = (unsigned char)j;
                            else // negative number
                                ucCoeff = (unsigned char)(j - ((1<<ucMag)-1));
                            for (k=0; k<iLoop; k++)
                            {
                                *d++ = ucCoeff;
                            } // for k
                        } // for j
                    }
#endif
                    else
                    {
                        ucCode |= (iBitNum << 4);
                    }
                    if (count) // do it as dwords to save time
                    {
                        repeat = (1<<count);
                        memset(pucTable, ucCode, repeat);
                        //                  pLongTable = (uint32_t *)pTable;
                        //                  repeat = 1 << (count-2); // store as dwords (/4)
                        //                  ul = code | (code << 16);
                        //                  for (j=0; j<repeat; j++)
                        //                     *pLongTable++ = ul;
                    }
                    else
                    {
                        pucTable[0] = ucCode;
                    }
                    cc++;
                    iLen--;
                }
                cc <<= 1;
            }
        } // if table defined
    }
    // now do AC components (up to 4 tables of 16-bit codes)
    // We split the codes into a short table (9 bits or less) and a long table (first 5 bits are 1)
    for (iTable = 0; iTable < 4; iTable++)
    {
        if (pJPEG->ucHuffTableUsed & (1 << (iTable+4)))  // if this table is defined
        {
            pBits = &pJPEG->ucHuffVals[(iTable+4) * HUFF_TABLEN];
            p = pBits;
            p += 16; // point to bit data
            pShort = &pJPEG->usHuffACShort[iTable*DCTSIZE];
            pLong = &pJPEG->usHuffACLong[iTable*HUFF11SIZE];
            cc = 0; // start with a code of 0
            // construct the decode table
            for (iBitNum = 1; iBitNum <= 16; iBitNum++)
            {
                iLen = *pBits++; // get number of codes for this bit length
                while (iLen)
                {
                    if ((cc >> (iBitNum-6)) == 0x3f) // first 6 bits are 1 - use long table
                    {
                        count = 16 - iBitNum;
                        codestart = cc << count;
                        pTable = &pLong[codestart & 0x3ff]; // use lower 10 bits of code
                    }
                    else
                    {
                        count = 10 - iBitNum;
                        if (count < 0) // an 11/12-bit? code - that doesn't fit our optimized scheme, see if we can do a bigger table version
                        {
                            if (count == -1 && iTablesUsed <= 4) // we need to create "slow" tables
                            {
                                j = JPEGMakeHuffTables_Slow(pJPEG, bThumbnail);
                                return j;
                            }
                            else
                                return -1; // DEBUG - fatal error, more than 2 big tables we currently don't support
                        }
                        codestart = cc << count;
                        pTable = &pShort[codestart]; // 10 bits or shorter
                    }
                    code = *p++;  // get actual huffman code
                    if (bThumbnail && code != 0) // add "extra" bits to code length since we skip these codes
                    {
                        // get rid of extra bits in code and add increment (1) for AC index
                        code = ((iBitNum+(code & 0xf)) << 8) | ((code >> 4)+1);
                    }
#ifdef BOGUS // precalculating the AC coeff makes it run slightly slower
                    else if ((code & 0xf) != 0 && (code + iBitNum) <= 10) // we can fit the magnitude value + huffman code in a single read
                    {
                        int k, iLoop;
                        unsigned short usCoeff;
                        unsigned short *d = &pTable[4096]; // use unused table slots 2+3 for extra coeff data
                        unsigned char ucMag = (unsigned char)(code & 0xf);
                        code |= ((iBitNum + (code & 0xf)) << 8); // add magnitude bits to length
                        repeat = 1<<ucMag;
                        iLoop = 1<<(count-ucMag);
                        for (j=0; j<repeat; j++)
                        { // calcuate the magnitude coeff already
                            if (j & 1<<(ucMag-1)) // positive number
                                usCoeff = (unsigned short)j;
                            else // negative number
                                usCoeff = (unsigned short)(j - ((1<<ucMag)-1));
                            for (k=0; k<iLoop; k++)
                            {
                                *d++ = usCoeff;
                            } // for k
                        } // for j
                    }
#endif
                    else
                    {
                        code |= (iBitNum << 8);
                    }
                    if (count) // do it as dwords to save time
                    {
                        repeat = 1 << (count-1); // store as dwords (/2)
                        ul = code | (code << 16);
                        pLongTable = (uint32_t *)pTable;
                        for (j=0; j<repeat; j++)
                            *pLongTable++ = ul;
                    }
                    else
                    {
                        pTable[0] = (unsigned short)code;
                    }
                    cc++;
                    iLen--;
                }
                cc <<= 1;
            } // for each bit length
        } // if table defined
    }
    return 0;
} /* JPEGMakeHuffTables() */

//
// TIFFSHORT
// read a 16-bit unsigned integer from the given pointer
// and interpret the data as big endian (Motorola) or little endian (Intel)
//
uint16_t TIFFSHORT(unsigned char *p, int bMotorola)
{
    unsigned short s;

    if (bMotorola)
        s = *p * 0x100 + *(p+1); // big endian (AKA Motorola byte order)
    else
        s = *p + *(p+1)*0x100; // little endian (AKA Intel byte order)
    return s;
} /* TIFFSHORT() */
//
// TIFFLONG
// read a 32-bit unsigned integer from the given pointer
// and interpret the data as big endian (Motorola) or little endian (Intel)
//
uint32_t TIFFLONG(unsigned char *p, int bMotorola)
{
    uint32_t l;

    if (bMotorola)
        l = *p * 0x1000000 + *(p+1) * 0x10000 + *(p+2) * 0x100 + *(p+3); // big endian
    else
        l = *p + *(p+1) * 0x100 + *(p+2) * 0x10000 + *(p+3) * 0x1000000; // little endian
    return l;
} /* TIFFLONG() */
//
// TIFFVALUE
// read an integer value encoded in a TIFF TAG (12-byte structure)
// and interpret the data as big endian (Motorola) or little endian (Intel)
//
int TIFFVALUE(unsigned char *p, int bMotorola)
{
    int i, iType;
    
    iType = TIFFSHORT(p+2, bMotorola);
    /* If pointer to a list of items, must be a long */
    if (TIFFSHORT(p+4, bMotorola) > 1)
    {
        iType = 4;
    }
    switch (iType)
    {
        case 3: /* Short */
            i = TIFFSHORT(p+8, bMotorola);
            break;
        case 4: /* Long */
        case 7: // undefined (treat it as a long since it's usually a multibyte buffer)
            i = TIFFLONG(p+8, bMotorola);
            break;
        case 6: // signed byte
            i = (signed char)p[8];
            break;
        case 2: /* ASCII */
        case 5: /* Unsigned Rational */
        case 10: /* Signed Rational */
            i = TIFFLONG(p+8, bMotorola);
            break;
        default: /* to suppress compiler warning */
            i = 0;
            break;
    }
    return i;
    
} /* TIFFVALUE() */
uint8_t GetEXIFOrientation(JPEGIMAGE *pPage, int bMotorola, int iOffset)
{
    int iTag, iTagCount, i;
    uint8_t c = 0, *cBuf = pPage->ucFileBuf;
    
    iTagCount = TIFFSHORT(&cBuf[iOffset], bMotorola);  /* Number of tags in this dir */
    if (iTagCount < 1 || iTagCount > 256) // invalid tag count
        return -1; /* Bad header info */
    /*--- Search the TIFF tags ---*/
    for (i=0; i<iTagCount; i++)
    {
        unsigned char *p = &cBuf[iOffset + (i*12) +2];
        iTag = TIFFSHORT(p, bMotorola);  /* current tag value */
        if (iTag == 274) // we're only looking for the orientation tag
        {
            c = TIFFVALUE(p, bMotorola);
            i = iTagCount; // exit loop - we're done
        }
    }
    return c;
}
int JPEGGetSOS(JPEGIMAGE *pJPEG, int *iOff)
{
    int16_t sLen;
    int iOffset = *iOff;
    int i, j;
    uint8_t uc,c,cc;
    uint8_t *buf = pJPEG->ucFileBuf;
    
    sLen = MOTOSHORT(&buf[iOffset]);
    iOffset += 2;
    
    // Assume no components in this scan
    for (i=0; i<4; i++)
        pJPEG->JPCI[i].component_needed = 0;
    
    uc = buf[iOffset++]; // get number of components
    pJPEG->ucComponentsInScan = uc;
    sLen -= 3;
    if (uc < 1 || uc > MAX_COMPS_IN_SCAN || sLen != (uc*2+3)) // check length of data packet
        return 1; // error
    for (i=0; i<uc; i++)
    {
        cc = buf[iOffset++];
        c = buf[iOffset++];
        sLen -= 2;
        for (j=0; j<4; j++) // search for component id
        {
            if (pJPEG->JPCI[j].component_id == cc)
                break;
        }
        if (j == 4) // error, not found
            return 1;
        if ((c & 0xf) > 3 || (c & 0xf0) > 0x30)
            return 1; // bogus table numbers
        pJPEG->JPCI[j].dc_tbl_no = c >> 4;
        pJPEG->JPCI[j].ac_tbl_no = c & 0xf;
        pJPEG->JPCI[j].component_needed = 1; // mark this component as being included in the scan
    }
    pJPEG->iScanStart = buf[iOffset++]; // Get the scan start (or lossless predictor) for this scan
    pJPEG->iScanEnd = buf[iOffset++]; // Get the scan end for this scan
    c = buf[iOffset++]; // successive approximation bits
    pJPEG->cApproxBitsLow = c & 0xf; // also point transform in lossless mode
    pJPEG->cApproxBitsHigh = c >> 4;
    
    *iOff = iOffset;
    return 0;
    
} /* JPEGGetSOS() */
//
// Remove markers from the data stream to allow faster decode
// Stuffed zeros and restart interval markers aren't needed to properly decode
// the data, but they make reading VLC data slower, so I pull them out first
//
static int JPEGFilter(uint8_t *pBuf, uint8_t *d, int iLen, uint8_t *bFF)
{
    // since we have the entire jpeg buffer in memory already, we can just change it in place
    unsigned char c, *s, *pEnd, *pStart;
    
    pStart = d;
    s = pBuf;
    pEnd = &s[iLen-1]; // stop just shy of the end to not miss a final marker/stuffed 0
    if (*bFF) // last byte was a FF, check the next one
    {
        if (s[0] == 0) // stuffed 0, keep the FF
            *d++ = 0xff;
        s++;
        *bFF = 0;
    }
    while (s < pEnd)
    {
        c = *d++ = *s++;
        if (c == 0xff) // marker or stuffed zeros?
        {
            if (s[0] != 0) // it's a marker, skip both
            {
                d--;
            }
            s++; // for stuffed 0's, store the FF, skip the 00
        }
    }
    if (s == pEnd) // need to test the last byte
    {
        c = s[0];
        if (c == 0xff) // last byte is FF, take care of it next time through
            *bFF = 1; // take care of it next time through
        else
            *d++ = c; // nope, just store it
    }
    return (int)(d-pStart); // filtered output length
} /* JPEGFilter() */
//
// Read and filter more VLC data for decoding
//
static void JPEGGetMoreData(JPEGIMAGE *pPage)
{
    // move any existing data down
    if ((pPage->iVLCSize - pPage->iVLCOff) >= FILE_HIGHWATER)
        return; // buffer is already full; no need to read more data
    if (pPage->iVLCOff != 0)
    {
      memcpy(pPage->ucFileBuf, &pPage->ucFileBuf[pPage->iVLCOff], pPage->iVLCSize - pPage->iVLCOff);
      pPage->iVLCSize -= pPage->iVLCOff;
      pPage->iVLCOff = 0;
    }
    if (pPage->JPEGFile.iPos < pPage->JPEGFile.iSize && pPage->iVLCSize < FILE_HIGHWATER)
    {
        int i;
        // Try to read enough to fill the buffer
        i = (*pPage->pfnRead)(&pPage->JPEGFile, &pPage->ucFileBuf[pPage->iVLCSize], FILE_BUF_SIZE - pPage->iVLCSize); // max length we can read
        // Filter out the markers
        pPage->iVLCSize += JPEGFilter(&pPage->ucFileBuf[pPage->iVLCSize], &pPage->ucFileBuf[pPage->iVLCSize], i, &pPage->ucFF);
    }
} /* JPEGGetMoreData() */

//
// Parse the JPEG header, gather necessary info to decode the image
// Returns 1 for success, 0 for failure
//
static int JPEGParseInfo(JPEGIMAGE *pPage)
{
    int iBytesRead;
    int i, iOffset, iTableOffset;
    uint8_t ucTable, *s = pPage->ucFileBuf;
    uint16_t usMarker, usLen;
    int iFilePos = 0;
    
    iBytesRead = (*pPage->pfnRead)(&pPage->JPEGFile, s, FILE_BUF_SIZE);
    if (iBytesRead < 256) // a JPEG file this tiny? probably bad
        return 0;
    iFilePos += iBytesRead;
    if (MOTOSHORT(pPage->ucFileBuf) != 0xffd8)
        return 0; // not a JPEG file
    iOffset = 2; /* Start at offset of first marker */
    usMarker = 0; /* Search for SOFx (start of frame) marker */
    while (usMarker != 0xffda && iOffset < pPage->JPEGFile.iSize)
    {
        if (iOffset >= FILE_BUF_SIZE/2) // too close to the end, read more data
        {
            // Do we need to seek first?
            if (iOffset >= FILE_BUF_SIZE)
            {
                iFilePos += iOffset;
                iOffset = 0;
                (*pPage->pfnSeek)(&pPage->JPEGFile, iFilePos);
                iBytesRead = 0; // throw away any old data
            }
            // move existing bytes down
            if (iOffset)
            {
                memcpy(pPage->ucFileBuf, &pPage->ucFileBuf[iOffset], iBytesRead - iOffset);
                iBytesRead -= iOffset;
                iOffset = 0;
            }
            i = (*pPage->pfnRead)(&pPage->JPEGFile, &pPage->ucFileBuf[iBytesRead], FILE_BUF_SIZE-iBytesRead);
            iFilePos += i;
            iBytesRead += i;
        }
        usMarker = MOTOSHORT(&s[iOffset]);
        iOffset += 2;
        usLen = MOTOSHORT(&s[iOffset]); // marker length

        if (usMarker < 0xffc0 || usMarker == 0xffff) // invalid marker, could be generated by "Arles Image Web Page Creator" or Accusoft
        {
            iOffset++;
            continue; // skip 1 byte and try to resync
        }
        switch (usMarker)
        {
            case 0xffc1:
            case 0xffc2:
            case 0xffc3:
                return 0; // currently unsupported modes
                
            case 0xffe1: // App1 (EXIF?)
                if (s[iOffset+2] == 'E' && s[iOffset+3] == 'x' && (s[iOffset+8] == 'M' || s[iOffset+8] == 'I')) // the EXIF data we want
                {
                    pPage->iEXIF = iFilePos + iOffset + 8; // start of TIFF file
                    // Get the orientation value (if present)
                    int bMotorola = (s[iOffset+8] == 'M');
                    int IFD = TIFFLONG(&s[iOffset+12], bMotorola);
                    pPage->ucOrientation = GetEXIFOrientation(pPage, bMotorola, IFD+iOffset+8);
                }
                break;
            case 0xffc0: // SOFx - start of frame
                pPage->ucMode = (uint8_t)usMarker;
                pPage->ucBpp = s[iOffset+2]; // bits per sample
                pPage->iHeight = MOTOSHORT(&s[iOffset+3]);
                pPage->iWidth = MOTOSHORT(&s[iOffset+5]);
                pPage->ucNumComponents = s[iOffset+7];
                pPage->ucBpp = pPage->ucBpp * pPage->ucNumComponents; /* Bpp = number of components * bits per sample */
                pPage->ucSubSample = s[iOffset+9]; // subsampling option for the second color component
                pPage->ucSubSample = (pPage->ucSubSample & 0xf) | (pPage->ucSubSample >> 2);
                break;
            case 0xffdd: // Restart Interval
                if (usLen == 4)
                    pPage->iResInterval = MOTOSHORT(&s[iOffset+2]);
                break;
            case 0xffc4: /* M_DHT */ // get Huffman tables
                iOffset += 2; // skip length
                usLen -= 2; // subtract length length
                if (JPEGGetHuffTables(&s[iOffset], usLen, pPage) != 0) // bad tables?
                    return 0; // error
                break;
            case 0xffdb: /* M_DQT */
                /* Get the quantization tables */
                /* first byte has PPPPNNNN where P = precision and N = table number 0-3 */
                iOffset += 2; // skip length
                usLen -= 2; // subtract length length
                while (usLen > 0)
                {
                    ucTable = s[iOffset++]; // table number
                    if ((ucTable & 0xf) > 3) // invalid table number
                        return 0;
                    iTableOffset = (ucTable & 0xf) * DCTSIZE;
                    if (ucTable & 0xf0) // if word precision
                    {
                        for (i=0; i<DCTSIZE; i++)
                        {
                            pPage->sQuantTable[i+iTableOffset] = MOTOSHORT(&s[iOffset]);
                            iOffset += 2;
                        }
                        usLen -= (DCTSIZE*2 + 1);
                    }
                    else // byte precision
                    {
                        for (i=0; i<DCTSIZE; i++)
                        {
                            pPage->sQuantTable[i+iTableOffset] = (unsigned short)s[iOffset++];
                        }
                        usLen -= (DCTSIZE + 1);
                    }
                }
                break;
        } // switch on JPEG marker
        iOffset += usLen;
    } // while
    if (usMarker == 0xffda) // start of image
    {
        JPEGGetSOS(pPage, &iOffset); // get Start-Of-Scan info for decoding
        JPEGMakeHuffTables(pPage, 0); //int bThumbnail) DEBUG
        // Now the offset points to the start of compressed data
        i = JPEGFilter(&pPage->ucFileBuf[iOffset], pPage->ucFileBuf, iBytesRead-iOffset, &pPage->ucFF);
        pPage->iVLCOff = 0;
        pPage->iVLCSize = i;
        JPEGGetMoreData(pPage); // read more VLC data
        return 1;
    }
    return 0;
} /* JPEGParseInfo() */
//
// Fix and reorder the quantization table for faster decoding.*
//
static void JPEGFixQuantD(JPEGIMAGE *pJPEG)
{
    int iTable, iTableOffset;
    signed short sTemp[DCTSIZE];
    int i;
    uint16_t *p;
    
    for (iTable=0; iTable<pJPEG->ucNumComponents; iTable++)
    {
        iTableOffset = iTable * DCTSIZE;
        p = (uint16_t *)&pJPEG->sQuantTable[iTableOffset];
        for (i=0; i<DCTSIZE; i++)
            sTemp[i] = p[cZigZag[i]];
        memcpy(&pJPEG->sQuantTable[iTableOffset], sTemp, DCTSIZE*sizeof(short)); // copy back to original spot
        
        // Prescale for DCT multiplication
        p = (uint16_t *)&pJPEG->sQuantTable[iTableOffset];
        for (i=0; i<DCTSIZE; i++)
        {
            p[i] = (uint16_t)((p[i] * iScaleBits[i]) >> 12);
        }
    }
} /* JPEGFixQuantD() */

//
// Decode the image
//
static int DecodeJPEG(JPEGIMAGE *pJPEG, int iOptions)
{
    // reorder and fix the quantization table for decoding
    JPEGFixQuantD(pJPEG);

    return 0;
} /* DecodeJPEG() */
