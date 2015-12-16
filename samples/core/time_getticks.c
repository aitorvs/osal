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
static struct params p;

struct params {
    int ntasks;
    int prio;
    int ms_period;

	int ntasks_min;
	int ntasks_max;
	int prio_min;
	int prio_max;
    int ms_period_max;
    int ms_period_min;
};

static void task (void *_apid)   
{   
    int32_t apid = *(int32_t*)_apid;
    uint32_t ul_Ticks = 0;
    int ret;

    while(counters[apid]++ < 100)
    {
        ret = OS_GetTicks(&ul_Ticks);
        if( ret == OS_STATUS_SUCCESS )
            printf("OS Ticks %u\n", (int)ul_Ticks);
        else
            printf("OS_GetTicks() Error\n");

        p.ms_period = RANDI (p.ms_period_min, p.ms_period_max);  
        OS_Sleep(p.ms_period);
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
    struct s_osal_info osal_info;
	struct timeval tm;

	printf("Spordic task Example\n");

	p.ntasks_min = 1;
	p.ntasks_max = 10;
	p.prio_min = 10;
	p.prio_max = 50;
	p.ms_period_min = 500;
	p.ms_period_max = 1500;

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



