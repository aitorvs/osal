/**
 *  \file   ostimer.c
 *  \brief  This file features the timer API implementation for the OSAL
 *  library under the Linux OS
 *
 *  Detailed description starts here.
 *
 *  \author  Aitor Viana Sanchez (avs), Aitor.Viana.Sanchez@esa.int
 *
 *  \internal
 *    Created:  11/25/2009
 *   Revision:  $Id: ostimer.c 1.4 11/25/2009 avs Exp $
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

#include <osal/osapi.h>
#include <osal/osstats.h>
#include <public/lock.h>
#include <osal/osdebug.h>
#include <glue/tod.h>

#include <time.h>
#include <signal.h>
#include <pthread.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <errno.h>

/****************************************************************************************
  EXTERNAL FUNCTION PROTOTYPES
 ****************************************************************************************/


/****************************************************************************************
  DEFINES
 ****************************************************************************************/

#define INIT_THREAD_MUTEX() \
    do{ \
        int ret;    \
        ret = lock_rw_init(&_rwlock);   \
        ASSERT( ret >= 0 ); \
    }while(0);


#define WLOCK()   __WLOCK()
#define WUNLOCK() __WUNLOCK()

#define CRITICAL(x) \
    WLOCK();    \
    {   \
        x;  \
    }   \
    WUNLOCK();

#define _IS_TIMER_INIT()   \
{   \
    if( !_timer_is_init ) \
    { \
        _os_timer_init(); \
        _timer_is_init = 1; \
    } \
}
#define _CHECK_TIMER_INIT()  (_IS_TIMER_INIT())

/** This variable flags whether the timer class has been yet initialized or not
 */
static uint8_t _timer_is_init = 0;

/**
 * Since the API is storing the timer values in a 32-bit integer as
 * microseconds, there is a limit in the number of seconds that van be
 * represented
 */
#define MAX_SEC_IN_USEC 4293

/*
 ** The timers use the RT Signals. The system that this code was developed
 ** and tested on has 32 available RT signals ( SIGRTMIN -> SIGRTMAX ).
 ** OS_MAX_TIMERS should not be set to more than this number.
 */
#define OS_STARTING_SIGNAL  (SIGRTMAX-1)
#define SIGTIMER            (SIGRTMAX)

/****************************************************************************************
  LOCAL TYPEDEFS 
 ****************************************************************************************/

/**
 *  \class OS_timers_record_t
 *  \brief This class maintains all the information related to the OSAL timers.
 */
typedef struct
{
    /** Free flag */
    int         free;   
    /** Resource identifier */
    timer_t     id;
    /** Resource Creator Identifier */
    int         creator;
    /** Timer callback function provided by the user    */
    OS_TSR_entry_t callback_ptr;
    void *user_param;
    uint32_t              start_time;
    uint32_t              interval_time;
    timer_t             host_timerid;
}OS_timers_record_t;

/****************************************************************************************
  INTERNAL FUNCTION PROTOTYPES
 ****************************************************************************************/

LOCAL OS_timers_record_t    OS_timer_table     [OS_MAX_TIMERS];
LOCAL uint32_t os_clock_accuracy = 0;

extern void timespec_to_micros(struct timespec time_spec, uint32_t *usecs);
extern void micros_to_timespec(uint32_t usecs, struct timespec *time_spec);


LOCAL void OS_timer_signal_handler(int signum)
{
    uint32_t timer_id;
    void *param;

    timer_id = OS_STARTING_SIGNAL - signum;

    if( (timer_id < OS_MAX_TIMERS) )
    {
        if( OS_timer_table[timer_id].free == FALSE )
        {
            param = OS_timer_table[timer_id].user_param;
            OS_timer_table[timer_id].callback_ptr(param);
        }
    }
}

static void  _os_timer_init(void)
{
    int i;
    struct timespec clock_resolution;

    STATS_INIT_TIMER();

    /*  Initialize the Timers table */
    for(i = 0; i < OS_MAX_TIMERS; i++)
    {
        OS_timer_table[i].free        = TRUE;
        OS_timer_table[i].id          = FALSE;
        OS_timer_table[i].creator     = FALSE;
    }

    /*  Get the resolution of the realtime clock    */
    if( clock_getres(CLOCK_REALTIME, &clock_resolution) < 0 )
    {
        DEBUG("Error Init the Timer Class");
        exit(-1);
    }

    /*  Converto to microseconds    */
    timespec_to_micros(clock_resolution, &os_clock_accuracy);

    /*  Round up the clock accuracy */
    os_clock_accuracy++;

    /*  Create the timer table mutex    */
    INIT_THREAD_MUTEX();
}

/*-----------------------------------------------------------------------------
 *  OS TIMER INTERFACE
 *-----------------------------------------------------------------------------*/

/* 
 * ===  FUNCTION  ======================================================================
 *         Name:  OS_TimerCreate
 *  Description:  This function creates a timer. The assigned 'timer_id' is
 *  returned when the timer is created successfuly.
 *
 *  Parameters:
 *      - timer_id: This parameter is the timer identifier to be returned when
 *      success
 *
 *  Return:
 * =====================================================================================
 */
int    OS_TimerCreate(uint32_t *timer_id)
{
    UNUSED(timer_id);
    uint32_t             possible_tid;
    int                status;
    struct  sigaction  sig_act;
    struct  sigevent   evp;

    _CHECK_TIMER_INIT();

    ASSERT(timer_id);
    if ( timer_id == NULL )
    {
        os_return_minus_one_and_set_errno(OS_STATUS_EINVAL);
    }

    /*  Check parameters    */
    WLOCK();
    {
        for(possible_tid = 0; possible_tid < OS_MAX_TIMERS; possible_tid++)
        {
            if (OS_timer_table[possible_tid].free == TRUE)
                break;
        }
        if( possible_tid >= OS_MAX_TIMERS || OS_timer_table[possible_tid].free != TRUE)
        {
            WUNLOCK();
            os_return_minus_one_and_set_errno(OS_STATUS_NO_FREE_IDS);
        }

        /* 
         * Set the possible timer Id to not free so that
         * no other task can try to use it 
         */
        OS_timer_table[possible_tid].free = FALSE;
        OS_timer_table[possible_tid].creator = FALSE;
        OS_timer_table[possible_tid].start_time = 0;
        OS_timer_table[possible_tid].interval_time = 0;
    }
    WUNLOCK();

    /*
     *  Initialize the sigaction and sigevent structures for the handler.
     */
    sigemptyset(&sig_act.sa_mask);
    sig_act.sa_flags = SA_SIGINFO;
    sig_act.sa_handler = OS_timer_signal_handler;

    /*
     ** Set the signal action for the timer
     */
    if( sigaction(OS_STARTING_SIGNAL - possible_tid, &(sig_act), 0) < 0 )
    {
        perror("sigaction failed");
        os_return_minus_one_and_set_errno(OS_STATUS_EERR);
    }

    evp.sigev_notify = SIGEV_SIGNAL; 
    evp.sigev_signo = OS_STARTING_SIGNAL - possible_tid;
    evp.sigev_value.sival_ptr = &(OS_timer_table[possible_tid].host_timerid);

    /*
     ** Create the timer
     */
    status = timer_create(
            CLOCK_REALTIME, 
            &evp, 
            (timer_t *)&(OS_timer_table[possible_tid].host_timerid));
    if (status < 0) 
    {
        CRITICAL( (OS_timer_table[possible_tid].free = TRUE) );
        os_return_minus_one_and_set_errno(OS_STATUS_EERR);
    }

    /* FIXME --  Return the clock accuracy to the user   */
    //    *clock_accuracy = os_clock_accuracy;

    /*
     ** Return timer ID 
     */
    WLOCK();
    {
        *timer_id = possible_tid;

        /*  Stats   */
        STATS_CREAT_TIMER();
    }
    WUNLOCK();

    return 0;
}

/* 
 * ===  FUNCTION  ======================================================================
 *         Name:  OS_TimerDelete
 *  Description:  This function deletes the timer specified with the identifier
 *  'timer_id'.
 *
 *  Parameter:
 *      - timer_id:     Timer identifier
 *
 *  Return:
 * =====================================================================================
 */
int    OS_TimerDelete(uint32_t timer_id)
{
    int status;

    _CHECK_TIMER_INIT();

    /* 
     ** Check to see if the timer_id given is valid 
     */
    if (timer_id >= OS_MAX_TIMERS || OS_timer_table[timer_id].free == TRUE)
    {
        os_return_minus_one_and_set_errno(OS_STATUS_EINVAL);
    }

    /*
     * Delete the timer 
     */
    status = timer_delete((timer_t)(OS_timer_table[timer_id].host_timerid));
    if (status < 0)
    {
        os_return_minus_one_and_set_errno(OS_STATUS_EERR);
    }

    WLOCK();
    {
        OS_timer_table[timer_id].free = TRUE;

        /*  Stats   */
        STATS_DEL_TIMER();
    }
    WUNLOCK();

    return 0;
}

/* 
 * ===  FUNCTION  ======================================================================
 *         Name:  OS_TimerCancel
 *  Description:  This function cancels the 'timer_id' timer. The timer will be
 *  reinitiated by the next invocation of OS_TimerFireAfter() or
 *  OS_TimerFireWhen() call.
 *
 *  Parameters:
 *      - timer_id:     Timer identifier.
 *  Return:
 * =====================================================================================
 */
int    OS_TimerCancel(uint32_t timer_id)
{
    _CHECK_TIMER_INIT();

    UNUSED(timer_id);
    os_return_minus_one_and_set_errno(OS_STATUS_NOT_SUPPORTED);
}

/* 
 * ===  FUNCTION  ======================================================================
 *         Name:  OS_TimerFireAfter
 *  Description:  This function initiates the timer specified by 'timer_id'. If
 *  the timer is running, it is canceled before being re-initiated. When the
 *  timer fires, it calls the 'routine' function.
 *
 *  Parameter:
 *      - timer_id:     Timer id
 *      - millis:       milliseconds to be fired
 *      - routine:      Function the timer calls when fires
 *  Return:
 * =====================================================================================
 */
int    OS_TimerFireAfter(
        uint32_t timer_id, 
        uint32_t millis, 
        OS_TSR_entry_t callback_ptr, 
        void *user_data)
{
    UNUSED(user_data);
    int    status;
    struct itimerspec timeout;
    uint32_t interval_time = millis * 1000;

    _CHECK_TIMER_INIT();

    /* 
     ** Check to see if the timer_id given is valid 
     */
    if (timer_id >= OS_MAX_TIMERS || OS_timer_table[timer_id].free == TRUE)
    {
        os_return_minus_one_and_set_errno(OS_STATUS_EINVAL);
    }

    /*
     ** Round up the accuracy of the start time and interval times 
     */
    //    if (( start_time > 0 ) && ( start_time < os_clock_accuracy ))
    //    {
    //        start_time = os_clock_accuracy;
    //    }

    //    if (( interval_time > 0) && ( interval_time < os_clock_accuracy ))
    //    {
    //        interval_time = os_clock_accuracy;
    //    }

    /*
     ** Save the start and interval times 
     */
    WLOCK();
    {
        OS_timer_table[timer_id].start_time = 0;    // means NOW
        OS_timer_table[timer_id].interval_time = interval_time;  // micros

        /*  Stablish the callback function  */
        OS_timer_table[timer_id].callback_ptr = callback_ptr;
        OS_timer_table[timer_id].user_param = user_data;
    }
    WUNLOCK();

    /*
     ** Convert from Microseconds to timespec structures
     */
    micros_to_timespec(interval_time, &(timeout.it_value));
    micros_to_timespec(0, &(timeout.it_interval));

    /*
     ** Program the real timer
     */
    status = timer_settime((timer_t)(OS_timer_table[timer_id].host_timerid), 
            0,              /* Flags field can be zero */
            &timeout,       /* struct itimerspec */
            NULL);         /* Oldvalue */
    if (status < 0) 
    {
        perror("timer_settime failed");
        os_return_minus_one_and_set_errno(OS_STATUS_EERR);
    }

    return 0;

}

/* 
 * ===  FUNCTION  ======================================================================
 *         Name:  OS_TimerFireAfter
 *  Description:  This function initiates the timer specified by 'timer_id'. If
 *  the timer is running, it is canceled before being re-initiated. The timer
 *  is schedule to fire at when_time. when timer fires, it calls the 'routine' function.
 *
 *  Parameter:
 *      - timer_id:     Timer id
 *      - when_time:    Time when the timer will be fired
 *      - routine:      Function the timer calls when fires
 *  Return:
 * =====================================================================================
 */
int    OS_TimerFireWhen(
        uint32_t timer_id, 
        const OS_time_of_day_t *when_time, 
        OS_TSR_entry_t routine, 
        void *user_data)
{
    uint32_t atseconds, todseconds;
    OS_time_of_day_t tod, tmp;

    _CHECK_TIMER_INIT();

    /*  Sanity checks   */
    ASSERT( when_time );
    if( when_time == NULL )
        os_return_minus_one_and_set_errno(OS_STATUS_EINVAL);

    if (timer_id >= OS_MAX_TIMERS || OS_timer_table[timer_id].free == TRUE)
        os_return_minus_one_and_set_errno(OS_STATUS_EINVAL);

    memcpy((void*)&tmp, when_time, sizeof(OS_time_of_day_t));
    tmp.mul_Year -= TOD_BASE_YEAR;
    atseconds = OS_tod_to_seconds(&tmp);

    OS_GetTimeOfDay(&tod);
    tod.mul_Year -= TOD_BASE_YEAR;
    todseconds = OS_tod_to_seconds(&tod);

    return OS_TimerFireAfter(timer_id, (atseconds-todseconds)*1000, routine, user_data);

}

int    OS_TimerReset(uint32_t timer_id)
{
    int status;
    uint32_t interval_time, start_time;
    struct itimerspec timeout;

    _CHECK_TIMER_INIT();

    interval_time = OS_timer_table[timer_id].interval_time;
    start_time = OS_timer_table[timer_id].start_time;

    micros_to_timespec(interval_time, &(timeout.it_value));
    micros_to_timespec(start_time, &(timeout.it_interval));
    /*
     ** Program the real timer
     */
    status = timer_settime((timer_t)(OS_timer_table[timer_id].host_timerid), 
            0,              /* Flags field can be zero */
            &timeout,       /* struct itimerspec */
            NULL);         /* Oldvalue */
    if (status < 0) 
    {
        os_return_minus_one_and_set_errno(OS_STATUS_EERR);
    }

    return 0;
}


