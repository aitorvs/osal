/**
 *  \file   ostimer.c
 *  \brief  This file implements the Timer API of the OSAL library for the
 *  RTEMS operating system
 *
 *  Detailed description starts here.
 *
 *  \author  Aitor Viana Sanchez (avs), Aitor.Viana.Sanchez@esa.int
 *
 *  \internal
 *    Created:  19/02/09
 *   Revision:  $Id: ostimer.c 1.4 19/02/09 avs Exp $
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
#include <osal/osstats.h>
#include <public/lock.h>
#include <public/list.h>

#include <stdlib.h>
#include <stdio.h>
#include <assert.h>
#include <rtems.h>
#include <rtems/score/tod.h>

/* 
 * ===  MACRO  ======================================================================
 *         Name:  NEXT_RESOURCE_NAME
 *  Description:  This MACRO gets the next resource name which will be used
 *  during the resorce creation process.
 * =====================================================================================
 */
#define NEXT_RESOURCE_NAME(c1, c2, c3, c4)  \
                 if (c4 == '9') {       \
                   if (c3 == '9') {     \
                     if (c2 == 'z') {   \
                       if (c1 == 'z') { \
                         printf("not enough task letters for names !!!\n"); \
                         exit( 1 );     \
                       } else           \
                         c1++;          \
                       c2 = 'a';        \
                     } else             \
                      c2++;             \
                     c3 = '0';          \
                   } else               \
                     c3++;              \
                   c4 = '0';            \
                 }                      \
                 else                   \
                   c4++                 \

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

#define USE_TIMER_SERVER

#if defined( USE_TIMER_SERVER )
#warning "Timers implemented as timer-Server"
#endif

/** This variable flags whether the timer class has been yet initialized or not
 */
static uint8_t _timer_is_init = 0;

/********************************* FILE CLASSES/STRUCTURES */

typedef void(*pfunc)(void*);
/**
 *  \class OS_timers_record_t
 *  \brief This class maintains all the information related to the OSAL timers.
 */
typedef struct s_timer_record
{
    struct s_list_head node;

    int         free; /**< \brief Free flag */
    rtems_id    id; /**< \brief Resource identifier */
    int         creator; /**< \brief Resource Creator Identifier */
    pfunc       timer_func;
    void        *user_data;

}OS_timers_record_t;


/********************************* FILE PRIVATE VARIABLES  */

/** this is the Initial name for the timers  */
LOCAL char ntimers_name[] = "0000";
LOCAL OS_timers_record_t    OS_timer_table     [OS_MAX_TIMERS];

/********************************* PRIVATE INTERFACE    */

LIST_HEAD(gtimer_list);
static uint32_t server_sem;

static void _timer_handle(rtems_id ignored, void *userdata)
{
    OS_timers_record_t *timer = (OS_timers_record_t*)userdata;
    list_add_tail(&timer->node, &gtimer_list);
    rtems_semaphore_release(server_sem);
}

static rtems_task _timer_server(rtems_task_argument arg)
{
    while(1)
    {
        rtems_semaphore_obtain(server_sem, RTEMS_WAIT, RTEMS_NO_TIMEOUT);

        OS_timers_record_t *timer = NULL;
        timer = list_first_entry(&gtimer_list, struct s_timer_record, node);
        if( timer != NULL )
        {
            list_del(&timer->node);
            timer->timer_func(timer->user_data);
        }
    }
}

static int _rtems_timer_intiate_server(void)
{
    rtems_status_code ret;
    uint32_t id;


    ret = rtems_task_create(
            rtems_build_name('S', 'E', 'R', 'V'),
            1,
            4*1024,
            RTEMS_PREEMPT,
            RTEMS_LOCAL|RTEMS_FLOATING_POINT|RTEMS_SYSTEM_TASK,
            &id);
    if( ret != RTEMS_SUCCESSFUL ) goto err;

    ret = rtems_semaphore_create( 
            rtems_build_name('S', 'E', 'M', 'V'),
            0, 
            0,
            RTEMS_COUNTING_SEMAPHORE,
            &server_sem);
    if( ret != RTEMS_SUCCESSFUL ) goto err;

    ret = rtems_task_start (id, _timer_server, (rtems_task_argument)NULL);
    if( ret != RTEMS_SUCCESSFUL ) goto err;


    return 0;
err:
    return -1;
}

static void _os_timer_init(void)
{
    int i;

    STATS_INIT_TIMER();

    /*  Initialize the Timers table */
    for(i = 0; i < OS_MAX_TIMERS; i++)
    {
        OS_timer_table[i].free        = TRUE;
        OS_timer_table[i].id          = UNINITIALIZED;
        OS_timer_table[i].creator     = UNINITIALIZED;
        OS_timer_table[i].timer_func = NULL;
        OS_timer_table[i].user_data = NULL;
    }

#if defined(USE_TIMER_SERVER)
//    if( rtems_timer_initiate_server(RTEMS_TIMER_SERVER_DEFAULT_PRIORITY, 4*1024, RTEMS_DEFAULT_ATTRIBUTES) != RTEMS_SUCCESSFUL )
    if( _rtems_timer_intiate_server() < 0 )
        printf("Error initiating the SERVER TIMER\n");
    else
        printf("Timer Server initiated successfully\n");
#endif

    INIT_THREAD_MUTEX();
}


/********************************* PUBLIC  INTERFACE    */

/*-----------------------------------------------------------------------------
 *  OS TIMER INTERFACE
 *-----------------------------------------------------------------------------*/


/* 
 * ===  FUNCTION  ======================================================================
 *         Name:  OS_TimerCreate
 *  Description:  
 *  This function creates a timer. The assigned 'timer_id' is
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

    rtems_status_code return_code = 0;
    rtems_name r_name;
    uint32_t possible_timerid;

    _CHECK_TIMER_INIT();

    if (timer_id == NULL )
        os_return_minus_one_and_set_errno(OS_STATUS_EINVAL);

    WLOCK();
    {
        for (possible_timerid = 0; possible_timerid < OS_MAX_TIMERS; possible_timerid++)
        {
            if (OS_timer_table[possible_timerid].free == TRUE)    
                break;
        }

        if((possible_timerid >= OS_MAX_TIMERS) ||  
                (OS_timer_table[possible_timerid].free != TRUE))
        {
            WUNLOCK();
            os_return_minus_one_and_set_errno(OS_STATUS_NO_FREE_IDS);
        }

        OS_timer_table[possible_timerid].free = FALSE;
    }
    WUNLOCK();

    /* Create RTEMS Semaphore */
    NEXT_RESOURCE_NAME(ntimers_name[0],ntimers_name[1],ntimers_name[2],ntimers_name[3]);
    r_name = rtems_build_name(ntimers_name[0],ntimers_name[1],ntimers_name[2],ntimers_name[3]);
    return_code = rtems_timer_create(r_name, &(OS_timer_table[possible_timerid].id));

    /* check if Create failed */
    if ( return_code != RTEMS_SUCCESSFUL )
    {
        WLOCK();
        {
            OS_timer_table[possible_timerid].free = TRUE;
        }
        WUNLOCK();

        os_return_minus_one_and_set_errno(OS_STATUS_TIMER_FAILURE);
    }
    /* Set the sem_id to the one that we found available */
    /* Set the name of the semaphore,creator and free as well */

    *timer_id = possible_timerid;

    WLOCK();
    {
        OS_timer_table[*timer_id].free = FALSE;
        OS_timer_table[*timer_id].creator = OS_TaskGetId();

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
    _CHECK_TIMER_INIT();

#if defined (CONFIG_OS_STATIC_RESOURCE_ALLOCATION)
    os_return_minus_one_and_set_errno(OS_STATUS_EERR);
#else
    /* Check to see if this timer_id is valid */
    if (timer_id >= OS_MAX_TIMERS || OS_timer_table[timer_id].free == TRUE)
        os_return_minus_one_and_set_errno(OS_STATUS_EINVAL);

    if (rtems_timer_delete(OS_timer_table[timer_id].id) != RTEMS_SUCCESSFUL) 
    {
        os_return_minus_one_and_set_errno(OS_STATUS_TIMER_FAILURE);
    }

    /* Remove the Id from the table, and its name, so that it cannot be found again */
    WLOCK();
    {
        OS_timer_table[timer_id].free = TRUE;
        OS_timer_table[timer_id].creator = UNINITIALIZED;
        OS_timer_table[timer_id].id = UNINITIALIZED;
        OS_timer_table[timer_id].timer_func = NULL;
        OS_timer_table[timer_id].user_data = NULL;

        /*  Stats   */
        STATS_DEL_TIMER();
    }
    WUNLOCK();

    return 0;
#endif
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

#if defined (CONFIG_OS_STATIC_RESOURCE_ALLOCATION)
    os_return_minus_one_and_set_errno(OS_STATUS_EERR);
#else
    /* Check to see if this timer_id is valid */
    if (timer_id >= OS_MAX_TIMERS || OS_timer_table[timer_id].free == TRUE)
        os_return_minus_one_and_set_errno(OS_STATUS_EINVAL);

    if (rtems_timer_cancel(OS_timer_table[timer_id].id) != RTEMS_SUCCESSFUL) 
    {
        os_return_minus_one_and_set_errno(OS_STATUS_TIMER_FAILURE);
    }

    return 0;
#endif
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
int    OS_TimerFireAfter
(
        uint32_t timer_id, 
        uint32_t millis, 
        OS_TSR_entry_t routine, 
        void *user_data
)
{
    uint32_t ticks = 0;
    rtems_status_code rtems_status;

    _CHECK_TIMER_INIT();

    ticks = millis*OS_TICKS_PER_SECOND/1000;
    ticks = (ticks < 10) ? 10 : ticks;
    OS_timer_table[timer_id].timer_func = routine, 
    OS_timer_table[timer_id].user_data = user_data;
//#if defined( USE_TIMER_SERVER )
//    rtems_status = rtems_timer_server_fire_after
//#else
    rtems_status = rtems_timer_fire_after
//#endif
        (
            OS_timer_table[timer_id].id, 
            ticks,
            _timer_handle,
            (void*)&OS_timer_table[timer_id]
            );

    switch(rtems_status)
    {
        case RTEMS_SUCCESSFUL:  return 0;
        case RTEMS_INVALID_ADDRESS: os_return_minus_one_and_set_errno(OS_STATUS_EINVAL);
        case RTEMS_INVALID_ID:  os_return_minus_one_and_set_errno(OS_STATUS_EINVAL);
        case RTEMS_INVALID_NUMBER:  os_return_minus_one_and_set_errno(OS_STATUS_EERR);
        default: os_return_minus_one_and_set_errno(OS_STATUS_EERR);
    }

    os_return_minus_one_and_set_errno(OS_STATUS_EERR);
}

/* 
 * ===  FUNCTION  =============================================================
 *         Name:  OS_TimerFireWhen
 *  Description:  
 * ============================================================================
 */

int    OS_TimerFireWhen
(
        uint32_t timer_id, 
        const OS_time_of_day_t *when_time, 
        OS_TSR_entry_t routine, 
        void *user_data
)
{
    rtems_status_code rtems_status;
    rtems_time_of_day rtems_tod;
    int ret;

    _CHECK_TIMER_INIT();

    /*  Sanity checks   */
    ASSERT( when_time );
    if( when_time == NULL )
        os_return_minus_one_and_set_errno(OS_STATUS_EINVAL);

    if (timer_id >= OS_MAX_TIMERS || OS_timer_table[timer_id].free == TRUE)
        os_return_minus_one_and_set_errno(OS_STATUS_EINVAL);


    rtems_tod.year      = when_time->mul_Year;
    rtems_tod.month     = when_time->mul_Month;
    rtems_tod.day       = when_time->mul_Day;
    rtems_tod.hour      = when_time->mul_Hour;
    rtems_tod.minute    = when_time->mul_Minute;
    rtems_tod.second    = when_time->mul_Seconds;
    rtems_tod.ticks     = (when_time->mul_MicroSeconds) * OS_TICKS_PER_SECOND/1000000;

    /*  Sanity checks   */
    ret = _TOD_Validate( &rtems_tod );
    if( !ret )
    {
        os_return_minus_one_and_set_errno(OS_STATUS_EERR);
    }

    OS_timer_table[timer_id].timer_func = routine, 
    OS_timer_table[timer_id].user_data = user_data;

//#if defined( USE_TIMER_SERVER )
//    rtems_status = rtems_timer_server_fire_when
//#else
    rtems_status = rtems_timer_fire_when
//#endif
        (
            OS_timer_table[timer_id].id,
            &rtems_tod,
            _timer_handle,
            (void*)&OS_timer_table[timer_id]
            );

    switch(rtems_status)
    {
        case RTEMS_SUCCESSFUL:  return 0;
        case RTEMS_INVALID_ADDRESS: os_return_minus_one_and_set_errno(OS_STATUS_EINVAL);
        case RTEMS_INVALID_ID:  os_return_minus_one_and_set_errno(OS_STATUS_EINVAL);
        case RTEMS_INVALID_NUMBER:  os_return_minus_one_and_set_errno(OS_STATUS_EERR);
        case RTEMS_NOT_DEFINED:  os_return_minus_one_and_set_errno(OS_STATUS_EERR);
        case RTEMS_INVALID_CLOCK:  os_return_minus_one_and_set_errno(OS_STATUS_EERR);
        default: os_return_minus_one_and_set_errno(OS_STATUS_EERR);
    }

    os_return_minus_one_and_set_errno(OS_STATUS_EERR);
}

/* 
 * ===  FUNCTION  ======================================================================
 *         Name:  OS_TimerReset
 *  Description:  This function resets the timer 'timer_id'. This timer must
 *  have been initiated before calling either OS_TimerFireAfter or
 *  OS_TimerFireWhen functions.
 *  If active, the timer is cancelling and afeter, is re-armed using the same
 *  interval and timer service routine used by the original OS_TimerFireWhen()
 *  or OS_TimerFireAfter() calls.
 *
 *  Parameter:
 *      - timer_id:     Timer id
 *
 *  Return:
 * =====================================================================================
 */
int    OS_TimerReset(uint32_t timer_id)
{
    rtems_status_code rtems_status;

    _CHECK_TIMER_INIT();

    assert( timer_id <= OS_MAX_TIMERS );
    assert( OS_timer_table[timer_id].free == FALSE );
    if( OS_timer_table[timer_id].free == TRUE )
        os_return_minus_one_and_set_errno(OS_STATUS_EERR);

    rtems_status = rtems_timer_reset(OS_timer_table[timer_id].id);

    switch(rtems_status)
    {
        case RTEMS_SUCCESSFUL:  return 0;
        case RTEMS_INVALID_ID:  os_return_minus_one_and_set_errno(OS_STATUS_EINVAL);
        default: os_return_minus_one_and_set_errno(OS_STATUS_EERR);
    }

    os_return_minus_one_and_set_errno(OS_STATUS_EERR);
}





