/**
 *  \file   ossem.c
 *  \brief  This file features the semaphore implementation for the OSAL
 *  library under Linux OS
 *
 *  Detailed description starts here.
 *
 *  \author  Aitor Viana Sanchez (avs), Aitor.Viana.Sanchez@esa.int
 *
 *  \internal
 *    Created:  11/25/2009
 *   Revision:  $Id: ossem.c 1.4 11/25/2009 avs Exp $
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

#include <semaphore.h>
#include <pthread.h>
#include <errno.h>
#include <unistd.h>
#include <stdio.h>

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

/* Binary Semaphores */
typedef struct
{
    int free;
    sem_t id;
    int mul_Creator;
    uint32_t blocked;
}OS_bin_sem_record_t;

LOCAL OS_bin_sem_record_t OS_bin_sem_table       [OS_MAX_BIN_SEMAPHORES];
/** This mutex is used to help to perform the OS_BinSemFlush() operation
 * properly */
LOCAL pthread_mutex_t m_flush = PTHREAD_MUTEX_INITIALIZER;

extern uint32_t  OS_CompAbsDelayedTime( uint32_t milli_second , struct timespec * tm);

#define SEM_BLOCKED_ADD(sem_id) \
        CRITICAL( (OS_bin_sem_table[sem_id].blocked++) );

#define SEM_BLOCKED_DEL(sem_id) \
        CRITICAL( (OS_bin_sem_table[sem_id].blocked--) );

#define _IS_BINSEM_INIT()   \
{   \
    if( !_binsem_is_init ) \
    { \
        _os_binsem_init(); \
        _binsem_is_init = 1; \
    } \
}
#define _CHECK_BINSEM_INIT()  (_IS_BINSEM_INIT())

static uint8_t _binsem_is_init = 0;


static void _os_binsem_init(void)
{
    int i;

    STATS_INIT_BINSEM();

    /* Initialize Binary Semaphore Table */

    for(i = 0; i < OS_MAX_BIN_SEMAPHORES; i++)
    {
        OS_bin_sem_table[i].free        = TRUE;
        OS_bin_sem_table[i].mul_Creator     = FALSE;
        OS_bin_sem_table[i].blocked     = 0;
    }

    INIT_THREAD_MUTEX();

    return;

}

/****************************************************************************************
  SEMAPHORE API
 ****************************************************************************************/


int OS_BinSemCreate (uint32_t *sem_id, 
        uint32_t sem_initial_value,
        uint32_t options)
{
    UNUSED(options);
    uint32_t possible_semid;
    int Status;

    _CHECK_BINSEM_INIT();

    if ( (sem_id == NULL) )
    {
        os_return_minus_one_and_set_errno(OS_STATUS_EINVAL);
    }

    /* Check Parameters */

    WLOCK();  
    {

        for (possible_semid = 0; possible_semid < OS_MAX_BIN_SEMAPHORES; possible_semid++)
        {
            if (OS_bin_sem_table[possible_semid].free == TRUE)    
                break;
        }



        if((possible_semid >= OS_MAX_BIN_SEMAPHORES) ||  
                (OS_bin_sem_table[possible_semid].free != TRUE))
        {
            WUNLOCK();
            os_return_minus_one_and_set_errno(OS_STATUS_NO_FREE_IDS);
        }

        /* Set the ID to be taken so another task doesn't try to grab it */
        OS_bin_sem_table[possible_semid].free = FALSE;
    }

    WUNLOCK();

    /*
     ** Create semaphore
     */

    errno = 0;


    Status = sem_init( &(OS_bin_sem_table[possible_semid].id) ,0 , sem_initial_value);

    if( Status == -1 )
    {
        /* Since the call failed, set the free flag back to true */
        CRITICAL( (OS_bin_sem_table[possible_semid].free = TRUE) );

        DEBUG("Error Creating semaphore in OS_BinSemCreate! errno = %d\n", errno);
        os_return_minus_one_and_set_errno(OS_STATUS_EERR);
    }


    *sem_id = possible_semid;

    WLOCK();  
    {
        OS_bin_sem_table[*sem_id].mul_Creator = OS_TaskGetId();

        /*  Stats   */
        STATS_CREAT_BINSEM();
    }
    WUNLOCK();

    return 0;
}/* end OS_BinSemCreate */

int OS_BinSemDelete (uint32_t sem_id)
{
    _CHECK_BINSEM_INIT();

#if defined (CONFIG_OS_STATIC_RESOURCE_ALLOCATION)
    os_return_minus_one_and_set_errno(OS_STATUS_EERR);
#else
    /* Check to see if this sem_id is valid */
    if (sem_id >= OS_MAX_BIN_SEMAPHORES || OS_bin_sem_table[sem_id].free == TRUE)
        os_return_minus_one_and_set_errno(OS_STATUS_EINVAL);



    if (sem_destroy( &(OS_bin_sem_table[sem_id].id)) != 0) /* 0 = success */ 
    {
        os_return_minus_one_and_set_errno(OS_STATUS_SEM_FAILURE);
    }

    /* Remove the Id from the table, and its name, so that it cannot be found again */

    WLOCK();  
    {
        OS_bin_sem_table[sem_id].free = TRUE;
        OS_bin_sem_table[sem_id].mul_Creator = UNINITIALIZED;
        OS_bin_sem_table[sem_id].blocked = 0;

        /*  Stats   */
        STATS_DEL_BINSEM();
    }
    WUNLOCK();

    return 0;
#endif

}/* end OS_BinSemDelete */


int OS_BinSemGive ( uint32_t sem_id )
{
    int32_t    ret;

    _CHECK_BINSEM_INIT();

    /* Check Parameters */

    if(sem_id >= OS_MAX_BIN_SEMAPHORES || OS_bin_sem_table[sem_id].free == TRUE)
        os_return_minus_one_and_set_errno(OS_STATUS_EINVAL);

    ret = sem_post(&(OS_bin_sem_table[sem_id].id));

    if ( ret != 0 )
    {
        os_return_minus_one_and_set_errno(OS_STATUS_SEM_FAILURE);
    }

    return 0;
}/* end OS_BinSemGive */

int OS_BinSemFlush (uint32_t sem_id)
{
    int32_t    ret = 0;
    uint32_t i;

    _CHECK_BINSEM_INIT();

    /* Check Parameters */
    if(sem_id >= OS_MAX_BIN_SEMAPHORES || OS_bin_sem_table[sem_id].free == TRUE)
        os_return_minus_one_and_set_errno(OS_STATUS_EINVAL);


    /*  This is done to perform the OS_BinSemFlush() operation properly */
    pthread_mutex_lock(&m_flush);
    for (i = 0; i < OS_bin_sem_table[sem_id].blocked; i++)
    {
        if( (ret = sem_post(&(OS_bin_sem_table[sem_id].id))) < 0 )
            break;
    }

    /*  This is done to perform the OS_BinSemFlush() operation properly */
    pthread_mutex_unlock(&m_flush);
    if ( ret < 0 )
    {
        os_return_minus_one_and_set_errno(OS_STATUS_SEM_FAILURE);
    }

    return 0;

}/* end OS_BinSemFlush */

int OS_BinSemTake ( uint32_t sem_id )
{
    int    ret;

    _CHECK_BINSEM_INIT();

    if(sem_id >= OS_MAX_BIN_SEMAPHORES  || OS_bin_sem_table[sem_id].free == TRUE)
        os_return_minus_one_and_set_errno(OS_STATUS_EINVAL);

    SEM_BLOCKED_ADD(sem_id);
    while( (ret = sem_wait(&(OS_bin_sem_table[sem_id].id))) == -1 )
    {
        if( errno != EINTR )    break;
        else 
        {
            /*  Restart if interrupted by a signal  */
//            perror("sem_wait");
            continue;
        }
    }

    /*  This is done to perform the OS_BinSemFlush() operation properly */
    pthread_mutex_lock(&m_flush);
    pthread_mutex_unlock(&m_flush);

    SEM_BLOCKED_DEL(sem_id);
    if ( ret != 0 )
    {
        os_return_minus_one_and_set_errno(OS_STATUS_SEM_FAILURE);
    }

    return 0;
}/* end OS_BinSemTake */

int OS_BinSemTryTake (uint32_t sem_id)
{
    _CHECK_BINSEM_INIT();

    /* Check Parameters */

    if(sem_id >= OS_MAX_BIN_SEMAPHORES  || OS_bin_sem_table[sem_id].free == TRUE)
        os_return_minus_one_and_set_errno(OS_STATUS_EINVAL);

    /* Note to self: Check out sem wait in the manual */
    if ( sem_trywait(&(OS_bin_sem_table[sem_id].id)) != 0)
    {
        os_return_minus_one_and_set_errno(OS_STATUS_SEM_FAILURE);
    }

    return 0;

}/* end OS_BinSemTryTake */

int OS_BinSemTimedWait ( uint32_t sem_id, uint32_t msecs )
{
    uint32_t           ret_val ;
    struct timespec  temp_timespec ;
    int timeloop;


    _CHECK_BINSEM_INIT();

    if( (sem_id >= OS_MAX_BIN_SEMAPHORES) || (OS_bin_sem_table[sem_id].free == TRUE) )
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
        if ( (status = sem_trywait(&(OS_bin_sem_table[sem_id].id))) == -1 && errno == EAGAIN)
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
            if( status == -1 )
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

int OS_BinSemGetInfo (uint32_t sem_id, OS_bin_sem_prop_t *bin_prop)  
{
    _CHECK_BINSEM_INIT();

    /* Check to see that the id given is valid */

    if (sem_id >= OS_MAX_BIN_SEMAPHORES || OS_bin_sem_table[sem_id].free == TRUE)
        os_return_minus_one_and_set_errno(OS_STATUS_EINVAL);

    if (bin_prop == NULL)
        os_return_minus_one_and_set_errno(OS_STATUS_EINVAL);

    /* put the info into the stucture */
    CRITICAL( (bin_prop ->mul_Creator =    OS_bin_sem_table[sem_id].mul_Creator) );


    return 0;

} /* end OS_BinSemGetInfo */


