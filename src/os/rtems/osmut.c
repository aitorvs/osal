/**
 *  \file   osmut.c
 *  \brief  This file implements the Mutual Exclusion semaphore interface of
 *  the OSAL library for the RTEMS operating System
 *
 *  \author  Aitor Viana Sanchez (avs), Aitor.Viana.Sanchez@esa.int
 *
 *  \internal
 *    Created:  19/02/09
 *   Revision:  $Id: osmut.c 1.4 19/02/09 avs Exp $
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
        int ret;    \
        ret = lock_rw_init(&_rwlock);   \
        ASSERT( ret >= 0 ); \
    }while(0);


#define WLOCK()   __WLOCK()
#define WUNLOCK() __WUNLOCK()

/** This macros allow to lock and unlock RTEMS semaphores   */
#define RTEMS_LOCK(id)                          rtems_semaphore_obtain(id, RTEMS_WAIT, RTEMS_NO_TIMEOUT)
#define RTEMS_TRYLOCK(id)                       rtems_semaphore_obtain(id, RTEMS_NO_WAIT, RTEMS_NO_TIMEOUT)
#define RTEMS_UNLOCK(id)                        rtems_semaphore_release(id)
#define RTEMS_INHERIT_MUTEX_CREATE(name, id)    rtems_semaphore_create( name, 1, 0, RTEMS_BINARY_SEMAPHORE|RTEMS_INHERIT_PRIORITY, &id)
#define RTEMS_CEILING_MUTEX_CREATE(name, id)    rtems_semaphore_create( name, 1, 0, RTEMS_BINARY_SEMAPHORE|RTEMS_PRIORITY_CEILING, &id)
#define RTEMS_SEMAPHORE_DELETE(id)              rtems_semaphore_delete(id)


/********************************* FILE CLASSES/STRUCTURES */

/* Mutexes */
typedef struct
{
    int free;
    rtems_id id;
    int mul_Creator;
}OS_mut_sem_record_t;


/********************************* FILE PRIVATE VARIABLES  */

/** this is the Initial name for the mutex  */
LOCAL char nmut_name[] = "0000";
LOCAL OS_mut_sem_record_t   OS_mut_sem_table    [OS_MAX_MUTEXES];


/********************************* PRIVATE INTERFACE    */


/********************************* PUBLIC  INTERFACE    */

int OS_MutSemInit(void)
{
    int i;

    STATS_INIT_MUTSEM();

    /* Initialize Mutex Semaphore Table */

    for(i = 0; i < OS_MAX_MUTEXES; i++)
    {
        OS_mut_sem_table[i].free        = TRUE;
        OS_mut_sem_table[i].id          = UNINITIALIZED;
        OS_mut_sem_table[i].mul_Creator     = UNINITIALIZED;
    }

    INIT_THREAD_MUTEX();

    return 0;
}

/****************************************************************************************
  MUTEX API
 ****************************************************************************************/

/* 
 * ===  FUNCTION  ======================================================================
 *         Name:  OS_MutSemCreate
 *  Description:  This function creates a mutex object.
 *  The mutex object is created according with the priority ceiling protocol
 *  Parameters:
 *      - sem_id:   mutex object identifier
 *      - sem_name: This parameter must be NULL.
 *      - options:  not used
 *  Return:
 *      0 when the call success
 *      OS_STATUS_EINVAL when any of the pointer parameters are not valid.
 *      OS_STATUS_NAME_TOO_LONG when the mutex name is too long
 *      OS_STATUS_NO_FREE_IDS when there are no more resources to create another
 *      mutex
 *      OS_STATUS_NAME_TAKEN when the mutex name is already being use
 *      OS_STATUS_SEM_FAILURE when the OS call fails creating the mutex
 * =====================================================================================
 */
int OS_MutSemCreate (
        uint32_t *sem_id, 
        uint32_t options)
{
    UNUSED(options);
    ASSERT(sem_id);

    rtems_status_code   return_code;
    rtems_name          r_name;
    uint32_t				possible_semid;

    /* Check Parameters */

    if (sem_id == NULL )
        os_return_minus_one_and_set_errno(OS_STATUS_EINVAL);

    /* we don't want to allow names too long*/
    /* if truncated, two names might be the same */

    WLOCK();
    {
        for (possible_semid = 0; possible_semid < OS_MAX_MUTEXES; possible_semid++)
        {
            if (OS_mut_sem_table[possible_semid].free == TRUE)    
                break;
        }

        if( (possible_semid >= OS_MAX_MUTEXES) ||
                (OS_mut_sem_table[possible_semid].free != TRUE) )
        {
            WUNLOCK();
            os_return_minus_one_and_set_errno(OS_STATUS_NO_FREE_IDS);
        }

        OS_mut_sem_table[possible_semid].free = FALSE;
    }
    WUNLOCK();


    /* 
     ** upon successful initialization, the state of the mutex becomes initialized and ulocked 
     */
    NEXT_RESOURCE_NAME(nmut_name[0],nmut_name[1],nmut_name[2],nmut_name[3]);
    r_name = rtems_build_name(nmut_name[0],nmut_name[1],nmut_name[2],nmut_name[3]);
/*    return_code = RTEMS_INHERIT_MUTEX_CREATE(r_name, OS_mut_sem_table[possible_semid].id);    */
    return_code = RTEMS_CEILING_MUTEX_CREATE(r_name, OS_mut_sem_table[possible_semid].id);
    if ( return_code != RTEMS_SUCCESSFUL )
    { 
        WLOCK();
        {
            OS_mut_sem_table[possible_semid].free = TRUE;
        }
        WUNLOCK();
        os_return_minus_one_and_set_errno(OS_STATUS_SEM_FAILURE);
    }    

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

}/* end OS_MutSemCreate */


/* 
 * ===  FUNCTION  ======================================================================
 *         Name:  OS_MutSemDelete
 *  Description:  This function removes the mutex object represented by 'sem_id'
 *  Parameters:
 *      - sem_id:   mutex identifier
 *  Returns:
 *      0 when the call success
 *      OS_STATUS_EINVAL when the mutex identifier is not valid
 *      OS_STATUS_SEM_FAILURE when the OS call fails deleteing the mutex
 *
 * NOTE: This function is only allow in the non-LOCAL resource allocation mode,
 * which can be seleceted during OSAL configuraiton.
 * =====================================================================================
 */
int OS_MutSemDelete (uint32_t sem_id)
{
#if defined (CONFIG_OS_STATIC_RESOURCE_ALLOCATION)
    os_return_minus_one_and_set_errno(OS_STATUS_EERR);
#else
    rtems_status_code status;

    /* Check to see if this sem_id is valid   */
    if (sem_id >= OS_MAX_MUTEXES || OS_mut_sem_table[sem_id].free == TRUE)
        os_return_minus_one_and_set_errno(OS_STATUS_EINVAL);


    /* we must make sure the mutex is given  to delete it */
    RTEMS_UNLOCK(OS_mut_sem_table[sem_id].id);


    status = RTEMS_SEMAPHORE_DELETE(OS_mut_sem_table[sem_id].id); 
    ASSERT(status == RTEMS_SUCCESSFUL);

    if( status != RTEMS_SUCCESSFUL)
        os_return_minus_one_and_set_errno(OS_STATUS_SEM_FAILURE);

    /* Delete its presence in the table */

    WLOCK();
    {
        OS_mut_sem_table[sem_id].free = TRUE;
        OS_mut_sem_table[sem_id].id = UNINITIALIZED;
        OS_mut_sem_table[sem_id].mul_Creator = UNINITIALIZED;

        /*  Stats   */
        STATS_DEL_MUTSEM();
    }
    WUNLOCK();


    return 0;
#endif

}/* end OS_MutSemDelete */

/* 
 * ===  FUNCTION  ======================================================================
 *         Name:  OS_MutSemGive
 *  Description:  This function releases the mutex object referenced by
 *  'sem_id'.
 *  Parameters:
 *      - sem_id:   mutex identifier
 *  Returns:
 *      0 when the call success
 *      OS_STATUS_EINVAL when the 'sem_id' is not valid
 *      OS_STATUS_SEM_FAILURE when the OS call fails while signaling the mutex
 * =====================================================================================
 */
int OS_MutSemGive (uint32_t sem_id)
{
    /* Check Parameters */

    if(sem_id >= OS_MAX_MUTEXES || OS_mut_sem_table[sem_id].free == TRUE)
        os_return_minus_one_and_set_errno(OS_STATUS_EINVAL);

    if(RTEMS_UNLOCK(OS_mut_sem_table[sem_id].id) != RTEMS_SUCCESSFUL)
    {
        os_return_minus_one_and_set_errno(OS_STATUS_SEM_FAILURE) ;
    }
    else
    {
        return 0 ;
    }

}/* end OS_MutSemGive */

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
int OS_MutSemTake (uint32_t sem_id)
{
    /* Check Parameters */
    if(sem_id >= OS_MAX_MUTEXES || OS_mut_sem_table[sem_id].free == TRUE)
        os_return_minus_one_and_set_errno(OS_STATUS_EINVAL);

    if( RTEMS_LOCK(OS_mut_sem_table[sem_id].id) != RTEMS_SUCCESSFUL )
    {
        os_return_minus_one_and_set_errno(OS_STATUS_SEM_FAILURE) ;
    }
    else
    {
        return 0 ;
    }

}/* end OS_MutSemGive */

/* 
 * ===  FUNCTION  ======================================================================
 *         Name:  OS_MutSemTryTake
 *  Description:  This function tries to catch the mutex object referenced by
 *  'sem_id'. If the mutex is catched the function will return success. If the
 *  mutex was already taken by another thread, the function will return with an
 *  error code.
 *  Parameters:
 *      - sem_id:   mutex identifier
 *  Returns:
 *      0 when the call success
 *      OS_STATUS_EINVAL when the 'sem_id' is not valid
 *      OS_STATUS_SEM_NOT_AVAIL when the mutex was already taken by another thread
 *      OS_STATUS_SEM_FAILURE when the OS call fails while catching the mutex
 * =====================================================================================
 */
int OS_MutSemTryTake (uint32_t sem_id)
{
    rtems_status_code status;

    /* Check Parameters */
    if(sem_id >= OS_MAX_BIN_SEMAPHORES  || OS_mut_sem_table[sem_id].free == TRUE)
        os_return_minus_one_and_set_errno(OS_STATUS_EINVAL);

    status =  RTEMS_TRYLOCK(OS_mut_sem_table[sem_id].id);

    /* Note to self: Check out sem wait in the manual */
    if ( RTEMS_UNSATISFIED == status )
    {
        os_return_minus_one_and_set_errno(OS_STATUS_SEM_NOT_AVAIL);
    }
    else if( RTEMS_SUCCESSFUL == status )
    {
        return 0;
    }

    /*  Error   */
    os_return_minus_one_and_set_errno(OS_STATUS_SEM_FAILURE);

}/* end OS_MutSemTryTake */

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
 *      OS_STATUS_TIMEOUT          when the timeout expires
 *      OS_STATUS_SEM_FAILURE          when any other error occurs
 * =====================================================================================
 */
int OS_MutSemTimedWait (uint32_t sem_id, uint32_t msecs)
{
    /* msecs rounded to the closest system tick count */
    rtems_status_code ret_val ;
    uint32_t TimeInTicks;

    /* Check Parameters */

    if( (sem_id >= OS_MAX_MUTEXES) || (OS_mut_sem_table[sem_id].free == TRUE) )
        os_return_minus_one_and_set_errno(OS_STATUS_EINVAL);	

    TimeInTicks = msecs * OS_TICKS_PER_SECOND/1000;

    ret_val  =  
        rtems_semaphore_obtain(OS_mut_sem_table[sem_id].id, RTEMS_WAIT,TimeInTicks ) ;

    switch (ret_val)
    {
        case RTEMS_TIMEOUT :
            os_return_minus_one_and_set_errno(OS_STATUS_TIMEOUT);
            break ;

        case RTEMS_SUCCESSFUL :
            return 0;
            break ;

        default :
            os_return_minus_one_and_set_errno(OS_STATUS_SEM_FAILURE);
            break ;

    }
    os_return_minus_one_and_set_errno(OS_STATUS_EERR);
}/* end OS_MutSemTimedWait */


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
int OS_MutSemGetInfo (uint32_t sem_id, OS_mut_sem_prop_t *sem_prop)  
{
    /* Check to see that the id given is valid */

    if (sem_id >= OS_MAX_MUTEXES || OS_mut_sem_table[sem_id].free == TRUE)
        os_return_minus_one_and_set_errno(OS_STATUS_EINVAL);

    if (sem_prop == NULL)
        os_return_minus_one_and_set_errno(OS_STATUS_EINVAL);

    /* put the info into the stucture */    
    WLOCK();
    {
        sem_prop -> mul_Creator =   OS_mut_sem_table[sem_id].mul_Creator;
    }
    WUNLOCK();

    return 0;

} /* end OS_MutSemGetInfo */


