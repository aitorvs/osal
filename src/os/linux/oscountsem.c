/**
 *  \file   oscountsem.c
 *  \brief  Counting semaphore implementation under Linux
 *
 *  \author  Aitor Viana Sanchez (avs), Aitor.Viana.Sanchez@esa.int
 *
 *  \internal
 *    Created:  11/28/2010
 *   Revision:  $Id: oscountsem.c 1.4 11/28/2010 avs Exp $
 *   Compiler:  gcc/g++
 *    Company:  European Space Agency (ESA-ESTEC)
 *  Copyright:  Copyright (c) 2010, Aitor Viana Sanchez
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

#include <semaphore.h>
#include <pthread.h>
#include <errno.h>
#include <unistd.h>
#include <osal/osdebug.h>

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

#define _IS_COUNTSEM_INIT()   \
{   \
    if( !_countsem_is_init ) \
    { \
        _os_countsem_init(); \
        _countsem_is_init = 1; \
    } \
}
#define _CHECK_COUNTSEM_INIT()  (_IS_COUNTSEM_INIT())

/** This variable flags whether the counting semaphores are yet initialized or
 * not
 */
static uint8_t _countsem_is_init = 0;


/* Binary Semaphores */
typedef struct
{
    int free;
    sem_t id;
    int mul_Creator;
}OS_count_sem_record_t;

LOCAL OS_count_sem_record_t OS_count_sem_table   [OS_MAX_COUNT_SEMAPHORES];

extern uint32_t  OS_CompAbsDelayedTime( uint32_t milli_second , struct timespec * tm);

static void _os_countsem_init(void)
{
    int i;

    STATS_INIT_COUNTSEM();

    /* Initialize Counting Semaphores */
    for(i = 0; i < OS_MAX_COUNT_SEMAPHORES; i++)
    {
        OS_count_sem_table[i].free        = TRUE;
        OS_count_sem_table[i].mul_Creator     = UNINITIALIZED;
    }

    INIT_THREAD_MUTEX();
}

/****************************************************************************************
  COUNT SEMAPHORE API
 ****************************************************************************************/

int OS_CountSemCreate (uint32_t *sem_id, 
        uint32_t sem_initial_value,
        uint32_t options)
{
    UNUSED(options);
    uint32_t possible_semid;
    int Status;

    _CHECK_COUNTSEM_INIT();

    if ( (sem_id == NULL) )
        os_return_minus_one_and_set_errno(OS_STATUS_EINVAL);

    /* Check Parameters */

    WLOCK();
    {

        for (possible_semid = 0; possible_semid < OS_MAX_COUNT_SEMAPHORES; possible_semid++)
        {
            if (OS_count_sem_table[possible_semid].free == TRUE)    
                break;
        }


        if((possible_semid >= OS_MAX_COUNT_SEMAPHORES) ||  
                (OS_count_sem_table[possible_semid].free != TRUE))
        {
            WUNLOCK();
            os_return_minus_one_and_set_errno(OS_STATUS_NO_FREE_IDS);
        }

        /* set the ID to taken so no other task can grab it */
        OS_count_sem_table[possible_semid].free = FALSE;
    }
    WUNLOCK();

    /*
     ** Create semaphore
     */

    errno = 0;

    Status = sem_init( &(OS_count_sem_table[possible_semid].id) ,0 , sem_initial_value);
    if( Status == -1 )
    {
        /* Since the call failed, set it the free flag back to true */
        CRITICAL( (OS_count_sem_table[possible_semid].free = TRUE) );
        DEBUG("Error Creating semaphore in OS_CountSemCreate! errno = %d\n", errno);

        os_return_minus_one_and_set_errno(OS_STATUS_EERR);
    }

    *sem_id = possible_semid;

    WLOCK();
    {
        OS_count_sem_table[*sem_id].free = FALSE;
        OS_count_sem_table[*sem_id].mul_Creator = OS_TaskGetId();

        /*  Stats   */
        STATS_CREAT_COUNTSEM();
    }
    WUNLOCK();

    return 0;
}/* end OS_CountSemCreate */

int OS_CountSemDelete (uint32_t sem_id)
{
    _CHECK_COUNTSEM_INIT();

#if defined (CONFIG_OS_STATIC_RESOURCE_ALLOCATION)
    os_return_minus_one_and_set_errno(OS_STATUS_EERR);
#else
    /* Check to see if this sem_id is valid */
    if (sem_id >= OS_MAX_COUNT_SEMAPHORES || OS_count_sem_table[sem_id].free == TRUE)
        os_return_minus_one_and_set_errno(OS_STATUS_EINVAL);



    if (sem_destroy( &(OS_count_sem_table[sem_id].id)) != 0) /* 0 = success */ 
    {
        os_return_minus_one_and_set_errno(OS_STATUS_SEM_FAILURE);
    }

    /* Remove the Id from the table, and its name, so that it cannot be found again */

    WLOCK();
    {
        OS_count_sem_table[sem_id].free = TRUE;
        OS_count_sem_table[sem_id].mul_Creator = UNINITIALIZED;

        /*  Stats   */
        STATS_DEL_COUNTSEM();
    }
    WUNLOCK();

    return 0;
#endif

}/* end OS_CountSemDelete */

int OS_CountSemGive ( uint32_t sem_id )
{
    int32_t    ret;

    _CHECK_COUNTSEM_INIT();

    /* Check Parameters */

    if(sem_id >= OS_MAX_COUNT_SEMAPHORES || OS_count_sem_table[sem_id].free == TRUE)
        os_return_minus_one_and_set_errno(OS_STATUS_EINVAL);

    ret = sem_post(&(OS_count_sem_table[sem_id].id));

    if ( ret != 0 )
    {
        os_return_minus_one_and_set_errno(OS_STATUS_SEM_FAILURE);
    }

    return 0;
}/* end OS_CountSemGive */

int OS_CountSemTake ( uint32_t sem_id )
{
    int    ret;

    _CHECK_COUNTSEM_INIT();

    if(sem_id >= OS_MAX_COUNT_SEMAPHORES  || OS_count_sem_table[sem_id].free == TRUE)
        os_return_minus_one_and_set_errno(OS_STATUS_EINVAL);

    ret = sem_wait(&(OS_count_sem_table[sem_id].id));

    if ( ret != 0 )
    {
        os_return_minus_one_and_set_errno(OS_STATUS_SEM_FAILURE);
    }

    return 0;
}/* end OS_CountSemTake */

int OS_CountSemTimedWait ( uint32_t sem_id, uint32_t msecs )
{
    uint32_t           ret_val ;
    struct timespec  temp_timespec ;
    int timeloop;


    _CHECK_COUNTSEM_INIT();

    if( (sem_id >= OS_MAX_COUNT_SEMAPHORES) || (OS_count_sem_table[sem_id].free == TRUE) )
        os_return_minus_one_and_set_errno(OS_STATUS_EINVAL);   


    /*
     ** Compute an absolute time for the delay
     */
    ret_val = OS_CompAbsDelayedTime( msecs , &temp_timespec) ;

    /* try it this way */
    timeloop = (msecs == 0) ? 100 : msecs;
    for (; timeloop >0; timeloop -= 100)
    {
        int status = -1;
        if ( ((status = sem_trywait(&(OS_count_sem_table[sem_id].id))) == -1) && 
                (errno == EAGAIN) )
        {
            /* sleep for 100 msecs */
            usleep(100*1000);
        }

        else
        {   /* something besides the sem being taken made it fail */
            /* 
             * The value of the semaphore is not left whether the call
             * successed and the semaphore was locke, so that another
             * sem_trywait would try to lock the sempahore twice...and
             * the routine would fail.
             */
            /*             if(sem_trywait(&(OS_count_sem_table[sem_id].id)) == -1)
            */
            if(status == -1)
            {
                os_return_minus_one_and_set_errno(OS_STATUS_SEM_FAILURE);
            }
            else
            {
                return 0;
            }
        }
    }
    os_return_minus_one_and_set_errno(OS_STATUS_TIMEOUT);
}

int OS_CountSemTryTake (uint32_t sem_id)
{
    _CHECK_COUNTSEM_INIT();

    /* Check Parameters */

    if(sem_id >= OS_MAX_BIN_SEMAPHORES  || OS_count_sem_table[sem_id].free == TRUE)
        os_return_minus_one_and_set_errno(OS_STATUS_EINVAL);

    /* Note to self: Check out sem wait in the manual */
    if ( sem_trywait(&(OS_count_sem_table[sem_id].id)) != 0)
    {
        os_return_minus_one_and_set_errno(OS_STATUS_SEM_FAILURE);
    }
    else
    {
        return 0;
    }

    os_return_minus_one_and_set_errno(OS_STATUS_EERR);
}

int OS_CountSemGetInfo (uint32_t sem_id, OS_count_sem_prop_t *count_prop)  
{
    _CHECK_COUNTSEM_INIT();

    /* Check to see that the id given is valid */

    if (sem_id >= OS_MAX_COUNT_SEMAPHORES || OS_count_sem_table[sem_id].free == TRUE)
        os_return_minus_one_and_set_errno(OS_STATUS_EINVAL);

    if (count_prop == NULL)
        os_return_minus_one_and_set_errno(OS_STATUS_EINVAL);

    /* put the info into the stucture */
    CRITICAL( (count_prop ->mul_Creator = OS_count_sem_table[sem_id].mul_Creator) );


    return 0;

} /* end OS_CountSemGetInfo */


