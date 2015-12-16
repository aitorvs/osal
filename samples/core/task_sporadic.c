/**
 *  \file   monotonic.c
 *  \brief  This file implements a sporadic task using OSAL.
 *
 *  The task are created with incrementing priorities so they have to execute in
 *  order. When the highest priority has finished the execution, the next task
 *  in priority starts and so on.
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

#define SECS_PER_DAY    (24*60*60)
#define SECS_PER_HOUR   (60*60)
#define SECS_PER_MIN    (60)

/* misc stuff */
#define RAND(max) ((uint32_t) (((float) (max)) * (rand() / (RAND_MAX + 1.0))))
#define FRAND(max) (((float) (max)) * (rand() / (RAND_MAX + 1.0)))
#define RANDI(min,max) ((min) + RAND ((max) - (min)))
#define FRANDI(min,max) ((min) + FRAND ((max) - (min)))
#define RANDB(prob) (FRAND (1024) < (1024.0 * (prob)))

static uint32_t *counters;

struct params {
    int ntasks;
    int prio;

	int ntasks_min;
	int ntasks_max;
	int prio_min;
	int prio_max;
};

static void task (void *_apid)   
{   
    int32_t apid = *(int32_t*)_apid;

    while(1)
    {
        printf("Task APID %d\n", (int)apid);
        if( counters[apid]++ > 10 ) OS_TaskExit();
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

	printf("Spordic task Example\n");

	p.ntasks_min = 1;
	p.ntasks_max = 10;
	p.prio_min = 10;
	p.prio_max = 50;

    OS_Init();

    /* seed random number generator */
	gettimeofday (&tm, 0);
	srand (tm.tv_usec);

    p.prio = RANDI (p.prio_min, p.prio_max);  
    p.ntasks = RANDI (p.ntasks_min, p.ntasks_max);  
    apids = (uint32_t*)malloc(p.ntasks*sizeof(uint32_t));
    counters = (uint32_t*)malloc(p.ntasks*sizeof(uint32_t));
    bzero( (void*)counters, p.ntasks*sizeof(uint32_t));
    if( !apids )    return -1;
    
    ret = OS_GetConfigInfo(&osal_info);
    ASSERT( ret == OS_STATUS_SUCCESS );

    for(i = 0; i < p.ntasks; ++i )
    {
        apids[i] = i+1;

        /*  Periodic Tasks  */
        ret = OS_TaskCreate(&t1, (void*)task, 2048, p.prio+i, 0, (void*)&apids[i]);
        if( ret < 0 )
        {
            ASSERT( i >= osal_info.max_periodic_task_number );
            break;
        }
    }
    
    OS_Start();


    return 0;

} /* end OS_Application Startup */



