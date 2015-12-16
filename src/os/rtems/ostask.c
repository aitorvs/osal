/**
 *  \file   ostask.c
 *  \brief  This file implements the Tasks API of the OSAL library for the
 *  Linux operating system
 *
 *  \author  Aitor Viana Sanchez (avs), Aitor.Viana.Sanchez@esa.int
 *
 *  \internal
 *    Created:  19/02/09
 *   Revision:  $Id: ostask.c 1.4 19/02/09 avs Exp $
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

#include <rtems.h>
#include <stdlib.h>
#include <stdio.h>

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
        int _ret;    \
        _ret = lock_rw_init(&_rwlock);   \
        ASSERT( _ret >= 0 ); \
    }while(0);


#define WLOCK()   __WLOCK()
#define WUNLOCK() __WUNLOCK()

/********************************* FILE CLASSES/STRUCTURES */

typedef void(*periodic_action_t)(void*);

struct monotonic_task_info
{
    periodic_action_t pfunc;
    periodic_action_t perr;
    rtems_id period;
    void *arg;
    rtems_interval timeout_ticks;
    uint32_t free;
};

struct oneshot_task_info
{
    periodic_action_t pfunc;
    void *arg;
    uint32_t free;
};

/*tasks */
typedef struct
{
    int errno;
    int free;
    rtems_id mul_RtemsId;
    uint32_t mul_TaskId;
    int mul_Creator;
    uint32_t mul_StackSize;
    uint32_t mul_Priority;
    uint32_t is_monotonic;
    struct monotonic_task_info *info_periodic;
    struct oneshot_task_info *info_oneshot;
}OS_task_record_t;

static struct monotonic_task_info os_monotonic_task_table[OS_MAX_MONOTONIC_TASKS];

#define MAX_SHOT_TASKS  (OS_MAX_TASKS - OS_MAX_MONOTONIC_TASKS)
static struct oneshot_task_info os_oneshot_task_table[MAX_SHOT_TASKS];

typedef uint32_t OS_task_slot_t;

/********************************* FILE PRIVATE VARIABLES  */

LOCAL OS_task_record_t      OS_task_table       [OS_MAX_TASKS];

/** this is the Initial name for the tasks  */
LOCAL char ntask_name[] = "0000";

LOCAL OS_task_slot_t        OS_task_slots       [OS_MAX_TASKS];

LOCAL OS_task_record_t *pt_TaskStructure = 0;

LOCAL uint32_t sem_join = 0;

/********************************* PRIVATE INTERFACE    */

int *__os_errno_addr(void)
{
    static OS_STATUS_T _errno;
    int *v = &_errno;
    if( pt_TaskStructure )
        v = &pt_TaskStructure->errno;

    return v;
}

/* 
 * ===  FUNCTION  ======================================================================
 *         Name:  _os_taskexit
 *  Description:  Performs the deletetion of the caller task.
 * =====================================================================================
 */
static int _os_taskexit(void)
{
    uint32_t task_id;
    rtems_status_code status;

    task_id = OS_TaskGetId();

    WLOCK();
    {
        OS_task_table[task_id].errno = 0;
        OS_task_table[task_id].free = TRUE;
        OS_task_table[task_id].mul_RtemsId = UNINITIALIZED;
        OS_task_table[task_id].mul_Creator = UNINITIALIZED;
        OS_task_table[task_id].mul_StackSize = UNINITIALIZED;
        OS_task_table[task_id].mul_Priority = UNINITIALIZED;

        if( OS_task_table[task_id].is_monotonic )
        {
            int32_t ret;
            ret = rtems_rate_monotonic_delete(
                    OS_task_table[task_id].info_periodic->period);

            ASSERT( ret == RTEMS_SUCCESSFUL );
            if( ret != RTEMS_SUCCESSFUL )
            {
                os_return_minus_one_and_set_errno(OS_STATUS_EERR);
            }
            else
            {
                DEBUG("Periodic Task deleted");
            }
        }
        else
        {
            DEBUG("Regular Task deleted");
        }
        OS_task_table[task_id].is_monotonic = FALSE;
        if( OS_task_table[task_id].info_periodic )
            OS_task_table[task_id].info_periodic->free = TRUE;
        OS_task_table[task_id].info_periodic = NULL;

        /*  Statistics  */
        STATS_DEL_TASK();
    }
    WUNLOCK();

    /*  If it is the last task...release the main   */
    if( STATS_CURR_TASKS() == 0 )
        rtems_semaphore_release( sem_join );

    status = rtems_task_delete(RTEMS_SELF);
    ASSERT(status == RTEMS_SUCCESSFUL);
    if (status != RTEMS_SUCCESSFUL)
        os_return_minus_one_and_set_errno(OS_STATUS_EERR);

    return 0;

}/*end _os_taskexit */


/********************************* PUBLIC  INTERFACE    */

int OS_TaskInit(void)
{
    int i;
    rtems_name r_name;
    rtems_status_code ret;

    STATS_INIT_TASK();

    /* Initialize Task Table */

    for(i = 0; i < OS_MAX_TASKS; i++)
    {
        OS_task_table[i].errno = 0;
        OS_task_table[i].free        = TRUE;
        OS_task_table[i].mul_RtemsId  = UNINITIALIZED;
        OS_task_table[i].mul_TaskId   = i;
        OS_task_table[i].mul_Creator   = UNINITIALIZED;
        OS_task_table[i].is_monotonic = FALSE;

        OS_task_slots[i] =  UNINITIALIZED; 

    }
    for(i = 0; i < OS_MAX_MONOTONIC_TASKS; i++)
    {
        os_monotonic_task_table[i].free = TRUE;
        os_monotonic_task_table[i].pfunc = NULL;
        os_monotonic_task_table[i].perr = NULL;
    }
    for(i = 0; i < MAX_SHOT_TASKS; i++)
    {
        os_oneshot_task_table[i].free = TRUE;
        os_oneshot_task_table[i].pfunc = NULL;
    }

    /*  Create the Task Join semaphore  */
    r_name = rtems_build_name('J', 'O', 'I', 'N');
    ret = rtems_semaphore_create( r_name, 0, 0, 
            RTEMS_BINARY_SEMAPHORE|RTEMS_PRIORITY_CEILING, &sem_join);
    if ( ret != RTEMS_SUCCESSFUL )
        os_return_minus_one_and_set_errno(OS_STATUS_EERR);

    INIT_THREAD_MUTEX();

    return 0;

}

/****************************************************************************************
  TASK API
 ****************************************************************************************/

static void _register_task(void)
{
    rtems_status_code rtems_status;
    rtems_id id;
    int i;

    rtems_status = rtems_task_ident(RTEMS_SELF, 0, &id);
    ASSERT( rtems_status == RTEMS_SUCCESSFUL );
    /*  Register the task Structure */
    rtems_status = rtems_task_variable_add(
            id,
            (void*)&pt_TaskStructure,
            NULL);

    for(i = 0; i < OS_MAX_TASKS; i++)
    {
        if(OS_task_table[i].mul_RtemsId == id)
        {
            pt_TaskStructure = &OS_task_table[i];
            break;
        }
    }
    ASSERT( pt_TaskStructure );

}

static rtems_task oneshot_task(rtems_task_argument arg)
{

    struct oneshot_task_info *info = (struct oneshot_task_info*)arg;

    _register_task();

    /*  Execute the user task function  */
    ASSERT( info->pfunc );
    if( info->pfunc )
        info->pfunc(info->arg);

    /*  Delete the task */
    OS_TaskExit();
}

int OS_TaskExit(void)
{
    /*  Delete the task */
    return _os_taskexit();
}

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
static rtems_task periodic_task(rtems_task_argument arg)
{
    rtems_name name;
    rtems_id period;
    rtems_status_code ret;
    struct monotonic_task_info *info = (struct monotonic_task_info*) arg;
    int32_t status;

    NEXT_RESOURCE_NAME(ntask_name[0],ntask_name[1],ntask_name[2],ntask_name[3]);
    name = rtems_build_name(ntask_name[0],ntask_name[1],ntask_name[2],ntask_name[3]);

    ret = rtems_rate_monotonic_create(name, &period);
    ASSERT( ret == RTEMS_SUCCESSFUL );

    if( ret != RTEMS_SUCCESSFUL )
    {
        /*  Periodic task cannot be created */
        status = OS_STATUS_EERR;
        goto err;
    }

    /*  Store the RTEMS id for the period in the info structure */
    info->period = period;

    for( ;; )
    {
        ret = rtems_rate_monotonic_period( period, info->timeout_ticks );
        if( ret == RTEMS_TIMEOUT ) break;

        info->pfunc(info->arg);
    }

    ret = rtems_rate_monotonic_delete(period);
    ASSERT( ret == RTEMS_SUCCESSFUL );
    if( ret != RTEMS_SUCCESSFUL )
    {
        status = OS_STATUS_EERR;
        goto err;
    }

    status = OS_STATUS_PERIODIC_TASK_MISSED;

err:
    if( info->perr )
        info->perr((void*)&status);
    else DEBUG("No perr() callback");
    OS_TaskExit();
}

int OS_TaskMonotonicCreate(
        uint32_t *task_id,
        const void *pfunc,
        const void *perr_handler,
        uint32_t stack_size,
        uint32_t priority,
        uint32_t flags,
        void *arg,
        uint32_t ms_period
        )
{
    int32_t ret;
    uint32_t n_periods;
    struct monotonic_task_info *info;

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
        info = &os_monotonic_task_table[n_periods];
    }
    WUNLOCK();


    info->pfunc = pfunc;

    info->arg = arg;
    if( perr_handler ) 
        info->perr = perr_handler;
    else info->perr = NULL;

    info->timeout_ticks = ms_period * OS_TICKS_PER_SECOND / 1000;


    ret = OS_TaskCreate(
            task_id, 
            (void*)periodic_task,
            stack_size,
            priority,
            flags|OS_IS_PERIODIC,
            (void*)info);

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
 *      - priority:         Task priority, needs to be lower than
 *      OS_TASK_MAX_PRIORITY.
 *      - flags:            OS_FP_ENABLED if the task to be created is floating
 *      point task. OS_FP_DISABLED if the task to be created is non-FP task. By
 *      default, FP tasks are created.
 *  Return:
 *      0              when the task is created successfuly
 *      OS_STATUS_EINVAL      when any of the necessary pointers are NULL
 *      OS_STATUS_NAME_TOO_LONG    when the task name is too long
 *      OS_STATUS_INVALID_PRIORITY when the provided priority value is not correct
 *      OS_STATUS_NO_FREE_IDS      when it is not possible to create more tasks
 *      OS_STATUS_NAME_TAKEN       when the task already exist
 *      OS_STATUS_EERR                when OS call error
 * =====================================================================================
 */
int OS_TaskCreate (
        uint32_t *task_id, 
        const void *function_pointer,
        uint32_t stack_size, 
        uint32_t priority, 
        uint32_t flags,
        void* arg
        )
{

    ASSERT(task_id);
    ASSERT(function_pointer);
    ASSERT(stack_size);

    uint32_t possible_taskid;
    uint32_t tbl_index;
    rtems_status_code  status;
    rtems_name         r_name;
    rtems_mode         r_mode;
    rtems_attribute    r_attributes;
    struct oneshot_task_info *info;

    /* Check for NULL pointers */
    if( (function_pointer == NULL) || (task_id == NULL) )
        os_return_minus_one_and_set_errno(OS_STATUS_EINVAL);


    /* Check for bad priority */

    if ( (priority > OS_TASK_MAX_PRIORITY) || (priority < OS_TASK_MIN_PRIORITY) )
        os_return_minus_one_and_set_errno(OS_STATUS_EINVAL);

    WLOCK();
    {
        for(tbl_index = 0; tbl_index < MAX_SHOT_TASKS; tbl_index++)
        {
            if (os_oneshot_task_table[tbl_index].free == TRUE)
            {
                break;
            }
        }

        /* Check to see if the id is out of bounds */

        if(     tbl_index >= MAX_SHOT_TASKS || 
                os_oneshot_task_table[tbl_index].free != TRUE
          )
        {    
            WUNLOCK();
            os_return_minus_one_and_set_errno(OS_STATUS_NO_FREE_IDS);
        }

        /* Set the possible task Id to not free so that
         * no other task can try to use it */

        os_oneshot_task_table[tbl_index].free  = FALSE;
        info = &os_oneshot_task_table[tbl_index];
    }
    WUNLOCK();

    info->pfunc = function_pointer;
    info->arg = arg;
    ASSERT( info->pfunc );

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

        OS_task_table[possible_taskid].free  = FALSE;
    }
    WUNLOCK();

    NEXT_RESOURCE_NAME(ntask_name[0],ntask_name[1],ntask_name[2],ntask_name[3]);
    r_name = rtems_build_name(ntask_name[0],ntask_name[1],ntask_name[2],ntask_name[3]);
    r_mode = RTEMS_PREEMPT | RTEMS_NO_ASR | RTEMS_NO_TIMESLICE | RTEMS_INTERRUPT_LEVEL(0);

    /* see if the user wants floating point disabled. If 
     * so, then se the correct option.
     * It is most probable that the user wants the FP to be enabled.
     */
    if ( __builtin_expect((flags == OS_FP_DISABLED), 0) )
    {
        r_attributes = RTEMS_LOCAL;
    }
    else
    {
        r_attributes = RTEMS_FLOATING_POINT | RTEMS_LOCAL;
    }

    status = rtems_task_create(
            r_name,
            priority,
            stack_size,
            r_mode,
            r_attributes,
            &OS_task_table[possible_taskid].mul_RtemsId); 

    /* check if task_create failed */

    if (status != RTEMS_SUCCESSFUL )
    {       
        WLOCK();
        {
            OS_task_table[possible_taskid].free  = TRUE;
        }
        WUNLOCK();
        os_return_minus_one_and_set_errno(OS_STATUS_EERR);
    } 

    /* will place the task in 'ready for scheduling' state */
    status = rtems_task_start (OS_task_table[possible_taskid].mul_RtemsId, /*rtems task id*/
            oneshot_task,                                /*task entry point */
            (rtems_task_argument)info );                      /* passed argument  */

    if (status != RTEMS_SUCCESSFUL )
    {		
        WLOCK();
        {
            OS_task_table[possible_taskid].free  = TRUE;
        }
        WUNLOCK();
        os_return_minus_one_and_set_errno(OS_STATUS_EERR);		
    }

    /* Set the task_id to the id that was found available 
       Set the name of the task, the stack size, and priority */

    *task_id = possible_taskid;


    /* this Id no longer free */
    WLOCK();
    {
        OS_task_table[*task_id].errno = 0;
        OS_task_table[*task_id].mul_Creator = OS_TaskGetId();
        OS_task_table[*task_id].mul_StackSize = stack_size;
        OS_task_table[*task_id].mul_Priority = priority;
        if( flags & OS_IS_PERIODIC )
        {
            DEBUG("Periodic task");
            OS_task_table[possible_taskid].is_monotonic  = TRUE;
            OS_task_table[possible_taskid].info_periodic  = (struct monotonic_task_info*)arg;
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

} /* end OS_TaskCreate */

int OS_TaskSuspend(uint32_t task_id)
{
    rtems_id          rtems_task_id;
    int ret;

    if( task_id == OS_SELF )
    {
        ret = rtems_task_suspend(RTEMS_SELF);
    }
    else
    {
        if( task_id >= OS_MAX_TASKS )
            os_return_minus_one_and_set_errno(OS_STATUS_EINVAL);

        WLOCK();
        {
            if( OS_task_table[task_id].free == TRUE )
            {
                WUNLOCK();
                os_return_minus_one_and_set_errno(OS_STATUS_EINVAL);
            }

            rtems_task_id = OS_task_table[task_id].mul_RtemsId;
        }
        WUNLOCK();


        ret = rtems_task_suspend(rtems_task_id);
    }

    ASSERT( ret == RTEMS_SUCCESSFUL );
    switch(ret)
    {
        case RTEMS_SUCCESSFUL:
            return 0;
        case RTEMS_ALREADY_SUSPENDED:
            os_return_minus_one_and_set_errno(OS_STATUS_EERR);
        case RTEMS_INVALID_ID:
            os_return_minus_one_and_set_errno(OS_STATUS_EINVAL);
        default:
            os_return_minus_one_and_set_errno(OS_STATUS_EERR);
    }

    return 0;
}

int OS_TaskResume(uint32_t task_id)
{
    rtems_id          rtems_task_id;
    int ret; 

    if( task_id >= OS_MAX_TASKS )
        os_return_minus_one_and_set_errno(OS_STATUS_EINVAL);

    WLOCK();
    {
        if( OS_task_table[task_id].free == TRUE )
        {
            WUNLOCK();
            os_return_minus_one_and_set_errno(OS_STATUS_EINVAL);
        }

        rtems_task_id = OS_task_table[task_id].mul_RtemsId;
    }
    WUNLOCK();


    ret = rtems_task_resume(rtems_task_id);

//    ASSERT( ret == RTEMS_SUCCESSFUL );
    switch(ret)
    {
        case RTEMS_SUCCESSFUL:
            return 0;
        case RTEMS_INCORRECT_STATE:
            os_return_minus_one_and_set_errno(OS_STATUS_EERR);
        case RTEMS_INVALID_ID:
            os_return_minus_one_and_set_errno(OS_STATUS_EINVAL);
        default:
            os_return_minus_one_and_set_errno(OS_STATUS_EERR);
    }

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
#if defined(CONFIG_OS_STATIC_RESOURCE_ALLOCATION)
    os_return_minus_one_and_set_errno(OS_STATUS_EERR);
#else
    rtems_status_code status;

    /* Check to see if the task_id given is valid */
    if (task_id >= OS_MAX_TASKS || OS_task_table[task_id].free == TRUE)
        os_return_minus_one_and_set_errno(OS_STATUS_EINVAL);

    /* Try to delete the task */
    status = rtems_task_delete(OS_task_table[task_id].mul_RtemsId);
    ASSERT(status == RTEMS_SUCCESSFUL);
    if (status != RTEMS_SUCCESSFUL)
        os_return_minus_one_and_set_errno(OS_STATUS_EERR);

    /*
     * Now that the task is deleted, remove its 
     * "presence" in OS_task_table
     */

    WLOCK();
    {
        OS_task_table[task_id].errno = 0;
        OS_task_table[task_id].free = TRUE;
        OS_task_table[task_id].mul_RtemsId = UNINITIALIZED;
        OS_task_table[task_id].mul_Creator = UNINITIALIZED;
        OS_task_table[task_id].mul_StackSize = UNINITIALIZED;
        OS_task_table[task_id].mul_Priority = UNINITIALIZED;

        if( OS_task_table[task_id].is_monotonic )
        {
            int32_t ret;
            ret = rtems_rate_monotonic_delete(
                    OS_task_table[task_id].info_periodic->period);

            ASSERT( ret == RTEMS_SUCCESSFUL );
            if( ret != RTEMS_SUCCESSFUL )
            {
                os_return_minus_one_and_set_errno(OS_STATUS_EERR);
            }
        }
        OS_task_table[task_id].is_monotonic = FALSE;
        if( OS_task_table[task_id].info_periodic )
            OS_task_table[task_id].info_periodic->free = TRUE;
        OS_task_table[task_id].info_periodic = NULL;

        /*  Statistics  */
        STATS_DEL_TASK();
    }
    WUNLOCK();

    return 0;
#endif

}/* end OS_TaskDelete */

/* 
 * ===  FUNCTION  ======================================================================
 *         Name:  OS_TaskSetPriority
 *  Description:  Modify the task priority
 *  Parameter:
 *      - task_id:      Task identifier
 *      - new_priority: new task priority
 *      
 *  Return:
 *      0 when the call is successfuly executed
 *      OS_STATUS_EINVAL when the task identifier is not valid
 *      OS_STATUS_INVALID_PRIORITY when the priority provided is not good
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
    os_return_minus_one_and_set_errno(OS_STATUS_EERR);
#else
    rtems_task_priority _old_prio;

    /* Check Parameters */
    if(task_id >= OS_MAX_TASKS || OS_task_table[task_id].free == TRUE)
        os_return_minus_one_and_set_errno(OS_STATUS_EINVAL);

    if ( (new_priority > OS_TASK_MAX_PRIORITY) || (new_priority < OS_TASK_MIN_PRIORITY) )
        os_return_minus_one_and_set_errno(OS_STATUS_EINVAL);

    /* Set RTEMS Task Priority */
    if (rtems_task_set_priority(OS_task_table[task_id].mul_RtemsId, new_priority, &_old_prio)
            != RTEMS_SUCCESSFUL )
        os_return_minus_one_and_set_errno(OS_STATUS_EERR);

    OS_task_table[task_id].mul_Priority = new_priority;
    *old_prio = _old_prio;

    return 0;
#endif

}/* end OS_TaskSetPriority */

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
    /*  Do not assert as if the main creates a task pt_TaskStructure will be
     *  NULL. This means that when mul_Creator is OS_STATUS_EERR, the creator is
     *  actually main
     */
    if( pt_TaskStructure )
        return pt_TaskStructure->mul_TaskId;
    

    os_return_minus_one_and_set_errno(OS_STATUS_EERR);
}

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
    ASSERT( pt_TaskStructure );

    return (void*)OS_task_slots[pt_TaskStructure->mul_TaskId];

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

    ASSERT( pt_TaskStructure );

    OS_task_slots[pt_TaskStructure->mul_TaskId] = (uint32_t)data;

    return 0;

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
    ASSERT(task_prop);

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
        task_prop -> mul_TaskId =  (uint32_t) OS_task_table[task_id].mul_RtemsId;
    }
    WUNLOCK();


    return 0;

} /* end OS_TaskGetInfo */

void OS_TaskYield(void)
{
    int ret;
    ret = rtems_task_wake_after(0);


    if( unlikely( RTEMS_SUCCESSFUL != ret ) )
    {
        DEBUG("ERROR");
    }
}

void OS_TaskJoin(void)
{
    rtems_semaphore_obtain(sem_join, RTEMS_WAIT, RTEMS_NO_TIMEOUT);
}

