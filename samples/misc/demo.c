#include <osal/osapi.h>

#include "demo.h"

#define SECS_PER_DAY    (24*60*60)
#define SECS_PER_HOUR   (60*60)
#define SECS_PER_MIN    (60)

static void print_time(void);

#define TASK(num)	\
static void task##num (void)		\
{									\
    static int32_t count = 0;   \
    OS_printf("(Task %d) : ", num); \
    print_time();   \
    if( count++ == 100 ) OS_TaskExit();   \
}									\

#define NAME_TASK(num)  task##num

TASK(1)
TASK(2)

static void print_time(void)
{
    OS_time_t time;
    long l_Hours, l_Mins, l_Secs, l_mSecs;

    OS_GetLocalTime(&time);

    /*  Calculating the time    */
    l_Secs = time.seconds % SECS_PER_DAY;
    l_Hours = l_Secs / SECS_PER_HOUR;
    l_Secs = l_Secs % SECS_PER_HOUR;
    l_Mins = l_Secs / SECS_PER_MIN;
    l_Secs = l_Secs % SECS_PER_MIN;
    l_mSecs = ( time.nanoseconds + 500 ) / 1000000;

    OS_printf("Current time: %02ld:%02ld:%02ld:%03ld\n", l_Hours, l_Mins, l_Secs, l_mSecs);
}


static void perr(void* error_code)
{
    int32_t error = *(int32_t*)error_code;
    os_err_name_t s_error;

    OS_GetErrorName(error, &s_error);

    OS_printf("Task %d : Error %s\n", OS_TaskGetId(), s_error);
}

int main(void)
{
    DEMORET
    uint32_t t1;

	OS_printf("Periodic task Example\n");
    
    OS_Init();
    
    if( OS_TaskMonotonicCreate (&t1,(void *)NAME_TASK(1), (void*)perr, NULL, 2048, 99, 0, (void*)NULL, 1000) != OS_SUCCESS)
    	OS_printf("ERR: unable to create the tasks\n");
    if( OS_TaskMonotonicCreate (&t1,(void *)NAME_TASK(2), (void*)perr, NULL, 2048, 99, 0, (void*)NULL, 500) != OS_SUCCESS)
    	OS_printf("ERR: unable to create the tasks\n");
    
    OS_Start();


    return 0;

} /* end OS_Application Startup */



