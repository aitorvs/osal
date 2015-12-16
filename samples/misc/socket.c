/**
 *  \file   socket.c
 *  \brief  This file implements abstracts the socket communication means
 *
 *  \author  Aitor Viana Sanchez (avs), Aitor.Viana.Sanchez@esa.int
 *
 *  \internal
 *    Created:  09/10/2009
 *   Revision:  $Id: socket.c 1.4 09/10/2009 avs Exp $
 *   Compiler:  gcc/g++
 *    Company:  European Space Agency (ESA-ESTEC)
 *  Copyright:  Copyright (c) 2009, Aitor Viana Sanchez
 *
 *  This source code is released for free distribution under the terms of the
 *  GNU General Public License as published by the Free Software Foundation.
 * =====================================================================================
 */

#include <stdint.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <netdb.h>

int32_t socket_create(int32_t *m_sock, uint32_t sock_type)
{
    int32_t ret;
    int on;

    ASSERT(m_sock);
    if( m_sock == NULL )    return -1;

    switch( sock_type )
    {
        case SOCK_UDP:
        {
            ret = socket( AF_INET, SOCK_DGRAM, IPPROTO_UDP );
            ASSERT( ret > 0 );
            if( ret > 0 ) *m_sock = ret;
            else return ret;

            break;
        }
        case SOCK_TCP:
        {
            ret = socket( AF_INET, SOCK_STREAM, 0 );
            ASSERT( ret > 0 );
            if( ret > 0 ) *m_sock = ret;
            else return ret;

            break;
        }
        default:
            return -1;
    }

    on = 1;
    ret = setsockopt( *m_sock, SOL_SOCKET, SO_REUSEADDR, (const char*)&on, sizeof(on) );
    ASSERT( ret == 0 );
    if( ret < 0 ) return -1;

    return 0;
}

int32_t socket_bind( int32_t port, int32_t m_sock )
{
    struct sockaddr_in addr;
    int32_t ret;

    memset(( char*)&addr, 0, sizeof(addr) );

    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = INADDR_ANY;
    addr.sin_port = htons( port );

    ret = bind( m_sock, (struct sockaddr*)&addr, sizeof(addr) );
    ASSERT( ret >= 0 );

    return ret;

}

int32_t socket_connect( const char *host, int32_t port, int32_t m_sock )
{
    int32_t ret;
    struct sockaddr_in addr;

    ASSERT( host );
    if( host == NULL ) return -1;

    addr.sin_family = AF_INET;
    addr.sin_port = htons(port);

    ret = inet_pton( AF_INET, host, &addr.sin_addr);
    ASSERT( ret > 0 );
    if( ret <= 0 ) return -1;

    ret = connect( m_sock, (struct sockaddr*)&addr, sizeof(addr) );
    ASSERT(ret == 0 );

    return ret;
}

int32_t socket_send( void *buf, uint32_t len, int32_t m_sock)
{
    int32_t ret;

    ASSERT( buf );
    if( buf == NULL ) return -1;
    ret = send( m_sock, buf, len, MSG_DONTROUTE);
    ASSERT( ret == len );
    if( ret != len ) return -1;

    return ret;
}

int32_t socket_recv( void *buf, uint32_t len, uint32_t m_sock )
{
    int32_t ret;

    ASSERT( buf );
    if( buf == NULL ) return -1;
    ret = recv( m_sock, buf, len, 0 );
    ASSERT( ret >= 0 );
    if ( ret < 0 ) return -1;

    return ret;
}

int32_t socket_sendto( void *buf, uint32_t len, const char *host, int32_t port, int32_t m_sock )
{
    int32_t ret;
    struct sockaddr_in dest;

    ASSERT( buf );
    if( buf == NULL ) return -1;
    ASSERT( host );
    if( host == NULL ) return -1;

    memset(( char*)&dest, 0, sizeof(dest) );
    dest.sin_family = AF_INET;
    dest.sin_port = htons(port);

    ret = inet_aton(host, &dest.sin_addr);
    ASSERT( ret != 0 );
    if( ret == 0 ) return -1;

    ret = sendto( m_sock, buf, len, 0, (struct sockaddr *)&dest, sizeof(dest) );
    ASSERT( ret > 0 );
    if( ret < 0 ) return -1;

    return ret;
}

int32_t socket_recvfrom( void *buf, uint32_t len, int32_t m_sock )
{
    struct sockaddr_in addr;
    int32_t socklen = sizeof(addr);
    int32_t ret;

    ASSERT( buf );
    if( buf == NULL ) return -1;

    ret = recvfrom( m_sock, buf, len, 0, (struct sockaddr*)&addr, (socklen_t*)&socklen);
    ASSERT( ret >= 0 );

    return ret;
}


int32_t socket_close(int m_sock)
{
    int32_t ret;

    ret = close(m_sock);

    ASSERT( ret >= 0 );
    if( ret < 0 ) return -1;
    else return 0;
}

