/**
 *  \file   hello.c
 *  \brief  This file implements the basic hello world example over the OSAL
 *  API.
 *
 *  Detailed description starts here.
 *
 *  \author  Aitor Viana Sanchez (avs), Aitor.Viana.Sanchez@esa.int
 *
 *  \internal
 *    Created:  28/01/08
 *   Revision:  $Id: doxygen.templates.example,v 1.4 2007/08/02 14:35:24 mehner Exp $
 *   Compiler:  gcc/g++
 *    Company:  European Space Agency (ESA-ESTEC)
 *  Copyright:  Copyright (c) 2008, Aitor Viana Sanchez
 *
 *  This source code is released for free distribution under the terms of the
 *  GNU General Public License as published by the Free Software Foundation.
 * =====================================================================================
 */

/*-----------------------------------------------------------------------------
 * Changelog:
 * 29/01/08 10:16:44
 * - Some include header files has been taken out.
 *-----------------------------------------------------------------------------*/

#include <osal/osapi.h>
#include <osal/osdebug.h>

#include "socket.h"
#include "socket.c"

#include <stdio.h>

#if defined (CONFIG_OS_ENABLE_NETWORKING)
char osal_ip_addr[] = "10.4.17.129";    // Scisys network
#endif

static void task1(void)
{
    int ret;
    int32_t m_sock = 0;
    static uint8_t buffer[2048];

    ret = socket_create( &m_sock, SOCK_UDP );
    ASSERT( ret >= 0 );
    if( ret < 0 ) return;
    ret = socket_bind( 30000, m_sock );
    ASSERT( ret >= 0 );
    if( ret < 0 ) return;

    printf("Application task %d running\n", (int)OS_TaskGetId());
    while(1)
    {
        ret = socket_recvfrom( buffer, sizeof(buffer), m_sock);
        ASSERT( ret >= 0 );
        if( ret < 0 )
        {
            printf("Error trying to receiving packets\n");
        }
        else
        {
            printf("Packet received: %d bytes\n", ret);
        }
    }
}

int main(void)
{
    uint32_t t1;

	printf("=====================\n");
	printf("SOCKET client example\n");
	printf("=====================\n");
    
    OS_Init();
    
    if( OS_TaskCreate (&t1,(void *)task1, 2048, 99, 0, (void*)NULL) != 0)
    	printf("ERR: unable to create the tasks\n");
    
    OS_Start();


    return 0;

} /* end OS_Application Startup */



