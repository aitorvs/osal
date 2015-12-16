/**
 *  \file   ostask.c
 *  \brief  This file implements the task API for the Linux operating system.
 *
 *  The source file implements all the task related primitives for the OSAL
 *  library over Linux.
 *
 *  \author  Aitor Viana Sanchez (avs), Aitor.Viana.Sanchez@esa.int
 *
 *  \internal
 *    Created:  07/30/2009
 *   Revision:  $Id: ostask.c 1.4 07/30/2009 avs Exp $
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

#include <pthread.h>
#include <errno.h>
#include <string.h>
#include "linconfig.h"

#define INIT_THREAD_MUTEX() \
    do{ \
        int _ret;    \
        _ret = lock_rw_init(&_rwlock);   \
        ASSERT( _ret >= 0 ); \
    }while(0);


#define WLOCK()   __WLOCK()
#define WUNLOCK() __WUNLOCK()

#ifndef PTHREAD_STACK_MIN
#define PTHREAD_STACK_MIN (20*1024)
#endif

#define THREAD_HOOK_FUNCTION

extern void timespec_add_us(struct timespec *t, uint64_t us);
extern int32_t timespec_cmp(struct timespec *a, struct timespec *b);

/********************************* FILE CLASSES/STRUCTURES */

/*  Periodic tasks structure    */
typedef void(*periodic_action_t)(void*);

struct task_hook_param
{
    void (*task_function)(void*);
    void *arg;
    uint32_t mul_Priority;
    uint32_t taskid;
};


struct periodic_task_info
{
    periodic_action_t pfunc;
    periodic_action_t perr;
    uint32_t index;
    uint64_t period_us;
    uint32_t wcet;
    uint32_t free;
    void *arg;
};

/*tasks */
typedef struct
{
    int l_errno;
    int free;
    pthread_t id;
    int mul_Creator;
    uint32_t mul_StackSize;
    uint32_t mul_Priority;
    uint32_t is_monotonic;
    pthread_mutex_t suspend_mutex;
    pthread_cond_t  suspend_cond;
    struct periodic_task_info *info_periodic;
    struct task_hook_param hook_param;
}OS_task_record_t;

/********************************* FILE PRIVATE VARIABLES  */

LOCAL pthread_key_t    thread_key;
LOCAL OS_task_record_t    OS_task_table          [OS_MAX_TASKS];
LOCAL struct periodic_task_info os_monotonic_task_table[OS_MAX_MONOTONIC_TASKS];

/** Mutex to be used in the conditional variable to sync the startup of all
 * tasks
 */
LOCAL pthread_mutex_t m_var = PTHREAD_MUTEX_INITIALIZER;
/** Condition variable to be used in the conditional variable to sync the 
 * startup of all tasks
 */
LOCAL pthread_cond_t c_var = PTHREAD_COND_INITIALIZER;
LOCAL uint32_t task_startup_cond = 0;

/************************************** PRIVATE INTERFACE  */

int *__os_errno_addr(void)
{
    pthread_t    pthread_id;
    static OS_STATUS_T _errno;
    int *v = &_errno;
    int i;

    pthread_id = pthread_self();
    for (i = 0; i < OS_MAX_TASKS; i++)
    {
        if (pthread_equal(pthread_id, OS_task_table[i].id) != 0 )
        {
            v = &OS_task_table[i].l_errno;
            break;
        }
    }

    return v;
}


/* 
 * ===  FUNCTION  ======================================================================
 *         Name:  task_init_hook
 *  Description:  This function is executed before executing the user thread
 *  function. This allows to perfom some initialization if needed.
 *  Parameters:
 *      hook_param This parameter contains some thread information like the
 *      ul_Priority, the threadid, the thread_function and the argument to be
 *      passed to the user thread_function.
 * =====================================================================================
 */
static void task_init_hook(struct task_hook_param *hook_param)
{
//    struct sched_param thread_param;
//    int ret;
//    int policy;
//    pthread_getschedparam( pthread_self(), &policy, &thread_param );
//    TRACE( policy, "d" );
//    TRACE( thread_param.sched_priority, "d" );
//    thread_param.sched_priority = hook_param->mul_Priority;
//    ret = pthread_setschedparam(pthread_self(), OS_TASK_SCHED_POLICY, &thread_param);
//    ASSERT( ret == 0 );

    /*  FIXME -- perform some actions if needed */
    pthread_mutex_lock(&m_var);
    while( !task_startup_cond )
    {
        pthread_cond_wait(&c_var, &m_var);
    }
    pthread_mutex_unlock(&m_var);

    /*  Call the user thread function   */
    hook_param->task_function(hook_param->arg);
}


/********************************* PUBLIC  INTERFACE    */

int OS_TaskJoin(void)
{
    int i;


    DEBUG("Idle Thread started");

    /*  By clearing this variable we sync all the task execution. See ostask.c
     */
    pthread_mutex_lock(&m_var);
    task_startup_cond = 1;
    pthread_cond_broadcast(&c_var);
    pthread_mutex_unlock(&m_var);

    for(i = 0; i < OS_MAX_TASKS; i++)
    {
        if( OS_task_table[i].free == FALSE )
        {
            pthread_join(OS_task_table[i].id, NULL );
        }
    }

    DEBUG("Idle Thread terminated");

    return 0;
}

int OS_TaskInit(void)
{
    int i;
    int ret;

    STATS_INIT_TASK();

    /* Initialize Task Table */

    for(i = 0; i < OS_MAX_TASKS; i++)
    {
        OS_task_table[i].l_errno = 0;
        OS_task_table[i].free        = TRUE;
        OS_task_table[i].mul_Creator     = UNINITIALIZED;
        OS_task_table[i].is_monotonic = FALSE;

        pthread_cond_init( 
                &OS_task_table[i].suspend_cond,
                NULL);

        pthread_mutex_init(
                &OS_task_table[i].suspend_mutex,
                NULL); 
    }

    for(i = 0; i < OS_MAX_MONOTONIC_TASKS; i++)
    {
        os_monotonic_task_table[i].free = TRUE;
        os_monotonic_task_table[i].pfunc = NULL;
        os_monotonic_task_table[i].perr = NULL;
    }

    ret = pthread_key_create(&thread_key, NULL );
    if ( ret != 0 )
    {
        DEBUG("Error creating thread key\n");
        os_return_minus_one_and_set_errno(OS_STATUS_EERR);
    }


    INIT_THREAD_MUTEX();

    return 0;

}

/*
 **********************************************************************************
 **          TASK API
 **********************************************************************************
 */

/**
 *  \brief This function implements the periodic task in OSAL.
 *
 *  The function suspends a thread for the remaining period after executing the
 *  thread activities. The function is also able to detect if the period
 *  (deadline) has been missed, in such a case calls the \ref perr() function
 *  if this has been previously register by the user. Otherwise, the periodic
 *  thread ends without any notification.
 *
 *  \param  arg This argument is the \ref periodic_task_info structure which
 *  contains all the information related to the periodic tasks.
 *
 *  \return none
 */
static void periodic_task(void *arg)
{
    struct periodic_task_info *ps = (struct periodic_task_info*)arg;
    int32_t status;
    struct timespec next;
    struct timespec now;

    ASSERT( ps->pfunc );
    if( ps->pfunc == NULL )
        goto err;

    clock_gettime(CLOCK_REALTIME, &next);

    while(1)
    {
        clock_gettime(CLOCK_REALTIME, &now);
        timespec_add_us(&next, ps->period_us);

        if( (timespec_cmp(&now, &next) > 0) )
        {
            DEBUG("Deadline miss");
            goto err;
        }

        clock_nanosleep(CLOCK_REALTIME, TIMER_ABSTIME, &next, NULL);

        ps->pfunc(ps->arg);
    }

err:
    if( ps->perr )
        ps->perr((void*)&status);
    else 
        DEBUG("No perr() callback");
    
    OS_TaskExit();
}

int OS_TaskMonotonicCreate(
        uint32_t *task_id,
        const void *pfunc,
        const void *perr_handler,
        uint32_t stack_size,
        uint32_t ul_Priority,
        uint32_t flags,
        void *arg,
        uint32_t ms_period
        )
{
    int32_t ret;
    uint32_t n_periods;
    struct periodic_task_info *ps;

    /*  Sanity checks   */
    ASSERT( pfunc != NULL );
    if( pfunc == NULL )
        os_return_minus_one_and_set_errno(OS_STATUS_EINVAL);

    WLOCK(); 
    {
        for(n_periods = 0; n_periods < OS_MAX_MONOTONIC_TASKS; n_periods++)
        {
            if (os_monotonic_task_table[n_periods].free == TRUE)
            {
                break;
            }
        }

        /* Check to see if the id is out of bounds */
        if(     n_periods >= OS_MAX_MONOTONIC_TASKS || 
                os_monotonic_task_table[n_periods].free != TRUE
          )
        {    
            WUNLOCK();
            os_return_minus_one_and_set_errno(OS_STATUS_NO_FREE_IDS);
        }

        /* Set the possible task Id to not free so that
         * no other task can try to use it */
        os_monotonic_task_table[n_periods].free  = FALSE;
    }
    WUNLOCK();

    ps = &os_monotonic_task_table[n_periods];

    ps->pfunc = pfunc;

    ps->arg = arg;
    if( perr_handler ) 
        ps->perr = perr_handler;
    else ps->perr = NULL;

    ps->period_us = ms_period * 1000;


    ret = OS_TaskCreate(
            task_id, 
            (void*)periodic_task,
            stack_size,
            ul_Priority,
            flags|OS_IS_PERIODIC,
            (void*)ps);

    ASSERT( ret == 0 );
    if( ret != 0 )
    {
        WLOCK(); 
        {
            os_monotonic_task_table[n_periods].free  = TRUE;
        }
        WUNLOCK();
    }


    return ret;
}

/* 
 * ===  FUNCTION  ======================================================================
 *         Name:  OS_TaskCreate
 *  Description:  This function creates a task.
 *  Paramteres:
 *      - task_id:          The task identifier will be returned in this
 *      parameter
 *      - task_name:        Task name must be NULL
 *      - function_pointer: The task function.
 *      - stack_size:       Size of the provided stack.
 *      - ul_Priority:         Task ul_Priority, needs to be lower than MAX_PRIORITY.
 *      - flags:            OS_FP_ENABLED if the task to be created is floating
 *      point task. OS_FP_DISABLED if the task to be created is non-FP task. By
 *      default, FP tasks are created.
 *  Return:
 *      0              when the task is created successfuly
 *      OS_STATUS_EINVAL      when any of the necessary pointers are NULL
 *      OS_STATUS_EINVAL when the provided ul_Priority value is not correct
 *      OS_STATUS_NO_FREE_IDS      when it is not possible to create more tasks
 *      OS_STATUS_EERR                when OS call error
 * =====================================================================================
 */
int OS_TaskCreate (
        uint32_t *task_id, 
        const void *function_pointer,
        uint32_t stack_size, 
        uint32_t ul_Priority,
        uint32_t flags, 
        void* arg)
{
    UNUSED(flags);
    int                return_code = 0;
    pthread_attr_t     attr ;
    struct sched_param thread_param ;
    int                possible_taskid;
    int             local_stack_size;
    int                ret;
    uint32_t thread_prio=0;

    /* we don't want to allow names too long*/
    /* if truncated, two names might be the same */

    /* Check for NULL pointers */

    if( (function_pointer == NULL) || (task_id == NULL) )
        os_return_minus_one_and_set_errno(OS_STATUS_EINVAL);

    /* Check for bad ul_Priority */

    if ( (ul_Priority > OS_TASK_MAX_PRIORITY) || (ul_Priority < OS_TASK_MIN_PRIORITY) )
    {
        TRACE(ul_Priority, "d");
        os_return_minus_one_and_set_errno(OS_STATUS_EINVAL);
    }


    /* Check Parameters */


    WLOCK(); 
    {
        for(possible_taskid = 0; possible_taskid < OS_MAX_TASKS; possible_taskid++)
        {
            if (OS_task_table[possible_taskid].free == TRUE)
            {
                break;
            }
        }


        /* Check to see if the id is out of bounds */

        if( possible_taskid >= OS_MAX_TASKS || OS_task_table[possible_taskid].free != TRUE)
        {
            WUNLOCK();
            os_return_minus_one_and_set_errno(OS_STATUS_NO_FREE_IDS);
        }

        /* Set the possible task Id to not free so that
         * no other task can try to use it */
        OS_task_table[possible_taskid].free = FALSE;
    }
    WUNLOCK();


    if ( stack_size < PTHREAD_STACK_MIN )
        local_stack_size = PTHREAD_STACK_MIN;
    else
        local_stack_size = (int)stack_size;


    ret=pthread_attr_init(&attr);
    ASSERT( ret == 0 );
    if(ret != 0)
    {
        TRACE( ret, "d" );
        goto err;
    }

    ret=pthread_attr_setinheritsched(&attr, PTHREAD_EXPLICIT_SCHED);
    ASSERT( ret == 0 );
    if(ret != 0)
    {
        TRACE( ret, "d" );
        goto err;
    }

    ret=pthread_attr_setschedpolicy(&attr, OS_TASK_SCHED_POLICY);
    ASSERT( ret == 0 );
    if(ret != 0)
    {
        TRACE( ret, "d" );
        goto err;
    }

    memset(&thread_param, 0, sizeof(thread_param));
    if( ul_Priority > sched_get_priority_max( OS_TASK_SCHED_POLICY ) )
    {
        thread_param.sched_priority = sched_get_priority_max( OS_TASK_SCHED_POLICY );
        DEBUG("Priority down to %d", thread_param.sched_priority);
    }
    else if( ul_Priority < sched_get_priority_min( OS_TASK_SCHED_POLICY ) )
    {
        thread_param.sched_priority = sched_get_priority_min( OS_TASK_SCHED_POLICY );
        DEBUG("Priority up to %d", thread_param.sched_priority);
    }
    else
        thread_param.sched_priority = ul_Priority;

    /*  Invert the ul_Priority as lower values represent higher priorities in OSAL
     *  but is conversely in LINUX
     */
    thread_param.sched_priority = 
        sched_get_priority_max(OS_TASK_SCHED_POLICY) - thread_param.sched_priority + 1;
    TRACE(thread_param.sched_priority, "d");
    ret=pthread_attr_setschedparam(&attr, &thread_param);
    ASSERT( ret == 0 );
    if(ret != 0)
    {
        TRACE( ret, "d" );
        goto err;
    }

//    ret = pthread_attr_setdetachstate( &attr, PTHREAD_CREATE_JOINABLE );
//    ASSERT( ret == 0 );
//    if(ret != 0)
//    {
//        TRACE( ret, "d" );
//        goto err;
//    }

    ret = pthread_attr_setstacksize(&attr, local_stack_size );
    ASSERT( ret == 0 );
    if( ret != 0 )
    {
        DEBUG("pthread_attr_setstacksize error in OS_TaskCreate, Task ID = %d\n", possible_taskid);
        TRACE( ret, "d" );
        goto err;
    }

    /*
     ** Create thread
     */
#ifndef THREAD_HOOK_FUNCTION
    return_code = pthread_create(&(OS_task_table[possible_taskid].id),
            &attr,
            function_pointer,
            arg);
#else
    OS_task_table[possible_taskid].hook_param.mul_Priority = thread_prio;
    OS_task_table[possible_taskid].hook_param.task_function = function_pointer;
    OS_task_table[possible_taskid].hook_param.arg = arg;
    OS_task_table[possible_taskid].hook_param.taskid = possible_taskid;
    return_code = pthread_create(&(OS_task_table[possible_taskid].id),
            &attr,
            (void*)task_init_hook,
            (void*)&OS_task_table[possible_taskid].hook_param);
#endif
    if (return_code != 0)
    {
        TRACE(return_code, "d");
        TRACE(errno, "d");
        TRACE(SCHED_FIFO, "d");
        DEBUG("(%d) - pthread_create error in OS_TaskCreate, Task ID = %d\n", errno, possible_taskid);
        goto err;
    }

    pthread_attr_destroy(&attr);

    *task_id = possible_taskid;

    /* this Id no longer free */

    WLOCK(); 
    {
        OS_task_table[possible_taskid].l_errno = 0;
        OS_task_table[possible_taskid].mul_Creator = OS_TaskGetId();
        OS_task_table[possible_taskid].mul_StackSize = stack_size;
        OS_task_table[possible_taskid].mul_Priority = thread_prio;
        if( flags & OS_IS_PERIODIC )
        {
            DEBUG("Periodic task");
            OS_task_table[possible_taskid].is_monotonic  = TRUE;
            OS_task_table[possible_taskid].info_periodic  = (struct periodic_task_info*)arg;
        }
        else
        {
            DEBUG("Regular task");
        }

        /*  Statistics  */
        STATS_CREAT_TASK();
    }
    WUNLOCK();

    return 0;

    /*  Error conditions    */
err:
    WLOCK(); 
    {
        OS_task_table[possible_taskid].free = TRUE;
    }
    WUNLOCK(); 

    os_return_minus_one_and_set_errno(OS_STATUS_EERR);

}/* end OS_TaskCreate */

int OS_TaskSuspend(uint32_t task_id)
{
    int32_t ret;

    if( task_id == OS_SELF )
    {
        int32_t me = OS_TaskGetId();
        ASSERT( me >= 0 );
        if( me < 0 ) os_return_minus_one_and_set_errno(OS_STATUS_EERR);

        ret = pthread_mutex_lock( &OS_task_table[me].suspend_mutex); 
        ret = pthread_cond_wait(
                &OS_task_table[me].suspend_cond,
                &OS_task_table[me].suspend_mutex); 
        ret = pthread_mutex_unlock( &OS_task_table[me].suspend_mutex); 

    }
    else
    {
        os_return_minus_one_and_set_errno(OS_STATUS_NOT_SUPPORTED);
    }

    ASSERT( ret == 0 );
    if( ret ) os_return_minus_one_and_set_errno(OS_STATUS_EERR);

    return 0;
}

int OS_TaskResume(uint32_t task_id)
{
    int32_t ret = 0;

    if( task_id >= OS_MAX_TASKS )
        os_return_minus_one_and_set_errno(OS_STATUS_EINVAL);

    WLOCK(); 
    {
        if( OS_task_table[task_id].free == TRUE )
        {
            WUNLOCK();
            os_return_minus_one_and_set_errno(OS_STATUS_EINVAL);
        }
    }
    WUNLOCK();

    ret = pthread_cond_signal( &OS_task_table[task_id].suspend_cond );

    ASSERT( ret == 0 );
    if( ret ) os_return_minus_one_and_set_errno(OS_STATUS_EERR);

    return 0;
}


/* 
 * ===  FUNCTION  ======================================================================
 *         Name:  OS_TaskDelete
 *  Description:  This function destroys a given task.
 *  Parameters:
 *      - task_id:  The task identifier
 *  Return:
 *      0          when the task is successfuly deleted
 *      OS_STATUS_EINVAL   when the provided task identifier is wrong
 *      OS_STATUS_EERR            when OS call error
 *
 * NOTE: This function is only allow in the non-LOCAL resource allocation mode,
 * which can be seleceted during OSAL configuraiton.
 * =====================================================================================
 */
int OS_TaskDelete (uint32_t task_id)
{    
#if defined (CONFIG_OS_STATIC_RESOURCE_ALLOCATION)
    os_return_minus_one_and_set_errno(OS_STATUS_NOT_SUPPORTED);
#else
    /* Check to see if the task_id given is valid */
    int ret;
    if (task_id >= OS_MAX_TASKS || OS_task_table[task_id].free == TRUE)
        os_return_minus_one_and_set_errno(OS_STATUS_EINVAL);

    /* Try to delete the task */

    ret = pthread_cancel(OS_task_table[task_id].id);
    if (ret != 0)
    {
        /*debugging statement only*/
        /*DEBUG("FAILED PTHREAD CANCEL %d, %d \n",ret, ESRCH); */
        os_return_minus_one_and_set_errno(OS_STATUS_EERR);
    }    

    /*
     * Now that the task is deleted, remove its 
     * "presence" in OS_task_table
     */

    WLOCK(); 
    {
        OS_task_table[task_id].free = TRUE;
        OS_task_table[task_id].l_errno = 0;
        OS_task_table[task_id].mul_Creator = UNINITIALIZED;
        OS_task_table[task_id].mul_StackSize = UNINITIALIZED;
        OS_task_table[task_id].mul_Priority = UNINITIALIZED;    
        OS_task_table[task_id].id = UNINITIALIZED;
        pthread_cond_destroy( &OS_task_table[task_id].suspend_cond );
        pthread_mutex_destroy( &OS_task_table[task_id].suspend_mutex );

        if( OS_task_table[task_id].is_monotonic )
        {
            OS_task_table[task_id].is_monotonic = FALSE;
            if( OS_task_table[task_id].info_periodic )
                OS_task_table[task_id].info_periodic->free = TRUE;
            OS_task_table[task_id].info_periodic = NULL;
        }

        /*  Statistics  */
        STATS_DEL_TASK();
    }
    WUNLOCK();

    return 0;
#endif

}/* end OS_TaskDelete */

/* 
 * ===  FUNCTION  ======================================================================
 *         Name:  OS_TaskExit
 *  Description:  Performs the deletetion of the caller task.
 * =====================================================================================
 */
int OS_TaskExit(void)
{
    uint32_t task_id;

    task_id = OS_TaskGetId();

    WLOCK(); 
    {
        OS_task_table[task_id].free = TRUE;
        OS_task_table[task_id].l_errno = 0;
        OS_task_table[task_id].mul_Creator = UNINITIALIZED;
        OS_task_table[task_id].mul_StackSize = UNINITIALIZED;
        OS_task_table[task_id].mul_Priority = UNINITIALIZED;
        OS_task_table[task_id].id = UNINITIALIZED;
        pthread_cond_destroy( &OS_task_table[task_id].suspend_cond );
        pthread_mutex_destroy( &OS_task_table[task_id].suspend_mutex );

        if( OS_task_table[task_id].is_monotonic )
        {
            OS_task_table[task_id].is_monotonic = FALSE;
            if( OS_task_table[task_id].info_periodic )
                OS_task_table[task_id].info_periodic->free = TRUE;
            OS_task_table[task_id].info_periodic = NULL;

            DEBUG("Periodic Task deleted");
        }
        else
        {
            DEBUG("Regular Task deleted");
        }

        /*  Statistics  */
        STATS_DEL_TASK();
    }
    WUNLOCK();

    pthread_exit(NULL);


    return 0;

}/*end OS_TaskExit */

/* 
 * ===  FUNCTION  ======================================================================
 *         Name:  OS_TaskSetPriority
 *  Description:  Modify the task ul_Priority
 *  Parameter:
 *      - task_id:      Task identifier
 *      - new_priority: new task ul_Priority
 *      
 *  Return:
 *      0 when the call is successfuly executed
 *      OS_STATUS_EINVAL when the task identifier is not valid
 *      OS_STATUS_EINVAL when the ul_Priority provided is not good
 *      OS_STATUS_EERR when OS call error
 *
 * NOTE: This function is only allow in the non-LOCAL resource allocation mode,
 * which can be seleceted during OSAL configuraiton.
 * =====================================================================================
 */
int OS_TaskSetPriority (
        uint32_t task_id, 
        uint32_t new_priority, 
        uint32_t *old_prio)
{
#if defined(CONFIG_OS_STATIC_RESOURCE_ALLOCATION)
    os_return_minus_one_and_set_errno(OS_STATUS_NOT_SUPPORTED);
#else
    int32_t ret;
    struct sched_param param ;

    if(task_id >= OS_MAX_TASKS || OS_task_table[task_id].free == TRUE)
        os_return_minus_one_and_set_errno(OS_STATUS_EINVAL);

    if ( (new_priority > sched_get_priority_max(OS_TASK_SCHED_POLICY)) || 
         (new_priority < sched_get_priority_min(OS_TASK_SCHED_POLICY)) )
        os_return_minus_one_and_set_errno(OS_STATUS_EINVAL);

    /* 
     ** Set ul_Priority -- This is currently incomplete ..
     */
    param.sched_priority = new_priority ;
    ret = pthread_setschedparam(OS_task_table[task_id].id, OS_TASK_SCHED_POLICY, &param);
    if( ret != 0 )
    {
        DEBUG("pthread_attr_setschedparam error in OS_TaskSetPriority, Task ID = %d\n",(int)task_id);
        os_return_minus_one_and_set_errno(OS_STATUS_EERR);
    }

    /*  Return the old ul_Priority */
    *old_prio = OS_task_table[task_id].mul_Priority;
    /* Change the ul_Priority in the table as well */
    OS_task_table[task_id].mul_Priority = new_priority;

    return 0;
#endif
} /* end OS_TaskSetPriority */

/* 
 * ===  FUNCTION  ======================================================================
 *         Name:  OS_TaskRegister
 *  Description:  Registers the calling 'task_id' with the task by adding the
 *  task variable to the TCB. It searches the OS_task_table to find the
 *  'task_id' corresponding to the TBC id.
 *  
 *  Parameters:
 *      - none
 *  Return:
 *      0          when success
 *      OS_STATUS_EINVAL   when the task identifier is wrong
 *      OS_STATUS_EERR            when Os call error
 * =====================================================================================
 */
LOCAL int OS_TaskRegister (void)
{
    int          i;
    int          ret;
    uint32_t       task_id;
    pthread_t    pthread_id;
    /* 
     ** Get PTHREAD Id
     */
    pthread_id = pthread_self();

    /*
     ** Look our task ID in table 
     */
    for(i = 0; i < OS_MAX_TASKS; i++)
    {
        if(OS_task_table[i].id == pthread_id)
        {
            break;
        }
    }
    task_id = i;

    if(task_id == OS_MAX_TASKS)
    {
        os_return_minus_one_and_set_errno(OS_STATUS_EINVAL);
    }

    /*
     ** Add pthread variable
     */
    ret = pthread_setspecific(thread_key, (void *)task_id);
    if ( ret != 0 )
    {
        DEBUG("OS_TaskRegister Failed during pthread_setspecific function\n");
        os_return_minus_one_and_set_errno(OS_STATUS_EERR);
    }

    return 0;
}/* end OS_TaskRegister */

/* 
 * ===  FUNCTION  ======================================================================
 *         Name:  OS_TaskGetId
 *  Description:  Return the task identifier
 *  Parameters:
 *      - none
 *  Return:
 *      Task identifier
 *
 *  NOTE:   The function OS_TaskRegister() shall be called before calling this
 *  one.
 * =====================================================================================
 */
int OS_TaskGetId(void)
{
    pthread_t    pthread_id;
    uint32_t i;  


    pthread_id = pthread_self();
    /* 
     ** Get PTHREAD Id
     */
    for (i = 0; i < OS_MAX_TASKS; i++)
    {
        if (pthread_equal(pthread_id, OS_task_table[i].id) != 0 )
        {
            break;
        }
    }

    if( i >= OS_MAX_TASKS ) 
    {
        os_return_minus_one_and_set_errno(OS_STATUS_EERR);
    }

    return i;
}

//int OS_TaskGetId (void)
//{ 
//    void*   task_id;
//    int     task_id_int;
//    uint32_t   task_key;
//    task_key = 0;
//
//    task_id = (void *)pthread_getspecific(thread_key);
//
//    memcpy(& task_id_int,&task_id, sizeof(uint32_t));
//    task_key = task_id_int & 0xFFFF;
//
//    return(task_key);
//}/* end OS_TaskGetId */

/* 
 * ===  FUNCTION  ======================================================================
 *         Name:  OS_TaskGetSlot
 *  Description:  Returns the pointer to the task slot.
 *  Parameters:
 *      - none
 *  Return:
 *      The pointer to the task slot
 * =====================================================================================
 */
void* OS_TaskGetSlot(void)
{

    return NULL;

}/* end OS_TaskGetSlot */

/* 
 * ===  FUNCTION  ======================================================================
 *         Name:  OS_TaskSetSlot
 *  Description:  Set the task slot.
 *  Parameters:
 *      - data: Pointer to be store on the task slot
 *  Return:
 *      0  when successfuly executed.
 * =====================================================================================
 */
int OS_TaskSetSlot(void* data)
{
    UNUSED(data);

    os_return_minus_one_and_set_errno(OS_STATUS_NOT_SUPPORTED);

}/* end OS_TaskSetSlot */

/* 
 * ===  FUNCTION  ======================================================================
 *         Name:  OS_TaskGetInfo
 *  Description:  Return information about the requested 'task_id'
 *  Parameters:
 *      - task_id:      Task identifier.
 *      - task_prop:    Task information returned.
 *  Return:
 *      0          when the call is executed successfuly
 *      OS_STATUS_EINVAL  when some of the parameters is wrong
 *      OS_STATUS_EINVAL   when the 'task_id' is not valid
 * =====================================================================================
 */
int OS_TaskGetInfo (uint32_t task_id, OS_task_prop_t *task_prop)  
{
    /* Check to see that the id given is valid */

    if (task_id >= OS_MAX_TASKS || OS_task_table[task_id].free == TRUE)
        os_return_minus_one_and_set_errno(OS_STATUS_EINVAL);

    if( task_prop == NULL)
        os_return_minus_one_and_set_errno(OS_STATUS_EINVAL);

    /* put the info into the stucture */


    WLOCK(); 
    {
        task_prop -> mul_Creator =    OS_task_table[task_id].mul_Creator;
        task_prop -> mul_StackSize = OS_task_table[task_id].mul_StackSize;
        task_prop -> mul_Priority =   OS_task_table[task_id].mul_Priority;
        task_prop -> mul_TaskId =  (uint32_t) OS_task_table[task_id].id;
    }
    WUNLOCK();


    return 0;

} /* end OS_TaskGetInfo */

void OS_TaskYield(void)
{
    sched_yield();
}

