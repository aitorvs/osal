/**
 *  \file   osecc.c
 *  \brief  This file implements the Error Checking and Correction functions.
 *
 *  The file provides to the user the capabilities to calculate and check ECC
 *  codes.
 *
 *  \author  Aitor Viana Sanchez (avs), Aitor.Viana.Sanchez@esa.int
 *
 *  \internal
 *    Created:  02/04/09
 *   Revision:  $Id: osecc.c 1.4 02/04/09 avs Exp $
 *   Compiler:  gcc/g++
 *    Company:  European Space Agency (ESA-ESTEC)
 *  Copyright:  Copyright (c) 2009, Aitor Viana Sanchez
 *
 *  This source code is released for free distribution under the terms of the
 *  GNU General Public License as published by the Free Software Foundation.
 * =====================================================================================
 */

#include <osal/osapi.h>
#include <string.h>

#define MAX_ECC_LEN		8		// Maximum ECC Size is 64 bit (2*32 bits for 512MByte block!)

// The fastest way to calculate parity in a byte is via lookup table, and it's only 256 bytes
// in size. Each ECC calculation performs a lot of parity calculations, so this is important.

#define parity_byte( x) 	parity_byte_lookuptable[x]
#define parity_word( x)		parity_byte( ((x)^((x)>>8)) & 0xFFU)
//#define parity_dword( x)	parity_word( ((x)^((x)>>16)) & 0xFFFFU)
#define parity_dword( x)	parity_byte( (((x)^((x)>>8)^((x)>>16)^((x)>>24)) & 0xFFU))

/** Parity Look-up table (256 ytes) */
LOCAL const uint8_t parity_byte_lookuptable[ 256] = {
	0x00,	0x01,	0x01,	0x00,	0x01,	0x00,	0x00,	0x01,
	0x01,	0x00,	0x00,	0x01,	0x00,	0x01,	0x01,	0x00,
	0x01,	0x00,	0x00,	0x01,	0x00,	0x01,	0x01,	0x00,
	0x00,	0x01,	0x01,	0x00,	0x01,	0x00,	0x00,	0x01,
	0x01,	0x00,	0x00,	0x01,	0x00,	0x01,	0x01,	0x00,
	0x00,	0x01,	0x01,	0x00,	0x01,	0x00,	0x00,	0x01,
	0x00,	0x01,	0x01,	0x00,	0x01,	0x00,	0x00,	0x01,
	0x01,	0x00,	0x00,	0x01,	0x00,	0x01,	0x01,	0x00,
	0x01,	0x00,	0x00,	0x01,	0x00,	0x01,	0x01,	0x00,
	0x00,	0x01,	0x01,	0x00,	0x01,	0x00,	0x00,	0x01,
	0x00,	0x01,	0x01,	0x00,	0x01,	0x00,	0x00,	0x01,
	0x01,	0x00,	0x00,	0x01,	0x00,	0x01,	0x01,	0x00,
	0x00,	0x01,	0x01,	0x00,	0x01,	0x00,	0x00,	0x01,
	0x01,	0x00,	0x00,	0x01,	0x00,	0x01,	0x01,	0x00,
	0x01,	0x00,	0x00,	0x01,	0x00,	0x01,	0x01,	0x00,
	0x00,	0x01,	0x01,	0x00,	0x01,	0x00,	0x00,	0x01,
	0x01,	0x00,	0x00,	0x01,	0x00,	0x01,	0x01,	0x00,
	0x00,	0x01,	0x01,	0x00,	0x01,	0x00,	0x00,	0x01,
	0x00,	0x01,	0x01,	0x00,	0x01,	0x00,	0x00,	0x01,
	0x01,	0x00,	0x00,	0x01,	0x00,	0x01,	0x01,	0x00,
	0x00,	0x01,	0x01,	0x00,	0x01,	0x00,	0x00,	0x01,
	0x01,	0x00,	0x00,	0x01,	0x00,	0x01,	0x01,	0x00,
	0x01,	0x00,	0x00,	0x01,	0x00,	0x01,	0x01,	0x00,
	0x00,	0x01,	0x01,	0x00,	0x01,	0x00,	0x00,	0x01,
	0x00,	0x01,	0x01,	0x00,	0x01,	0x00,	0x00,	0x01,
	0x01,	0x00,	0x00,	0x01,	0x00,	0x01,	0x01,	0x00,
	0x01,	0x00,	0x00,	0x01,	0x00,	0x01,	0x01,	0x00,
	0x00,	0x01,	0x01,	0x00,	0x01,	0x00,	0x00,	0x01,
	0x01,	0x00,	0x00,	0x01,	0x00,	0x01,	0x01,	0x00,
	0x00,	0x01,	0x01,	0x00,	0x01,	0x00,	0x00,	0x01,
	0x00,	0x01,	0x01,	0x00,	0x01,	0x00,	0x00,	0x01,
	0x01,	0x00,	0x00,	0x01,	0x00,	0x01,	0x01,	0x00
};

// Another operation frequently performed in ECC checking is to count how many bits are
// set in a byte. The lookup table approach seems reasonable here too (although a lot less
// critical than in parity calculation.

#define countbits_byte( x) bits_set_lookuptable[x]

/** Look-up table to calculate how many bits are set in a byte */
LOCAL const uint8_t bits_set_lookuptable[ 256] = 
{
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
  4, 5, 5, 6, 5, 6, 6, 7, 5, 6, 6, 7, 6, 7, 7, 8
};

LOCAL int _ecc_gen8( uint8_t* vec, uint32_t order, uint8_t * ecc);
LOCAL int _ecc_gen32( uint8_t* vec, uint32_t order, uint8_t * ecc);

/*
 *  \ingroup OSAL_utils
 *  \brief ECC generation function.
 *
 *  The function generates the ECC for the given data block. The maximum block
 *  size is 4Gbit (512MBytes). This size should be enough for all cases.
 *  The data block size must be always power of two (e.g. 128, 256, 512, etc).
 *
 *  \param  vec Data block for ECC calculation
 *  \param  order   Log2 of the data block size (e.g. 8 for 256byte block)
 *  \param  ecc     Byte array where the calculated ECC is returned to the
 *  user
 *
 *  The Calculated ECC consist of two parts of order+3 bits concatenated. First
 *  part is for even parity and second is for odd parity. Bytes are store
 *  low-endian and packed.
 *
 *  As an example, for 512 bytes data block (order=9), two 12-bit values are
 *  generated. Once packed they use 3 bytes of space for the generated ECC (
 *  (9+3) + (9+3) ).
 *
 *  This function is an 8-bit version of ECC calculation using 32 or 64 bit
 *  math
 *
 *  \return 0 always.
 */
LOCAL int _ecc_gen8( uint8_t* vec, uint32_t order, uint8_t * ecc)
{
	
	uint32_t i;
	uint32_t LPodd, LPeven, CPodd, CPeven;
	uint8_t colparity;
	uint32_t blocksize, eccsize;
	int32_t bits;
	uint32_t bitoff, byteoff;
	
	// First, calculate the number of bytes in the data block
	
	blocksize = 0x1 << order;
	
	// Parity calculation
	// This is a faster variant of the way some manufacturers store ECC in flash memories.
	// Row and column parity is calculated separately.
	
	// Row parity and preliminary column parity calculation:
	
	LPodd = LPeven = 0;
	colparity = 0;
	
	for( i = 0; i < blocksize ; i++){
		colparity ^= vec[i];
		if( parity_byte( vec[i])){
			LPodd ^= i;
			LPeven ^= ~i;	
		}
	}
	
	// Column parity
	
	CPodd = CPeven = 0;
	
	CPodd = parity_byte( colparity & 0xF0); CPodd <<= 1;
	CPodd |= parity_byte( colparity & 0xCC); CPodd <<= 1;
	CPodd |= parity_byte( colparity & 0xAA);
	CPeven = parity_byte( colparity & 0x0F); CPeven <<= 1;
	CPeven |= parity_byte( colparity & 0x33); CPeven <<= 1;
	CPeven |= parity_byte( colparity & 0x55);

	// Whole bit-offset parity storage
	
	LPodd <<= 3;	
	LPodd |= CPodd;
	LPeven <<= 3;	
	LPeven |= CPeven;
	
	// ECC packing and writing
	
	// First we zero the ECC buffer.
	
	eccsize = ((2*order)+5)/8 + 1;
	memset( ecc, 0, eccsize);
	
	// Odd and even parity values are now packed and stored. Odd first, little endian.
	
	bitoff = 0;
	byteoff = 0;
	bits = order + 3;
	while( bits>0){
		ecc[byteoff++] = LPodd & 0x000000FF;
		bits-=8;
		LPodd >>=8;
	}
	bits = order + 3;
	if( bits % 8){
		ecc[byteoff-1] |= (LPeven << (bits % 8)) & 0x000000FF;
		LPeven >>= (8 - (bits % 8));
		bits -= (8 - bits % 8);
	} 
	while( bits>0){
		ecc[byteoff++] = LPeven & 0x000000FF;
		bits-=8;
		LPeven >>=8;
	}
	
	// Finally, we complete the spare bits with 1's (by convention, just like flash manufacturers do)
	
	if( bits)
		ecc[ byteoff-1] |= (0x000000FF << (8+bits));
	
	return 0;	
}

LOCAL int _ecc_gen32( uint8_t* vec, uint32_t order, uint8_t * ecc)
{
	
	uint32_t i;
	uint32_t LPodd, LPeven, CPodd, CPeven;
	uint32_t colparity;
	uint32_t blocksize, eccsize;
	int32_t bits;
	uint32_t bitoff, byteoff;
	uint32_t* vec32;
	
	if( order < 2)				// Only blocks of 4 bytes or more are supported
		os_return_minus_one_and_set_errno(OS_STATUS_ECC_INVALID_ORDER);
	
	// First, calculate the number of 32-bit positions in the data block
	
	blocksize = 0x1 << (order - 2);
	vec32 = (uint32_t *) vec;
	
	// Parity calculation
	// This is a faster variant of the way some manufacturers store ECC in flash memories.
	// Row and column parity is calculated separately.
	
	// Row parity and preliminary column parity calculation:
	
	LPodd = LPeven = 0;
	colparity = 0;
	
	for( i = 0; i < blocksize ; i++){
		
//		uint32_t temp;
//		temp = vec32[i];
//		colparity ^= temp;
//		if( parity_dword( temp)){

//		uint32_t temp;
//		temp = vec32[i];
//		colparity ^= temp;
//		temp ^= temp >> 16;
//		temp ^= temp >> 8;
//		if( parity_byte( temp & 0xFFU)){

		colparity ^= vec32[i];
		if( parity_dword( vec32[i])){

			LPodd ^= i;
			LPeven ^= ~i;	
		}
	}
	
	// Column parity
	
	CPodd = CPeven = 0;
	
	CPodd = parity_dword( colparity & 0xFFFF0000); CPodd <<= 1;
	CPodd |= parity_dword( colparity & 0xFF00FF00); CPodd <<= 1;
	CPodd |= parity_dword( colparity & 0xF0F0F0F0); CPodd <<= 1;
	CPodd |= parity_dword( colparity & 0xCCCCCCCC); CPodd <<= 1;
	CPodd |= parity_dword( colparity & 0xAAAAAAAA);
	CPeven = parity_dword( colparity & 0x0000FFFF); CPeven <<= 1;
	CPeven |= parity_dword( colparity & 0x00FF00FF); CPeven <<= 1;
	CPeven |= parity_dword( colparity & 0x0F0F0F0F); CPeven <<= 1;
	CPeven |= parity_dword( colparity & 0x33333333); CPeven <<= 1;
	CPeven |= parity_dword( colparity & 0x55555555);

	// Whole bit-offset parity storage
	
	LPodd <<= 5;	
	LPodd |= CPodd;
	LPeven <<= 5;	
	LPeven |= CPeven;
	
	// ECC packing and writing
	
	// First we zero the ECC buffer.
	
	eccsize = ((2*order)+5)/8 + 1;
	memset( ecc, 0, eccsize);
	
	// Odd and even parity values are now packed and stored. Odd first, little endian.
	
	bitoff = 0;
	byteoff = 0;
	bits = order + 3;
	while( bits>0){
		ecc[byteoff++] = LPodd & 0x000000FF;
		bits-=8;
		LPodd >>=8;
	}
	bits = order + 3;
	if( bits % 8){
		ecc[byteoff-1] |= (LPeven << (bits % 8)) & 0x000000FF;
		LPeven >>= (8 - (bits % 8));
		bits -= (8 - bits % 8);
	} 
	while( bits>0){
		ecc[byteoff++] = LPeven & 0x000000FF;
		bits-=8;
		LPeven >>=8;
	}
	
	// Finally, we complete the spare bits with 1's (by convention, just like flash manufacturers do)
	
	if( bits)
		ecc[ byteoff-1] |= (0x000000FF << (8+bits));
	
	return 0;	
}

inline int OS_ecc_gen( uint8_t* vec, uint32_t order, uint8_t * ecc)
{
    return _ecc_gen8(vec, order, ecc);
}

/*  ECC checking and correcting function.
 *  Limited to 4Gbit blocks (512MBytes). This should be much more than enough.
 *  Blocks must always be a power of two in size (i.e. 128, 256, 512, etc.).
 *  vec		-> Block of which we want to check and correct the ECC
 *  order	-> Log2 of the block size (i.e. 8 for a 256 byte block)
 *  ecc		-> Byte array containing the previously stored ECC.
 *  The function recalculates the ECC and checks for errors. Apart from reporting, the function does
 *  the following:
 *  - If there are no errors, nothing is done apart from reporting.
 *  - If there is a single, correctable error in the data, the error is corrected in the vec array.
 *  - If there is a single, correctable error in the ECC, the error is corrected in the ecc array.
 *  - If there are multiple errors correction is not possible and nothing else is done, only report.
 */
int OS_ecc_check_and_correct8( uint8_t* vec, uint32_t order, uint8_t * ecc){

	uint8_t newecc[ MAX_ECC_LEN];
	uint8_t ecccheck[ MAX_ECC_LEN];
	int32_t eccsize;
	uint32_t err = 0;
	int32_t i;
	
	eccsize = ((2*order)+5)/8 + 1;
	
	OS_ecc_gen( vec, order, newecc);
	
	for( i = 0; i < eccsize; i++){
		ecccheck[i] = ecc[i]^newecc[i];
		err+= countbits_byte( ecccheck[i]);
	}
	
	if( err == 0)				// No error
		return 0;
	
	if( err == 1){				// Error in the ECC field
		memcpy( ecc, newecc, eccsize);					// Copy back the ECC (faster)
		os_return_minus_one_and_set_errno(OS_STATUS_ECC_INVALID_ORDER);
	}
	
	if( err == (order + 3)){		// Single error in the data
		int32_t bitoffset = 0;
		int32_t bits = order+3;
		
		for( i = 0; bits > 0; bits-=8, i++)
			bitoffset |= ((uint32_t)ecccheck[i]) << (i * 8);

		bitoffset &= ~(0xFFFFFFFF << (order + 3));
		
		vec[ bitoffset / 8] ^= 0x01 << (bitoffset %8);	// Flip the wrong bit
		os_return_minus_one_and_set_errno(OS_STATUS_ECC_SINGLE_ERROR);
	}
	
	os_return_minus_one_and_set_errno(OS_STATUS_ECC_UNCORRECTABLE_ERROR);
}

int OS_ecc_check_and_correct( uint8_t* vec, uint32_t order, uint8_t * ecc){

	uint8_t newecc[ MAX_ECC_LEN];
	uint8_t ecccheck[ MAX_ECC_LEN];
	int32_t eccsize;
	uint32_t err = 0;
	int32_t i;
	
	eccsize = ((2*order)+5)/8 + 1;
	
	OS_ecc_gen( vec, order, newecc);
	
	for( i = 0; i < eccsize; i++){
		ecccheck[i] = ecc[i]^newecc[i];
		err+= countbits_byte( ecccheck[i]);
	}
	
	if( err == 0)				// No error
		return 0;
	
	if( err == 1){				// Error in the ECC field
		memcpy( ecc, newecc, eccsize);					// Copy back the ECC (faster)
		os_return_minus_one_and_set_errno(OS_STATUS_ECC_ERROR);
	}
	
	if( err == (order + 3)){		// Single error in the data
		int32_t bitoffset = 0;
		int32_t bits = order+3;
		
		for( i = 0; bits > 0; bits-=8, i++)
			bitoffset |= ((uint32_t)ecccheck[i]) << (i * 8);

		bitoffset &= ~(0xFFFFFFFF << (order + 3));
		
		vec[ bitoffset / 8] ^= 0x01 << (bitoffset %8);	// Flip the wrong bit
		os_return_minus_one_and_set_errno(OS_STATUS_ECC_SINGLE_ERROR);
	}
	
	os_return_minus_one_and_set_errno(OS_STATUS_ECC_UNCORRECTABLE_ERROR);
}

