/**
 *  \file   yield.c
 *  \brief  This file implements an example of how to yield tasks. Sort of round
 *  robin
 *
 *  The test creates several random number of tasks and demostrate how to
 *  implement a sort of round robin using the OSAL. OS_Sleep(0) is used to put
 *  the caller task in the ready queue and force a rescheduling.
 *
 *  \author  Aitor Viana Sanchez (avs), Aitor.Viana.Sanchez@esa.int
 *
 *  \internal
 *    Created:  07/17/2009
 *   Revision:  $Id: yield.c 1.4 07/17/2009 avs Exp $
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

#define TEST_MIN_PRIO   1
#define TEST_MAX_PRIO   50
#define TEST_MIN_TASKS  2
#define TEST_MAX_TASKS  4

static void print_time(void);
static uint32_t counters[TEST_MAX_TASKS];

struct params {
    int ntasks;
    int prio;

	int ntasks_min;
	int ntasks_max;
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
            (int)t_time.mul_MicroSeconds/1000);
}


static void task (void *_apid)   
{   
    int32_t apid = *(int32_t*)_apid;

    while(1)
    {
        printf("Task APID %d -- ", (int)apid);
        print_time();
        /*  This call should put the current task at the end of the ready queue for
         *  the current priority
         */
        OS_TaskYield();
        OS_Sleep(500);
        if( counters[apid]++ > 40 ) break;
    }

    OS_TaskExit();

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

	printf("===================\n");
	printf("Yield task Example\n");
	printf("===================\n");

	p.ntasks_min = TEST_MIN_TASKS;
	p.ntasks_max = TEST_MAX_TASKS;
	p.prio_min = TEST_MIN_PRIO;
	p.prio_max = TEST_MAX_PRIO;

    OS_Init();

	/* seed random number generator */
	gettimeofday (&tm, 0);
	srand (tm.tv_usec);

    p.ntasks = RANDI (p.ntasks_min, p.ntasks_max);  
    apids = (uint32_t*)malloc(p.ntasks*sizeof(uint32_t));
    bzero( (void*)counters, p.ntasks*sizeof(uint32_t));
    if( !apids )    return -1;
    
    ret = OS_GetConfigInfo(&osal_info);
    ASSERT( ret == OS_STATUS_SUCCESS );

    for(i = 0; i < p.ntasks; ++i )
    {
//        p.prio = RANDI (p.prio_min, p.prio_max);  
        p.prio = 50;
        apids[i] = i+1;

        /*  Periodic Tasks  */
        ret = OS_TaskCreate(&t1, (void*)task, 512, p.prio, 0, (void*)&apids[i]);
        if( ret < 0 )
        {
            ASSERT( i >= osal_info.max_periodic_task_number );
            break;
        }
    }

    printf("Starting...\n");
    
    OS_Start();


    return 0;

} /* end OS_Application Startup */



