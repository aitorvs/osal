/**
 *  \file   osstats.h
 *  \brief  
 *
 *  Detailed description starts here.
 *
 *  \author  Aitor Viana Sanchez (avs), Aitor.Viana.Sanchez@esa.int
 *
 *  \internal
 *    Created:  10/14/2009
 *   Revision:  $Id: osstats.h 1.4 10/14/2009 avs Exp $
 *   Compiler:  gcc/g++
 *    Company:  European Space Agency (ESA-ESTEC)
 *  Copyright:  Copyright (c) 2009, Aitor Viana Sanchez
 *
 *  This source code is released for free distribution under the terms of the
 *  GNU General Public License as published by the Free Software Foundation.
 * =====================================================================================
 */

#ifndef _OSAL_STATS_H_
#define _OSAL_STATS_H_


struct resource_stats
{
    uint32_t mt_Created;
    uint32_t mt_Deleted;
    uint32_t mt_Current;
};

extern struct resource_stats t_TaskStats;
extern struct resource_stats t_BinSemStats;
extern struct resource_stats t_MutSemStats;
extern struct resource_stats t_TimerStats;
extern struct resource_stats t_CountSemStats;
extern struct resource_stats t_QueueStats;
extern struct resource_stats t_PoolStats;

#define STATS_INIT_RESOURCE(x)   \
    do {    \
        x.mt_Created = 0;   \
        x.mt_Deleted = 0;   \
        x.mt_Current = 0;   \
    }while(0);

#define STATS_CREAT_RESOURCE(x)   \
    do {    \
        x.mt_Created++;   \
        x.mt_Current++;   \
    }while(0);

#define STATS_DEL_RESOURCE(x)   \
    do {    \
        x.mt_Deleted++;   \
        x.mt_Current--;   \
    }while(0);

#define STATS_INIT_TASK()       STATS_INIT_RESOURCE(t_TaskStats);
#define STATS_INIT_BINSEM()     STATS_INIT_RESOURCE(t_BinSemStats);
#define STATS_INIT_MUTSEM()     STATS_INIT_RESOURCE(t_MutSemStats);
#define STATS_INIT_TIMER()      STATS_INIT_RESOURCE(t_TimerStats);
#define STATS_INIT_COUNTSEM()   STATS_INIT_RESOURCE(t_CountSemStats);
#define STATS_INIT_QUEUE()      STATS_INIT_RESOURCE(t_QueueStats);
#define STATS_INIT_POOL()       STATS_INIT_RESOURCE(t_PoolStats);

#define STATS_CREAT_TASK()      STATS_CREAT_RESOURCE(t_TaskStats);
#define STATS_CREAT_BINSEM()    STATS_CREAT_RESOURCE(t_BinSemStats);
#define STATS_CREAT_MUTSEM()    STATS_CREAT_RESOURCE(t_MutSemStats);
#define STATS_CREAT_TIMER()     STATS_CREAT_RESOURCE(t_TimerStats);
#define STATS_CREAT_COUNTSEM()  STATS_CREAT_RESOURCE(t_CountSemStats);
#define STATS_CREAT_QUEUE()     STATS_CREAT_RESOURCE(t_QueueStats);
#define STATS_CREAT_POOL()      STATS_CREAT_RESOURCE(t_PoolStats);

#define STATS_DEL_TASK()        STATS_DEL_RESOURCE(t_TaskStats);
#define STATS_DEL_BINSEM()      STATS_DEL_RESOURCE(t_BinSemStats);
#define STATS_DEL_MUTSEM()      STATS_DEL_RESOURCE(t_MutSemStats);
#define STATS_DEL_TIMER()       STATS_DEL_RESOURCE(t_TimerStats);
#define STATS_DEL_COUNTSEM()    STATS_DEL_RESOURCE(t_CountSemStats);
#define STATS_DEL_QUEUE()       STATS_DEL_RESOURCE(t_QueueStats);
#define STATS_DEL_POOL()        STATS_DEL_RESOURCE(t_PoolStats);

#define STATS_CURR_TASKS()      (t_TaskStats.mt_Current)

/**
 *  \brief This function prints statistical information using the console.
 *
 *  The function prints statistics about the number of resources being created,
 *  deleted and currently used.
 *
 *  \return None.
 */
void osal_stats_print(void);

#endif

