/**
 *  \file   oserr.h
 *  \brief  This file defines all the possible error codes used in the OSAL
 *  library
 *
 *  \author  Aitor Viana Sanchez (avs), Aitor.Viana.Sanchez@esa.int
 *
 *  \internal
 *    Created:  10/19/2009
 *   Revision:  $Id: oserr.h 1.4 10/19/2009 avs Exp $
 *   Compiler:  gcc/g++
 *    Company:  European Space Agency (ESA-ESTEC)
 *  Copyright:  Copyright (c) 2009, Aitor Viana Sanchez
 *
 *  This source code is released for free distribution under the terms of the
 *  GNU General Public License as published by the Free Software Foundation.
 * =====================================================================================
 */

#ifndef _OS_STATUS_ERRCODE_H_
#define _OS_STATUS_ERRCODE_H_

/**
 *  \brief This function returns the address of the variable os_errno
 *  \param  none
 *  \return value of the address of the variable os_errno
 */
int *__os_errno_addr(void);

/**
 * \ingroup OSAL
 *  \brief  This class structure contains all the OSAL error codes
 */
typedef enum 
{
    OS_STATUS_ERROR                        = -1, /**< \brief ERROR code */
    OS_STATUS_SUCCESS                      = 0, /**< \brief SUCCESS code */
    OS_STATUS_EERR                         , /**< \brief ERROR code */
    OS_STATUS_EINVAL                       , /**< \brief Function Parameter not valid */
    OS_STATUS_ADDRESS_MISALIGNED           , /**< \brief Address misaligned */
    OS_STATUS_TIMEOUT                      , /**< \brief Timeout Error */
    OS_STATUS_NOT_SUPPORTED                , /**< \brief not Supported Feature Error */
    OS_STATUS_EBUSY                        , /**< \brief The resource is in use */
    OS_STATUS_SEM_FAILURE                  , /**< \brief Semaphore Failure Error */
    OS_STATUS_SEM_NOT_AVAIL                , /**< \brief Semaphore Not Available Error */
    OS_STATUS_QUEUE_EMPTY                  , /**< \brief Queue Empty Error */
    OS_STATUS_QUEUE_FULL                   , /**< \brief Queue Full Error */
    OS_STATUS_NO_FREE_IDS                  , /**< \brief No More Resource IDs Error */
    OS_STATUS_TIME_NOT_SET                 , /**< \brief System Time Not Set Error */
    OS_STATUS_TIMER_FAILURE                , /**< \brief OS Timer Failure Error */
    OS_STATUS_TIMER_NOTE_AVAIL             , /**< \brief The timer is not available */
    OS_STATUS_ECC_ERROR                    , /**< \brief ECC Error */
    OS_STATUS_ECC_UNCORRECTABLE_ERROR      , /**< \brief ECC Uncorrectable Error */
    OS_STATUS_ECC_SINGLE_ERROR             , /**< \brief ECC Single Correctable Error */
    OS_STATUS_ECC_INVALID_ORDER            , /**< \brief ECC provided order Error */
    OS_STATUS_PERIODIC_TASK_MISSED         , /**< \brief Periodic Task deadline miss Error */
}OS_STATUS_T;

/**
 *  \ingroup OSAL
 *  \brief  Number of the last error
 *
 *  \see OS_STATUS_T
 */
#define os_errno (*(volatile int*)__os_errno_addr())

/**
 *  \ingroup OSAL
 *  \brief  Sets os_errno and returns -1
 *  \return -1
 */
#define os_return_minus_one_and_set_errno(val) \
    do { \
        os_errno = val; \
        return -1; \
    }while(0);

#endif

