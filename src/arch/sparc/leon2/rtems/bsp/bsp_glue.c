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
#include <public/lock.h>

/*============================================= MACRO DEFINITIONS */

#define INIT_THREAD_MUTEX() \
    do{ \
        int ret;    \
        ret = lock_rw_init(&_rwlock);   \
        ASSERT( ret >= 0 ); \
    }while(0);

#define WLOCK()   __WLOCK()
#define WUNLOCK() __WUNLOCK()

#define WDOG_MAX_TIME_TICKS         0x00ffffff

/*============================================= PRIVATE VARIABLES */

LOCAL uint32_t wdog_expire = 0;
LOCAL volatile uint32_t *wdog_reg = (uint32_t*)0x8000004c;
LOCAL volatile uint32_t *prescaler = (uint32_t*)0x80000064;



/*============================================= PUBLIC INTERFACE  */
int OS_BSP_WdogInit(uint32_t timeout)
{

    if( wdog_expire == 0 )
    {
        INIT_THREAD_MUTEX();

        /*  timeout is in ms, convert to timer_ticks    */
        wdog_expire = (uint32_t)((CONFIG_HARDWARE_CLOCK_FREQ * 1000 * timeout)/((*prescaler+1)));
        wdog_expire = ( wdog_expire > WDOG_MAX_TIME_TICKS ) ? WDOG_MAX_TIME_TICKS : wdog_expire;
    }

    return 0;
}

int OS_BSP_WdogEnable(void)
{
    if( wdog_expire )
    {
        DEBUG("Wdog Enabled");
        WLOCK();
        {
            *wdog_reg = wdog_expire;
        }
        WUNLOCK();
        return 0;
    }
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
        DEBUG("Wdog reset 0x%x", (unsigned)wdog_expire);
        WLOCK();
        {
            *wdog_reg = wdog_expire;
        }
        WUNLOCK();
        return 0;
    }
    os_return_minus_one_and_set_errno(OS_STATUS_EERR);
}

