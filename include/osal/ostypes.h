/**
 *  \file   common_types.h
 *  \brief  This file defines all the basic types being used by the OSAL
 *  abstraction layer.
 *
 *  Detailed description starts here.
 *
 *  \author  Aitor Viana Sanchez (avs), Aitor.Viana.Sanchez@esa.int
 *
 *  \internal
 *    Created:  29/01/08
 *   Revision:  $Id: common_types.h 1.4 29/01/08 avs Exp $
 *   Compiler:  gcc/g++
 *
 * =====================================================================================
 */

#ifndef _COMMON_TYPES_
#define _COMMON_TYPES_

#include <stdint.h>
#include <stddef.h>

/*----- Basic data types -----*/

#define LOCAL static /**< \brief Private Data type */

#ifndef BOOLEAN
#undef BOOLEAN
typedef uint32_t         BOOLEAN; /**< \brief Boolean Type */
#endif

/*----- BOOLEAN constants -----*/
#ifndef FALSE
#   define FALSE (0)
#endif

#ifndef TRUE
#   define TRUE (!FALSE)
#endif

#define UNINITIALIZED FALSE

/** NULL pointer   */
#ifndef NULL
#   define NULL ((void *) 0) 
#endif

/**
  * \todo Other macros (NOTE: these may be removed eventually)
  */
#define OS_ABS(x)    ( ((x) <  0 ) ? -(x) : (x) )

/**
  * \todo Other macros (NOTE: these may be removed eventually)
  */
#define OS_PACK

#define likely(x)   __builtin_expect(!!(x), 1)
#define unlikely(x) __builtin_expect(!!(x), 0)


#endif /*_COMMON_TYPES_*/

