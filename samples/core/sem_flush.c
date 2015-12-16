/**
 *  \file   semflush.c
 *  \brief  This example implements a test to verify that the OS_BinSemFlush()
 *  function is working properly.
 *
 *  The example creates a number of tasks with different priorities. All the
 *  tasks block in a semaphore waiting for the startup task to flush it. The
 *  test shall print the task id in order (0, 1, 2, 3...), otherwise the
 *  semaphore mechanism is not working properly.
 *
 *  \author  Aitor Viana Sanchez (avs), Aitor.Viana.Sanchez@esa.int
 *
 *  \internal
 *    Created:  01/14/2010
 *   Revision:  $Id: semflush.c 1.4 01/14/2010 avs Exp $
 *   Compiler:  gcc/g++
 *    Company:  European Space Agency (ESA-ESTEC)
 *  Copyright:  Copyright (c) 2010, Aitor Viana Sanchez
 *
 *  This source code is released for free distribution under the terms of the
 *  GNU General Public License as published by the Free Software Foundation.
 * =====================================================================================
 */

#include <osal/osapi.h>
#include <osal/osdebug.h>

#include <stdio.h>

#define TEST_TASKS  (5)
#define TEST_TASK_BASE_PRIO 50
#define TEST_TASK_STACK 512

static uint32_t semid;
static uint32_t tid[TEST_TASKS];

void task(void *_taskid)
{
    uint32_t taskid = (uint32_t)_taskid;

    printf("Starting Task %d\n", (int)taskid);
    OS_BinSemTake(semid);
    printf("Task %d\n", (int)taskid);

    OS_TaskExit();
}

void startup(void *ignored)
{
    OS_Sleep(500);
    PRINT("Startup...");
    OS_BinSemFlush(semid);
    PRINT("Done\n");
    OS_TaskExit();
}

int main(void)
{
    uint32_t id;
    int i;

    OS_Init();

    if( OS_BinSemCreate(&semid, 0, 0) != 0 )
    {
        PRINT("ERR: unable to create the semaphore\n");
        goto err;
    }

    for( i = 0; i < TEST_TASKS; ++i )
    {
        if( OS_TaskCreate (&tid[i],(void *)task, TEST_TASK_STACK, 
                    TEST_TASK_BASE_PRIO+i, 0, (void*)i) != 0 )
        {
            PRINT("ERR: unable to create the task %d\n", i);
            goto err;
        }
    }

    if( OS_TaskCreate (&id,(void *)startup, TEST_TASK_STACK, 
                40, 0, (void*)i) != 0 )
    {
        PRINT("ERR: unable to create the task %d\n", i);
        goto err;
    }

    OS_Start();

err:
    return 0;
}

