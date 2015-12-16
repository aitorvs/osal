/**
 *  \file   osdebug.h
 *  \brief  This header file features all the debuggin capabilities used
 *  internaly in the OSAL library
 *
 *  Detailed description starts here.
 *
 *  \author  Aitor Viana Sanchez (avs), Aitor.Viana.Sanchez@esa.int
 *
 *  \internal
 *    Created:  11/25/2009
 *   Revision:  $Id: osdebug.h 1.4 11/25/2009 avs Exp $
 *   Compiler:  gcc/g++
 *    Company:  European Space Agency (ESA-ESTEC)
 *  Copyright:  Copyright (c) 2009, Aitor Viana Sanchez
 *
 *  This source code is released for free distribution under the terms of the
 *  GNU General Public License as published by the Free Software Foundation.
 * =====================================================================================
 */


#ifndef _OSDEBUG_H_
#define _OSDEBUG_H_

#include <public/osal_config.h>
#include <glue/breakme.h>

#ifdef CONFIG_DEBUG
#define _DEBUG


#include <stdio.h>
#include <stdlib.h>

/* #define PRINT(args...) fprintf (stderr, ##args) */
#   define PRINT printf
#   define DEBUG(fmt,args...)      PRINT ("%s: " fmt "\n", __func__, ##args)
#   define TRACE(expr,type)        DEBUG (#expr "='%" type "'", expr)
#   define BREAK(expr)    		if (expr) breakme ()

#else
#   undef _DEBUG
#   define PRINT(args...)       do{}while(0)
#   define DEBUG(fmt,args...)   do{}while(0)
#   define TRACE(expr,type)     do{}while(0)
#   define BREAK(expr)          do{}while(0)
#endif  // CONFIG_DEBUG

#ifdef CONFIG_ASSERT

#include <stdio.h>
#include <stdlib.h>

#   define ASSERT(expr)    \
	if (! (expr)) { \
		printf(__FILE__ ":%d: %s: Assertion `" #expr "' failed.\n", \
				__LINE__, __func__); \
		breakme (); \
		exit (1); \
	}

#else
#   define ASSERT(expr) do{}while(0);
#endif  //CONFIG_ASSERT

#endif

