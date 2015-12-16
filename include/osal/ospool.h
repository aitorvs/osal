/**
 *  \file   ospool.h
 *  \brief  This file defines the interface the user may use to handle memory
 *  pool buffers
 *
 *  Detailed description starts here.
 *
 *  \author  Aitor Viana Sanchez (avs), Aitor.Viana.Sanchez@esa.int
 *
 *  \internal
 *    Created:  10/01/2009
 *   Revision:  $Id: ospool.h 1.4 10/01/2009 avs Exp $
 *   Compiler:  gcc/g++
 *    Company:  European Space Agency (ESA-ESTEC)
 *  Copyright:  Copyright (c) 2009, Aitor Viana Sanchez
 *
 *  This source code is released for free distribution under the terms of the
 *  GNU General Public License as published by the Free Software Foundation.
 * =====================================================================================
 */

#ifndef _OSAL_POOL_H_
#define _OSAL_POOL_H_

/**
 *  \ingroup OSAL
 *  \defgroup Pool_API Library Memory Pool API
 *
 *  This API contain a set of functions for creating and managing memory pools.
 */

/*-----------------------------------------------------------------------------
 *  OS MEMORY POOL INTERFACE
 *-----------------------------------------------------------------------------*/

/**
 * \ingroup Pool_API
 *  \brief This function creates a memory pool of fixed size buffers from a
 *  physically contiguous memory space statically provided in the call.
 *
 *  \param  pv_Address Starting address where the pool will be located
 *  \param  ul_Size    Size of the pool
 *  \param  ul_BufSize Fixed size of the buffer that can be allocated from the
 *  memory pool
 *  \param  pul_PoolId  Pool identifier returned
 *
 * \return Upon successful the function returns '0' otherwise -1 is returned and
 * os_errno is set to indicate the error.
 */
int OS_PoolCreate(
        void *pv_Address, 
        uint32_t ul_Size, 
        uint32_t ul_BufSize, 
        uint32_t *pul_PoolId);

/**
 * \ingroup Pool_API
 *  \brief This call deletes the pool associated with the provided id.
 *
 *  The pool is only deleted if all the buffers have been previously returned.
 *
 *  \param  ul_PoolId  Pool identifier
 *
 * \return Upon successful the function returns '0' otherwise -1 is returned and
 * os_errno is set to indicate the error.
 */
int OS_PoolDelete(uint32_t ul_PoolId);

/**
 * \ingroup Pool_API
 *  \brief This call allows a buffer to be obtained from the memory pool
 *  specified by the 'ul_PoolId'.
 *
 *  The buffer is returned to the user
 *
 *  \param  ul_PoolId  Memory pool identifier
 *  \param  ppv_Buffer The buffer returned from the memory pool
 *
 * \return Upon successful the function returns '0' otherwise -1 is returned and
 * os_errno is set to indicate the error.
 */
int OS_GetPoolBuffer(uint32_t ul_PoolId, void **buffer);

/**
 * \ingroup Pool_API
 *  \brief This directive returns the buffer speficied by 'pv_Buffer' to the
 *  memory pool specified by 'ul_PoolId'
 *
 *  \param  ul_PoolId  Memory pool identifier
 *  \param  pv_Buffer  Buffer to be returned to the memory pool
 *
 * \return Upon successful the function returns '0' otherwise -1 is returned and
 * os_errno is set to indicate the error.
 */
int OS_ReturnPoolBuffer(uint32_t ul_PoolId, void *pv_Buffer);



#endif


