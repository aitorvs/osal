/**
 *  \file   socket.h
 *  \brief  
 *
 *  Detailed description starts here.
 *
 *  \author  Aitor Viana Sanchez (avs), Aitor.Viana.Sanchez@esa.int
 *
 *  \internal
 *    Created:  09/10/2009
 *   Revision:  $Id: socket.h 1.4 09/10/2009 avs Exp $
 *   Compiler:  gcc/g++
 *    Company:  European Space Agency (ESA-ESTEC)
 *  Copyright:  Copyright (c) 2009, Aitor Viana Sanchez
 *
 *  This source code is released for free distribution under the terms of the
 *  GNU General Public License as published by the Free Software Foundation.
 * =====================================================================================
 */

#ifndef _SOCK_H_
#define _SOCK_H_

enum
{
    SOCK_TCP = 0,
    SOCK_UDP = 1,
};

int32_t socket_create(int32_t *m_sock, uint32_t sock_type);
int32_t socket_send( void *buf, uint32_t len, int32_t m_sock);
int32_t socket_recv( void *buf, uint32_t len, uint32_t m_sock );
int32_t socket_close(int m_sock);
int32_t socket_sendto( void *buf, uint32_t len, const char *host, int32_t port, int32_t m_sock );
int32_t socket_recvfrom( void *buf, uint32_t len, int32_t m_sock );
int32_t socket_bind( int32_t port, int32_t m_sock );
int32_t socket_connect( const char *host, int32_t port, int32_t m_sock );

#endif

