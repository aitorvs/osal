/**
 *  \file   osmemram.c
 *  \brief  This file implements architecture specific RAM memory access calls 
 *
 *  The file provides to the library-user the interface to read from/write from
 *  RAM memory. This implementation should be architecture specific.
 *
 *  \author  Aitor Viana Sanchez (avs), Aitor.Viana.Sanchez@esa.int
 *  \author  Ezra Yeheksli
 *
 *  \internal
 *    Created:  27/03/09
 *   Revision:  $Id: osmemram.c 1.4 27/03/09 avs Exp $
 *   Compiler:  gcc/g++
 *    Company:  European Space Agency (ESA-ESTEC)
 *  Copyright:  Copyright (c) 2009, Aitor Viana Sanchez
 *
 *  This source code is released for free distribution under the terms of the
 *  GNU General Public License as published by the Free Software Foundation.
 * =====================================================================================
 */

#include "osal/osapi.h"

/** Check pointer of 8-bit size */
#define INVALID_CHAR_PTR(ptr)   ( (ptr == NULL) ? TRUE : FALSE )
/** Check pointer of 16-bit size */
#define INVALID_SHORT_PTR(ptr)  ( ((ptr == NULL) || ((int)ptr & 0x1) ) ? TRUE : FALSE )
/** Check pointer of 32-bit size */
#define INVALID_INT_PTR(ptr)    ( ((ptr == NULL) || ((int)ptr & 0x3) ) ? TRUE : FALSE )

/*============================================= PUBLIC INTERFACE  */

int32_t OS_MemRead8( uint32_t addr, uint8_t *value )
{
    if( INVALID_CHAR_PTR(value) )
        return OS_STATUS_EINVAL;
    else
        *value = *((uint8_t*)addr);

    return OS_STATUS_SUCCESS;
}

int32_t OS_MemRead16( uint32_t addr, uint16_t *value )
{
    if( INVALID_SHORT_PTR(value) )
        return OS_STATUS_EINVAL;
    else
        *value = *((uint16_t*)addr);

    return OS_STATUS_SUCCESS;
}

int32_t OS_MemRead32( uint32_t addr, uint32_t *value )
{
    if( INVALID_INT_PTR(value) )
        return OS_STATUS_EINVAL;
    else
        *value = *((uint32_t*)addr);

    return OS_STATUS_SUCCESS;
}

int32_t OS_MemWrite8( uint32_t addr, uint8_t value )
{
    *((uint8_t*)addr) = value;

    return OS_STATUS_SUCCESS;
}

int32_t OS_MemWrite16( uint32_t addr, uint16_t value )
{
    if( addr & 0x1 )
        return OS_STATUS_ADDRESS_MISALIGNED;
    else
        *((uint16_t*)addr) = value;

    return OS_STATUS_SUCCESS;
}

int32_t OS_MemWrite32( uint32_t addr, uint32_t value )
{
    if( addr & 0x3 )
        return OS_STATUS_ADDRESS_MISALIGNED;
    else
        *((uint32_t*)addr) = value;

    return OS_STATUS_SUCCESS;
}


