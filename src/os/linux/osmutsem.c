/**
 *  \file   osmutsem.c
 *  \brief  This file features the Mutex semaphore implementation for the OSAL
 *  library under Linux operating system
 *
 *  Detailed description starts here.
 *
 *  \author  Aitor Viana Sanchez (avs), Aitor.Viana.Sanchez@esa.int
 *
 *  \internal
 *    Created:  11/25/2009
 *   Revision:  $Id: osmutsem.c 1.4 11/25/2009 avs Exp $
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

#include <pthread.h>
#include <errno.h>
#include <unistd.h>

#include <osal/osapi.h>
#include <osal/osstats.h>
#include <public/lock.h>
#include <osal/osdebug.h>

#define INIT_THREAD_MUTEX() \
    do{ \
        int ret;    \
        ret = lock_rw_init(&_rwlock);   \
        ASSERT( ret >= 0 ); \
    }while(0);


#define WLOCK()   __WLOCK()
#define WUNLOCK() __WUNLOCK()

/* Mutexes */
typedef struct
{
    int free;
    pthread_mutex_t id;
    int mul_Creator;
}OS_mut_sem_record_t;

LOCAL OS_mut_sem_record_t OS_mut_sem_table       [OS_MAX_MUTEXES];

extern uint32_t  OS_CompAbsDelayedTime( uint32_t milli_second , struct timespec * tm);


int OS_MutSemInit(void)
{
    int i;

    STATS_INIT_MUTSEM();

    /* Initialize Mutex Semaphore Table */

    for(i = 0; i < OS_MAX_MUTEXES; i++)
    {
        OS_mut_sem_table[i].free        = TRUE;
        OS_mut_sem_table[i].mul_Creator     = UNINITIALIZED;
    }

    INIT_THREAD_MUTEX();


    return 0;
}

/****************************************************************************************
  MUTEX API
 ****************************************************************************************/

int OS_MutSemCreate (
        uint32_t *sem_id, 
        uint32_t options)
{
    UNUSED(options);
    int                 return_code;
    int                 mutex_init_attr_status;
    pthread_mutexattr_t mutex_attr ;    
    uint32_t              possible_semid;

    /* Check Parameters */

    if ( (sem_id == NULL) )
        os_return_minus_one_and_set_errno(OS_STATUS_EINVAL);

    WLOCK();
    {

        for (possible_semid = 0; possible_semid < OS_MAX_MUTEXES; possible_semid++)
        {
            if (OS_mut_sem_table[possible_semid].free == TRUE)    
                break;
        }

        if( (possible_semid == OS_MAX_MUTEXES) ||
                (OS_mut_sem_table[possible_semid].free != TRUE) )
        {
            WUNLOCK();
            os_return_minus_one_and_set_errno(OS_STATUS_NO_FREE_IDS);
        }

        /* Set the free flag to false to make sure no other task grabs it */

        OS_mut_sem_table[possible_semid].free = FALSE;
    }
    WUNLOCK();

    /* 
     ** initialize the attribute with default values 
     */
    mutex_init_attr_status = pthread_mutexattr_init( &mutex_attr) ; 
    /*  Priority inheritance    */
//    return_code = pthread_mutexattr_setprotocol(&mutex_attr,PTHREAD_PRIO_PROTECT);
//    if( return_code < 0 )
//        DEBUG("Error setting the Priority Ceiling Protocol");

    /* 
     ** create the mutex 
     ** upon successful initialization, the state of the mutex becomes initialized and ulocked 
     */
    return_code =  pthread_mutex_init((pthread_mutex_t *) &OS_mut_sem_table[possible_semid].id,&mutex_attr); 
    if ( return_code != 0 )
    {
        /* Since the call failed, set free back to true */
        WLOCK();
        {
            OS_mut_sem_table[possible_semid].free = TRUE;
        }
        WUNLOCK();

        DEBUG("Error: Mutex could not be created. ID = %d\n", (int)possible_semid);
        os_return_minus_one_and_set_errno(OS_STATUS_EERR);
    }
    else
    {
        /*
         ** Mark mutex as initialized
         */
        /*     DEBUG("Mutex created, mutex_id = %d \n" ,possible_semid) ;*/

        *sem_id = possible_semid;

        WLOCK();
        {
            OS_mut_sem_table[*sem_id].free = FALSE;
            OS_mut_sem_table[*sem_id].mul_Creator = OS_TaskGetId();

            /*  Stats   */
            STATS_CREAT_MUTSEM();
        }
        WUNLOCK();

        return 0;
    }

}/* end OS_MutexSemCreate */


int OS_MutSemDelete (uint32_t sem_id)
{
#if defined (CONFIG_OS_STATIC_RESOURCE_ALLOCATION)
    os_return_minus_one_and_set_errno(OS_STATUS_EERR);
#else
    int status=-1;
    /* Check to see if this sem_id is valid   */
    if (sem_id >= OS_MAX_MUTEXES || OS_mut_sem_table[sem_id].free == TRUE)
        os_return_minus_one_and_set_errno(OS_STATUS_EINVAL);


    status = pthread_mutex_destroy( &(OS_mut_sem_table[sem_id].id)); /* 0 = success */   

    if( status != 0)
        os_return_minus_one_and_set_errno(OS_STATUS_SEM_FAILURE);
    /* Delete its presence in the table */

    WLOCK();
    {
        OS_mut_sem_table[sem_id].free = TRUE;
        OS_mut_sem_table[sem_id].mul_Creator = UNINITIALIZED;

        /*  Stats   */
        STATS_DEL_MUTSEM();
    }
    WUNLOCK();


    return 0;
#endif

}/* end OS_MutSemDelete */

int OS_MutSemGive ( uint32_t sem_id )
{
    /* Check Parameters */

    if(sem_id >= OS_MAX_MUTEXES || OS_mut_sem_table[sem_id].free == TRUE)
        os_return_minus_one_and_set_errno(OS_STATUS_EINVAL);

    /*
     ** Unlock the mutex
     */
    if(pthread_mutex_unlock(&(OS_mut_sem_table[sem_id].id)))
    {
        os_return_minus_one_and_set_errno(OS_STATUS_SEM_FAILURE);
    }

    return 0;
} /* end OS_MutSemGive */


/* 
 * ===  FUNCTION  ======================================================================
 *         Name:  OS_MutSemTake
 *  Description:  This function catches the mutex object referenced by
 *  'sem_id'.
 *  Parameters:
 *      - sem_id:   mutex identifier
 *  Returns:
 *      0 when the call success
 *      OS_STATUS_EINVAL when the 'sem_id' is not valid
 *      OS_STATUS_SEM_FAILURE when the OS call fails while catching the mutex
 * =====================================================================================
 */
int OS_MutSemTake ( uint32_t sem_id )
{
    /* 
     ** Check Parameters
     */  
    if(sem_id >= OS_MAX_MUTEXES || OS_mut_sem_table[sem_id].free == TRUE)
        os_return_minus_one_and_set_errno(OS_STATUS_EINVAL);

    /*
     ** Lock the mutex
     */
    if( pthread_mutex_lock(&(OS_mut_sem_table[sem_id].id) ))
    {
        os_return_minus_one_and_set_errno(OS_STATUS_SEM_FAILURE);
    }

    return 0;
}

/* 
 * ===  FUNCTION  ======================================================================
 *         Name:  OS_MutSemTryTake
 *  Description:  This function tries to catch a semaphore.
 *  Parameters:
 *      - sem_id:   Semaphore identifier
 *  Return:
 *      0 when the semaphore is catched
 *      OS_STATUS_EINVAL when the semaphore id is erroneous
 *      OS_STATUS_SEM_FAILURE when there is an error trying the get the semaphore
 * =====================================================================================
 */
int OS_MutSemTryTake (uint32_t sem_id)
{
    /* Check Parameters */
    if(sem_id >= OS_MAX_BIN_SEMAPHORES  || OS_mut_sem_table[sem_id].free == TRUE)
        os_return_minus_one_and_set_errno(OS_STATUS_EINVAL);

    /* Note to self: Check out sem wait in the manual */
    if ( pthread_mutex_trylock(&(OS_mut_sem_table[sem_id].id)) != 0)
    {
        os_return_minus_one_and_set_errno(OS_STATUS_SEM_FAILURE);
    }

    return 0;
}

/* 
 * ===  FUNCTION  ======================================================================
 *         Name:  OS_MutSemGetInfo
 *  Description:  This function retrieves information related to the mutex
 *  referenced by 'sem_id'.
 *  Parameters:
 *      - sem_id:   mutex identifier
 *      - sem_prop: structure where the mutex info will be stored
 *  Returns:
 *      0 when the call success
 *      OS_STATUS_EINVAL when the 'sem_id' is not valid
 *      OS_STATUS_EINVAL when the 'sem_prop' structure pointer is not valid
 * =====================================================================================
 */
int OS_MutSemGetInfo (uint32_t sem_id, OS_mut_sem_prop_t *mut_prop)  
{
    /* Check to see that the id given is valid */

    if (sem_id >= OS_MAX_MUTEXES || OS_mut_sem_table[sem_id].free == TRUE)
        os_return_minus_one_and_set_errno(OS_STATUS_EINVAL);

    if (mut_prop == NULL)
        os_return_minus_one_and_set_errno(OS_STATUS_EINVAL);

    /* put the info into the stucture */    

    WLOCK();
    {
        mut_prop -> mul_Creator =   OS_mut_sem_table[sem_id].mul_Creator;
    }
    WUNLOCK();



    return 0;

} /* end OS_BinSemGetInfo */

/* 
 * ===  FUNCTION  ======================================================================
 *         Name:  OS_MutSemTimedWait
 *  Description:  This function tries to acquire a semaphore specified by the
 *  sempahore identifier. The calling thread is blocked until the semaphore is
 *  release of until the timeout expires.
 *  Parameters:
 *      - sem_id:   Sempahore identifier
 *      - msecs:    Timeout in milliseconds
 *  Return:
 *      0              when the sempahore is successfuly taken
 *      OS_STATUS_EINVAL       when the sempahore identifier is not valid
 *      OS_STATUS_SEM_TIMEOUT          when the timeout expires
 *      OS_STATUS_SEM_FAILURE          when any other error occurs
 * =====================================================================================
 */
int OS_MutSemTimedWait ( uint32_t sem_id, uint32_t msecs )
{
    uint32_t           ret_val ;
    struct timespec  temp_timespec ;
    int timeloop;


    if( (sem_id >= OS_MAX_MUTEXES) || (OS_mut_sem_table[sem_id].free == TRUE) )
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
        if ( (status = pthread_mutex_trylock(&(OS_mut_sem_table[sem_id].id))) == EBUSY )
        {
            /* sleep for 100 msecs */
            usleep(100*1000);
        }

        else
        {   /* something besides the sem being taken made it fail */
            /*             if(sem_trywait(&(OS_bin_sem_table[sem_id].id)) == -1)
            */
            /* 
             * The value of the semaphore is not left whether the call
             * successed and the semaphore was locke, so that another
             * sem_trywait would try to lock the sempahore twice...and
             * the routine would fail.
             */
            if( status != 0 )
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


