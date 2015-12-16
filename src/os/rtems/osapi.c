/**
 *  \file   osapi.c
 *  \brief  This file implements the OSAL abstraction layer core file.
 *
 *  The file implements the main functionalities to allow the user initializing
 *  the OSAL library.
 *  Upon configuration the file also implementes log capabilities registering
 *  extension handlers in RTEMS operating system.
 *
 *  \author  Aitor Viana Sanchez (avs), Aitor.Viana.Sanchez@esa.int
 *
 *  \internal
 *    Created:  25/03/09
 *   Revision:  $Id: osapi.c 1.4 25/03/09 avs Exp $
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


/*
** File   :	osapi.c
**
** Author :	Ezra Yeheskeli
**
** Purpose: 
**	   This file  contains some of the OS APIs abstraction layer.It 
**     contains those APIs that call the  OS. In this case the OS is the Rtems OS.
**
*/

#include <stdio.h>
#include <sys/types.h>
#include <ctype.h>
#include <string.h>
#include <rtems.h>
#include <stdarg.h>

#include <osal/osapi.h>
#include <osal/osstats.h>
#include <osal/osdebug.h>

/*-----------------------------------------------------------------------------
 *  MACROS
 *-----------------------------------------------------------------------------*/


/*  LOCAL uint32_t  OS_CompAbsDelayedTime( uint32_t milli_second , struct timespec * tm);  */


/*****************************************************************************
  PRIVATE DATA
 *****************************************************************************/

LOCAL int is_init = FALSE;

uint32_t OS_TICKS_PER_SECOND = 0;

/*****************************************************************************
  PRIVATE INTERFACE
 *****************************************************************************/

extern int OS_TaskInit(void);
extern int OS_MutSemInit(void);

#ifdef RTEMS_INCLUDE_EXTENSION_HANDLERS
#include "oshandlers.c"
#endif


/* 
 * ===  FUNCTION  ======================================================================
 *         Name:  show_error_code
 *  Description:  This functions prints out the RTEMS error code
 * =====================================================================================
 */
void show_error_code (rtems_status_code return_status)
{
    printf ("Error code: ");
    switch (return_status)
    {
        case RTEMS_TASK_EXITTED:
            printf ("RTEMS_TASK_EXITTED");
            break;
        case RTEMS_MP_NOT_CONFIGURED:
            printf ("RTEMS_MP_NOT_CONFIGURED");
            break;
        case RTEMS_INVALID_NAME:
            printf ("RTEMS_INVALID_NAME");
            break;
        case RTEMS_INVALID_ID:
            printf ("RTEMS_INVALID_ID");
            break;
        case RTEMS_TOO_MANY:
            printf ("RTEMS_TOO_MANY");
            break;
        case RTEMS_TIMEOUT:
            printf ("RTEMS_TIMEOUT");
            break;
        case RTEMS_OBJECT_WAS_DELETED:
            printf ("RTEMS_OBJECT_WAS_DELETED");
            break;
        case RTEMS_INVALID_SIZE:
            printf ("RTEMS_INVALID_SIZE");
            break;
        case RTEMS_INVALID_ADDRESS:
            printf ("RTEMS_INVALID_ADDRESS");
            break;
        case RTEMS_INVALID_NUMBER:
            printf ("RTEMS_INVALID_NUMBER");
            break;
        case RTEMS_NOT_DEFINED:
            printf ("RTEMS_NOT_DEFINED");
            break;
        case RTEMS_RESOURCE_IN_USE:
            printf ("RTEMS_RESOURCE_IN_USE");
            break;
        case RTEMS_UNSATISFIED:
            printf ("RTEMS_UNSATISFIED");
            break;
        case RTEMS_INCORRECT_STATE:
            printf ("RTEMS_INCORRECT_STATE");
            break;
        case RTEMS_ALREADY_SUSPENDED:
            printf ("RTEMS_ALREADY_SUSPENDED");
            break;
        case RTEMS_ILLEGAL_ON_SELF:
            printf ("RTEMS_ILLEGAL_ON_SELF");
            break;
        case RTEMS_ILLEGAL_ON_REMOTE_OBJECT:
            printf ("RTEMS_ILLEGAL_ON_REMOTE_OBJECT");
            break;
        case RTEMS_CALLED_FROM_ISR:
            printf ("RTEMS_CALLED_FROM_ISR");
            break;
        case RTEMS_INVALID_PRIORITY:
            printf ("RTEMS_INVALID_PRIORITY");
            break;
        case RTEMS_INVALID_CLOCK:
            printf ("RTEMS_INVALID_CLOCK");
            break;
        case RTEMS_INVALID_NODE:
            printf ("RTEMS_INVALID_NODE");
            break;
        case RTEMS_NOT_CONFIGURED:
            printf ("RTEMS_NOT_CONFIGURED");
            break;
        case RTEMS_NOT_OWNER_OF_RESOURCE:
            printf ("RTEMS_NOT_OWNER_OF_RESOURCE");
            break;
        case RTEMS_NOT_IMPLEMENTED:
            printf ("RTEMS_NOT_IMPLEMENTED");
            break;
        case RTEMS_INTERNAL_ERROR:
            printf ("RTEMS_INTERNAL_ERROR");
            break;
        case RTEMS_NO_MEMORY:
            printf ("RTEMS_NO_MEMORY");
            break;
        case RTEMS_IO_ERROR:
            printf ("RTEMS_IO_ERROR");
            break;
        case RTEMS_PROXY_BLOCKING:
            printf ("RTEMS_PROXY_BLOCKING");
            break;
        default:
            printf ("UNKNOWN ERROR CODE");
    }
    printf ("\n");
}

#if 0
LOCAL uint32_t  OS_CompAbsDelayedTime( uint32_t milli_second , struct timespec * tm)
{
    int return_status ;
    
    /* get the current time */
    return_status = clock_gettime( CLOCK_REALTIME,  tm );
        
    /* add the delay to the current time */
    tm->tv_sec  += (time_t) (milli_second / 1000) ;
    /* convert residue ( milli seconds)  to nano second */
    tm->tv_nsec +=  (milli_second % 1000) * 1000000 ;
    
    if(tm->tv_nsec > 999999999 )
    {
        tm->tv_nsec -= 1000000000 ;
        tm->tv_sec ++ ;
    }
    /*  This must be an error because the unique value accepted in the
     *  tv_nsec field is the zero one
     */
    tm->tv_nsec = 0;
    
    return(0) ;
}
#endif

/*****************************************************************************
  PUBLIC INTERFACE
 *****************************************************************************/

/* 
 * ===  FUNCTION  ======================================================================
 *         Name:  OS_Start
 *  Description:  This function must be called at the end of the 'main'
 *  function. It starts the OS kernel.
 *  Return:
 *      - OS_STATUS_EERR if the function reaches the end
 * =====================================================================================
 */
int OS_Start(void)
{
//    rtems_task_priority old_pri;

    ASSERT(is_init);
    if(!is_init)	os_return_minus_one_and_set_errno(OS_STATUS_EERR);

//    status = rtems_task_set_priority(RTEMS_SELF, 254, &old_pri);
//    ASSERT( status == 0 );
//
//    sched_yield();

    /*  Print stats */
    osal_stats_print();
//    rtems_cpu_usage_reset();

//    status = rtems_task_delete(RTEMS_SELF);
//    ASSERT(status == RTEMS_SUCCESSFUL);
//    if (status != RTEMS_SUCCESSFUL)
//    {
//        os_return_minus_one_and_set_errno(OS_STATUS_EERR);
//    }
//    else 
//    {
//        return 0;
//    }
    return 0;
}

/* 
 * ===  FUNCTION  ======================================================================
 *         Name:  OS_Init
 *  Description:  Initialization of the OSAL library
 *  Parameters:
 *      none
 *  Returns:
 *      - 0 when the OSAL is successfuly initialized
 *      - OS_STATUS_EERR when an error in the initialization occurs
 * =====================================================================================
 */
int OS_Init(void)
{
    int ret;

    if(is_init)	return 0;

#ifdef RTEMS_INCLUDE_EXTENSION_HANDLERS
    if( rtems_init_extension_handlers() != 0 )
        os_return_minus_one_and_set_errno(OS_STATUS_EERR);
#endif

    /*  Get tick per second */
    ret = rtems_clock_get(RTEMS_CLOCK_GET_TICKS_PER_SECOND, (void*)&OS_TICKS_PER_SECOND);
    ASSERT( RTEMS_SUCCESSFUL == ret );

    /*  Init all the OSAL APIs  */
    /*  OS_MutSemInit() shall be called first because lock.h uses it    */
    if( (ret = OS_MutSemInit()) != 0 ) goto ret;
    if( (ret = OS_TaskInit()) != 0 ) goto ret;

    /*  flag the initialization of the OSAPI */
    is_init = TRUE;


ret:
    ASSERT( ret >= 0 );
    return ret;

} /* end OS_Init */




