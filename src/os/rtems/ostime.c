/**
 *  \file   ostime.c
 *  \brief  This file implements all the Time access API of the OSAL library
 *  for the RTEMS operating system.
 *
 *  Detailed description starts here.
 *
 *  \author  Aitor Viana Sanchez (avs), Aitor.Viana.Sanchez@esa.int
 *
 *  \internal
 *    Created:  19/02/09
 *   Revision:  $Id: ostime.c 1.4 19/02/09 avs Exp $
 *   Compiler:  gcc/g++
 *    Company:  European Space Agency (ESA-ESTEC)
 *  Copyright:  Copyright (c) 2009, Aitor Viana Sanchez
 *
 *  This source code is released for free distribution under the terms of the
 *  GNU General Public License as published by the Free Software Foundation.
 * =====================================================================================
 */
/*
 * Author : Ezra Yeheskeli
 * Purpose: 
 *         This file  contains some of the OS APIs abstraction layer 
 *         implementation for POSIX, specifically for Linux / Mac OS X.
 *
 */

#include <osal/osdebug.h>
#include <osal/osapi.h>

#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#include <rtems.h>
#include <rtems/score/cpuopts.h>
#include <rtems/score/tod.h>

#ifndef RTEMS_VERSION_LATER_THAN
#define RTEMS_VERSION_LATER_THAN(ma,mi,re) \
	(    __RTEMS_MAJOR__  > (ma)	\
	 || (__RTEMS_MAJOR__ == (ma) && __RTEMS_MINOR__  > (mi))	\
	 || (__RTEMS_MAJOR__ == (ma) && __RTEMS_MINOR__ == (mi) && __RTEMS_REVISION__ > (re)) \
    )
#endif
#ifndef RTEMS_VERSION_ATLEAST
#define RTEMS_VERSION_ATLEAST(ma,mi,re) \
	(    __RTEMS_MAJOR__  > (ma)	\
	|| (__RTEMS_MAJOR__ == (ma) && __RTEMS_MINOR__  > (mi))	\
	|| (__RTEMS_MAJOR__ == (ma) && __RTEMS_MINOR__ == (mi) && __RTEMS_REVISION__ >= (re)) \
	)
#endif

/********************************* FILE PRIVATE VARIABLES  */

#define MICROS_PER_TICK ( 1000000 / OS_TICKS_PER_SECOND )

#define YEAR    1988
#define MONTH   1
#define DAY     1

#define _IS_TIME_INIT()   \
{   \
    if( !_time_is_init ) \
    { \
        _os_time_init(); \
        _time_is_init = 1; \
    } \
}
#define _CHECK_TIME_INIT()  (_IS_TIME_INIT())

/** This variable flags whether the counting semaphores are yet initialized or
 * not
 */
static uint8_t _time_is_init = 0;

/** Establsh the time of day    */
OS_time_of_day_t OS_system_tod = 
/* YEAR     MONTH   DAY HOUR    MINUTE  SECOND  */
{  YEAR,    MONTH,  DAY,  0,        0,     0};

LOCAL uint32_t tod_seconds = 0;

/********************************* PRIVATE INTERFACE    */

extern void show_error_code (rtems_status_code return_status);
extern void timespec_to_micros(struct timespec time_spec, uint32_t *usecs);
extern void micros_to_timespec(uint32_t usecs, struct timespec *time_spec);

static uint32_t _os_milli2ticks (uint32_t milli_seconds);
static void _os_time_init(void);

static void _os_time_init(void)
{
    rtems_time_of_day rtems_tod;
    rtems_status_code rtems_status = RTEMS_NOT_CONFIGURED;

    /*  Get tick per second */
//    rtems_status = rtems_clock_get(RTEMS_CLOCK_GET_TICKS_PER_SECOND, (void*)&OS_TICKS_PER_SECOND);
//    ASSERT( RTEMS_SUCCESSFUL == rtems_status );
//    TRACE(OS_TICKS_PER_SECOND, "d");

    ASSERT( OS_TICKS_PER_SECOND > 0 );
    if( OS_TICKS_PER_SECOND == 0 )
    {
        printf("ERROR: OSAL not init\n");
        exit(0);
    }

    /*  Set the local time  */
    rtems_tod.year      = OS_system_tod.mul_Year;
    rtems_tod.month     = OS_system_tod.mul_Month;
    rtems_tod.day       = OS_system_tod.mul_Day;
    rtems_tod.hour      = OS_system_tod.mul_Hour;
    rtems_tod.minute    = OS_system_tod.mul_Minute;
    rtems_tod.second    = OS_system_tod.mul_Seconds;
    rtems_tod.ticks     = OS_system_tod.mul_MicroSeconds * OS_TICKS_PER_SECOND / 1000000;

    rtems_status = rtems_clock_set( &rtems_tod );
    ASSERT( RTEMS_SUCCESSFUL == rtems_status );

    if( RTEMS_SUCCESSFUL != rtems_status )
    {
        show_error_code(rtems_status);
        ASSERT( 0 );
        exit(-1);
    }

    tod_seconds = _TOD_To_seconds(&rtems_tod);
}

static uint32_t _os_milli2ticks (uint32_t milli_seconds)
{
    uint32_t num_of_ticks,tick_duration_usec ;

    _CHECK_TIME_INIT();

    if( 0 == milli_seconds ) return 0;

    tick_duration_usec = MICROS_PER_TICK;

    num_of_ticks = 
        ( (milli_seconds * 1000) + tick_duration_usec -1 ) / tick_duration_usec ;

    return(num_of_ticks) ; 

/*     return( (milli_seconds / 1000) * OS_TICKS_PER_SECOND );
 */


}/* end _os_milli2ticks */

/********************************* PUBLIC  INTERFACE    */

/* 
 * ===  FUNCTION  ======================================================================
 *         Name:  OS_Sleep
 *  Description:  Delays a task for specified amount of milliseconds.
 *  Parameters:
 *      - milli_seconds:    The amount of milliseconds delay
 *  Return:
 *      0  when waking up from the delay
 *      OS_STATUS_EERR    when OS call error
 * =====================================================================================
 */
int OS_Sleep (uint32_t milli_seconds)
{
    rtems_status_code status;
    rtems_interval interval = 0;

    _CHECK_TIME_INIT();

/*    interval = (milli_seconds / 1000) * OS_TICKS_PER_SECOND; */
    if( milli_seconds )
        interval = _os_milli2ticks(milli_seconds);
    else interval = 0;

    status = rtems_task_wake_after(interval);

    if( RTEMS_SUCCESSFUL == status )
        return 0;
    else
        os_return_minus_one_and_set_errno(OS_STATUS_EERR);

}/* end OS_Sleep */

/* 
 * ===  FUNCTION  ======================================================================
 *         Name:  OS_uSleep
 *  Description:  Delays a task for specified amount of mul_MicroSeconds.
 *  Parameters:
 *      - microsecs:    The amount of milliseconds delay
 *  Return:
 *      0  when waking up from the delay
 *      OS_STATUS_EERR    when OS call error
 * =====================================================================================
 */
int OS_uSleep (uint32_t microsecs)
{
    rtems_status_code status;
    rtems_interval interval = 0;

    _CHECK_TIME_INIT();

    if( microsecs )
    {
        microsecs = (microsecs > MICROS_PER_TICK) ? MICROS_PER_TICK : microsecs;
        interval = microsecs / MICROS_PER_TICK;
    }
    else 
    {
        interval = 0;
    }

    status = rtems_task_wake_after(interval);

    if( RTEMS_SUCCESSFUL == status )
        return 0;
    else
        os_return_minus_one_and_set_errno(OS_STATUS_EERR);

}/* end OS_uSleep */

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
    rtems_clock_time_value local_time;
    rtems_status_code status;

    _CHECK_TIME_INIT();

    status = rtems_clock_get(RTEMS_CLOCK_GET_TIME_VALUE, (void*)&local_time);
    ASSERT( RTEMS_SUCCESSFUL == status );

    switch(status)
    {
        case RTEMS_SUCCESSFUL:
            time_struct->mul_Seconds = local_time.seconds;
            time_struct->mul_MicroSeconds = local_time.microseconds;
            return 0;
            break;
        case RTEMS_NOT_DEFINED:
            os_return_minus_one_and_set_errno(OS_STATUS_TIME_NOT_SET);
            break;
        case RTEMS_INVALID_ADDRESS:
            os_return_minus_one_and_set_errno(OS_STATUS_EINVAL);
            break;
        default:
            os_return_minus_one_and_set_errno(OS_STATUS_EERR);
            break;
    }

    os_return_minus_one_and_set_errno(OS_STATUS_EERR);

} /* end OS_GetLocalTime */

/* 
 * ===  FUNCTION  ======================================================================
 *         Name:  OS_GetTimeSinceBoot()
 *  Description:  
 * =====================================================================================
 */
int OS_GetTimeSinceBoot(OS_time_t *t_time)
{
    rtems_status_code status;
    struct timespec uptime;

    _CHECK_TIME_INIT();

#if ( RTEMS_VERSION_LATER_THAN(4, 6, 5) )

    status = rtems_clock_get_uptime(&uptime);
#else
    rtems_interval local_time;
    status = rtems_clock_get(RTEMS_CLOCK_GET_TICKS_SINCE_BOOT, (void*)&local_time);
#endif
    ASSERT( RTEMS_SUCCESSFUL == status );
    ASSERT( t_time );

    switch(status)
    {
        case RTEMS_SUCCESSFUL:
#if ( !RTEMS_VERSION_LATER_THAN(4, 6, 5) )
            uint32_t usecs = local_time * MICROS_PER_TICK;
            micros_to_timespec(usecs, &uptime);
#endif
            t_time->mul_Seconds = uptime.tv_sec;
            t_time->mul_MicroSeconds = (uint32_t)(uptime.tv_nsec)/1000;
            return 0;
            break;
        case RTEMS_INVALID_ADDRESS:
            os_return_minus_one_and_set_errno(OS_STATUS_EINVAL);
            break;
        default:
            os_return_minus_one_and_set_errno(OS_STATUS_EERR);
            break;
    }
    os_return_minus_one_and_set_errno(OS_STATUS_EERR);
}

/* 
 * ===  FUNCTION  ======================================================================
 *         Name:  OS_GetTicks()
 *  Description:  
 * =====================================================================================
 */
int OS_GetTicks(uint32_t *pull_UpTicks)
{
    rtems_status_code status;

    _CHECK_TIME_INIT();

    ASSERT( pull_UpTicks );
    if( pull_UpTicks == NULL ) os_return_minus_one_and_set_errno(OS_STATUS_EINVAL);

    rtems_interval local_time;
    status = rtems_clock_get(RTEMS_CLOCK_GET_TICKS_SINCE_BOOT, (void*)&local_time);

    *pull_UpTicks = (uint32_t)local_time;

    switch(status)
    {
        case RTEMS_SUCCESSFUL:
            return 0;
        case RTEMS_INVALID_ADDRESS:
            os_return_minus_one_and_set_errno(OS_STATUS_EINVAL);
        default:
            os_return_minus_one_and_set_errno(OS_STATUS_EERR);
    }

    os_return_minus_one_and_set_errno(OS_STATUS_EERR);
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
 * FIXME: It is not possible to change the local time during execution in RTEMS.
 * It needs to be configured at compiling time.
 * In RTEMS always returns 0
 * =====================================================================================
 */
int OS_SetLocalTime(OS_time_t *time_struct)
{
    ASSERT(time_struct);

    _CHECK_TIME_INIT();

    return 0;

#if 0
    int status;
    struct  timespec  time;

    if (time_struct == NULL)
        os_return_minus_one_and_set_errno(OS_STATUS_EINVAL);

    time.tv_sec = time_struct -> mul_Seconds;
    time.tv_nsec = (time_struct -> microsecs) * 1000;

    status = clock_settime(CLOCK_REALTIME, &time);
    if (status != 0)
        os_return_minus_one_and_set_errno(OS_STATUS_EERR);


    return 0;
#endif
} /* end OS_SetLocalTime */

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

    upseconds = uptime.mul_Seconds + tod_seconds;

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

