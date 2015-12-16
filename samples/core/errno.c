/**
 *  \file   errno.c
 *  \brief  This program checks the thread-safe errno implementation
 *
 *  \author  Aitor Viana Sanchez (avs), Aitor.Viana.Sanchez@esa.int
 *
 *  \internal
 *    Created:  09/16/2009
 *   Revision:  $Id: prio_send.c 1.4 09/16/2009 avs Exp $
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
#include <time.h>
#include <string.h>
#include <stdlib.h>
#include <assert.h>

uint32_t qid;

static void t1(void)
{
    int i = 50;
    int ret;

    ret = OS_QueueCreate (&qid, (char*)NULL, 10, 10, 30, OS_NONBLOCKING);
    if( ret < 0 )
    {
        printf("Error %d creating the queue - EXPECTED\n", os_errno);
    }
    printf("Performing the TEST\n");

    while(--i)
    {
        assert(os_errno != 0);
        OS_Sleep(500);
        printf(".");fflush(stdout);
    }
    printf("\n");
    printf("-----> TEST OK\n");

    OS_TaskExit();
}

static void t2(void)
{
    int i = 50;

    while(--i)
    {
        assert(os_errno == 0);
        OS_Sleep(500);
    }

    OS_TaskExit();
}

int main(void)
{
    uint32_t id;
    int32_t ret;

	printf("Priority Queue Test Application\n");
    
    OS_Init();

    ret = OS_TaskCreate (&id,(void *)t2, 2048, 99, 0, (void*)NULL);
    ret = OS_TaskCreate (&id,(void *)t1, 2048, 98, 0, (void*)NULL);
    if( ret < 0 )
    {
        printf("Error creating tasks\n");
        return -1;
    }

    OS_Start();


    return 0;

} /* end OS_Application Startup */

