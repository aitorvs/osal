//----------------------------------------------------------------
// Statically-allocated memory manager
//
// by Eli Bendersky (eliben@gmail.com)
//  
// This code is in the public domain.
//----------------------------------------------------------------
#include <osal/osapi.h>
#include <osal/osdebug.h>

#include <stdio.h>

// A rudimentary test of the memory manager.
// Runs assuming default flags in memmgr.h:
//
// #define POOL_SIZE 8 * 1024
// #define MIN_POOL_ALLOC_QUANTAS 16
//
// And a 32-bit machine (sizeof(unsigned long) == 4)
//

#define ALLOCATIONS_SIZE    (1024-8)
#ifdef CONFIG_MAX_MEMORY_MANAGER_POOL_CHUNK_LOG2_SIZE
#define NUMBER_OF_ALLOC ((1 << CONFIG_MAX_MEMORY_MANAGER_POOL_LOG2_SIZE) / (ALLOCATIONS_SIZE+8))
#else
#define NUMBER_OF_ALLOC ( (1 << 20) / (ALLOCATIONS_SIZE+8) )
#endif

void test_memmgr(void *ignored)
{
    uint8_t *p[NUMBER_OF_ALLOC] = {0};
    int i;

    TRACE(NUMBER_OF_ALLOC, "d");
    printf("Press any button\n");
    getchar();

    /*  allocate all    */
    for (i = 0; i < NUMBER_OF_ALLOC; ++i)
    {
        p[i] = OS_Malloc(ALLOCATIONS_SIZE);
        ASSERT(p[i] != 0);
    }

    /*  Must fail   */
//    aux = OS_Malloc(ALLOCATIONS_SIZE);
//    ASSERT( aux == 0 );

    /*  Free all   */
    for (i = 0; i < NUMBER_OF_ALLOC; ++i)
    {
        OS_Free(p[i]);
        p[i] = 0;
    }

    // Each header uses 8 bytes, so this allocates
    // N * (ALLOCATIONS_SIZE + 8), leaving us
    // with only one ALLOCATIONS_SIZE more allocation
    //
    for (i = 0; i < NUMBER_OF_ALLOC-1; ++i)
    {
        p[i] = OS_Malloc(ALLOCATIONS_SIZE);
        ASSERT(p[i] != 0);
    }
    printf("%d bytes allocated\n", i * ALLOCATIONS_SIZE);

    // Allocate all the remaining memory
    // Also the previous p[4] is lost...we will free it using
    // OS_MemGarbageCollector()
    //
    p[4] = OS_Malloc(ALLOCATIONS_SIZE);
    ASSERT(p[4]);

    // Nothing left...
    // Also the previous p[5] is lost...we will free it using
    // OS_MemGarbageCollector()
    //
//    p[5] = OS_Malloc(1);
//    ASSERT(p[5] == 0);

    // Release the second block. This frees 2048 + 8 bytes.
    //
    OS_Free(p[1]);
    p[1] = 0;

    // Now we can allocate several smaller chunks from the 
    // free list. There, they can be smaller than the 
    // minimal allocation size.
    // Allocations of 100 require 14 quantas (13 for the 
    // requested space, 1 for the header). So it allocates
    // 112 bytes. We have 18 allocations to make:
    //
    p[1] = OS_Malloc(1);
    ASSERT(p[1]);

    // Not enough for another one...
    // Also the previous p[28] is lost...we will free it using
    // OS_MemGarbageCollector()
    //
//    p[28] = OS_Malloc(ALLOCATIONS_SIZE);
//    ASSERT(p[28] == 0);

    // Now free everything
    //
    for (i = 0; i < NUMBER_OF_ALLOC; ++i)
    {
        if (p[i])
            OS_Free(p[i]);
    }

    printf("======================\n");
    printf("     TEST PASSED!!    \n");
    printf("======================\n");

}


int main(void)
{

    uint32_t t1;
    int32_t ret;

    OS_Init();

    printf("======================\n");
    printf("MEMORY MANAGER example\n");
    printf("======================\n");
    
    ret = OS_TaskCreate (&t1,(void *)test_memmgr, 2048, 99, 0, (void*)NULL);
    if( ret < 0 )
    {
    	printf("ERR: unable to create the tasks\n");
        return -1;
    }
    
    OS_Start();

    return 0;
}

