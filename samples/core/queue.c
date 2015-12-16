/**
 *  \file   prio_send.c
 *  \brief  This program creates message queue, sends 10 messages to it with
 *  random priorities and finishes
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

uint32_t id;

static char buffer[1024];

static void receiver(void)
{
    int32_t ret;
    char text[512];
    size_t rb = 0;
    int i;

    srand(time(NULL));
    for( i = 0; i < 10; ++i)
    {
        rb = 0;
        bzero(text, sizeof(text));
        ret = OS_QueueGet (id, (void*)text, sizeof(text), &rb, 3000);
        if( ret < 0 )
        {
            if(os_errno == OS_STATUS_TIMEOUT)
                printf("%s: ERROR Timeout\n", __func__);
            else
                printf("(%s: ERROR Unknown (%d)\n", __func__, (int)os_errno);
        }
        else
        {
            printf("Received %d bytes %s\n", (int)rb, text);
        }

        OS_Sleep(500);
    }

    OS_QueueDelete(id);

    OS_TaskExit();
}

#define MESSAGE "Hello message %d prio %d"
static void sender(void)
{
    int32_t ret;
    char text[32];
    int i;

    OS_Sleep(4000);
    srand(time(NULL));
    for( i = 0; i < 10; ++i)
    {
        int p = rand()%10;
        snprintf (text, sizeof(text), MESSAGE, i, p);
        ret = OS_QueuePut (id, (void*)text, strlen(text), p);
        if( ret < 0 )
        {
            printf("Error sending message:(%d)\n", (int)ret);
        }
        else
            printf("Message sent: %s\n", text);
    }

    OS_TaskExit();
}

int main(void)
{
    uint32_t t1;
    int32_t ret;

	printf("Priority Queue Test Application\n");
    
    OS_Init();

    ret = OS_QueueCreate (&id, (char*)buffer, sizeof(buffer), 10, 30, OS_NONBLOCKING);
    if( ret < 0 )
    {
        printf("Error creating the queue\n");
        return -1;
    }
    
    ret = OS_TaskCreate (&t1,(void *)sender, 2048, 99, 0, (void*)NULL);
    ret = OS_TaskCreate (&t1,(void *)receiver, 2048, 98, 0, (void*)NULL);
    if( ret < 0 )
    {
        printf("Error creating tasks\n");
        return -1;
    }

    OS_Start();


    return 0;

} /* end OS_Application Startup */

