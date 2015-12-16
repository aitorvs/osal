/**
 *  \file   osqueue.h
 *  \brief  This file defines all the primitives related to queues within OSAL
 *  library
 *
 *  Detailed description starts here.
 *
 *  \author  Aitor Viana Sanchez (avs), Aitor.Viana.Sanchez@esa.int
 *
 *  \internal
 *    Created:  10/01/2009
 *   Revision:  $Id: osqueue.h 1.4 10/01/2009 avs Exp $
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

#ifndef _OSAL_QUEUE_H_
#define _OSAL_QUEUE_H_

/**
 *  \ingroup OSAL
 *  \defgroup Queue_API Library Queue APi
 *
 *  This API contains a set of functions allowing the user to perform the
 *  creation and handling of queues.
 */

/****************************************************************************************
  QUEUE API
 ****************************************************************************************/
/**
 * \ingroup Queue_API
 * \brief Create a message queue which can be refered to by name or ID
 * 
 * \param pul_QueueId    an id to refer to a specific queue, is passed back to the caller
 * 
 * \param pc_QueueBuffer  This input parameter shall point to the queue message
 * buffer provided by the service user.
 *
 * \param ul_QueueBufSize This input parameter shall contain the size of the
 * queue buffer.
 *
 * \param ul_QueueDepht This is the maximum number of elements that can be 
 *                     stored in the queue.
 * 
 * \param ul_DataSize   This is the size of each data element on the queue. 
 *                     If the queue is setup to have variable sized items, 
 *                     it is the maximum size.
 * \param ul_Flags       This is for extra queue creation flags. The current 
 *                     flags are
 *                     OS_FIFO_QUEUE – use the FIFO queue policy ( default )
 *                     OS_PRIORITY_QUEUE – use priority based queue policy
 *                     OS_FIXED_SIZE_QUEUE
 *                     OS_VARIABLE_SIZED_QUEUE
 *                     This flag capability is not implemented yet.
 * 
 * \return Upon successful the function returns '0' otherwise -1 is returned and
 * os_errno is set to indicate the error.
 *
 */  
int OS_QueueCreate (
        uint32_t *pul_QueueId, 
        char     *pc_QueueBuffer,
        uint32_t ul_QueueBufSize,
        uint32_t ul_QueueDepht, 
        uint32_t ul_DataSize, 
        uint32_t ul_Flags);

/**
 * \ingroup Queue_API
 * \brief Deletes the specified message queue.
 * 
 * \param ul_QueueId    This is the queue identifier
 *
 * \return Upon successful the function returns '0' otherwise -1 is returned and
 * os_errno is set to indicate the error.
 */  
int OS_QueueDelete (uint32_t ul_QueueId);

/**
 * \ingroup Queue_API
 * \brief Receive a message on a message queue.  Will pend or timeout on the receive.
 * 
 * \param ul_QueueId    This is the queue identifier
 * \param pv_Data    This a pointer to the data to be received
 * \param ul_Size    This is the size of the data to be received
 * \param pul_SizeCopied This output parameter will store the data size copied
 * into the data buffer
 * \param l_Timeout This is the timeout
 *
 * \return Upon successful the function returns '0' otherwise -1 is returned and
 * os_errno is set to indicate the error.
 */  
int OS_QueueGet (
        uint32_t ul_QueueId, 
        void *pv_Data, 
        uint32_t ul_Size, 
        size_t *pul_SizeCopied, 
        int32_t l_Timeout);

/**
 * \ingroup Queue_API
 * \brief Put a message on a message queue.
 * 
 * \param ul_QueueId    This is the queue identifier
 * \param pv_Data        This is the pointer to the data to be sent
 * \param ul_Size        This is the size of the data
 * \param ul_Prio        This is the priority of the messages to be queued.
 *
 * \return Upon successful the function returns '0' otherwise -1 is returned and
 * os_errno is set to indicate the error.
 */  
int OS_QueuePut (uint32_t ul_QueueId, void *pv_Data, uint32_t ul_Size, uint32_t ul_Prio);

/**
 * \ingroup Queue_API
 * \brief This function will pass back a pointer to structure that contains 
 all of the relevant info (name and creator) about the specified queue. 
 * 
 * \param ul_QueueId        This is the queue identifier
 * \param pt_QueueProp      This is the pointer to the queue information
 *
 * \return Upon successful the function returns '0' otherwise -1 is returned and
 * os_errno is set to indicate the error.
 */  
int OS_QueueGetInfo (uint32_t ul_QueueId, OS_queue_prop_t *pt_QueueProp);

#endif

