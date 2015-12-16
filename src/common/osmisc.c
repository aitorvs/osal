/**
 *  \file   osmisc.c
 *  \brief  This file implements comon OSAL primitives for all RTOS
 *
 *  \author  Aitor Viana Sanchez (avs), Aitor.Viana.Sanchez@esa.int
 *
 *  \internal
 *    Created:  09/28/2009
 *   Revision:  $Id: osmisc.c 1.4 09/28/2009 avs Exp $
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
#include <osal/osconfig.h>   // The OS configuration is placed

#include <string.h>


/** This structure stores all the configuration information of the OSAL
 * library. The structure is defined in the osmis.c source file of every RTOS
 * implementation
 */
LOCAL struct s_osal_info osal_info;


int OS_GetConfigInfo(struct s_osal_info *info)
{
    static int32_t init_info = 0;

    ASSERT( info );
    if( info == NULL ) return OS_STATUS_EINVAL;

    if( unlikely(!init_info) )
    {
        osal_info.max_sporadic_task_number = OS_MAX_TIMERS;
        osal_info.max_periodic_task_number = OS_MAX_MONOTONIC_TASKS;
        osal_info.max_sem_number = OS_MAX_SEMAPHORES;
        osal_info.max_mutex_number = OS_MAX_MUTEXES;
        osal_info.max_pool_number = OS_MAX_POOLS;
        osal_info.max_queue_number = OS_MAX_QUEUES;
        osal_info.max_timer_number = OS_MAX_TIMERS;

        init_info = 1;
    }


    memcpy((void*)info, (void*)&osal_info, sizeof(struct s_osal_info));

    return 0;


}

