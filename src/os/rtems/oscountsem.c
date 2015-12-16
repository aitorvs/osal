/**
 *  \file   oscountsem.c
 *  \brief  This file implements the OSAL counter semaphore interface.
 *
 *  The OSAL coutern semaphore interface provides the library-user the avility
 *  to block in a semaphore couter.
 *  The library-user will get blocked when the counter associated to the
 *  semaphore is less than '0' value.
 *
 *  \author  Aitor Viana Sanchez (avs), Aitor.Viana.Sanchez@esa.int
 *
 *  \internal
 *    Created:  19/02/09
 *   Revision:  $Id: oscountsem.c 1.4 19/02/09 avs Exp $
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

/********************************* FILE CLASSES/STRUCTURES */

/* Counting Semaphores */
typedef struct
{
    int free;
    rtems_id id;
    int mul_Creator;
}OS_count_sem_record_t;


/********************************* FILE PRIVATE VARIABLES  */

/** this is the Initial name for the counting semaphores  */
LOCAL char ncsem_name[] = "0000";
LOCAL OS_count_sem_record_t OS_count_sem_table	[OS_MAX_COUNT_SEMAPHORES];

/********************************* PRIVATE INTERFACE    */

static void _os_countsem_init(void)
{
    int i;

    STATS_INIT_COUNTSEM();

    /* Initialize Counting Semaphores */
    for(i = 0; i < OS_MAX_COUNT_SEMAPHORES; i++)
    {
        OS_count_sem_table[i].free        = TRUE;
        OS_count_sem_table[i].id          = UNINITIALIZED;
        OS_count_sem_table[i].mul_Creator     = UNINITIALIZED;
    }

    INIT_THREAD_MUTEX();
}


/********************************* PUBLIC  INTERFACE    */

/****************************************************************************************
  COUNT SEMAPHORE API
 ****************************************************************************************/

/* 
 * ===  FUNCTION  ======================================================================
 *         Name:  OS_CountSemCreate
 *  Description:  This function creates a counting semaphore.
 *  Parameters:
 *      - sem_id:               This is the sempahore identifier (returned value)
 *      - sem_name:             This parameter must be NULL.
 *      - sem_initial_value:    This is the initial value for the semaphore
 *  Returns:
 *      0 when the call success
 *      OS_STATUS_EINVAL  when some of the pointer input parameters are not
 *      valid
 *      OS_STATUS_NAME_TOO_LONG when the semaphore name is to long
 *      OS_STATUS_NO_FREE_IDS when it is not possible to create more counting
 *      semaphores
 *      OS_STATUS_NAME_TAKEN when the semaphore name is already taken by another
 *      sempahore
 *      OS_STATUS_SEM_FAILURE when an error in the semaphore creation occurs
 * =====================================================================================
 */
int OS_CountSemCreate (
        uint32_t *sem_id, 
        uint32_t sem_initial_value, 
        uint32_t options)
{
    UNUSED(options);
    /* the current candidate for the new sem id */
    rtems_status_code return_code = 0;
    rtems_name r_name;
    uint32_t possible_semid;

    _CHECK_COUNTSEM_INIT();

    if (sem_id == NULL )
        os_return_minus_one_and_set_errno(OS_STATUS_EINVAL);

    /* we don't want to allow names too long*/
    /* if truncated, two names might be the same */

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

        OS_count_sem_table[possible_semid].free = FALSE;
    }
    WUNLOCK();

    /* Create RTEMS Semaphore */
    NEXT_RESOURCE_NAME(ncsem_name[0],ncsem_name[1],ncsem_name[2],ncsem_name[3]);
    r_name = rtems_build_name(ncsem_name[0],ncsem_name[1],ncsem_name[2],ncsem_name[3]);

    return_code = rtems_semaphore_create( r_name, sem_initial_value,0,
            RTEMS_COUNTING_SEMAPHORE,
            &(OS_count_sem_table[possible_semid].id));

    /* check if Create failed */
    if ( return_code != RTEMS_SUCCESSFUL )
    {        
        CRITICAL( (OS_count_sem_table[possible_semid].free = TRUE) );

        os_return_minus_one_and_set_errno(OS_STATUS_SEM_FAILURE);
    }
    /* Set the sem_id to the one that we found available */
    /* Set the name of the semaphore,mul_Creator and free as well */

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

/* 
 * ===  FUNCTION  ======================================================================
 *         Name:  OS_CoundSemDelete
 *  Description:  Deletes the specified Counting Semaphore.
 *  Parameters:
 *      - sem_id:   semaphore identifier.
 *  Return:
 *      0 if success
 *      OS_STATUS_EINVAL if the id passed in is not a valid countary semaphore
 *      OS_STATUS_SEM_FAILURE the OS call failed
 *
Notes: Since we can't delete a semaphore which is currently locked by some task 
(as it may ber crucial to completing the task), the semaphore must be full to
allow deletion.
 *
 * NOTE: This function is only allow in the non-LOCAL resource allocation mode,
 * which can be seleceted during OSAL configuraiton.
 * =====================================================================================
 */
int OS_CountSemDelete (uint32_t sem_id)
{
    _CHECK_COUNTSEM_INIT();

#if defined (CONFIG_OS_STATIC_RESOURCE_ALLOCATION)
    os_return_minus_one_and_set_errno(OS_STATUS_EERR);
#else
    rtems_status_code rtems_ret = 0;

    /* Check to see if this sem_id is valid */
    if (sem_id >= OS_MAX_COUNT_SEMAPHORES || OS_count_sem_table[sem_id].free == TRUE)
        os_return_minus_one_and_set_errno(OS_STATUS_EINVAL);


    /* we must make sure the semaphore is given  to delete it */
    rtems_semaphore_release(OS_count_sem_table[sem_id].id);

    rtems_ret = rtems_semaphore_delete( OS_count_sem_table[sem_id].id);
    ASSERT(rtems_ret == RTEMS_SUCCESSFUL );
    if( rtems_ret != RTEMS_SUCCESSFUL )
    {
        os_return_minus_one_and_set_errno(OS_STATUS_SEM_FAILURE);
    }

    /* Remove the Id from the table, and its name, so that it cannot be found again */
    WLOCK();
    {
        OS_count_sem_table[sem_id].free = TRUE;
        OS_count_sem_table[sem_id].mul_Creator = UNINITIALIZED;
        OS_count_sem_table[sem_id].id = UNINITIALIZED;

        /*  Stats   */
        STATS_DEL_COUNTSEM();
    }
    WUNLOCK();

    return 0;
#endif

}/* end OS_CountSemDelete */

/* 
 * ===  FUNCTION  ======================================================================
 *         Name:  OS_CoundSemGive
 *  Description:  The function tries to catch the semaphore 'sem_id'
 *  Parameters:
 *      -sem_id:    semaphore identifier.
 *  Return:
 *      0 when the function success
 *      OS_STATUS_EINVAL when the 'sem_id' is not valid
 *      OS_STATUS_SEM_FAILURE when any other error has occurred
 * =====================================================================================
 */
int OS_CountSemGive (uint32_t sem_id)
{
    rtems_status_code rtems_ret = 0;

    _CHECK_COUNTSEM_INIT();

    /* Check Parameters */

    if(sem_id >= OS_MAX_COUNT_SEMAPHORES || OS_count_sem_table[sem_id].free == TRUE)
        os_return_minus_one_and_set_errno(OS_STATUS_EINVAL);

    /* Give Semaphore */

    rtems_ret = rtems_semaphore_release(OS_count_sem_table[sem_id].id);
    ASSERT(rtems_ret == RTEMS_SUCCESSFUL );
    switch( rtems_ret )
    {
        case RTEMS_NOT_OWNER_OF_RESOURCE:
        case RTEMS_INVALID_ID: os_return_minus_one_and_set_errno(OS_STATUS_EINVAL);
        case RTEMS_SUCCESSFUL: return 0;
        default: os_return_minus_one_and_set_errno(OS_STATUS_EERR);
    }

    return  OS_STATUS_EERR;

}/* end OS_CountSemGive */

/* 
 * ===  FUNCTION  ======================================================================
 *         Name:  OS_CoundSemTake
 *  Description:  The function tries to catch the semaphore 'sem_id'
 *  Parameters:
 *      -sem_id:    semaphore identifier.
 *  Return:
 *      0 when the function success
 *      OS_STATUS_EINVAL when the 'sem_id' is not valid
 *      OS_STATUS_SEM_FAILURE when any other error has occurred
 * =====================================================================================
 */
int OS_CountSemTake (uint32_t sem_id)
{
    rtems_status_code rtems_ret = 0;

    _CHECK_COUNTSEM_INIT();

    /* Check Parameters */
    if(sem_id >= OS_MAX_COUNT_SEMAPHORES  || OS_count_sem_table[sem_id].free == TRUE)
        os_return_minus_one_and_set_errno(OS_STATUS_EINVAL);

    /* Note to self: Check out sem wait in the manual */
    rtems_ret = rtems_semaphore_obtain(OS_count_sem_table[sem_id].id, RTEMS_WAIT, 
                RTEMS_NO_TIMEOUT);
    ASSERT(rtems_ret == RTEMS_SUCCESSFUL );
    switch( rtems_ret )
    {
        case RTEMS_SUCCESSFUL: return 0;
        case RTEMS_UNSATISFIED: os_return_minus_one_and_set_errno(OS_STATUS_SEM_NOT_AVAIL);
        case RTEMS_OBJECT_WAS_DELETED:
        case RTEMS_INVALID_ID: os_return_minus_one_and_set_errno(OS_STATUS_EINVAL);
        default: os_return_minus_one_and_set_errno(OS_STATUS_EERR);
    }

    os_return_minus_one_and_set_errno(OS_STATUS_EERR);

}/* end OS_CountSemTake */


/* 
 * ===  FUNCTION  ======================================================================
 *         Name:  OS_CountSemTimedWait
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
int OS_CountSemTimedWait (uint32_t sem_id, uint32_t msecs)
{
    /* msecs rounded to the closest system tick count */
    rtems_status_code rtems_ret ;
    uint32_t TimeInTicks;

    _CHECK_COUNTSEM_INIT();

    /* Check Parameters */

    if( (sem_id >= OS_MAX_COUNT_SEMAPHORES) || (OS_count_sem_table[sem_id].free == TRUE) )
        os_return_minus_one_and_set_errno(OS_STATUS_EINVAL);	

    TimeInTicks = msecs * OS_TICKS_PER_SECOND/1000;

    rtems_ret  =  
        rtems_semaphore_obtain(OS_count_sem_table[sem_id].id, RTEMS_WAIT,TimeInTicks ) ;

    switch (rtems_ret)
    {
        case RTEMS_TIMEOUT :    os_return_minus_one_and_set_errno(OS_STATUS_TIMEOUT);
        case RTEMS_SUCCESSFUL : return 0 ;
        default :               os_return_minus_one_and_set_errno(OS_STATUS_SEM_FAILURE) ;
    }

    os_return_minus_one_and_set_errno(OS_STATUS_EERR);

}/* end OS_CountSemTimedWait */

int OS_CountSemTryTake (uint32_t sem_id)
{
    rtems_status_code status;

    _CHECK_COUNTSEM_INIT();

    /* Check Parameters */

    if(sem_id >= OS_MAX_COUNT_SEMAPHORES  || OS_count_sem_table[sem_id].free == TRUE)
        os_return_minus_one_and_set_errno(OS_STATUS_EINVAL);

    /* Note to self: Check out sem wait in the manual */
    status = rtems_semaphore_obtain(
            OS_count_sem_table[sem_id].id, RTEMS_NO_WAIT, RTEMS_NO_TIMEOUT);

    switch (status)
    {
        case RTEMS_SUCCESSFUL : return 0 ;
        case RTEMS_UNSATISFIED: os_return_minus_one_and_set_errno(OS_STATUS_SEM_NOT_AVAIL);
        default :               os_return_minus_one_and_set_errno(OS_STATUS_SEM_FAILURE) ;
    }

    /*  Error   */
    os_return_minus_one_and_set_errno(OS_STATUS_EERR);

}/* end OS_CountSemTryTake */

/* 
 * ===  FUNCTION  ======================================================================
 *         Name:  OS_CountSemGetInfo
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
int OS_CountSemGetInfo (uint32_t sem_id, OS_count_sem_prop_t *count_prop)  
{
    _CHECK_COUNTSEM_INIT();

    /* Check to see that the id given is valid */

    if (sem_id >= OS_MAX_COUNT_SEMAPHORES || OS_count_sem_table[sem_id].free == TRUE)
        os_return_minus_one_and_set_errno(OS_STATUS_EINVAL);

    if (count_prop == NULL)
        os_return_minus_one_and_set_errno(OS_STATUS_EINVAL);

    /* put the info into the stucture */
    CRITICAL( (count_prop ->mul_Creator =    OS_count_sem_table[sem_id].mul_Creator) );

    return 0;

} /* end OS_CountSemGetInfo */



