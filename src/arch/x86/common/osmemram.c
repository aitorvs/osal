/**
 *  \file   osmemram.c
 *  \brief  This file implements architecture specific RAM memory access calls 
 *
 *  \author  Aitor Viana Sanchez (avs), Aitor.Viana.Sanchez@esa.int
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

int OS_MemRead8( uint32_t addr, uint8_t *value )
{
    UNUSED(value);
    UNUSED(addr);
    os_return_minus_one_and_set_errno(OS_STATUS_EERR);
}

int OS_MemRead16( uint32_t addr, uint16_t *value )
{
    UNUSED(value);
    UNUSED(addr);
    os_return_minus_one_and_set_errno(OS_STATUS_EERR);
}

int OS_MemRead32( uint32_t addr, uint32_t *value )
{
    UNUSED(value);
    UNUSED(addr);
    os_return_minus_one_and_set_errno(OS_STATUS_EERR);
}

int OS_MemWrite8( uint32_t addr, uint8_t value )
{
    UNUSED(value);
    UNUSED(addr);
    os_return_minus_one_and_set_errno(OS_STATUS_EERR);
}

int OS_MemWrite16( uint32_t addr, uint16_t value )
{
    UNUSED(value);
    UNUSED(addr);
    os_return_minus_one_and_set_errno(OS_STATUS_EERR);
}

int OS_MemWrite32( uint32_t addr, uint32_t value )
{
    UNUSED(value);
    UNUSED(addr);
    os_return_minus_one_and_set_errno(OS_STATUS_EERR);
}


