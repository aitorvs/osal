/**
 *  \file   ossem.c
 *  \brief  This file implements the Binary Semaphore API of the OSAL library
 *  for the Linux operating system
 *
 *  This API allows the user the creation of Binary semaphores. Bin semaphores
 *  get values '0'/'1' (close/open). The API provides the neccessary functions
 *  to perform create, take, give, flush, etc. operations on a semaphore.
 *
 *
 *  \author  Aitor Viana Sanchez (avs), Aitor.Viana.Sanchez@esa.int
 *
 *  \internal
 *    Created:  19/02/09
 *   Revision:  $Id: ossem.c 1.4 19/02/09 avs Exp $
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

#define CRITICAL(x) \
    WLOCK();    \
    {   \
        x;  \
    }   \
    WUNLOCK();

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

/********************************* FILE CLASSES/STRUCTURES */

/* Binary Semaphores */
typedef struct
{
    int free;
    rtems_id id;
    int mul_Creator;
}OS_bin_sem_record_t;

/********************************* FILE PRIVATE VARIABLES  */

LOCAL OS_bin_sem_record_t   OS_bin_sem_table    [OS_MAX_BIN_SEMAPHORES];

/** this is the Initial name for the semaphores  */
LOCAL char nsem_name[] = "0000";

static void _os_binsem_init(void)
{
    int i;

    STATS_INIT_BINSEM();

    /* Initialize Binary Semaphore Table */

    for(i = 0; i < OS_MAX_BIN_SEMAPHORES; i++)
    {
        OS_bin_sem_table[i].free        = TRUE;
        OS_bin_sem_table[i].id          = UNINITIALIZED;
        OS_bin_sem_table[i].mul_Creator     = UNINITIALIZED;
    }

    INIT_THREAD_MUTEX();

}

/********************************* PUBLIC  INTERFACE    */

/****************************************************************************************
  SEMAPHORE API
 ****************************************************************************************/

/* 
 * ===  FUNCTION  ======================================================================
 *         Name:  OS_BinSemCreate
 *  Description:  This function creates a new binary semaphore.
 *  Parameters:
 *      - sem_name:         This parameter must be NULL.
 *      - sem_id:           This is the pointer where the created 'sem_id' will be
 *      returned.
 *      - sem_initial_value: This is the initial value of the semaphore
 *      - options:          This parameters is not used
 *  Return:
 *      0 when the call success
 *      OS_STATUS_EINVAL when there is some pointer parameter not valid
 *      OS_STATUS_NAME_TOO_LONG when the semaphore name is too long
 *      OS_STATUS_NO_FREE_IDS when no more semaphores can be created
 *      OS_STATUS_NAME_TAKEN when the semaphore name has been taken already
 *      OS_STATUS_SEM_FAILURE when any other error occurs
 * =====================================================================================
 */
int OS_BinSemCreate (
        uint32_t *sem_id, 
        uint32_t sem_initial_value,
        uint32_t options
        )
{
    ASSERT(sem_id);

    UNUSED(options);

    _CHECK_BINSEM_INIT();

    /* the current candidate for the new sem id */
    rtems_status_code return_code = 0;
    rtems_name r_name;
    uint32_t possible_semid;

    if (sem_id == NULL )
        os_return_minus_one_and_set_errno(OS_STATUS_EINVAL);

    /* we don't want to allow names too long*/
    /* if truncated, two names might be the same */


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
            WUNLOCK()
            os_return_minus_one_and_set_errno(OS_STATUS_NO_FREE_IDS);
        }

        OS_bin_sem_table[possible_semid].free = FALSE;
    }
    WUNLOCK()

    /* Create RTEMS Semaphore */
    NEXT_RESOURCE_NAME(nsem_name[0],nsem_name[1],nsem_name[2],nsem_name[3]);
    r_name = rtems_build_name(nsem_name[0],nsem_name[1],nsem_name[2],nsem_name[3]);

    return_code = rtems_semaphore_create( r_name, sem_initial_value,0,
            RTEMS_SIMPLE_BINARY_SEMAPHORE,
            &(OS_bin_sem_table[possible_semid].id));

    /* check if Create failed */
    if ( return_code != RTEMS_SUCCESSFUL )
    {
        WLOCK();
        {
            OS_bin_sem_table[possible_semid].free = TRUE;
        }
        WUNLOCK()
        os_return_minus_one_and_set_errno(OS_STATUS_SEM_FAILURE);
    }
    /* Set the sem_id to the one that we found available */
    /* Set the name of the semaphore,mul_Creator and free as well */

    *sem_id = possible_semid;

    WLOCK();
    {
        OS_bin_sem_table[*sem_id].free = FALSE;
        OS_bin_sem_table[*sem_id].mul_Creator = OS_TaskGetId();

        /*  Stats   */
        STATS_CREAT_BINSEM();
    }
    WUNLOCK();

    return 0;

}/* end OS_BinSemCreate */

/* 
 * ===  FUNCTION  ======================================================================
 *         Name:  OS_BinSemDelete
 *  Description:  This function deletes the given 'sem_id' semaphore
 *  Parameters:
 *      - sem_id:   sempahore identifier
 *  Return:
 *      0          when the call success
 *      OS_STATUS_EINVAL   when the semaphore identifier is not valid
 *      OS_STATUS_SEM_FAILURE      when the call fails
 *
 * NOTE: This function is only allow in the non-LOCAL resource allocation mode,
 * which can be seleceted during OSAL configuraiton.
 * =====================================================================================
 */
int OS_BinSemDelete (uint32_t sem_id)
{
    _CHECK_BINSEM_INIT();

#if defined (CONFIG_OS_STATIC_RESOURCE_ALLOCATION)
    os_return_minus_one_and_set_errno(OS_STATUS_EERR);
#else
    /* Check to see if this sem_id is valid */
    if (sem_id >= OS_MAX_BIN_SEMAPHORES || OS_bin_sem_table[sem_id].free == TRUE)
        os_return_minus_one_and_set_errno(OS_STATUS_EINVAL);

    /* we must make sure the semaphore is given  to delete it */
    rtems_semaphore_release(OS_bin_sem_table[sem_id].id);

    if (rtems_semaphore_delete( OS_bin_sem_table[sem_id].id) != RTEMS_SUCCESSFUL) 
    {
        os_return_minus_one_and_set_errno(OS_STATUS_SEM_FAILURE);
    }

    /* Remove the Id from the table, and its name, so that it cannot be found again */
    WLOCK();
    {
        OS_bin_sem_table[sem_id].free = TRUE;
        OS_bin_sem_table[sem_id].mul_Creator = UNINITIALIZED;
        OS_bin_sem_table[sem_id].id = UNINITIALIZED;

        /*  Stats   */
        STATS_DEL_BINSEM();
    }
    WUNLOCK()

    return 0;
#endif

}/* end OS_BinSemDelete */

/* 
 * ===  FUNCTION  ======================================================================
 *         Name:  OS_BinSemGive
 *  Description:  This function releases the given 'sem_id' semaphore
 *  Parameters:
 *      - sem_id:   sempahore identifier
 *  Return:
 *      0          when the call success
 *      OS_STATUS_EINVAL   when the semaphore identifier is not valid
 *      OS_STATUS_SEM_FAILURE      when the call fails
 * =====================================================================================
 */
int OS_BinSemGive (uint32_t sem_id)
{
    _CHECK_BINSEM_INIT();

    /* Check Parameters */

    if(sem_id >= OS_MAX_BIN_SEMAPHORES || OS_bin_sem_table[sem_id].free == TRUE)
        os_return_minus_one_and_set_errno(OS_STATUS_EINVAL);

    /* Give Semaphore */

    if( rtems_semaphore_release(OS_bin_sem_table[sem_id].id) != RTEMS_SUCCESSFUL)
    {
        os_return_minus_one_and_set_errno(OS_STATUS_SEM_FAILURE) ;
    }
    else
    {
        return  0 ;
    }

}/* end OS_BinSemGive */

/* 
 * ===  FUNCTION  ======================================================================
 *         Name:  OS_BinSemflush
 *  Description:  This funcion unblocks all tasks waiting on the given
 *  semaphore 'sem_id'
 *  Parameters:
 *      - sem_id:   sempahore identifier
 *  Return:
 *      0          when the call success
 *      OS_STATUS_EINVAL   when the semaphore identifier is not valid
 *      OS_STATUS_SEM_FAILURE      when the call fails
 * =====================================================================================
 */
int OS_BinSemFlush (uint32_t sem_id)
{
    _CHECK_BINSEM_INIT();

    /* Check Parameters */

    if(sem_id >= OS_MAX_BIN_SEMAPHORES || OS_bin_sem_table[sem_id].free == TRUE)
        os_return_minus_one_and_set_errno(OS_STATUS_EINVAL);

    /* Give Semaphore */

    if( rtems_semaphore_flush(OS_bin_sem_table[sem_id].id) != RTEMS_SUCCESSFUL)
    {
        os_return_minus_one_and_set_errno(OS_STATUS_SEM_FAILURE) ;
    }
    else
    {
        return  0 ;
    }

}/* end OS_BinSemFlush */

/* 
 * ===  FUNCTION  ======================================================================
 *         Name:  OS_BinSemTake
 *  Description:  This function acquires a sempahore identified by 'sem_id'.
 *  Parameters:
 *      - sem_id:   sempahore identifier
 *  Return:
 *      0              when success
 *      OS_STATUS_EINVAL       when the sempahore identifier is not valid
 *      OS_STATUS_SEM_NOT_AVAIL    when the sempahore is already taken
 *      OS_STATUS_SEM_FAILURE          when any other error occurs
 * =====================================================================================
 */
int OS_BinSemTake (uint32_t sem_id)
{
    rtems_status_code status;

    _CHECK_BINSEM_INIT();

    /* Check Parameters */

    if(sem_id >= OS_MAX_BIN_SEMAPHORES  || OS_bin_sem_table[sem_id].free == TRUE)
        os_return_minus_one_and_set_errno(OS_STATUS_EINVAL);

    status = rtems_semaphore_obtain(
            OS_bin_sem_table[sem_id].id, RTEMS_WAIT, RTEMS_NO_TIMEOUT);

    /* Note to self: Check out sem wait in the manual */
    if ( RTEMS_SUCCESSFUL == status )
    {
        return 0;
    }
    else if( RTEMS_UNSATISFIED == status )
    {
        os_return_minus_one_and_set_errno(OS_STATUS_SEM_NOT_AVAIL);
    }


    os_return_minus_one_and_set_errno(OS_STATUS_SEM_FAILURE);

}/* end OS_BinSemTake */

/* 
 * ===  FUNCTION  ======================================================================
 *         Name:  OS_BinSemTryTake
 *  Description:  This function tries to acquire a semaphore specified by the
 *  sempahore identifier.
 *  Parameters:
 *      - sem_id:   Sempahore identifier
 *  Return:
 *      0              when the sempahore is successfuly taken
 *      OS_STATUS_EINVAL       when the sempahore identifier is not valid
 *      OS_STATUS_SEM_NOT_AVAIL    when the sempahore is taken already
 *      OS_STATUS_SEM_FAILURE          when any other error occurs
 * =====================================================================================
 */
int OS_BinSemTryTake (uint32_t sem_id)
{
    rtems_status_code status;

    _CHECK_BINSEM_INIT();

    /* Check Parameters */

    if(sem_id >= OS_MAX_BIN_SEMAPHORES  || OS_bin_sem_table[sem_id].free == TRUE)
        os_return_minus_one_and_set_errno(OS_STATUS_EINVAL);

    /* Note to self: Check out sem wait in the manual */
    status = rtems_semaphore_obtain(
            OS_bin_sem_table[sem_id].id, RTEMS_NO_WAIT, RTEMS_NO_TIMEOUT);

    if ( RTEMS_SUCCESSFUL == status )
    {
        return 0;
    }
    else if( RTEMS_UNSATISFIED == status )
    {
        /* semaphore taken already  */
        os_return_minus_one_and_set_errno(OS_STATUS_SEM_NOT_AVAIL);
    }

    /*  Error   */
    os_return_minus_one_and_set_errno(OS_STATUS_SEM_FAILURE);

}/* end OS_BinSemTryTake */

/* 
 * ===  FUNCTION  ======================================================================
 *         Name:  OS_BinSemTimedWait
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
int OS_BinSemTimedWait (uint32_t sem_id, uint32_t msecs)
{
    /* msecs rounded to the closest system tick count */
    rtems_status_code ret_val ;
    uint32_t TimeInTicks;

    _CHECK_BINSEM_INIT();

    /* Check Parameters */

    if( (sem_id >= OS_MAX_BIN_SEMAPHORES) || (OS_bin_sem_table[sem_id].free == TRUE) )
        os_return_minus_one_and_set_errno(OS_STATUS_EINVAL);	

    TimeInTicks = msecs * OS_TICKS_PER_SECOND/1000;

    ret_val  =  
        rtems_semaphore_obtain(OS_bin_sem_table[sem_id].id, RTEMS_WAIT,TimeInTicks ) ;

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
}/* end OS_BinSemTimedWait */


/* 
 * ===  FUNCTION  ======================================================================
 *         Name:  OS_BinSemGetInfo
 *  Description:  The function retrieves the information related to the given
 *  'sem_id' semaphore identifier.
 *  Parameters:
 *      - sem_id:   is the semaphore identifier
 *      - bin_prop: semaphore info. being returned
 *  Return:
 *      0 when the function success
 *      OS_STATUS_EINVAL when the 'sem_id' is wrong
 *      OS_STATUS_EINVAL when the parameters passed are wrong
 * =====================================================================================
 */
int OS_BinSemGetInfo (uint32_t sem_id, OS_bin_sem_prop_t *bin_prop)
{
    ASSERT(bin_prop);

    _CHECK_BINSEM_INIT();

    /* Check to see that the id given is valid */
    if (sem_id >= OS_MAX_BIN_SEMAPHORES || OS_bin_sem_table[sem_id].free == TRUE)
        os_return_minus_one_and_set_errno(OS_STATUS_EINVAL);

    if (bin_prop == NULL)
        os_return_minus_one_and_set_errno(OS_STATUS_EINVAL);

    /* put the info into the stucture */
    WLOCK();
    {
        bin_prop ->mul_Creator =    OS_bin_sem_table[sem_id].mul_Creator;
    }
    WUNLOCK()

    return 0;

} /* end OS_BinSemGetInfo */

