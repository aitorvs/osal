/*
 * =====================================================================================
 *
 *       Filename:  osapi.c
 *
 *    Description:  This file  contains some of the OS APIs abstraction layer
 *    implementation for POSIX Linux / Mac OSx
 *
 *        Version:  1.0
 *        Created:  01/09/08 14:06:47
 *       Modified:  01/09/08 14:08:20
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  Aitor Viana Sanchez (avs), aitor.viana.sanchez@esa.int
 *        Company:  European Space Agency (ESA-ESTEC)
 *
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

/****************************************************************************************
  INCLUDE FILES
 ****************************************************************************************/
#include <stdio.h>
#include <ctype.h>
#include <time.h>
#include <sys/time.h>
#include <unistd.h>
#include <stdarg.h>
#include <string.h>
#include <pthread.h>
#include "linconfig.h"

/*
 ** User defined include files
 */
#include <osal/osdebug.h>
#include <osal/osapi.h>
#include <osal/osstats.h>


LOCAL unsigned int is_init = FALSE;

/*  
 ** Tables for the properties of objects 
 */

/*****************************************************************************
  PRIVATE INTERFACE
 *****************************************************************************/

uint32_t  OS_CompAbsDelayedTime( uint32_t milli_second , struct timespec * tm);

extern int OS_TaskInit(void);
extern int OS_MutSemInit(void);
extern int OS_TaskJoin(void);

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

    ASSERT(is_init);
    if(!is_init)	os_return_minus_one_and_set_errno(OS_STATUS_EERR);

    /*  Print stats */
    osal_stats_print();

    /*  This call must be called to also sync the startup of all tasks  */
    OS_TaskJoin();

#if 0
    uint32_t semid;
    int ret = OS_BinSemCreate(&semid, 0, 0);
    ASSERT( ret >= 0 );

    /*  Idle task to maintain the resources   */
    while(1)
    {
        DEBUG("Idle task started");
        ret = OS_BinSemTake(semid);
        ASSERT( ret >= 0 );

        DEBUG("Ups!!! you should not reach this point!!!\n");
    }

    /*  Never rech here */
    os_return_minus_one_and_set_errno(OS_STATUS_EERR);
#endif

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
    int status;
    int ret;
    struct sched_param main_sp;

    if( is_init ) return 0;

    /*  OS_MutSemInit() shall be called first because lock.h uses it    */
    if( (status = OS_MutSemInit()) != 0 ) goto ret;
    if( (status = OS_TaskInit()) != 0 ) goto ret;

    /*  Change the main thread policy to OS_TASK_SCHED_POLICY */
    main_sp.sched_priority = sched_get_priority_max(OS_TASK_SCHED_POLICY);
    ret = sched_setscheduler(0, OS_TASK_SCHED_POLICY, &main_sp);
    sched_getparam(0, &main_sp);
    TRACE(main_sp.sched_priority, "d");

    is_init = TRUE;

ret:
    return status;;
}


/****************************************************************************************
  INFO API
 ****************************************************************************************/

uint32_t  OS_CompAbsDelayedTime( uint32_t milli_second , struct timespec * tm)
{

    /* 
     ** get the current time 
     */
    /* Note: this is broken at the moment! */
    /*clock_gettime( CLOCK_REALTIME,  tm ); */

    /* Using gettimeofday instead of clock_gettime because clock_gettime is not
     * implemented in the linux posix */
    struct timeval tv;

    gettimeofday(&tv, NULL);
    tm->tv_sec = tv.tv_sec;
    tm->tv_nsec = tv.tv_usec * 1000;




    /* add the delay to the current time */
    tm->tv_sec  += (time_t) (milli_second / 1000) ;
    /* convert residue ( milli seconds)  to nano second */
    tm->tv_nsec +=  (milli_second % 1000) * 1000000 ;

    if(tm->tv_nsec > 999999999 )
    {
        tm->tv_nsec -= 1000000000 ;
        tm->tv_sec ++ ;
    }

    return(0) ;    
}




