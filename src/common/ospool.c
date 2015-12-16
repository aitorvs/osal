/**
 *  \file   ospool.c
 *  \brief  This file implements a generic memory pool
 *
 *  The amount of static pool is given during the configuration of the OSAL
 *  layer. 
 *
 *  This implementation is common to all supported OSAL OSes and so it is not
 *  thread-safe. The thread-safe mechanisms shall be provided at user level.
 *
 *  \author  Aitor Viana Sanchez (avs), Aitor.Viana.Sanchez@esa.int
 *
 *  \internal
 *    Created:  07/17/2009
 *   Revision:  $Id: ospool.c 1.4 07/17/2009 avs Exp $
 *   Compiler:  gcc/g++
 *    Company:  European Space Agency (ESA-ESTEC)
 *  Copyright:  Copyright (c) 2009, Aitor Viana Sanchez
 *
 *  This source code is released for free distribution under the terms of the
 *  GNU General Public License as published by the Free Software Foundation.
 * =====================================================================================
 */


#include <osal/osdebug.h>
#include <osal/osapi.h>
#include <public/lock.h>

#include "pool.h"

/****************************************************************************************
  DEFINES
 ****************************************************************************************/

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

#define _IS_POOL_INIT()   \
{   \
    if( !_pool_is_init ) \
    { \
        _os_pool_init(); \
        _pool_is_init = 1; \
    } \
}
#define _CHECK_POOL_INIT()  (_IS_POOL_INIT())

static uint8_t _pool_is_init = 0;

#ifndef CONFIG_OS_MEM_POOL_ENABLE

static void  _os_pool_init(void)
{
    return 0;
}

#else

/********************************* FILE CLASSES/STRUCTURES */

/* pools */
typedef struct
{
    struct s_pool pool;
    uint32_t allocated;
    uint32_t free;
}OS_pool_t;

/********************************* FILE PRIVATE VARIABLES  */

LOCAL OS_pool_t os_pool[OS_MAX_POOLS];

/********************************* PUBLIC  INTERFACE    */

static void  _os_pool_init(void)
{
    int i;

    for( i = 0; i < OS_MAX_POOLS; ++i )
    {
        os_pool[i].free = TRUE;
        os_pool[i].allocated = 0;
        pool_init(&os_pool[i].pool);
    }

    INIT_THREAD_MUTEX();

    return;
}

int OS_PoolCreate(void *address, uint32_t size, uint32_t buffer_size, uint32_t *id)
{
    uint32_t possible_id;

    ASSERT(address);
    ASSERT(id);

    _CHECK_POOL_INIT();

    /*  Sanity checks   */
    if( (address == NULL) || (id == NULL) )
        os_return_minus_one_and_set_errno(OS_STATUS_EINVAL);
    if( (size == 0) || (buffer_size == 0) )
        os_return_minus_one_and_set_errno(OS_STATUS_EINVAL);

    WLOCK();
    {
        for( possible_id = 0; possible_id < OS_MAX_POOLS; ++possible_id)
        {
            if( os_pool[possible_id].free == TRUE )
                break;
        }

        /*  Check to see if the id is out of bounds */
        if( possible_id >= OS_MAX_POOLS || os_pool[possible_id].free != TRUE)
        {
            os_return_minus_one_and_set_errno(OS_STATUS_NO_FREE_IDS);
        }

        /*  Set the possible id to allocated    */
        os_pool[possible_id].free = FALSE;
    }
    WUNLOCK();

    /*  Create the partition    */
    pool_init_memory( &os_pool[possible_id].pool, (uint8_t*)address, size, buffer_size);


    /*  Set the poll id to the possible_id  */
    *id = possible_id;

    return 0;

}

int OS_PoolDelete(uint32_t id)
{
    _CHECK_POOL_INIT();


    if( (id >= OS_MAX_POOLS) || os_pool[id].free == TRUE )
        os_return_minus_one_and_set_errno(OS_STATUS_EINVAL);

    if( os_pool[id].allocated )
        os_return_minus_one_and_set_errno(OS_STATUS_EBUSY);

    WLOCK();
    {
        pool_init(&os_pool[id].pool);
        os_pool[id].free = TRUE;
    }
    WUNLOCK();

    return 0;

}

int OS_GetPoolBuffer(uint32_t id, void **buffer)
{
    _CHECK_POOL_INIT();


    if( (id >= OS_MAX_POOLS) || os_pool[id].free == TRUE )
        os_return_minus_one_and_set_errno(OS_STATUS_EINVAL);
    ASSERT( buffer != NULL );
    if( buffer == NULL )
        os_return_minus_one_and_set_errno(OS_STATUS_EINVAL);

    *buffer = pool_zalloc_elem( &os_pool[id].pool);

    if( *buffer == NULL )
        os_return_minus_one_and_set_errno(OS_STATUS_EERR);

    CRITICAL( (os_pool[id].allocated++) );

    return 0;
}

int OS_ReturnPoolBuffer(uint32_t id, void *buffer)
{
    _CHECK_POOL_INIT();


    if( (id >= OS_MAX_POOLS) || os_pool[id].free == TRUE )
        os_return_minus_one_and_set_errno(OS_STATUS_EINVAL);
    ASSERT( buffer != NULL );
    if( buffer == NULL )
        os_return_minus_one_and_set_errno(OS_STATUS_EINVAL);

    if( os_pool[id].allocated == 0 )
        os_return_minus_one_and_set_errno(OS_STATUS_EERR);

    pool_free_elem( &os_pool[id].pool, buffer );

    CRITICAL( (os_pool[id].allocated--) );

    return 0;
}

#endif

