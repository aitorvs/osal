/**
 *  \file   deadman.c
 *  \brief  This file implements a deadman timer.
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

#include <stdio.h>
#include <assert.h>
#include <stdlib.h>

static uint32_t g_timerid;
static uint32_t t1;

#define DEADMAN_TIMER_MS    5000

static void print_time(void)
{
    OS_time_t time;

    OS_GetTimeSinceBoot(&time);

    printf("Current time: %02d:%03d\n", 
            (int)time.mul_Seconds, (int)time.mul_MicroSeconds/1000);
}

static OS_TSR_entry_t wdogHandle(void *ignored)
{

    /*  wdog no reset...end */
//    printf("Deadman timer not reset...ERROR!!!!!!!!!\n");
    assert( 0 );

    exit(-1);
    return 0;
}

static void task1(void *id)
{
    uint32_t ms = 0;
    printf("Task started\n");

    for(;;)
    {
        OS_Sleep(ms);
        ms+=500;
        OS_TimerReset(g_timerid);
        print_time();
    }
}

int main(void)
{
    int ret;

	printf("\n==================\n");
	printf("Timer OSAL Example\n");
	printf("==================\n\n");
    
    if( OS_Init() != OS_STATUS_SUCCESS )   return 0;
    
    if( OS_TaskCreate (&t1,(void *)task1, 2048, 99, 0, (void*)NULL) != OS_STATUS_SUCCESS)
    	printf("ERR: unable to create the tasks\n");

    ret = OS_TimerCreate(&g_timerid);
    if( ret < 0 ) goto err;
    if( (ret == OS_STATUS_SUCCESS) )
    {
        /*  The timer will be fired every 10000ms */
        ret = OS_TimerFireAfter(g_timerid, DEADMAN_TIMER_MS, (OS_TSR_entry_t)wdogHandle, (void*)NULL);
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



