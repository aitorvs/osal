/**
 *  \file   ostime.c
 *  \brief  This file features the Time API implementation for the OSAL library
 *  under Linux operating system
 *
 *  Detailed description starts here.
 *
 *  \author  Aitor Viana Sanchez (avs), Aitor.Viana.Sanchez@esa.int
 *
 *  \internal
 *    Created:  11/25/2009
 *   Revision:  $Id: ostime.c 1.4 11/25/2009 avs Exp $
 *   Compiler:  gcc/g++
 *    Company:  European Space Agency (ESA-ESTEC)
 *  Copyright:  Copyright (c) 2009, Aitor Viana Sanchez
 *
 *  This source code is released for free distribution under the terms of the
 *  GNU General Public License as published by the Free Software Foundation.
 * =====================================================================================
 */

/*
** File   : osapi.c
**
** Author : Alan Cudmore
**
** Purpose: 
**         This file  contains some of the OS APIs abstraction layer 
**         implementation for POSIX, specifically for Linux / Mac OS X.
**
*/

#include <time.h>
#include <sys/time.h>

#include <unistd.h>
#include <osal/osapi.h>
#include <osal/osdebug.h>

#define _IS_TIME_INIT()   \
{   \
    if( !_time_is_init ) \
    { \
        _os_time_init(); \
        _time_is_init = 1; \
    } \
}
#define _CHECK_TIME_INIT()  (_IS_TIME_INIT())

#define MICROS_PER_TICK ( (1000000/(CLOCKS_PER_SEC)) )

/** This variable flags whether the counting semaphores are yet initialized or
 * not
 */
static uint8_t _time_is_init = 0;

static void _os_time_init(void);
static uint32_t _os_milli2ticks(uint32_t milli_seconds);

uint32_t OS_TICKS_PER_SECOND = 0;

static void _os_time_init(void)
{
    /*  Init the global variable that identifies the number of ticks per 
     *  second
     */
    OS_TICKS_PER_SECOND = sysconf(_SC_CLK_TCK);
    TRACE( (int)OS_TICKS_PER_SECOND, "d");

}

static uint32_t _os_milli2ticks(uint32_t milli_seconds)
{
    uint32_t num_of_ticks,tick_duration_usec ;

    _CHECK_TIME_INIT();

    if( 0 == milli_seconds ) return 0;

    tick_duration_usec = MICROS_PER_TICK;

    num_of_ticks = 
        ( (milli_seconds * 1000) + tick_duration_usec -1 ) / tick_duration_usec ;

    return(num_of_ticks) ; 

}


/* 
 * ===  FUNCTION  ======================================================================
 *         Name:  OS_uSleep
 *  Description:  Delays a task for specified amount of microseconds.
 *  Parameters:
 *      - milli_seconds:    The amount of microseconds delay
 *  Return:
 *      0  when waking up from the delay
 *      OS_STATUS_EERR    when OS call error
 * =====================================================================================
 */
int OS_uSleep(uint32_t microsecs )
{
    struct timespec rem, req;

    _CHECK_TIME_INIT();

    if( microsecs )
    {
        req.tv_sec = microsecs/1000000;
        req.tv_nsec = (microsecs%1000000)*1000;
    }
    else
    {
        req.tv_sec = 0;
        req.tv_nsec = 0;
    }

    while( nanosleep(&req, &rem) );

    return 0;

}/* end OS_uSleep */

/* 
 * ===  FUNCTION  ======================================================================
 *         Name:  OS_Sleep
 *  Description:  Delays a task for specified amount of microseconds.
 *  Parameters:
 *      - milli_seconds:    The amount of microseconds delay
 *  Return:
 *      0  when waking up from the delay
 *      OS_STATUS_EERR    when OS call error
 * =====================================================================================
 */
int OS_Sleep(uint32_t millis )
{
    _CHECK_TIME_INIT();

    return OS_uSleep(millis*1000);
}/* end OS_Sleep */


/* 
 * ===  FUNCTION  ======================================================================
 *         Name:  OS_GetLocalTime
 *  Description:  This function returns the system local time.
 *  Parameters:
 *      - time_struct:  This is the time structure where the local time will be
 *      stored.
 *  Return:
 *      0 when the call success
 *      OS_STATUS_EINVAL if the 'time_struct' parameter is not valid
 *      OS_STATUS_EERR when any other error has occurred
 * =====================================================================================
 */
int OS_GetLocalTime(OS_time_t *time_struct)
{
    _CHECK_TIME_INIT();

    /* the code that is #if 0'd out below is what we actually want to use. However 
     * clock_gettime doesn;t seem to be implemented in linux right now. The code that is
     * being used instead will return the clock of the system its in. Hopefully that will
     * be good enough for this port for now. */

    struct timeval tv;
    int ret;

    ret = gettimeofday(&tv, NULL);
    time_struct-> mul_Seconds = tv.tv_sec;
    time_struct-> mul_MicroSeconds = tv.tv_usec;

    if (ret == 0)
    {
        return 0;
    }

    os_return_minus_one_and_set_errno(OS_STATUS_EERR);

#if 0  
    int status;
    struct  timespec  time;

    if (time_struct == NULL)
        os_return_minus_one_and_set_errno(OS_STATUS_EINVAL);

    /*status = clock_gettime(CLOCK_REALTIME, &time);*/
    if (status != 0)
        os_return_minus_one_and_set_errno(OS_STATUS_EERR);

    time_struct -> mul_Seconds = time.tv_sec;
    time_struct -> microsecs = time.tv_nsec / 1000;

    return 0;

#endif

}/* end OS_GetLocalTime */

/* 
 * ===  FUNCTION  ======================================================================
 *         Name:  OS_GetTimeSinceBoot()
 *  Description:  
 * =====================================================================================
 */
int OS_GetTimeSinceBoot(OS_time_t *uptime)
{
    _CHECK_TIME_INIT();

    struct timespec t;
    int32_t ret;

    ASSERT( uptime );
    if( uptime == NULL ) os_return_minus_one_and_set_errno(OS_STATUS_EINVAL);

    ret = clock_gettime(CLOCK_REALTIME, &t);
    ASSERT( ret >= 0 );
    if( ret < 0 ) os_return_minus_one_and_set_errno(OS_STATUS_EERR);

    uptime->mul_Seconds = t.tv_sec;
    uptime->mul_MicroSeconds = t.tv_nsec/1000;

    return 0;
}

int OS_GetTicks(uint32_t *pull_UpTicks)
{
    struct timespec t;
    int32_t ret;

    _CHECK_TIME_INIT();

    ASSERT( pull_UpTicks );
    if( pull_UpTicks == NULL ) os_return_minus_one_and_set_errno(OS_STATUS_EINVAL);

    ret = clock_gettime(CLOCK_REALTIME, &t);
    ASSERT( ret >= 0 );
    if( ret < 0 ) os_return_minus_one_and_set_errno(OS_STATUS_EERR);

    *pull_UpTicks = (uint32_t)(t.tv_sec * OS_TICKS_PER_SECOND);
    *pull_UpTicks += (uint32_t)(t.tv_nsec * OS_TICKS_PER_SECOND / 1000000000);

    return 0;
}

int OS_GetTicksPerSecond(uint32_t *pul_TicksPerSecond)
{
    _CHECK_TIME_INIT();

    ASSERT(pul_TicksPerSecond);

    *pul_TicksPerSecond = OS_TICKS_PER_SECOND;

    return 0;
}


/* 
 * ===  FUNCTION  ======================================================================
 *         Name:  OS_SetLocalTime
 *  Description:  This function sets the local time in the system.
 *  Parameters:
 *      - time_struct:  this is the time structure where the local time is
 *      stored.
 *  Return:
 *      - 0 when the call success
 *      - OS_STATUS_EINVAL when the 'time_struct' is not valid.
 *      - OS_STATUS_EERR when an error occurred
 *
 * =====================================================================================
 */
int OS_SetLocalTime(OS_time_t *time_struct)
{
    struct timeval tv;
    int ret;

    _CHECK_TIME_INIT();

    if( (NULL == time_struct) )
        os_return_minus_one_and_set_errno(OS_STATUS_EINVAL);

    tv.tv_sec = time_struct -> mul_Seconds;
    tv.tv_usec = time_struct -> mul_MicroSeconds;

    ret = settimeofday(&tv, NULL);

    if (ret == 0)
    {
        return 0;
    }

    os_return_minus_one_and_set_errno(OS_STATUS_EERR);

} /*end OS_SetLocalTime */

int OS_GetTimeOfDay(OS_time_of_day_t *tod)
{
    int32_t ret;
    uint32_t upseconds;
    OS_time_t uptime;
    struct tm *t;

    _CHECK_TIME_INIT();

    /*  Sanity checks   */
    ASSERT( tod );
    if( tod == NULL )
        os_return_minus_one_and_set_errno(OS_STATUS_EINVAL);

    ret = OS_GetTimeSinceBoot( &uptime );
    ASSERT( ret >= 0 );
    if( ret < 0 ) os_return_minus_one_and_set_errno(OS_STATUS_EERR);

    upseconds = uptime.mul_Seconds;

    t = localtime((time_t*)&upseconds);
    ASSERT( t );
    tod->mul_Year = t->tm_year+1900;
    tod->mul_Month = t->tm_mon+1;
    tod->mul_Day = t->tm_mday;
    tod->mul_Hour = t->tm_hour;
    tod->mul_Minute = t->tm_min;
    tod->mul_Seconds = t->tm_sec;
    tod->mul_MicroSeconds = uptime.mul_MicroSeconds;

    return 0;
}


