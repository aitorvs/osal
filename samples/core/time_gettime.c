/**
 *  \file   time_gettime.c
 *  \brief  This file implements one task retrieving the time using both
 *  OS_GetTicksSinceBoot() and OS_GetLocalTime() to see the differences between
 *  them.
 *
 *  \author  Aitor Viana Sanchez (avs), Aitor.Viana.Sanchez@esa.int
 *
 *  \internal
 *    Created:  09/08/2010
 *   Revision:  $Id: time_gettime.c 1.4 09/08/2010 avs Exp $
 *   Compiler:  gcc/g++
 *    Company:  European Space Agency (ESA-ESTEC)
 *  Copyright:  Copyright (c) 2010, Aitor Viana Sanchez
 *
 *  This source code is released for free distribution under the terms of the
 *  GNU General Public License as published by the Free Software Foundation.
 * =====================================================================================
 */

#include <osal/osapi.h>
#include <osal/osdebug.h>

#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <sys/time.h>
#include <time.h>
#include <assert.h>

static void task (void *ignored)   
{   
    OS_time_t sinceboot, systime;
    OS_time_of_day_t tod;
    uint32_t ticks, ticks_per_sec;
    static int iter = 0;

    assert( OS_GetLocalTime(&systime) == 0 );
    assert( OS_GetTimeSinceBoot(&sinceboot) == 0 );
    assert( OS_GetTimeOfDay(&tod) == 0 );
    assert( OS_GetTicks(&ticks) == 0 );
    assert( OS_GetTicksPerSecond(&ticks_per_sec) == 0 );

    printf("-------------------\n");
    printf("*** Ticks Per Second\n");
    printf("%u\n", (unsigned)ticks_per_sec);

    printf("*** System Time\n");
    printf("%u.%u\n", (unsigned)systime.mul_Seconds, (unsigned)systime.mul_MicroSeconds);

    printf("*** Time Since Boot\n");
    printf("%u.%u\n", (unsigned)sinceboot.mul_Seconds, (unsigned)sinceboot.mul_MicroSeconds);

    printf("*** Ticks Since Boot\n");
    printf("%u\n", (unsigned)ticks);

    printf("*** Time of Day\n");
    printf("%u/%u/%u %u:%u:%u\n", 
            (unsigned)tod.mul_Day, (unsigned)tod.mul_Month, (unsigned)tod.mul_Year,
            (unsigned)tod.mul_Hour, (unsigned)tod.mul_Minute, (unsigned)tod.mul_Seconds);

    printf("-------------------\n");

    if( iter++ == 100 )
        OS_TaskExit();

}   

static void perr(void* error_code)
{
    printf("Task %d : Error\n", (int)OS_TaskGetId());
}

int main(void)
{
    uint32_t t1;
    int32_t ret;
    /** Test parameter structure */

	printf("*** GET TIME EXAMPLE\n");

    OS_Init();

    /*  Periodic Tasks  */
    ret = OS_TaskMonotonicCreate(&t1, (void*)task, (void*)perr, 2048, 50, 0, (void*)NULL, 2000);
    if( ret < 0 )
    {
        printf("ERROR!!!\n");
        exit(1);
    }
    
    OS_Start();


    return 0;

} /* end OS_Application Startup */



