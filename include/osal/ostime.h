/**
 *  \file   ostime.h
 *  \brief  This file defines all the primitives related to the Time
 *  functinalities within OSAL library
 *
 *  Detailed description starts here.
 *
 *  \author  Aitor Viana Sanchez (avs), Aitor.Viana.Sanchez@esa.int
 *
 *  \internal
 *    Created:  10/01/2009
 *   Revision:  $Id: ostime.h 1.4 10/01/2009 avs Exp $
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

#ifndef _OSAL_TIME_H_
#define _OSAL_TIME_H_

extern uint32_t OS_TICKS_PER_SECOND;

/**
 *  \ingroup OSAL
 *  \defgroup Time_API Library Time API
 *
 *  This API contains a set of functions allowing the user to perform the
 *  handling of system time.
 */

/****************************************************************************************
  OS Time/Tick related API
 ****************************************************************************************/


/**
 * \ingroup Time_API
 * \brief Delay a task for a specified amount of milliseconds
 * 
 * \param ul_MilliSeconds:   This is the amount of milliseconds the taks 
 * is gonna be slept
 *
 * \return Upon successful the function returns '0' otherwise -1 is returned and
 * os_errno is set to indicate the error.
 */  
int OS_Sleep (uint32_t ul_MilliSeconds);

/**
 * \ingroup Time_API
 * \brief Delay a task for a specified amount of microseconds
 * 
 * \param ul_MicroSeconds:   This is the amount of milliseconds the taks 
 * is gonna be slept
 *
 * \return Upon successful the function returns '0' otherwise -1 is returned and
 * os_errno is set to indicate the error.
 */  
int OS_uSleep (uint32_t ul_MicroSeconds);

/**
 * \ingroup Time_API
 * \brief This functions get the local time of the machine its on.
 * 
 * \param pt_TimeStruct This is time structure where the local time will be
 * stored.
 *
 * \return Upon successful the function returns '0' otherwise -1 is returned and
 * os_errno is set to indicate the error.
 */  
int OS_GetLocalTime (OS_time_t *pt_TimeStruct);

/**
 * \ingroup Time_API
 * \brief This functions returns the time with microsecond resolution since the
 * last boot.
 * 
 * \param pt_UpTime output parameter. Stores the time since boot.
 *
 * \return Upon successful the function returns '0' otherwise -1 is returned and
 * os_errno is set to indicate the error.
 */  
int OS_GetTimeSinceBoot(OS_time_t *pt_UpTime);

/**
 * \ingroup Time_API
 *  \brief The user shall invoke this function to request the number of ticks
 *  since boot.
 *
 *  \param  pull_UpTicks This ourput parameter will containt the number of ticks
 *  since system boot
 *
 * \return Upon successful the function returns '0' otherwise -1 is returned and
 * os_errno is set to indicate the error.
 */
int OS_GetTicks(uint32_t *pull_UpTicks);

/**
 * \ingroup Time_API
 *  \brief  This function sets the local system time.
 *  
 *  \param  pt_TimeStruct    This paramters stores the new local time.
 *
 * \return Upon successful the function returns '0' otherwise -1 is returned and
 * os_errno is set to indicate the error.
 */
int OS_SetLocalTime(OS_time_t *pt_TimeStruct);

/**
 * \ingroup Time_API
 *  \brief This function returns the current time of day back to the user.
 *
 *  \param  pt_Tod In this parameter the time of day will be stored after the call
 *  success.
 *
 *  see \ref OS_time_of_day_t
 *
 * \return Upon successful the function returns '0' otherwise -1 is returned and
 * os_errno is set to indicate the error.
 */
int OS_GetTimeOfDay(OS_time_of_day_t *pt_Tod);

/**
 * \ingroup Time_API
 *  \brief The user shall invoke this function to request the number of ticks
 *  per second configured in the system.
 *
 *  \param  pul_TicksPerSecond  This output parameter will contain the number of
 *  ticks per second the system has been configured with
 *
 * \return Upon successful the function returns '0' otherwise -1 is returned and
 * os_errno is set to indicate the error.
 */
int OS_GetTicksPerSecond(uint32_t *pul_TicksPerSecond);


#endif


