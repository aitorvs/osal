/**
 *  \file   osmemmgr.h
 *  \brief  This file implements a memory manager allocator based on the
 *  original buddy system memory allocation algorithm.
 *
 *  The original buddy system memory allocation algorithm was taken from
 *  "The Art of Computer Programming, Volume 1, 3rd Edition", by
 *  Donald E. Knuth, pages 442-444.
 *
 *  \author  Aitor Viana Sanchez (avs), Aitor.Viana.Sanchez@esa.int
 *
 *  \internal
 *    Created:  10/01/2009
 *   Revision:  $Id: osmemmgr.h 1.4 10/01/2009 avs Exp $
 *   Compiler:  gcc/g++
 *    Company:  European Space Agency (ESA-ESTEC)
 *  Copyright:  Copyright (c) 2009, Aitor Viana Sanchez
 *
 *  This source code is released for free distribution under the terms of the
 *  GNU General Public License as published by the Free Software Foundation.
 * =====================================================================================
 */

/*
 * Debug Memory allocator.
 *
 * The Debug Memory Allocator is based on the "Malloc Debug Library"
 * (http://www.hexco.de/rmdebug) by Rammi.
 * This allocator wraps malloc and free normal heap handling functions and by
 * demanding a bit more memory from the system writes some special bytefields
 * before and after the buffer they return to the user to store extra info. that
 * allows to debug the memory allocations and monitor the memory areas
 */

/**
 *  \ingroup OSAL
 *  \defgroup Memmgr_API Library Memory Manager API
 *
 *  This API allows the user to handle tasks. The API gets functions for
 *  creationg, deletion, etc.
 */

#ifndef _MM_H_
#define _MM_H_

#include <public/osal_config.h>

/*----------------------------------------------------------------------------*/

/* once again useful: INT2STRING(prepro_macro_containing_number)-->"number" */
#define FUNCTIONIZE(a,b)  a(b)
#define STRINGIZE(a)      #a
#define INT2STRING(i)     FUNCTIONIZE(STRINGIZE,i)
#define RM_FILE_POS       __FILE__ ":" INT2STRING(__LINE__)

/*----------------------------------------------------------------------------*/

/******************************************************************************
 * PUBLIC INTERFACE
 *****************************************************************************/

/**
 *  \ingroup Memmgr_API
 *
 * See man malloc
 * 
 * @param size:  This parameter is the amount of memory that must be allocated
 * 
 * @return The routine returns a pointer to the allocated memory or NULL in any
 * other case
 */

#ifdef CONFIG_OS_MALLOC_DEBUG_LIB
extern void *Rmalloc(size_t size, const char *file);
#   define OS_Malloc(s)  Rmalloc((s), RM_FILE_POS)
#else
#   define OS_Malloc(s)  malloc(s)
#endif

/** 
 *  \ingroup Memmgr_API
 *
 * See man free 
 * 
 * @param ptr:  pointer to the memory that must be freed
 * 
 * OS_Free() is only guaranteed to work if ptr is the address of a block
 * allocated by rt_malloc() (and not yet freed). 
 *
 * \return none
 */

#ifdef CONFIG_OS_MALLOC_DEBUG_LIB
extern void Rfree(void *p, const char *file);
#   define OS_Free(s)  Rfree((s), RM_FILE_POS)
#else
#   define OS_Free(s)  free(s)
#endif

#endif
