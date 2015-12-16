/**
 *  \file   osint.c
 *  \brief  This file implements the Interrupt API of the OSAL library for the
 *  Linux operating system
 *
 *  Detailed description starts here.
 *
 *  \author  Aitor Viana Sanchez (avs), Aitor.Viana.Sanchez@esa.int
 *
 *  \internal
 *    Created:  19/02/09
 *   Revision:  $Id: osint.c 1.4 19/02/09 avs Exp $
 *   Compiler:  gcc/g++
 *    Company:  European Space Agency (ESA-ESTEC)
 *  Copyright:  Copyright (c) 2009, Aitor Viana Sanchez
 *
 *  This source code is released for free distribution under the terms of the
 *  GNU General Public License as published by the Free Software Foundation.
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

#include "osal/osapi.h"

/********************************* FILE PRIVATE VARIABLES  */


/********************************* PRIVATE INTERFACE    */



/********************************* PUBLIC  INTERFACE    */

int OS_IntInit(void)
{

    return 0;
}

/* 
 * ===  FUNCTION  ======================================================================
 *         Name:  OS_IntAttachHandler
 *  Description:  This function installs an ISR for the given interrupt level
 *  (number).
 *  Parameters:
 *      - interrupt_no:         This is the interrupt number to be installed
 *      - interrupt_handler:    This is the interrupt handler
 *      - parameter:            Not used
 *  Return:
 *      0 when the function success
 *      OS_INVALID_INT_NUM when the interrupt number is not valid
 *      OS_INVALID_POINTER when the interrupt handler address is not valid
 *      OS_STATUS_EERR when any other error occurs
 *
 * FIXME: The old_handler needs to be returned 
 * =====================================================================================
 */
int OS_IntAttachHandler(uint32_t interrupt_no, void *interrupt_handler, int32_t parameter)
{
    UNUSED(interrupt_no);
    UNUSED(interrupt_handler);
    UNUSED(parameter);


    os_return_minus_one_and_set_errno(OS_STATUS_EERR);
}

/* 
 * ===  FUNCTION  ======================================================================
 *         Name:  OS_IntUnlock
 *  Description:  This function enables maskable interrupts to the level which
 *  was returned by a previous call to OS_IntLock.
 *  Parameters:
 *      - level:    interrupt level
 *  Return:
 *      0 always
 * =====================================================================================
 */
int OS_IntUnlock(int32_t level)
{
    UNUSED(level);

    os_return_minus_one_and_set_errno(OS_STATUS_EERR);
}

/* 
 * ===  FUNCTION  ======================================================================
 *         Name:  OS_IntLock
 *  Description:  This function disables all the maskable interrupts and returns
 *  the previous level.
 * =====================================================================================
 */
int32_t OS_IntLock(void)
{
    os_return_minus_one_and_set_errno(OS_STATUS_EERR);
}

/* 
 * ===  FUNCTION  ======================================================================
 *         Name:  OS_IntEnable
 *  Description:  Enables the coresponding interrupt number.
 *  Parameters:
 *      - level:    This is the interrupt level
 *  Return:
 *      0 always
 * =====================================================================================
 */
int OS_IntEnable(int32_t level)
{
    UNUSED(level);

    os_return_minus_one_and_set_errno(OS_STATUS_EERR);
}

/* 
 * ===  FUNCTION  ======================================================================
 *         Name:  OS_IntDisable
 *  Description:  Disable the corresponding interrupt number
 *  Parameters:
 *      - level:    This is the interrupt level
 *  Return:
 *      0 always
 * =====================================================================================
 */
int OS_IntDisable(int32_t level)
{
    UNUSED(level);

    os_return_minus_one_and_set_errno(OS_STATUS_EERR);
}

