/**
 *  \file   monotonic.c
 *  \brief  This file implements a monotonic task using OSAL
 *
 *  The test creates a monotonic task with a certain period and prints some
 *  messages using the console output
 *
 *  \author  Aitor Viana Sanchez (avs), Aitor.Viana.Sanchez@esa.int
 *
 *  \internal
 *    Created:  07/17/2009
 *   Revision:  $Id: monotonic.c 1.4 07/17/2009 avs Exp $
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

#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <sys/time.h>
#include <time.h>

/* misc stuff */
#define RAND(max) ((uint32_t) (((float) (max)) * (rand() / (RAND_MAX + 1.0))))
#define FRAND(max) (((float) (max)) * (rand() / (RAND_MAX + 1.0)))
#define RANDI(min,max) ((min) + RAND ((max) - (min)))
#define FRANDI(min,max) ((min) + FRAND ((max) - (min)))
#define RANDB(prob) (FRAND (1024) < (1024.0 * (prob)))

static void print_time(void);
static uint32_t *counters;

struct params {
    int ntasks;
    int period;
    int prio;

	int ntasks_min;
	int ntasks_max;
	int period_min;
	int period_max;
	int prio_min;
	int prio_max;
};

static void print_time(void)
{
    OS_time_of_day_t t_time;

    OS_GetTimeOfDay(&t_time);

    printf("Current t_time: %04d/%02d/%02d %02d:%02d:%02d:%03d\n", 
            (int)t_time.mul_Year, (int)t_time.mul_Month, (int)t_time.mul_Day,
            (int)t_time.mul_Hour, (int)t_time.mul_Minute, (int)t_time.mul_Seconds, 
            (int)t_time.mul_MicroSeconds);
}


static void perr(void* error_code)
{
    printf("Task %d : Error\n", (int)OS_TaskGetId());
}

static void task (void *_apid)   
{   
    int32_t apid = *(int32_t*)_apid;

    printf("Task APID %d -- ", (int)apid);
    print_time();
    if( counters[apid]++ > 20 ) 
    {
        OS_TaskExit();
    }

}   

int main(void)
{
    uint32_t t1;
    int32_t ret;
    uint32_t *apids;
    int32_t i;
    /** Test parameter structure */
    static struct params p;
    struct s_osal_info osal_info;
	struct timeval tm;

	printf("=====================\n");
	printf("Periodic task Example\n");
	printf("=====================\n");

	p.ntasks_min = 1;
	p.ntasks_max = 50;
	p.period_min = 1000;
	p.period_max = 2000;
	p.prio_min = 1;
	p.prio_max = 255;

    OS_Init();

	/* seed random number generator */
	gettimeofday (&tm, 0);
	srand (tm.tv_usec);

    p.ntasks = RANDI (p.ntasks_min, p.ntasks_max);  
    apids = (uint32_t*)OS_Malloc(p.ntasks*sizeof(uint32_t));
    counters = (uint32_t*)OS_Malloc(p.ntasks*sizeof(uint32_t));
    bzero( (void*)counters, p.ntasks*sizeof(uint32_t));
    if( !apids )    return -1;
    
    ret = OS_GetConfigInfo(&osal_info);
    ASSERT( ret == OS_STATUS_SUCCESS );

    for(i = 0; i < p.ntasks; ++i )
    {
        p.period = RANDI (p.period_min, p.period_max);  
        p.prio = RANDI (p.prio_min, p.prio_max);  
        apids[i] = i+1;

        /*  Periodic Tasks  */
        ret = OS_TaskMonotonicCreate(&t1, (void*)task, (void*)perr, 2048, p.prio, 0, (void*)&apids[i], p.period);
        if( ret < 0 )
        {
            ASSERT( i >= osal_info.max_periodic_task_number );
            break;
        }
    }
    
    OS_Start();


    return 0;

} /* end OS_Application Startup */



