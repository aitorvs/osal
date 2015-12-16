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

#include <stdio.h>

static uint32_t g_tiemrid;
static uint32_t timerid2;
static uint32_t semid;
static uint32_t semid2;
static uint32_t t1;

static void print_time(void)
{
    OS_time_of_day_t time;

    OS_GetTimeOfDay(&time);

    printf("Current time: %04d/%02d/%02d %02d:%02d:%02d:%03d\n", 
            (int)time.mul_Year, (int)time.mul_Month, (int)time.mul_Day,
            (int)time.mul_Hour, (int)time.mul_Minute, (int)time.mul_Seconds, 
            (int)time.mul_MicroSeconds);
}

static OS_TSR_entry_t isr_monotonic_timer(uint32_t timerid, void *ignored)
{
    int ret;
    /*  the timer handler CANNOT have any printout  */
    ret = OS_TimerReset(timerid);
    ASSERT( ret == OS_STATUS_SUCCESS )

    OS_BinSemFlush(*(uint32_t*)ignored);

    return 0;
}

static OS_TSR_entry_t isr_shot_timer(uint32_t timerid, void *ignored)
{

    OS_BinSemFlush(*(uint32_t*)ignored);

    return 0;
}

static void task1(void *arg)
{
    uint32_t sem_id = *(uint32_t*)arg;
    printf("Task started\n");

    for(;;)
    {
        OS_BinSemTake(sem_id);
        printf("Task %d - ", (int)OS_TaskGetId());
        print_time();
    }
}

int main(void)
{
    int ret;
    OS_time_of_day_t tod;

	printf("Timer example APP...\n");
    
    if( OS_Init() != OS_STATUS_SUCCESS )   return 0;
    
    if( OS_BinSemCreate(&semid, 0, 0) != OS_STATUS_SUCCESS )
    {
        printf("ERR: unable to create the semaphore\n");
        return -1;
    }
    if( OS_BinSemCreate(&semid2, 0, 0) != OS_STATUS_SUCCESS )
    {
        printf("ERR: unable to create the semaphore\n");
        return -1;
    }

    ret = OS_TimerCreate(&g_tiemrid);
    ASSERT( ret == OS_STATUS_SUCCESS );
    ret = OS_TimerCreate(&timerid2);
    ASSERT( ret == OS_STATUS_SUCCESS );
    if( (ret == OS_STATUS_SUCCESS) )
    {
        /*  The timer will be fired every 1000ms */
        ret = OS_TimerFireAfter(g_tiemrid, 1000, (OS_TSR_entry_t)isr_monotonic_timer, (void*)&semid);
        ASSERT( ret == OS_STATUS_SUCCESS );

        OS_GetTimeOfDay(&tod);
        tod.mul_Seconds +=10;

        ret = OS_TimerFireWhen(timerid2, &tod, (OS_TSR_entry_t)isr_shot_timer, (void*)&semid2);
        ASSERT( ret == OS_STATUS_SUCCESS );

        if( (ret != OS_STATUS_SUCCESS) )
            printf("ERR(%d): arming the timer\n", ret);
    }
    else
    {
        printf("ERR: unable to create the timer\n");
        TRACE(ret, "d");
        return 0;
    }

    if( OS_TaskCreate (&t1,(void *)task1, 2048, 99, 0, (void*)&semid) != OS_STATUS_SUCCESS)
    	printf("ERR: unable to create the tasks\n");

    if( OS_TaskCreate (&t1,(void *)task1, 2048, 99, 0, (void*)&semid2) != OS_STATUS_SUCCESS)
    	printf("ERR: unable to create the tasks\n");


    OS_Start();


    return 0;

} /* end OS_Application Startup */



