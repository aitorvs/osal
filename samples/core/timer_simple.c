/**
 *  \file   hello.c
 *  \brief  This file implements a basic example of timer usage over OSAL
 *
 *  Detailed description starts here.
 *
 *  \author  Aitor Viana Sanchez (avs), Aitor.Viana.Sanchez@esa.int
 *
 *  \internal
 *    Created:  28/01/08
 *   Revision:  $Id: doxygen.templates.example,v 1.4 2007/08/02 14:35:24 mehner Exp $
 *   Compiler:  gcc/g++
 *    Company:  European Space Agency (ESA-ESTEC)
 *  Copyright:  Copyright (c) 2008, Aitor Viana Sanchez
 *
 *  This source code is released for free distribution under the terms of the
 *  GNU General Public License as published by the Free Software Foundation.
 * =====================================================================================
 */

#include <osal/osapi.h>
#include <osal/osdebug.h>

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>

static uint32_t g_timerid1;
static uint32_t g_timerid2;

static void print_time(void)
{
    OS_time_of_day_t time;

    OS_GetTimeOfDay(&time);

    printf("Current time: %04d/%02d/%02d %02d:%02d:%02d:%03d\n", 
            (int)time.mul_Year, (int)time.mul_Month, (int)time.mul_Day,
            (int)time.mul_Hour, (int)time.mul_Minute, (int)time.mul_Seconds, 
            (int)time.mul_MicroSeconds/1000);
}

static OS_TSR_entry_t timer_TSR(void *_timerid)
{
    int ret;
    uint32_t timerid = (uint32_t)_timerid;

    /*  the timer handler CANNOT have any printout  */
    ret = OS_TimerReset(timerid);
    printf("%d - ", (int)timerid);
    print_time();
    assert( ret == OS_STATUS_SUCCESS );

    return 0;
}

int main(void)
{
    int ret;

	printf("\n==================\n");
	printf("Timer OSAL Example\n");
	printf("==================\n\n");
    
    if( OS_Init() != OS_STATUS_SUCCESS )   return 0;
    
    ret = OS_TimerCreate(&g_timerid2);
    if( ret < 0 ) goto err;
    ret = OS_TimerCreate(&g_timerid1);
    if( ret < 0 ) goto err;
    if( (ret == OS_STATUS_SUCCESS) )
    {
        /*  The timer will be fired every 1000ms */
        ret = OS_TimerFireAfter(g_timerid2, 1500, (OS_TSR_entry_t)timer_TSR, (void*)g_timerid2);
        assert( ret == OS_STATUS_SUCCESS );
        ret = OS_TimerFireAfter(g_timerid1, 4000, (OS_TSR_entry_t)timer_TSR, (void*)g_timerid1);
        assert( ret == OS_STATUS_SUCCESS );

        if( (ret != OS_STATUS_SUCCESS) )
            printf("ERR(%d): arming the timer\n", ret);
    }
    else
    {
        printf("ERR: unable to create the timer\n");
        TRACE(ret, "d");
        return 0;
    }
    
    OS_Start();


    return 0;
err:
    printf("%d: Error\n", ret);
    exit(-1);

} /* end OS_Application Startup */



