/**
 *  \file   oscrc.c
 *  \brief  This source file implements the CRC utils under the OSAL library.
 *
 *  The CRC utils provide the library-use to generate CRC(s) of given data
 *  according to a default CRC polynom key of a user-given polynom key
 *
 *  \author  Aitor Viana Sanchez (avs), Aitor.Viana.Sanchez@esa.int
 *
 *  \internal
 *    Created:  01/04/09
 *   Revision:  $Id: oscrc.c 1.4 01/04/09 avs Exp $
 *   Compiler:  gcc/g++
 *    Company:  European Space Agency (ESA-ESTEC)
 *  Copyright:  Copyright (c) 2009, Aitor Viana Sanchez
 *
 *  This source code is released for free distribution under the terms of the
 *  GNU General Public License as published by the Free Software Foundation.
 * =====================================================================================
 */

#include <osal/osapi.h>

#define DEFAULT_CRCKEY  0x04c11db7

LOCAL uint32_t		crc_table[ 256];
LOCAL uint32_t		crc_poly;
LOCAL BOOLEAN	crc_initialized = FALSE;

int OS_crc_init( uint32_t crckey){

	uint32_t i, k;
	uint32_t precalc;
	BOOLEAN bit;
	
	for( i = 0; i < 256; i++){
		precalc = i << 24;
		for( k = 0; k < 8; k++){
			bit = precalc & 0x80000000;
			precalc <<=1;
			if( bit)
				precalc ^= crckey;
		}
		crc_table[i] = precalc;
	}
	crc_poly = crckey;
	crc_initialized = TRUE;

    return 0;
}

/*
 *  CRC generator.
 * It is implemented as:
 * - Initial value = 0
 * - Final XOR value = 0
 * - Do NOT reverse data bytes
 * - Do NOT reverse output
 * - Nondirect method
 * It's been checked with the Ethernet poly (0x04CC11DB7) and it produces the
 * correct result (89A1897F) with the reference string "123456789"
 */
uint32_t	OS_crc_gen( void * buffer, uint32_t len){
	uint32_t	crc;
	uint32_t	i;
	uint8_t * data = (uint8_t *) buffer;

	if( crc_initialized == FALSE){
		OS_crc_init( DEFAULT_CRCKEY);
	}

	crc = 	((uint32_t)data[0]) << 24 | 
			((uint32_t)data[1]) << 16 | 
			((uint32_t)data[2]) << 8	|
			(uint32_t)data[3];
	
	i = 4;
	
	while( i < len){
		crc = ((crc << 8) | (uint32_t)data[i]) ^ crc_table[crc >> 24];
		i++;
	}

	return crc;
}
