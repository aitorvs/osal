/**
 *  \file   osqueue.c
 *  \brief  This file implements a generic priority message queue for all
 *  operating systems
 *
 *  \author  Aitor Viana Sanchez (avs), Aitor.Viana.Sanchez@esa.int
 *
 *  \internal
 *    Created:  11/27/2010
 *   Revision:  $Id: osqueue.c 1.4 11/27/2010 avs Exp $
 *   Compiler:  gcc/g++
 *    Company:  European Space Agency (ESA-ESTEC)
 *  Copyright:  Copyright (c) 2010, Aitor Viana Sanchez
 *
 *  This source code is released for free distribution under the terms of the
 *  GNU General Public License as published by the Free Software Foundation.
 * =====================================================================================
 */


#include <osal/osdebug.h>
#include <osal/osapi.h>
#include <osal/osstats.h>
#include <public/lock.h>
#include <public/list.h>

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "pool.h"

#define INIT_THREAD_MUTEX() \
    do{ \
        int ret;    \
        ret = lock_rw_init(&_rwlock);   \
        ASSERT( ret >= 0 ); \
    }while(0);


#define WLOCK()   __WLOCK()
#define WUNLOCK() __WUNLOCK()

#define CRITICAL(x) \
    WLOCK();    \
    {   \
        x;  \
    }   \
    WUNLOCK();

#define _IS_QUEUE_INIT()   \
{   \
    if( !_queue_is_init ) \
    { \
        _os_queue_init(); \
        _queue_is_init = 1; \
    } \
}
#define _CHECK_QUEUE_INIT()  (_IS_QUEUE_INIT())

/** This variable flags whether the queue class has been yet initialized or not
 */
static uint8_t _queue_is_init = 0;

/********************************* FILE CLASSES/STRUCTURES */

typedef struct os_queue_message
{
    struct s_list_head  list;
    void                *msg_data;
    uint32_t            msg_size;
    uint32_t            msg_prio;
}msg_t;

struct os_queue_msg_pool
{
    struct s_pool pool;
    uint32_t allocated;
};

typedef struct
{
    struct os_queue_msg_pool    msg_pool;
    struct s_list_head          msg_list;
    int                         free;
    int                         mul_Creator;
    int32_t                     is_blocking;
    uint32_t                    semid;
}OS_queue_record_t;


/********************************* FILE PRIVATE VARIABLES  */

#define OS_MAX_PENDING_MSG  25

/** This array contain all queue structures */
static OS_queue_record_t        os_queue_table[OS_MAX_QUEUES];
static struct os_queue_message  os_msg_list[OS_MAX_PENDING_MSG];
LIST_HEAD(os_free_msg_list);

/********************************* PRIVATE INTERFACE    */

/* This function adds a free message to the msg free list  */
static void _os_queue_return_free_msg(struct os_queue_message *msg)
{
    bzero(msg, sizeof(struct os_queue_message));
    list_add_tail(&msg->list, &os_free_msg_list);
    ASSERT( !list_empty(&os_free_msg_list) );
}

/*  This function returns a free message    */
static msg_t *_os_queue_get_free_msg(void)
{
    msg_t *msg;

    if( list_empty(&os_free_msg_list) ) return NULL;

    msg = list_first_entry(&os_free_msg_list, msg_t, list);
    if( msg != NULL ) list_del(&msg->list);

    return msg;
}

static void _os_queue_put_msg(msg_t *msg, struct s_list_head *msg_list)
{
    struct s_list_head *ptr;
    msg_t *entry;

    list_for_each(ptr, msg_list)
    {
        entry = list_entry(ptr, msg_t, list);
        if(entry->msg_prio < msg->msg_prio )
        {
            list_add_tail(&msg->list, ptr);
            return;
        }
    }
    list_add_tail(&msg->list, msg_list);
}

static msg_t *_os_queue_get_msg(struct s_list_head *msg_list)
{
    msg_t *msg;
    if( list_empty(msg_list) ) return NULL;

    msg = list_first_entry(msg_list, msg_t, list);
    if( msg != NULL) list_del(&msg->list);

    return msg;
}

static void _os_queue_init(void)
{
    int i;

    STATS_INIT_QUEUE();

    /* Initialize Message Queue Table */

    for(i = 0; i < OS_MAX_QUEUES; i++)
    {
        os_queue_table[i].free        = TRUE;
        os_queue_table[i].mul_Creator     = UNINITIALIZED;
        os_queue_table[i].is_blocking = UNINITIALIZED;

        INIT_LIST_HEAD(&os_queue_table[i].msg_list);
        pool_init(&os_queue_table[i].msg_pool.pool);

        /*  Create all semaphores to be used in the message queue   */
        if( OS_CountSemCreate (&os_queue_table[i].semid, 0, 0) < 0 ) 
        {
            printf("%s:%d: ERROR!!!\n", __func__, __LINE__);
        }
    }

    /*  Add all messages to the msg free list   */
    for(i = 0; i < OS_MAX_PENDING_MSG; i++)
    {
        _os_queue_return_free_msg(&os_msg_list[i]);
    }

    INIT_THREAD_MUTEX();
}


/********************************* PUBLIC  INTERFACE    */

/****************************************************************************************
  MESSAGE QUEUE API
 ****************************************************************************************/

/* 
 * ===  FUNCTION  ======================================================================
 *         Name:  OS_QueueCreate
 *  Description:  This function creates a message queue.
 *  Parameters:
 *      - queue_id:     This is the queue identifier to be returned
 *      - queue_name:   This parameter must be NULL.
 *      - queue_depth:  This is the depth of the queue
 *      - data_size:    This is the size of the data to be stored in the queue
 *      - flags:        Not used so far.
 *  Return:
 *      0 when the call success
 *      OS_STATUS_EINVAL when there is not valid pointers passed as parameters
 *      OS_STATUS_NAME_TOO_LONG when the queue name is too long
 *      OS_STATUS_NO_FREE_IDS when there are no more identifiers for the queues
 *      OS_STATUS_NAME_TAKEN when the 'queue_name' is already in use
 *      OS_STATUS_EERR when any other error occurs
 *
 * =====================================================================================
 */
int OS_QueueCreate (
        uint32_t *queue_id, 
        char     *buffer,
        uint32_t buffer_size,
        uint32_t queue_depth, 
        uint32_t data_size, 
        uint32_t flags)
{
    uint32_t size_aligned;

    ASSERT(queue_id);

    _CHECK_QUEUE_INIT();

    uint32_t possible_qid;

    /*  Sanity checks   */
    if ( queue_id == NULL )
    {
        os_return_minus_one_and_set_errno(OS_STATUS_EINVAL);
    }
    else if ( buffer == NULL )
    {
        os_return_minus_one_and_set_errno(OS_STATUS_EINVAL);
    }
    else if( (int)buffer & 0x3 )
    {
        printf("%s:%d: Buffer not aligned (0x%x)\n", __func__, __LINE__, (int)buffer);
        os_return_minus_one_and_set_errno(OS_STATUS_ADDRESS_MISALIGNED);
    }

    if(queue_depth == 0 || data_size == 0)
    {
        os_return_minus_one_and_set_errno(OS_STATUS_EINVAL);
    }
    else if ( (buffer_size == 0) || (buffer_size < data_size) )
    {
        os_return_minus_one_and_set_errno(OS_STATUS_EINVAL);
    }

    /* we don't want to allow names too long*/
    /* if truncated, two names might be the same */


    /* Check Parameters */
    WLOCK();
    {
        for(possible_qid = 0; possible_qid < OS_MAX_QUEUES; possible_qid++)
        {
            if (os_queue_table[possible_qid].free == TRUE)
                break;
        }

        if( possible_qid >= OS_MAX_QUEUES || os_queue_table[possible_qid].free != TRUE)
        {
            WUNLOCK();

            os_return_minus_one_and_set_errno(OS_STATUS_NO_FREE_IDS);
        }

        /* set the ID free to false to prevent other tasks from grabbing it */
        os_queue_table[possible_qid].free = FALSE;   
    }
    WUNLOCK();

    /*  Aligne the size fo 32 bits so it does not cause any memory trap  */
    if( data_size & 0x3 )
        size_aligned = (data_size+4) & ~0x00000003;
    else
        size_aligned = data_size;

    /*
     ** Create the message queue.
     */
    pool_init_memory( &os_queue_table[possible_qid].msg_pool.pool, (uint8_t*)buffer, buffer_size, size_aligned);

    /*
     ** If the operation failed, report the error */

    /* Set the name of the queue, and the mul_Creator as well */

    *queue_id = possible_qid;

    WLOCK();
    {
        os_queue_table[*queue_id].free = FALSE;
        os_queue_table[*queue_id].mul_Creator = OS_TaskGetId();
        os_queue_table[*queue_id].is_blocking = (flags == OS_NONBLOCKING) ? flags : OS_BLOCKING;
        os_queue_table[*queue_id].msg_pool.allocated = 0;    // clear the allocated buffers

        /*  Stats   */
        STATS_CREAT_QUEUE();
    }
    WUNLOCK();

    return 0;

} /* end OS_QueueCreate */

/* 
 * ===  FUNCTION  ======================================================================
 *         Name:  OS_QueueDelete
 *  Description:  This function deletes the queue object referenced by
 *  'queue_id'.
 *  Parameters:
 *      - queue_id: queue identifier
 *  Returns:
 *      0 when the call success
 *      OS_STATUS_EINVAL when the 'queue_id' is not valid
 *      OS_STATUS_EERR when any other error occurs
 *
 * NOTE: This function is only allow in the non-LOCAL resource allocation mode,
 * which can be seleceted during OSAL configuraiton.
 * =====================================================================================
 */
int OS_QueueDelete (uint32_t queue_id)
{
    _CHECK_QUEUE_INIT();

#if defined (CONFIG_OS_STATIC_RESOURCE_ALLOCATION)
    os_return_minus_one_and_set_errno(OS_STATUS_EERR);
#else
    /* Check to see if the queue_id given is valid */

    if (queue_id >= OS_MAX_QUEUES || os_queue_table[queue_id].free == TRUE)
        os_return_minus_one_and_set_errno(OS_STATUS_EINVAL);

    /* Try to delete the queue */
    if( os_queue_table[queue_id].msg_pool.allocated )
        os_return_minus_one_and_set_errno(OS_STATUS_EBUSY);

    /* 
     * Now that the queue is deleted, remove its "presence"
     * in os_queue_table
     */
    WLOCK();
    {
        pool_init(&os_queue_table[queue_id].msg_pool.pool);
        os_queue_table[queue_id].free = TRUE;
        os_queue_table[queue_id].mul_Creator = UNINITIALIZED;

        /*  Stats   */
        STATS_DEL_QUEUE();
    }
    WUNLOCK();

    return 0;
#endif

} /* end OS_QueueDelete */

/* 
 * ===  FUNCTION  ======================================================================
 *         Name:  OS_QueueGet
 *  Description:  This function retrieves data items from the referenced queue.
 *  Parameters:
 *      - queue_id:     queue identifier
 *      - data:         data retrieved
 *      - size:         maximum size of the data element being gathered
 *      - size_copied:  actual size of the data retrieved
 *      - timeout:      time out
 *  Returns:
 *      0 when the call success
 *      OS_STATUS_EINVAL when the queue identifier is not valid
 *      OS_STATUS_EINVAL when any of the pointer parameters are not valid
 *      OS_STATUS_QUEUE_EMPTY when the queue is empty
 *      OS_STATUS_TIMEOUT when the timeout expires
 * =====================================================================================
 */
int OS_QueueGet (
        uint32_t queue_id, 
        void *data, 
        uint32_t size, 
        size_t *size_copied, 
        int32_t timeout)
{
    int ret;
    msg_t *msg = NULL;

    /* msecs rounded to the closest system tick count */

    _CHECK_QUEUE_INIT();

    /* Check Parameters */

    if(queue_id >= OS_MAX_QUEUES || os_queue_table[queue_id].free == TRUE)
    {
        os_return_minus_one_and_set_errno(OS_STATUS_EINVAL);
    }
    else
    {
        if( (data == NULL) || (size_copied == NULL) )
            os_return_minus_one_and_set_errno(OS_STATUS_EINVAL);
    }

    if( timeout > 0 )
    {
        /*
         * Wait for up to a specified amount of time for a message to arrive.
         * If no message arrives within the timeout interval, return with a
         * failure indication.
         */
        ret = OS_CountSemTimedWait(os_queue_table[queue_id].semid, timeout);
        if( ret < 0 ) os_return_minus_one_and_set_errno(OS_STATUS_TIMEOUT);

        msg = _os_queue_get_msg( &os_queue_table[queue_id].msg_list );
    }
    else
    {
        if( os_queue_table[queue_id].is_blocking == OS_BLOCKING )
        {
            /*
             * Pend forever until a message arrives.
             */
            ret = OS_CountSemTake(os_queue_table[queue_id].semid);
            if( ret < 0 ) os_return_minus_one_and_set_errno(OS_STATUS_EERR);

            /*  Get a message without waiting.  If no message is present,
             *  return with a failure indication. 
             */
            msg = _os_queue_get_msg( &os_queue_table[queue_id].msg_list );
        }
        else if( os_queue_table[queue_id].is_blocking == OS_NONBLOCKING )
        {
            /*  Get a message without waiting.  If no message is present,
             *  return with a failure indication.
             */
            msg = _os_queue_get_msg( &os_queue_table[queue_id].msg_list );
        }
        else os_return_minus_one_and_set_errno(OS_STATUS_EINVAL);
    }

    if( msg )
    {
        *size_copied = msg->msg_size;
        memcpy(data, msg->msg_data, msg->msg_size);
        pool_free_elem( &os_queue_table[queue_id].msg_pool.pool, msg->msg_data );
        os_queue_table[queue_id].msg_pool.allocated--;    // decrease the allocated buffers

        return 0;
    }

    /*
     * Check the status of the read operation.  If a valid message was
     * obtained, indicate success.  If an error occurred, send an event
     * to indicate an unexpected queue read error.
     */

    os_return_minus_one_and_set_errno(OS_STATUS_EERR);

}/* end OS_QueueGet */

/* 
 * ===  FUNCTION  ======================================================================
 *         Name:  OS_QueuePut
 *  Description:  This function stores a data in the queue referenced by
 *  'queue_id'.
 *  Parameters:
 *      - queue_id: queue identifier
 *      - data:     pointer to the data to be put
 *      - size:     data size
 *      - prio:     message priority.
 *  Returns:
 *      0 when the call success
 *      OS_STATUS_EINVAL when the 'data' pointer is not valid
 *      OS_STATUS_EINVAL when the 'queue_id' is not valid
 *      OS_STATUS_QUEUE_FULL when the queue is already full of data
 *      OS_STATUS_EERR when any other error occurrs.
 * =====================================================================================
 */
int OS_QueuePut (uint32_t queue_id, void *data, uint32_t size, uint32_t prio)
{
    msg_t *new;

    _CHECK_QUEUE_INIT();

    /* Check Parameters */

    if(queue_id >= OS_MAX_QUEUES || os_queue_table[queue_id].free == TRUE)
        os_return_minus_one_and_set_errno(OS_STATUS_EINVAL);

    if (data == NULL)
        os_return_minus_one_and_set_errno(OS_STATUS_EINVAL);
    if(size == 0)
        os_return_minus_one_and_set_errno(OS_STATUS_EINVAL);

    /* Get Message From Message Queue */
    new = _os_queue_get_free_msg();
    if( new == NULL ) os_return_minus_one_and_set_errno(OS_STATUS_EERR);

    new->msg_data = pool_zalloc_elem( &os_queue_table[queue_id].msg_pool.pool);
    if( new->msg_data == NULL ) goto err_return_msg;
    os_queue_table[queue_id].msg_pool.allocated++;    // decrease the allocated buffers

    /** Write the buffer pointer to the queue.  If an error occurred, report it
     ** with the corresponding SB status code.
     */
    new->msg_prio = prio;
    new->msg_size = size;
    memcpy(new->msg_data, data, size);

    _os_queue_put_msg(new, &os_queue_table[queue_id].msg_list);
    int ret = OS_CountSemGive(os_queue_table[queue_id].semid);
    if( ret < 0 ) os_return_minus_one_and_set_errno(OS_STATUS_EERR);

    return 0;

err_return_msg:
    _os_queue_return_free_msg(new);
    os_return_minus_one_and_set_errno(OS_STATUS_EERR);

}/* end OS_QueuePut */

/* 
 * ===  FUNCTION  ======================================================================
 *         Name:  OS_QueueGetInfo
 *  Description:  This function retrieves the information related to the queue
 *  referenced by 'queue_id'
 *  Parameters:
 *      - queue_id:     queue identifier
 *      - queue_prop:   structure where the queue info will be stored
 *  Returns:
 *      0 when the call success.
 *      OS_STATUS_EINVAL if the queue_prop pointer is not valid
 *      OS_STATUS_EINVAL if the 'queue_id' is not valid
 * =====================================================================================
 */
int OS_QueueGetInfo (uint32_t queue_id, OS_queue_prop_t *queue_prop)  
{
    _CHECK_QUEUE_INIT();

    /* Check to see that the id given is valid */

    if (queue_prop == NULL)
        os_return_minus_one_and_set_errno(OS_STATUS_EINVAL);

    if (queue_id >= OS_MAX_QUEUES || os_queue_table[queue_id].free == TRUE)
        os_return_minus_one_and_set_errno(OS_STATUS_EINVAL);

    /* put the info into the stucture */
    WLOCK();
    {
        queue_prop -> mul_Creator =   os_queue_table[queue_id].mul_Creator;
    }
    WUNLOCK();


    return 0;

} /* end OS_QueueGetInfo */



