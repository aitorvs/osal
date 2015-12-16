/**
 *  \file   counting.c
 *  \brief  This file implements the example for using counting semaphores
 *  under OSAL
 *
 *  Detailed description starts here.
 *
 *  \author  Aitor Viana Sanchez (avs), Aitor.Viana.Sanchez@esa.int
 *
 *  \internal
 *    Created:  10/28/2009
 *   Revision:  $Id: counting.c 1.4 10/28/2009 avs Exp $
 *   Compiler:  gcc/g++
 *    Company:  European Space Agency (ESA-ESTEC)
 *  Copyright:  Copyright (c) 2009, Aitor Viana Sanchez
 *
 *  This source code is released for free distribution under the terms of the
 *  GNU General Public License as published by the Free Software Foundation.
 * =====================================================================================
 */

#include <osal/osapi.h>
#include <stdio.h>

#define LOOPS   100

static void task1(void)
{
    int32_t ret;
    uint32_t semid;
    int i;

    ret = OS_CountSemCreate (&semid, 0, 0);
    if( ret < 0 )
    {
        printf("Error creating the semaphore\n");
        goto err;
    }

    /*  try to get the semaphore several times...shall fail */
    for( i = 0; i < LOOPS; ++i )
    {
        ret = OS_CountSemTryTake(semid);
        if( ret >= 0 )
        {
            printf("OS_CountSemTryTake() call failed\n");
            goto err;
        }
    }

    for( i = 0; i < LOOPS/2; ++i )
    {
        ret = OS_CountSemGive(semid);
        if( ret < 0 )
        {
            printf("OS_CountSemGive() call failed\n");
            goto err;
        }
    }
    for( i = 0; i < LOOPS/2; ++i )
    {
        if( i & 0x1 )
            ret = OS_CountSemTake(semid);
        else
            ret = OS_CountSemTryTake(semid);

        if( ret < 0 )
        {
            printf("OS_CountSemTryTake() call failed\n");
            goto err;
        }
    }

    /*  The next call shall fail    */
    ret = OS_CountSemTryTake(semid);
    if( ret == 0 )
    {
        printf("OS_CountSemTryTake() call failed\n");
        goto err;
    }
    ret = OS_CountSemTimedWait(semid, 100);
    if( ret == 0 )
    {
        printf("OS_CountSemTimedWait() call failed\n");
        goto err;
    }

    /*  Give the semaphore so we can success in another call    */
    ret = OS_CountSemGive(semid);
    if( ret < 0 )
    {
        printf("OS_CountSemGive() call failed\n");
        goto err;
    }
    ret = OS_CountSemTimedWait(semid, 100);
    if( ret < 0 )
    {
        printf("OS_CountSemTimedWait() call failed\n");
        goto err;
    }

    ret = OS_CountSemDelete(semid);
    if( ret < 0 )
    {
        printf("OS_CountSemDelete() call failed\n");
        goto err;
    }


    printf("==============\n");
    printf("TEST PASSED!!!\n");
    printf("==============\n");

    return;

err:
    printf("============\n");
    printf("TEST ERROR!!\n");
    printf("============\n");
}

int main(void)
{
    uint32_t t1;

    printf("====================\n");
    printf("COUNTING SEM EXAMPLE\n");
    printf("====================\n");
    
    OS_Init();
    
    if( OS_TaskCreate (&t1,(void *)task1, 2048, 99, 0, (void*)NULL) != OS_STATUS_SUCCESS)
    	printf("ERR: unable to create the tasks\n");
    
    OS_Start();


    return 0;

} /* end OS_Application Startup */



