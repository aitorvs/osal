/**
 *  \file   bsp_glue.c
 *  \brief  This file contains the glue routines between the app and the OS
 *  Board Support Package (BSP).
 *
 *  The functionality here allow the app to interface features that are board
 *  or OS specific and dont fit well in the OSAL layer.
 *
 *  \author  Aitor Viana Sanchez (avs), Aitor.Viana.Sanchez@esa.int
 *
 *  \internal
 *    Created:  17/03/09
 *   Revision:  $Id: bsp_glue.c 1.4 17/03/09 avs Exp $
 *   Compiler:  gcc/g++
 *    Company:  European Space Agency (ESA-ESTEC)
 *  Copyright:  Copyright (c) 2009, Aitor Viana Sanchez
 *
 *  This source code is released for free distribution under the terms of the
 *  GNU General Public License as published by the Free Software Foundation.
 * =====================================================================================
 */

#include <osal/osapi.h>
#include <osal/osdebug.h>

#include <stdlib.h>

/*============================================= MACRO DEFINITIONS */

#define WDOG_MIN_TIME   0x0
#define WDOG_MAX_TIME   0xffffffff

#define INIT_THREAD_MUTEX() \
    do{ \
    }while(0);
#define WLOCK()
#define WUNLOCK()

#define WDOG_MAX_TIME_TICKS         0x00ffffff

/*============================================= PRIVATE VARIABLES */

LOCAL uint32_t wdog_expire = 0;
LOCAL uint8_t wdog_reset = 0;

/*============================================= PUBLIC INTERFACE  */

static void wdog_task (void *ignored)   
{   
    DEBUG("");
    if( wdog_reset ) wdog_reset = 0;
    else
    {
        DEBUG("Watchdog reset");
        exit(-1);
    }
}   

int OS_BSP_WdogInit(uint32_t timeout)
{
    uint32_t t1;
    if( 0 == wdog_expire )
    {
        int ret;
        ret = OS_TaskMonotonicCreate(
                &t1, (void*)wdog_task, NULL, 1024, 
                OS_TASK_MAX_PRIORITY, 0, (void*)NULL, timeout);
        ASSERT( ret >= 0 );
        if( ret >= 0 )
        {
            INIT_THREAD_MUTEX();
            wdog_expire = timeout;
        }

        return 0;
    }

    os_return_minus_one_and_set_errno(OS_STATUS_EERR);
}

int OS_BSP_WdogEnable(void)
{
    if( wdog_expire ) return 0;
    else
        os_return_minus_one_and_set_errno(OS_STATUS_EERR);
}

int OS_BSP_WdogDisable(void)
{
    os_return_minus_one_and_set_errno(OS_STATUS_EERR);
}

int OS_BSP_WdogReset(void)
{
    if( wdog_expire )
    {
        WLOCK();
        {
            wdog_reset = 1;
        }
        WUNLOCK();
        return 0;
    }

    os_return_minus_one_and_set_errno(OS_STATUS_EERR);
}

