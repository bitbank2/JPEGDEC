//
// ESP32-S3 SIMD optimized code
// Written by Larry Bank
// Copyright (c) 2024 BitBank Software, Inc.
// Project started Jan 21, 2024
//
#include "dsps_fft2r_platform.h"
#if (dsps_fft2r_sc16_aes3_enabled == 1)
	.text
	.align 4

// Convert 8 pixels of YCbCr 4:4:4 to RGB565
//                                       A2            A3            A4           A5               A6
// Call as void s3_ycbcr_convert_444(uint8_t *pY, uint8_t *pCB, uint8_t *pCR, uint16_t *pOut, int16_t *pConsts);
	.global s3_ycbcr_convert_444
    .type   s3_ycbcr_convert_444,@function

s3_ycbcr_convert_444:
	# no idea what this frequency keyword does
#	.frequency 1.000 0.000
	entry	a1,16
  ee.xorq q4,q4,q4          # load Q4 with 0's
  ee.vldbc.16.ip  q3,a6,2   # get constant 0x80 as 16-bits in all 128 bits of q3
  ee.vld.l.64.ip	q0,a2,0   # load 8 Y values into Q0
  ee.vld.l.64.ip	q1,a3,0   # load 8 Cb values into Q1
  ee.vld.l.64.ip	q2,a4,0   # load 8 Cr values into Q2
  ee.vzip.8 q0,q4           # expand 8-bit Y data to 16-bits
  ee.xorq q4,q4,q4        # need to reset to 0's
  ee.vzip.8 q1,q4           # expand 8-bit Cb data to 16-bits
  ee.xorq q4,q4,q4
  ee.vzip.8 q2,q4           # expand 8-bit Cr data to 16-bits

  ee.vsubs.s16 q1,q1,q3     # subtract 0x0080 from Cb's
  ee.vsubs.s16 q2,q2,q3     # subtract 0x0080 from Cr's
  ee.vldbc.16.ip  q3,a6,2   # get constant 1.77200 as 16-bits in all 128 bits of q3
 	movi.n	a7,6           # load the shift register with 6 
	wsr.sar	a7             # put it in the SAR (shift amount register)  
  ee.vmul.s16 q6,q1,q3   # (Cb *= 1.77200) >> 6
  ee.vadds.s16 q6,q6,q0  # Cb += y (8 blue pixels in q6)
  ee.vldbc.16.ip  q3,a6,2   # get constant 1.402 as 16-bits in all 128 bits of q3
  ee.vmul.s16 q7,q2,q3   # (Cr *= 1.402) >> 6
  ee.vadds.s16 q7,q7,q0  # Cr += y (8 red pixels in q7)
  ee.vldbc.16.ip  q3,a6,2   # get constant 0.34414 as 16-bits in all 128 bits of q3
 	movi.n	a7,0           # load the shift register with 0 
	wsr.sar	a7             # put it in the SAR (shift amount register)  
  ee.vmul.s16 q4,q1,q3   # (Cb * 0.34414) >> 0
  ee.vldbc.16.ip  q3,a6,2   # get constant 0.71414 as 16-bits in all 128 bits of q3
  ee.vmul.s16 q3,q2,q3   # (Cr * 0.71414) >> 0
  ee.vadds.s16 q3,q3,q4  # (Cb * 0.34414) + (Cr * 0.71414)
  ee.vldbc.16.ip  q4,a6,2   # get constant 1 (so we can do a 16-bit shift)
 	movi.n	a7,6           # load the shift register with 6 
	wsr.sar	a7             # put it in the SAR (shift amount register)
  ee.vmul.s16  q3,q3,q4  # shift right by 6
  ee.vsubs.s16 q3,q0,q3   # Y - ((Cb * 0.34414) + (Cr * 0.71414)) = green in Q3
// saturate to 8 bits
  ee.xorq q0,q0,q0
  ee.vmax.s16 q3,q3,q0
  ee.vmax.s16 q6,q6,q0
  ee.vmax.s16 q7,q7,q0
  ee.vcmp.eq.s16 q1,q1,q1	# create 255
  ee.vzip.8 q1,q0
  ee.vmin.s16 q3,q3,q1         # clamp to 255
  ee.vmin.s16 q6,q6,q1
  ee.vmin.s16 q7,q7,q1

 	movi.n	a7,3           # load the shift register with 3 (for blue and red) 
	wsr.sar	a7             # put it in the SAR (shift amount register)
  ee.vmul.s16 q6,q4,q6   # shift blue right by 3
  ee.vmul.s16 q7,q4,q7   # shift red right by 3
 	movi.n	a7,2           # load the shift register with 2 (for green) 
	wsr.sar	a7             # put it in the SAR (shift amount register)
  ee.vmul.s16 q3,q4,q3   # shift green right by 2
// now combine to form RGB565 pixels
 	movi.n	a7,0 
	wsr.sar	a7              # no shift after multiply
  ee.vldbc.16.ip  q4,a6,2   # get constant value 32 (to shift green left by 5 bits)
  ee.vldbc.16.ip  q5,a6,2   # get constant value 2048 (to shift red left by 11 bits)
  ee.vmul.s16 q3,q4,q3   # shift green left by 5
  ee.vmul.s16 q7,q5,q7   # shift red left by 11
  ee.orq q6,q6,q3        # combine blue + green
  ee.orq q6,q6,q7        # combine blue + green + red
  ee.vunzip.8 q6,q5      # swap the byte order to be big-endian
  ee.vzip.8 q5,q6
  ee.vst.128.ip q5,a5,0  # store the 8 RGB565 pixels

  # return value of 0
	movi.n	a2,0
	retw.n

#endif // dsps_fft2r_sc16_aes3_enabled
