/**
 *  \file   osmut.h
 *  \brief  This file defines all the primitives related to Mutex
 *  functionalities within OSAL
 *
 *  Detailed description starts here.
 *
 *  \author  Aitor Viana Sanchez (avs), Aitor.Viana.Sanchez@esa.int
 *
 *  \internal
 *    Created:  10/01/2009
 *   Revision:  $Id: osmut.h 1.4 10/01/2009 avs Exp $
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

#ifndef _OSAPI_MUT_H_
#define _OSAPI_MUT_H_

/**
 *  \ingroup OSAL
 *  \defgroup Mutex_API Library Mutual Exclusion API
 *
 *  This API contains a set of functions allowing the user to perform the
 *  creation and handling of mutex.
 */

/****************************************************************************************
  MUTEX API
 ****************************************************************************************/
/**
 * \ingroup Mutex_API
 * \brief Creates a mutex semaphore initially full
 * 
 * \param pul_SemId  This is the mutex identifier to be returned
 * \param sem_name    This is the mutex name
 * \param ul_Options Not used
 *
 * \return Upon successful the function returns '0' otherwise -1 is returned and
 * os_errno is set to indicate the error.
 */  
int OS_MutSemCreate (
        uint32_t *pul_SemId, 
        uint32_t ul_Options);

/**
 * \ingroup Mutex_API
 * \brief Deletes the specified Mutex Semaphore
 * 
 * \param ul_SemId  This is the identifier of the semaphore to be deleted
 *
 * \return Upon successful the function returns '0' otherwise -1 is returned and
 * os_errno is set to indicate the error.
 */  
int OS_MutSemDelete (uint32_t ul_SemId);

/**
 * \ingroup Mutex_API
 * \brief The function releases the mutex object referenced by ul_SemId.The 
 * manner in which a mutex is released is dependent upon the mutex's type 
 * attribute.  If there are threads blocked on the mutex object referenced by 
 * mutex when this function is called, resulting in the mutex becoming 
 * available, the scheduling policy shall determine which thread shall 
 * acquire the mutex.
 * 
 * \param ul_SemId  This is the sempahore identifier
 *
 * \return Upon successful the function returns '0' otherwise -1 is returned and
 * os_errno is set to indicate the error.
 */  
int OS_MutSemGive (uint32_t ul_SemId);

/**
 * \ingroup Mutex_API
 * \brief The mutex object referenced by ul_SemId shall be locked by calling this
 * function. If the mutex is already locked, the calling thread shall
 * block until the mutex becomes available. This operation shall return
 * with the mutex object referenced by mutex in the locked state with the 
 * calling thread as its owner.
 * 
 * \param ul_SemId  This is the sempahore identifier
 *
 * \return Upon successful the function returns '0' otherwise -1 is returned and
 * os_errno is set to indicate the error.
 */  
int OS_MutSemTake (uint32_t ul_SemId);

/**
 * \ingroup Mutex_API
 * \brief The mutex object referenced by ul_SemId shall be locked by calling this
 * function. If the mutex is already locked, the calling thread shall
 * block until the mutex becomes available. This operation shall return
 * with the mutex object referenced by mutex in the locked state with the 
 * calling thread as its owner.
 * 
 * \param ul_SemId  This is the sempahore identifier
 *
 * \return Upon successful the function returns '0' otherwise -1 is returned and
 * os_errno is set to indicate the error.
 */  
int OS_MutSemTryTake (uint32_t ul_SemId);

/**
 * \ingroup Mutex_API
 * The function locks the semaphore referenced by ul_SemId . However,
 * if the semaphore cannot be locked without waiting for another process
 * or thread to unlock the semaphore , this wait shall be terminated when
 * the specified timeout, msecs, expires.
 * 
 * \param ul_SemId	The sem identifier returned in the semaphore creation.
 * \param ul_Msecs	The timeout expressed in milliseconds.
 * 
 * \return Upon successful the function returns '0' otherwise -1 is returned and
 * os_errno is set to indicate the error.
 */
int OS_MutSemTimedWait (uint32_t ul_SemId, uint32_t ul_Msecs);

/**
 * \ingroup Mutex_API
 * \brief This function will pass back a pointer to structure that contains 
 * all of the relevant info( name and creator) about the specified mutex
 * semaphore.
 * 
 * \param ul_SemId      This is the semaphore identifier
 * \param pt_SemProp    This is a pointer to the structure that stores the
 * the information regarded to the semaphore
 *
 * \return Upon successful the function returns '0' otherwise -1 is returned and
 * os_errno is set to indicate the error.
 */  
int OS_MutSemGetInfo (uint32_t ul_SemId, OS_mut_sem_prop_t *pt_SemProp);

#endif


