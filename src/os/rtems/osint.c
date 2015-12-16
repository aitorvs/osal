/**
 *  \file   osint.c
 *  \brief  This file implements the Interrupt API of the OSAL library for the
 *  RTEMS operating system
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
 * Author : Ezra Yeheskeli
 * Purpose: 
 *         This file  contains some of the OS APIs abstraction layer 
 *         implementation for POSIX, specifically for Linux / Mac OS X.
 *
 */

#include "osal/osapi.h"

#include <rtems.h>

/********************************* FILE PRIVATE VARIABLES  */

#define _IS_INT_INIT()   \
{   \
    if( !_int_is_init ) \
    { \
        _os_int_init(); \
        _int_is_init = 1; \
    } \
}
#define _CHECK_INT_INIT()  (_IS_INT_INIT())

/** This variable flags whether the int class has been yet initialized or not
 */
static uint8_t _int_is_init = 0;


/********************************* PRIVATE INTERFACE    */


static void _os_int_init(void)
{
}


/********************************* PUBLIC  INTERFACE    */

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
 *      OS_STATUS_EINVAL when the interrupt number is not valid
 *      OS_STATUS_EINVAL when the interrupt handler address is not valid
 *      OS_STATUS_EERR when any other error occurs
 *
 * FIXME: The old_handler needs to be returned 
 * =====================================================================================
 */
int OS_IntAttachHandler(uint32_t interrupt_no, void *interrupt_handler, int32_t parameter)
{
    UNUSED(parameter);
    rtems_status_code status;
    rtems_isr_entry old_handler;

    _CHECK_INT_INIT();

    status = rtems_interrupt_catch(
            (rtems_isr_entry)interrupt_handler,
            (rtems_vector_number)interrupt_no,
            &old_handler);

    switch(status)
    {
        case RTEMS_SUCCESSFUL:
            return 0;
            break;
        case RTEMS_INVALID_NUMBER:
        case RTEMS_INVALID_ADDRESS:
            os_return_minus_one_and_set_errno(OS_STATUS_EINVAL);
            break;
        default:
            os_return_minus_one_and_set_errno(OS_STATUS_EERR);
            break;

    }

    return -1;
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
    _CHECK_INT_INIT();

    rtems_interrupt_enable( (rtems_interrupt_level)level );

    return 0;
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
    rtems_interrupt_level rtems_int_level;

    _CHECK_INT_INIT();

    rtems_interrupt_disable( rtems_int_level );

    return ( (int32_t)rtems_int_level );
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
    _CHECK_INT_INIT();

    rtems_interrupt_enable( (rtems_interrupt_level)level );

    return 0;
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
    _CHECK_INT_INIT();

    rtems_interrupt_level rtems_level = level;
    rtems_interrupt_disable( rtems_level );


    return 0;
}

