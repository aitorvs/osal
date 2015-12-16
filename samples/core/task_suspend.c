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

#include <sys/time.h>
#include <stdlib.h>
#include <stdio.h>
#include <time.h>

/* misc stuff */
#define RAND(max) ((uint32_t) (((float) (max)) * (rand() / (RAND_MAX + 1.0))))
#define FRAND(max) (((float) (max)) * (rand() / (RAND_MAX + 1.0)))
#define RANDI(min,max) ((min) + RAND ((max) - (min)))
#define FRANDI(min,max) ((min) + FRAND ((max) - (min)))
#define RANDB(prob) (FRAND (1024) < (1024.0 * (prob)))

static void print_time(void);
static uint32_t t2;

struct params {
    int prio;

	int prio_min;
	int prio_max;
};


static void task_suspend (void)		
{									
    while(1)
    {
        printf("Task (%d) : ", (int)OS_TaskGetId()); 
        print_time();   
        OS_TaskSuspend(OS_SELF);
    }
}									

static void task_controller (void)		
{
    int ret;
    ret = OS_TaskResume(t2);

    if( ret < 0 )
        printf("Error resuming task %d\n", (int)t2);
}									

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

int main(void)
{
    uint32_t t1;
    int ret;
    static struct params p;
	struct timeval tm;

	printf("========================\n");
	printf("Task Suspend/Resume Test\n");
	printf("========================\n");

	p.prio_min = 1;
	p.prio_max = 255;
    
    OS_Init();

	/* seed random number generator */
	gettimeofday (&tm, 0);
	srand (tm.tv_usec);

    p.prio = RANDI (p.prio_min, p.prio_max);  
    ret = OS_TaskCreate (&t2,(void *)task_suspend, 2048, p.prio, 0, (void*)NULL);
    ASSERT( ret == OS_STATUS_SUCCESS );

    p.prio = RANDI (p.prio_min, p.prio_max);  
    ret = OS_TaskMonotonicCreate (
            &t1,
            (void *)task_controller, 
            (void*)perr, 2048, p.prio, 0, (void*)NULL, 1000);
    ASSERT( ret == OS_STATUS_SUCCESS );
    
    OS_Start();


    return 0;

} /* end OS_Application Startup */



