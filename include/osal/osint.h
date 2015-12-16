/**
 *  \file   osint.h
 *  \brief  This file defines the interface the OSAL user may use to perform
 *  interrupt handling
 *
 *  Detailed description starts here.
 *
 *  \author  Aitor Viana Sanchez (avs), Aitor.Viana.Sanchez@esa.int
 *
 *  \internal
 *    Created:  10/01/2009
 *   Revision:  $Id: osint.h 1.4 10/01/2009 avs Exp $
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

#ifndef _OSAL_INT_H_
#define _OSAL_INT_H_

/**
 *  \ingroup OSAL
 *  \defgroup Interrupt_API Library Interrupt API
 *
 *  This API contains a set of functions to allow the user the interrupt
 *  handling mechanism.
 */

/****************************************************************************************
  OS Interrupt handling API
 ****************************************************************************************/

/**
 * \ingroup Interrupt_API
 *  \brief This function initializes the interrupt API.
 *  \param  none
 *
 * \return Upon successful the function returns '0' otherwise -1 is returned and
 * os_errno is set to indicate the error.
 */
int OS_IntInit(void);

/**
 * \ingroup Interrupt_API
 *  \brief  This function installs an Interrupt Service Routine for the given
 *  interrupt level number.
 *
 *  \param  interrupt_no       Interrupt level number
 *  \param  interrupt_handler  Interrupt Service Routine
 *  \param  parameter          Not used.
 *
 * \return Upon successful the function returns '0' otherwise -1 is returned and
 * os_errno is set to indicate the error.
 *
 *  \TODO The old interrupt handler is so far note returned. Might be return in
 *  the future.
 */
int OS_IntAttachHandler(uint32_t interrupt_no, void *interrupt_handler, int32_t parameter);

/**
 * \ingroup Interrupt_API
 *  \brief This function unmask the given interrupt level number.
 *
 *  If the interrupt is not maskable the function will do nothing.
 *
 *  \param  level The interrupt level number.
 * \return Upon successful the function returns '0' otherwise -1 is returned and
 * os_errno is set to indicate the error.
 */
int OS_IntUnlock(int32_t level);

/**
 * \ingroup Interrupt_API
 *  \brief This function masks the given interrupt level number.
 *
 *  If the interrupt is not maskable the function will not be able to do the
 *  job and will report no error.
 *
 *  \param  none
 * \return Upon successful the function returns '0' otherwise -1 is returned and
 * os_errno is set to indicate the error.
 */
int32_t OS_IntLock(void);

/**
 * \ingroup Interrupt_API
 *  \brief This function enables the given interupt level number.
 *  \param  level Interrupt level number.
 * \return Upon successful the function returns '0' otherwise -1 is returned and
 * os_errno is set to indicate the error.
 */
int OS_IntEnable(int32_t level);

/**
 * \ingroup Interrupt_API
 *  \brief This function disables the given interupt level number.
 *  \param  level Interrupt level number.
 * \return Upon successful the function returns '0' otherwise -1 is returned and
 * os_errno is set to indicate the error.
 */
int OS_IntDisable(int32_t level);



#endif

