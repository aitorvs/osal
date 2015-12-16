/**
 *  \file   gmon_eth.c
 *  \brief  This file implements the Ethernet communication used by the
 *  profilling tool
 *
 *  \author  Aitor Viana Sanchez (avs), Aitor.Viana.Sanchez@esa.int
 *
 *  \internal
 *    Created:  05/17/2010
 *   Revision:  $Id: gmon_eth.c 1.4 05/17/2010 avs Exp $
 *   Compiler:  gcc/g++
 *    Company:  European Space Agency (ESA-ESTEC)
 *  Copyright:  Copyright (c) 2010, Aitor Viana Sanchez
 *
 *  This source code is released for free distribution under the terms of the
 *  GNU General Public License as published by the Free Software Foundation.
 * =====================================================================================
 */

#include <public/osal_config.h>

#if (CONFIG_OS_PROFILE_OVER_ETH == 1)

#include <stdint.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <netdb.h>


static int sofd = 0;
#define HOST (CONFIG_OS_PROFILE_REMOTE_IPADDR)
#define PORT (CONFIG_OS_PROFILE_REMOTE_PORT)

int32_t gmon_initialize_eth(void)
{
    int ret;
    int on;

    ret = socket( AF_INET, SOCK_DGRAM, IPPROTO_UDP );
    if( ret <= 0 ) return -1;

    sofd = ret;
    
    on = 1;
    ret = setsockopt( sofd, SOL_SOCKET, SO_REUSEADDR, (const char*)&on, sizeof(on) );
    if( ret < 0 )
    {
        close( sofd );
        return ret;
    }

    return 0;
    
}

void gmon_write_eth( char vector[], uint32_t dim)
{
    struct sockaddr_in addr;

    if( vector == NULL ) return;

    bzero( &addr, sizeof(addr) );
    addr.sin_family = AF_INET;
    addr.sin_port = htons(PORT);

    inet_aton(HOST, &addr.sin_addr);
    sendto( sofd, vector, dim, 0, (struct sockaddr*)&addr, sizeof(addr));
}

#endif

