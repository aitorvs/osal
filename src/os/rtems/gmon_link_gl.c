/**
 *  \file   gmon_link_gl.c
 *  \brief  This file capitalizes all the possible functions to dump the
 *  profiling data using different links.
 *
 *  \author  Aitor Viana Sanchez (avs), Aitor.Viana.Sanchez@esa.int
 *
 *  \internal
 *    Created:  05/20/2010
 *   Revision:  $Id: gmon_link_gl.c 1.4 05/20/2010 avs Exp $
 *   Compiler:  gcc/g++
 *    Company:  European Space Agency (ESA-ESTEC)
 *  Copyright:  Copyright (c) 2010, Aitor Viana Sanchez
 *
 *  This source code is released for free distribution under the terms of the
 *  GNU General Public License as published by the Free Software Foundation.
 * =====================================================================================
 */


#include <stdint.h>
#include <public/osal_config.h>

#if (CONFIG_OS_PROFILE_OVER_SERIAL == 1)
extern int gmon_initialize_serial();
extern void gmon_write_serial(uint8_t vector[] , unsigned int dim);
#elif (CONFIG_OS_PROFILE_OVER_ETH == 1 )
extern int32_t gmon_initialize_eth(void);
void gmon_write_eth( char vector[], uint32_t dim);
#endif

int gmon_initialize_link(void)
{
    int ret = -1;

#if (CONFIG_OS_PROFILE_OVER_SERIAL == 1)
    ret = gmon_initialize_serial();
#elif (CONFIG_OS_PROFILE_OVER_ETH == 1 )
    ret = gmon_initialize_eth();
#endif

    return ret;

}

void gmon_write_link(char *buf, uint32_t dim)
{
#if (CONFIG_OS_PROFILE_OVER_SERIAL == 1)
        gmon_write_serial( (uint8_t*)buf , (unsigned int)dim );
#elif (CONFIG_OS_PROFILE_OVER_ETH == 1 )
        gmon_write_eth( (char*)buf , (uint32_t)dim );
#endif
}

