/**
 *  \file   ostimer.h
 *  \brief  This file provides the interface allowing the OSAL user to perform
 *  timer operations.
 *
 *  Detailed description starts here.
 *
 *  \author  Aitor Viana Sanchez (avs), Aitor.Viana.Sanchez@esa.int
 *
 *  \internal
 *    Created:  10/01/2009
 *   Revision:  $Id: ostimer.h 1.4 10/01/2009 avs Exp $
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

#ifndef _OSAL_TIMER_H_
#define _OSAL_TIMER_H_

/**
 *  \ingroup OSAL
 *  \defgroup Timer_API Library Timer API
 *
 *  This API contains a set of functions allowing the user to perform the
 *  creation and handling of timers.
 */

/*-----------------------------------------------------------------------------
 *  OS TIMER INTERFACE
 *-----------------------------------------------------------------------------*/

/**
 * \ingroup Timer_API
 *  \brief This function creates a timer. The assigned 'pul_TimerId' is
 *  returned when the timer is created successfuly.
 *
 *  \param  pul_TimerId This parameter is the timer identifier to be returned
 *  when success
 *
 * \return Upon successful the function returns '0' otherwise -1 is returned and
 * os_errno is set to indicate the error.
 */
int    OS_TimerCreate(uint32_t *pul_TimerId);

/**
 * \ingroup Timer_API
 *  \brief  This function deletes the timer specified with the identifier
 *  'ul_TimerId'.
 *
 *  \param  ul_TimerId   Timer ID
 *
 * \return Upon successful the function returns '0' otherwise -1 is returned and
 * os_errno is set to indicate the error.
 */
int    OS_TimerDelete(uint32_t ul_TimerId);

/**
 * \ingroup Timer_API
 *  \brief  This function cancels the 'ul_TimerId' timer. The timer will be
 *  reinitiated by the next invocation of OS_TimerFireAfter() or
 *  OS_TimerFireWhen() call.
 *
 *  \param  ul_TimerId   Timer ID
 *
 * \return Upon successful the function returns '0' otherwise -1 is returned and
 * os_errno is set to indicate the error.
 */
int    OS_TimerCancel(uint32_t ul_TimerId);

/**
 * \ingroup Timer_API
 *  \brief  This function initiates the timer specified by 'ul_TimerId'. If
 *  the timer is running, it is canceled before being re-initiated. When the
 *  timer fires, it calls the 't_Primitive' function.
 *
 *  \param  timer id
 *  \param  milliseconds to be fired
 *  \param  Function the timer calls when fires
 *
 * \return Upon successful the function returns '0' otherwise -1 is returned and
 * os_errno is set to indicate the error.
 */
int    OS_TimerFireAfter(
        uint32_t ul_TimerId, 
        uint32_t ul_MilliSeconds, 
        OS_TSR_entry_t t_Primitive, 
        void* pv_UserData);

/**
 * \ingroup Timer_API
 *  \brief  This function initiates the timer specified by 'ul_TimerId' based on
 *  an absolute value.
 *
 * The timer is schedule to fire at 'pt_WhenTime' and when it fires 't_Primitive'
 * function is called. This function might be executed under an interrupt
 * service routine, thus it is strongly recommended to perform as less as
 * possible.
 *
 *  \param  ul_TimerId     Timer id
 *  \param  pt_WhenTime    Time when the timer will be fired
 *  \param  t_Primitive      Function the timer calls when fires
 *
 * \return Upon successful the function returns '0' otherwise -1 is returned and
 * os_errno is set to indicate the error.
 */
int    OS_TimerFireWhen(
        uint32_t ul_TimerId, 
        const OS_time_of_day_t *pt_WhenTime, 
        OS_TSR_entry_t t_Primitive, 
        void *pv_UserData);

/**
 * \ingroup Timer_API
 *  \brief The user shall invoke this function to reset a timer
 *  \param  ul_TimerId is the timer identifier
 * \return Upon successful the function returns '0' otherwise -1 is returned and
 * os_errno is set to indicate the error.
 */
int    OS_TimerReset(uint32_t ul_TimerId);



#endif


