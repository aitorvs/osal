/**
 *  \file   ossem.h
 *  \brief  This file includes all the Binary and Count semaphore interface
 *  within OSAL library
 *
 *  Detailed description starts here.
 *
 *  \author  Aitor Viana Sanchez (avs), Aitor.Viana.Sanchez@esa.int
 *
 *  \internal
 *    Created:  10/01/2009
 *   Revision:  $Id: ossem.h 1.4 10/01/2009 avs Exp $
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

#ifndef _OSAL_SEM_H_
#define _OSAL_SEM_H_

/**
 *  \ingroup OSAL
 *  \defgroup Bin_Sem_API Library Binary Semaphore API
 *
 *  This API contains a set of functions allowing the user to perform the
 *  creation and handling of binary semaphores.
 */
/**
 *  \ingroup OSAL
 *  \defgroup Count_Sem_API Library Couting Semaphore API
 *
 *  This API contains a set of functions allowing the user to perform the
 *  creation and handling of counting semaphores.
 */

/****************************************************************************************
  SEMAPHORE API
 ****************************************************************************************/

/**
 * \ingroup Bin_Sem_API
 * \brief Creates a binary semaphore
 * 
 * The initial value is specified by
 * ul_SemInitialValue and name specified by sem_name. pul_SemId will be 
 * returned to the caller
 * 
 * \param pul_SemId  This is the semaphore identifier to be returned
 * \param ul_SemInitialValue This is the intial value of the semaphore
 * \param ul_Options These are the options (not used)
 *
 * \return Upon successful the function returns '0' otherwise -1 is returned and
 * os_errno is set to indicate the error.
 */  
int OS_BinSemCreate (
        uint32_t *pul_SemId, 
        uint32_t ul_SemInitialValue, 
        uint32_t ul_Options);


/**
 * \ingroup Bin_Sem_API
 * \brief Deletes the specified Binary Semaphore.
 * 
 * \param ul_SemId  This is the identifier of the semaphore to be 
 *
 * \return Upon successful the function returns '0' otherwise -1 is returned and
 * os_errno is set to indicate the error.
 * 
 * NOTES: Since we can't delete a semaphore which is currently locked by some task 
 * (as it may ber crucial to completing the task), the semaphore must be full to
 * allow deletion.
 */  
int OS_BinSemDelete (uint32_t ul_SemId);

/**
 * \ingroup Bin_Sem_API
 * \brief The function  unlocks the semaphore referenced by ul_SemId.
 *     If the semaphore value resulting from this operation is positive, 
 * then no threads were blocked waiting for the semaphore to become 
 * unlocked; the semaphore value is simply incremented for this semaphore.
 * 
 * \param ul_SemId  This is the semaphore identifier
 *
 * \return Upon successful the function returns '0' otherwise -1 is returned and
 * os_errno is set to indicate the error.
 */  
int OS_BinSemGive (uint32_t ul_SemId);

/**
 * \ingroup Bin_Sem_API
 * The function  releases all the tasks pending on this semaphore. Note
 * that the state of the semaphore is not changed by this operation.
 * 
 * \param ul_SemId This parameter is the semaphore identifer.	
 * 
 * \return Upon successful the function returns '0' otherwise -1 is returned and
 * os_errno is set to indicate the error.
 */
int OS_BinSemFlush (uint32_t ul_SemId);

/**
 * \ingroup Bin_Sem_API
 * \brief The locks the semaphore referenced by ul_SemId.
 * 
 * \param ul_SemId  This is the semaphore identifier
 *
 * \return Upon successful the function returns '0' otherwise -1 is returned and
 * os_errno is set to indicate the error.
 */  
int OS_BinSemTake (uint32_t ul_SemId);

/**
 * \ingroup Bin_Sem_API
 * \brief The tries to lock the semaphore referenced by ul_SemId.
 * 
 * The OS_BinSemTryTake() function shall lock the semaphore referenced 
 * by sem only if the semaphore is currently not locked; that is, if the 
 * semaphore value is currently positive. Otherwise, it shall not lock 
 * the semaphore.
 * 
 * \param ul_SemId  This is the semaphore identifier
 *
 * \return Upon successful the function returns '0' otherwise -1 is returned and
 * os_errno is set to indicate the error.
 */  
int OS_BinSemTryTake (uint32_t ul_SemId);

/**
 * \ingroup Bin_Sem_API
 * \brief The function locks the semaphore referenced by ul_SemId . However,
 if the semaphore cannot be locked without waiting for another process
 or thread to unlock the semaphore , this wait shall be terminated when 
 the specified timeout ,msecs, expires.
 * 
 * \param ul_SemId  This is the semaphore identifier
 * \param ul_Msecs   This is the timeout
 *
 * \return Upon successful the function returns '0' otherwise -1 is returned and
 * os_errno is set to indicate the error.
 */  
int OS_BinSemTimedWait (uint32_t ul_SemId, uint32_t ul_Msecs);

/**
 * \ingroup Bin_Sem_API
 * \brief This function will pass back a pointer to structure that contains 
 all of the relevant info( name and creator) about the specified binary
 semaphore.
 * 
 * \param ul_SemId  This is the semaphore identifier
 * \param pt_SemProp    This a pointer to the structure that stores all the 
 information regarded to the semaphore
 *
 * \return Upon successful the function returns '0' otherwise -1 is returned and
 * os_errno is set to indicate the error.
 */  
int OS_BinSemGetInfo (uint32_t ul_SemId, OS_bin_sem_prop_t *pt_SemProp);

/****************************************************************************************
  COUNTARY SEMAPHORE API
 ****************************************************************************************/

/**
 * \ingroup Count_Sem_API
 * Creates a counting semaphore with initial value specified by
 * sem_initial_value and name specified by sem_name. ul_SemId will be
 * returned to the caller.
 * 
 * \param ul_SemId	this is the semaphore identifier assigned to the
 * created count semaphore.
 * \param sem_name	This is the semaphore name that must be provided
 * by the caller.
 * \param sem_initial_value	Initial value for the semaphore.
 * \param ul_Options	This parameter is not used now.
 * 
 * \return Upon successful the function returns '0' otherwise -1 is returned and
 * os_errno is set to indicate the error.
 */
int OS_CountSemCreate (
        uint32_t *ul_SemId, 
        uint32_t ul_SemInitialValue, 
        uint32_t ul_Options);

/**
 * \ingroup Count_Sem_API
 * Deletes the specified Countary Semaphore.
 * 
 * \param ul_SemId	the countary sempahore identifier returned in the
 * sempahore creation.
 * 
 * \return Upon successful the function returns '0' otherwise -1 is returned and
 * os_errno is set to indicate the error.
 */
int OS_CountSemDelete (uint32_t ul_SemId);

/**
 * \ingroup Count_Sem_API
 * The function  unlocks the semaphore referenced by ul_SemId by performing
 * a semaphore unlock operation on that semaphore.If the semaphore value
 * resulting from this operation is positive, then no threads were blocked
 * waiting for the semaphore to become unlocked; the semaphore value is
 * simply incremented for this semaphore.
 * 
 * \param ul_SemId	The sem identifier returned in the semaphore creation.
 * 
 * \return Upon successful the function returns '0' otherwise -1 is returned and
 * os_errno is set to indicate the error.
 * 
 * @see OS_CountSemCreate
 */
int OS_CountSemGive (uint32_t ul_SemId);

/**
 * \ingroup Count_Sem_API
 * The locks the semaphore referenced by ul_SemId by performing a
 * semaphore lock operation on that semaphore.If the semaphore value
 * is currently zero, then the calling thread shall not return from
 * the call until it either locks the semaphore or the call is 
 * interrupted by a signal.
 * 
 * \param ul_SemId	The sem identifier returned in the semaphore creation.
 * 
 * \return Upon successful the function returns '0' otherwise -1 is returned and
 * os_errno is set to indicate the error.
 * 
 * @see OS_CountSemCreate
 */
int OS_CountSemTake (uint32_t ul_SemId);

/**
 * \ingroup Count_Sem_API
 * The function locks the semaphore referenced by ul_SemId . However,
 * if the semaphore cannot be locked without waiting for another process
 * or thread to unlock the semaphore , this wait shall be terminated when
 * the specified timeout, msecs, expires.
 * 
 * \param ul_SemId	The sem identifier returned in the semaphore creation.
 * \param ul_Msecs	The timeout expressed in milliseconds.
 * 
 */
int OS_CountSemTimedWait (uint32_t ul_SemId, uint32_t ul_Msecs);

/**
 *  \brief 
 *  \param  
 * \return Upon successful the function returns '0' otherwise -1 is returned and
 * os_errno is set to indicate the error.
 */
int OS_CountSemTryTake (uint32_t ul_SemId);

/**
 * \ingroup Count_Sem_API
 * This function will pass back a pointer to structure that contains
 * all of the relevant info( name and creator) about the specified counting
 * semaphore.
 * 
 * \param ul_SemId	Semaphore identifier returned in the sem creation.
 * \param pt_CountProp	Sempahore information being returned.
 * 
 * \return Upon successful the function returns '0' otherwise -1 is returned and
 * os_errno is set to indicate the error.
 */
int OS_CountSemGetInfo (uint32_t ul_SemId, OS_count_sem_prop_t *pt_CountProp);


#endif


