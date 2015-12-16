/**
 *  \file   ostask.h
 *  \brief  This header files defines all the OSAL primitives to give access to
 *  task functionalities.
 *
 *  \author  Aitor Viana Sanchez (avs), Aitor.Viana.Sanchez@esa.int
 *
 *  \internal
 *    Created:  10/01/2009
 *   Revision:  $Id: ostask.h 1.4 10/01/2009 avs Exp $
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

#ifndef _OSAL_TASK_H_
#define _OSAL_TASK_H_

/**
 *  \ingroup OSAL
 *  \defgroup Task_API Library Task API
 *
 *  This API allows the user to handle tasks. The API gets functions for
 *  creationg, deletion, etc.
 */

/****************************************************************************************
  TASK API
 ****************************************************************************************/

#define OS_TASK_MAX_PRIORITY 255
#define OS_TASK_MIN_PRIORITY 1


/**
 * \ingroup Task_API
 * Name: OS_TaskMonotonicCreate
 * 
 * \brief Purpose: Creates a periodic task and starts running it.
 * 
 * \param pul_TaskId         This parameter is the task identifier.
 * \param pf_Entry    This parameter is the start point of the task
 * \param pf_ErrHandler  The parameter is a pointer to the callback function
 * that will be called in case an error happens. If the parameter is NULL and
 * an error occurs the thread will end without notification
 * \param ul_StackSize  This parameter is the stack size
 * \param ul_Priority    The task priority
 * \param ul_Flags   unused
 * \param pv_Arg     The argument to be passed to the function pointer
 * \param ms_period The task period in milliseconds
 *
 * \return Upon successful the function returns '0' otherwise -1 is returned and
 * os_errno is set to indicate the error.
 *
 */
int OS_TaskMonotonicCreate(
        uint32_t *pul_TaskId,
        const void *pf_Entry,
        const void *pf_ErrHandler,
        uint32_t ul_StackSize,
        uint32_t ul_Priority,
        uint32_t ul_Flags,
        void *pv_Arg,
        uint32_t ms_period
        );

/**
 * \ingroup Task_API
 * Name: OS_TaskCreate
 * 
 * \brief Purpose: Creates a task and starts running it.
 * 
 * \param pul_TaskId         This parameter is the task identifier.
 * \param pf_Function    This parameter is the start point of the task
 * \param ul_StackSize  This parameter is the stack size
 * \param ul_Priority    The task priority
 * \param ul_Flags   unused
 * \param pv_Arg     The argument to be passed to the function pointer
 *
 * \return Upon successful the function returns '0' otherwise -1 is returned and
 * os_errno is set to indicate the error.
 *
 * Flags are unused at this point.
 */

int OS_TaskCreate (
        uint32_t *pul_TaskId, 
        const void *pf_Function,
        uint32_t ul_StackSize, 
        uint32_t ul_Priority, 
        uint32_t ul_Flags,
        void* pv_Arg
        );

/**
 * \ingroup Task_API
 *  \brief This primitive suspend the specified task.
 *
 *  The primitive suspend the thread specified as a parameter to the primitive.
 *  The thread is put into the suspend queue and needs to be resumed calling
 *  the associated \ref OS_TaskResume() call.
 *
 *  When it is needed to suspend the calling thread, the parameter \ref OS_SELF
 *  shall be passed as 'ul_TaskId'.
 *
 *  \param  ul_TaskId This parameter is the identifier of the task to be
 *  suspended
 *
 * \return Upon successful the function returns '0' otherwise -1 is returned and
 * os_errno is set to indicate the error.
 */
int OS_TaskSuspend(uint32_t ul_TaskId);

/**
 * \ingroup Task_API
 *  \brief This primitive resumes a previously suspended thread.
 *
 *  This call must be used after using the \ref OS_TaskSuspend() primitive. The
 *  primitive resumes a previously suspended thread.
 *
 *  \param  ul_TaskId This parameter is the identifier of the thread to be
 *  resumed
 *
 * \return Upon successful the function returns '0' otherwise -1 is returned and
 * os_errno is set to indicate the error.
 */
int OS_TaskResume(uint32_t ul_TaskId);

/**
 * \ingroup Task_API
 * \brief This primitive deletes a task.
 * 
 * \param ul_TaskId: This is the identifier of the task being removed.
 *
 * \return Upon successful the function returns '0' otherwise -1 is returned and
 * os_errno is set to indicate the error.
 */  
int OS_TaskDelete (uint32_t ul_TaskId);

/**
 * \ingroup Task_API
 * Exits the calling task and removes it from the OS_task_table.
 *
 * \return Upon successful the function returns '0' otherwise -1 is returned and
 * os_errno is set to indicate the error.
 */
int OS_TaskExit(void);

/**
 * \ingroup Task_API
 * \brief This primitive changes the task priority
 * 
 * \param ul_TaskId This parameter is the task identifier
 * \param ul_NewPrio    This is the new priority to be set
 * \param pul_OldPrio    This is the old thread priority
 *
 * \return Upon successful the function returns '0' otherwise -1 is returned and
 * os_errno is set to indicate the error.
 */  
int OS_TaskSetPriority (
        uint32_t ul_TaskId, 
        uint32_t ul_NewPrio, 
        uint32_t *pul_OldPrio);

/**
 * \ingroup Task_API
 * \brief This function returns the #defined task id of the calling task
 * 
 * \return Upon successful the function returns '0' otherwise -1 is returned and
 * os_errno is set to indicate the error.
 */  
int OS_TaskGetId (void);

/**
 * \ingroup Task_API
 * This function gets the thread specific storage data
 * 
 * \return Upon successful the function returns '0' otherwise -1 is returned and
 * os_errno is set to indicate the error.
 */
void* OS_TaskGetSlot(void);

/**
 * \ingroup Task_API
 * This function sets the thread specific storage data
 *
 * \param pv_Data
 * 
 * \return Upon successful the function returns '0' otherwise -1 is returned and
 * os_errno is set to indicate the error.
 */
int OS_TaskSetSlot(void* pv_Data);

/**
 * \ingroup Task_API
 * \brief This function will pass back a pointer to structure that contains 
 * all of the relevant info (creator, stack size, priority, name) about the 
 *  specified task. 
 * 
 * \param ul_TaskId This is the identifier of the task to be found
 * \param pt_TaskProp   This is a pointer to the structure containing all
 * the task related information
 *
 * \return Upon successful the function returns '0' otherwise -1 is returned and
 * os_errno is set to indicate the error.
 */  
int OS_TaskGetInfo (uint32_t ul_TaskId, OS_task_prop_t *pt_TaskProp);

/**
 * \ingroup Task_API
 *  \brief This function shall be called by the user to request the running
 *  thread to relinquish the procesor until it again becomes the head of the
 *  thread list.
 *
 *  \param  none
 * \return Upon successful the function returns '0' otherwise -1 is returned and
 * os_errno is set to indicate the error.
 */
void OS_TaskYield(void);

#endif


