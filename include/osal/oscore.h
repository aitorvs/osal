/**
 *  \file   oscore.h
 *  \brief  This header file contains the OSAL public functions declarations.
 *  This file is already included from the osapi.h header file.
 *
 *  \author  Aitor Viana Sanchez (avs), Aitor.Viana.Sanchez@esa.int
 *
 *  \internal
 *    Created:  25/03/09
 *   Revision:  $Id: osapi-os-core.h 1.4 25/03/09 avs Exp $
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
 * Author : Alan Cudmore - Code 852
 * Modifications/enhanced : Aitor Viana / ESA-ESTEC
 * Purpose: 
 *         This file  contains some of the OS APIs abstraction layer 
 *         implementation for POSIX, specifically for Linux / Mac OS X.
 *
 */

/**
 *  \defgroup OSAL Operating System Abstraction Library (OSAL)
 *
 *  The OSAL library is an abstraction library for different operating system
 *  which aims to isolate the application from the underlying operating System.
 *  
 *  So far supports RTEMS, Linux, OSx and more OS(s) are coming.
 */

/**
 *  \ingroup OSAL
 *  \defgroup Setup_API Library Setup API
 *
 *  This API allows the user to set-up and configure the OSAL Library
 *
 */

#ifndef _OSAL_OS_CORE_
#define _OSAL_OS_CORE_

#include <stdint.h>
#include <osal/ostypes.h>

#ifndef _OSCONFIG_
#error The "osconfig.h" header file must be included before this file.
#endif

/* #define for enabling floating point operations on a task*/
#define OS_FP_ENABLED   (1 << 0) /**< \brief Floating point ops enabled */
#define OS_FP_DISABLED  (1 << 1) /**< \brief Floating point ops disabled */
#define OS_IS_PERIODIC  (1 << 2) /**< \brief Task is periodic */

/**
 *  \brief This class structure defines the task information
 */
typedef struct
{
    /** Task Creator Identifier */
    uint32_t mul_Creator; 
    /** Task stack size */
    uint32_t mul_StackSize;
    /** Task Priority */
    uint32_t mul_Priority;
    /** Task Identifier */
    uint32_t mul_TaskId;
}OS_task_prop_t;

/** 
 * \class OS_ class structure defines the binary semaphores information. This
 * class is used by the \ref OS_BinSemGetInfo() function.
 */
typedef struct
{
    /** Semaphore Creator Identifier */
    uint32_t mul_Creator;
}OS_bin_sem_prop_t;

/**
 *  \class OS_count_sem_s class structure defines the couting semaphore information. This
 *  class is used by the \ref OS_CountSemGetInfo() function.
 */
typedef struct
{                     
    /** Semaphore Creator Identifier */
    uint32_t mul_Creator;
}OS_count_sem_prop_t;

/** 
 * \class OS_mut_sem class structure defines the mutal exclusion semaphore information.
 * This class is used by the \ref OS_MutSemGetInfo() function.
 */
typedef struct
{
    /** Semaphore Creator Identifier */
    uint32_t mul_Creator;
}OS_mut_sem_prop_t;

/**
 *  \class OS_qs class structure defines the OS queue informaton. This class is
 *  used by the \ref OS_QueueGetInfo() function
 */
typedef struct
{
    /** Queue Creator Identifier */
    uint32_t mul_Creator;
}OS_queue_prop_t;

/** 
 * \brief This class structure defines the information related to time. It is
 * used for some of the functions in the library
 * see also:
 *  - \ref OS_GetLocalTime
 *  - \ref OS_SetLocalTime
 *
 */
typedef struct
{
    /** Seconds */
    uint32_t mul_Seconds;
    /** Microseconds */
    uint32_t mul_MicroSeconds;
}OS_time_t;

/**
 *  \brief This class structure defines the time of the day. It is used for
 *  time and timer operations.
 */
typedef struct
{
    /** This field is the year */
    uint32_t mul_Year;
    /** This field is the month, 1 -> 12 */
    uint32_t mul_Month;
    /** This field is the day, 1 -> 31 */
    uint32_t mul_Day;
    /** This field is the hour, 0 -> 23 */
    uint32_t mul_Hour;
    /** This fiels is the minutes, 0 -> 59 */
    uint32_t mul_Minute;
    /** This fiels is the seconds, 0 -> 59 */
    uint32_t mul_Seconds;
    /** This field is the microsecond, 0 -> 999999999    */
    uint32_t mul_MicroSeconds;
}OS_time_of_day_t;

/** 
 * \ingroup Timer_API
 * This type defines the type used to manage and indirectly invoke Timer
 * Service Soutines (TSRs)
 */
typedef void(*OS_TSR_entry_t)(void*);

/** This global variable stores the system reference Time Of Day value */
extern OS_time_of_day_t OS_system_tod;

/*  Exported functions  */

/****************************************************************************************
  SETUP API
 ****************************************************************************************/
/**   
 * \ingroup Setup_API
 * Name: OS_Init
 * Purpose: Initialize the tables that the OS API uses to keep track of 
 * information about objects
 *
 * \return Upon successful the function returns '0' otherwise -1 is returned and
 * os_errno is set to indicate the error.
 */
int OS_Init(void);

/**
 * \ingroup Setup_API
 *  This function starts the kernel. It shall be invoked
 * at the end of the OS_Application_Startup() function.
 * The function creates a local semaphore and blocks himself on it forever.
 * 
 * \return Upon successful the function returns '0' otherwise -1 is returned and
 * os_errno is set to indicate the error.
 */
int OS_Start(void);

/****************************************************************************************
  Misc API
 ****************************************************************************************/

/*-----------------------------------------------------------------------------
 *  OS MISC INTERFACE
 *-----------------------------------------------------------------------------*/

/**
 *  \brief This class structure stores the configuration information of the
 *  OSAL library.
 */
struct s_osal_info
{
    /** Maximum number of sporadic tasks */
    uint32_t max_sporadic_task_number;
    /** Maximum number of periodic tasks */
    uint32_t max_periodic_task_number;
    /** Maximum number of binary semaphores */
    uint32_t max_sem_number;
    /** Maximum number of mutex */
    uint32_t max_mutex_number;
    /** Maximum number of memory pools */
    uint32_t max_pool_number;
    /** Maximum number of queues */
    uint32_t max_queue_number;
    /** Maximum number of timers */
    uint32_t max_timer_number;
};

/**
 * \ingroup Setup_API
 *  \brief This function returns the OSAL configuration parameters.
 *
 *  \see s_osal_info
 *
 *  \param  pt_Info    In this parameter the information will be copied.
 *
 * \return Upon successful the function returns '0' otherwise -1 is returned and
 * os_errno is set to indicate the error.
 */
int OS_GetConfigInfo(struct s_osal_info *pt_Info);


#endif 

