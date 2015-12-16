#include <osal/osapi.h>
#include <osal/osdebug.h>
#include <osal/osstats.h>

struct resource_stats t_TaskStats;
struct resource_stats t_BinSemStats;
struct resource_stats t_MutSemStats;
struct resource_stats t_TimerStats;
struct resource_stats t_CountSemStats;
struct resource_stats t_QueueStats;
struct resource_stats t_PoolStats;

void osal_stats_print(void)
{
    PRINT("======== Tasks =========\n");
    PRINT("created: %d\n", (int)t_TaskStats.mt_Created);
    PRINT("deleted: %d\n", (int)t_TaskStats.mt_Deleted);
    PRINT("current: %d\n", (int)t_TaskStats.mt_Current);
    PRINT("======== BinSems =======\n");
    PRINT("created: %d\n", (int)t_BinSemStats.mt_Created);
    PRINT("deleted: %d\n", (int)t_BinSemStats.mt_Deleted);
    PRINT("current: %d\n", (int)t_BinSemStats.mt_Current);
    PRINT("======== MutSems =======\n");
    PRINT("created: %d\n", (int)t_MutSemStats.mt_Created);
    PRINT("deleted: %d\n", (int)t_MutSemStats.mt_Deleted);
    PRINT("current: %d\n", (int)t_MutSemStats.mt_Current);
    PRINT("======== CountSem ======\n");
    PRINT("created: %d\n", (int)t_CountSemStats.mt_Created);
    PRINT("deleted: %d\n", (int)t_CountSemStats.mt_Deleted);
    PRINT("current: %d\n", (int)t_CountSemStats.mt_Current);
    PRINT("======== Timers ========\n");
    PRINT("created: %d\n", (int)t_TimerStats.mt_Created);
    PRINT("deleted: %d\n", (int)t_TimerStats.mt_Deleted);
    PRINT("current: %d\n", (int)t_TimerStats.mt_Current);
    PRINT("======== Queues ========\n");
    PRINT("created: %d\n", (int)t_QueueStats.mt_Created);
    PRINT("deleted: %d\n", (int)t_QueueStats.mt_Deleted);
    PRINT("current: %d\n", (int)t_QueueStats.mt_Current);
    PRINT("======== Pools  ========\n");
    PRINT("created: %d\n", (int)t_PoolStats.mt_Created);
    PRINT("deleted: %d\n", (int)t_PoolStats.mt_Deleted);
    PRINT("current: %d\n", (int)t_PoolStats.mt_Current);
}


