/**
 *  \file   pool.c
 *  \brief  This source file implements an example of using OSAL memory pools
 *
 *  Detailed description starts here.
 *
 *  \author  Aitor Viana Sanchez (avs), Aitor.Viana.Sanchez@esa.int
 *
 *  \internal
 *    Created:  07/20/2009
 *   Revision:  $Id: pool.c 1.4 07/20/2009 avs Exp $
 *   Compiler:  gcc/g++
 *    Company:  European Space Agency (ESA-ESTEC)
 *  Copyright:  Copyright (c) 2009, Aitor Viana Sanchez
 *
 *  This source code is released for free distribution under the terms of the
 *  GNU General Public License as published by the Free Software Foundation.
 * =====================================================================================
 */

#include <osal/osapi.h>
#include <osal/osdebug.h>

#include <sys/time.h>
#include <stdlib.h>
#include <time.h>
#include <stdio.h>

#define POOL_MAX_SIZE   (1*1024)
#define POOL_BUFFER_MAX_SIZE    POOL_MAX_SIZE/4
#define POOL_BUFFER_MIN_SIZE    16

/* misc stuff */
#define RAND(max) ((uint32_t) (((float) (max)) * (rand() / (RAND_MAX + 1.0))))
#define FRAND(max) (((float) (max)) * (rand() / (RAND_MAX + 1.0)))
#define RANDI(min,max) ((min) + RAND ((max) - (min)))
#define FRANDI(min,max) ((min) + FRAND ((max) - (min)))
#define RANDB(prob) (FRAND (1024) < (1024.0 * (prob)))

struct params {
    int p_size;
    int pb_size;

	int p_size_min;
	int p_size_max;
	int pb_size_min;
	int pb_size_max;
}p;

static uint8_t pool[POOL_MAX_SIZE];

static void task(void *param)
{
    int32_t ret;
    uint32_t id;
    uint32_t **b;
    uint32_t *aux;
    int32_t i;
    int max_allocs;
	struct timeval tm;

    /* seed random number generator */
	gettimeofday (&tm, 0);
	srand (tm.tv_usec);

    p.p_size_min = POOL_BUFFER_MAX_SIZE;
    p.p_size_max = POOL_MAX_SIZE;
    p.pb_size_min = 1;
    p.pb_size_max = POOL_BUFFER_MAX_SIZE;;

    p.p_size = RANDI (p.p_size_min, p.p_size_max);  
    p.p_size &= 0xfffffff0;
    p.pb_size = RANDI (p.pb_size_min, p.pb_size_max);  
    p.pb_size &= 0xfffffff0;
    max_allocs = p.p_size/p.pb_size;

    b = (uint32_t**)OS_Malloc(max_allocs*sizeof(uint32_t*));
    ASSERT( b );

    printf("Creating the pool...");
    ret =  OS_PoolCreate((void*)pool, p.p_size, p.pb_size, &id);
    if( ret != 0 )
    {
        printf("\n(%d) : Cannot create pool\n", (int)ret);
        OS_TaskExit();
    }
    printf("OK!!\n");

    for( i = 0; i < max_allocs; ++i )
    {
        ret = OS_GetPoolBuffer( id, (void**)&b[i] );
        ASSERT( ret == 0 );
        ASSERT( b[i] );
        if( ret != 0 )
            goto err;

        OS_Sleep(100);
    }

    /*  Perform over-allocation - error!!!! */
    ret = OS_GetPoolBuffer( id, (void**)&aux );
    ASSERT( ret != 0 );
    if( ret == 0 )
        goto err;

    /*  Release two of the buffers  */
    ret = OS_ReturnPoolBuffer(id, b[1]);
    ret = OS_ReturnPoolBuffer(id, b[2]);
    ASSERT( ret == 0 );
    if( ret != 0 )
        goto err;

    /*  Allocate one more - good!!  */
    ret = OS_GetPoolBuffer( id, (void**)&b[1] );
    ASSERT( ret == 0 );
    if( ret != 0 )
        goto err;

    printf("==============\n");
    printf("TEST SUCCESS!!\n");
    printf("==============\n");

    OS_PoolDelete(id);
    OS_Free(b);

    return;

err:
    OS_Free(b);
    OS_PoolDelete(id);
    printf("============\n");
    printf("TEST ERROR!!\n");
    printf("============\n");
}

int main(void)
{
    uint32_t t1;
    int32_t ret;

    OS_Init();

    printf("===================\n");
    printf("MEMORY POOL EXAMPLE\n");
    printf("===================\n");
    
    ret = OS_TaskCreate (&t1,(void *)task, 2048, 99, 0, (void*)NULL);
    if( ret < 0 )
    {
    	printf("ERR: unable to create the tasks\n");
        return -1;
    }
    
    OS_Start();


    return 0;

} /* end OS_Application Startup */



