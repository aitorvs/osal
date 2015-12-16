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

#include <osal/osdebug.h>
#include "osal/osapi.h"

static uint32_t g_tiemrid;
static uint32_t t1;

#define SECS_PER_DAY    (24*60*60)
#define SECS_PER_HOUR   (60*60)
#define SECS_PER_MIN    (60)

static void print_time(void)
{
    OS_time_t t_time;
    long l_Hours, l_Mins, l_Secs, l_mSecs;

    OS_GetLocalTime(&t_time);

    /*  Calculating the t_time    */
    l_Secs = t_time.mul_Seconds % SECS_PER_DAY;
    l_Hours = l_Secs / SECS_PER_HOUR;
    l_Secs = l_Secs % SECS_PER_HOUR;
    l_Mins = l_Secs / SECS_PER_MIN;
    l_Secs = l_Secs % SECS_PER_MIN;
    l_mSecs = ( t_time.mul_MicroSeconds + 500 ) / 1000;

    PRINT("Current time: %02ld:%02ld:%02ld:%03ld\n", l_Hours, l_Mins, l_Secs, l_mSecs);
}

static void task1(void *arg)
{
    PRINT("Task_%d -- ", *((int*)arg));
    print_time();
    OS_TaskExit();
}

static OS_TSR_entry_t timer_TSR(uint32_t timerid, void *ignored)
{
    if( OS_TimerReset(timerid) != OS_STATUS_SUCCESS )
        PRINT("ERR: reseting the timer\n");

    PRINT("ISR\n");

    if( OS_TaskCreate (&t1,(void *)task1, 2048, 99, 0, (void*)&t1) != OS_STATUS_SUCCESS)
    	PRINT("ERR: unable to create the tasks\n");

    return 0;
}

int main(void)
{
    int ret;
	PRINT("Timer example APP...\n");
    
    if( OS_Init() != OS_STATUS_SUCCESS )   return 0;
    
    if( (ret = OS_TimerCreate(&g_tiemrid)) == OS_STATUS_SUCCESS )
    {
        /*  The timer will be fired every 100ms */
        if( (ret = OS_TimerFireAfter(g_tiemrid, 1000, (OS_TSR_entry_t)timer_TSR, (void*)NULL)) != OS_STATUS_SUCCESS )
            PRINT("ERR(%d): arming the timer\n", ret);
    }
    else
    {
        PRINT("ERR: unable to create the timer\n");
        TRACE(ret, "d");
        return 0;
    }
    
    OS_Start();


    return 0;

} /* end OS_Application Startup */



