/**
 *  \file   osapi.h
 *  \brief  OSAL public items
 *
 *  This file includes all the header files where the osapi functions
 *  are declared.
 *  Also, all the error codes are defined here.
 *  <p>
 *  The file shall be include in every project that wants to use the OSAPI
 *  interfaces.
 *
 *  \author  Alan Cudmore - Code 582
 *  \author  Aitor Viana Sanchez (avs), Aitor.Viana.Sanchez@esa.int
 *
 *  \internal
 *    Created:  29/01/08
 *   Revision:  $Id: osapi.h 1.4 29/01/08 avs Exp $
 *   Compiler:  gcc/g++
 *
 * =====================================================================================
 */

#ifndef _OSAPI_H_
#define _OSAPI_H_

#ifdef __cplusplus
extern "C" {
#endif

#include "oserr.h"
#include "ostypes.h"

/** This macro allows to eliminate warnings from unused parameters  */
#define UNUSED(x)   ((void)x)

/** OSAL ticks type */
typedef uint64_t OS_ticks_t;

#define OS_BLOCKING    (0) /**< \brief Blocking resource */
#define OS_NONBLOCKING (1) /**< \brief Non-blocking resource */

/** Blocking mode configuration for Queues */
#define OS_QUEUE_BLOCK      (0)
/** Non-Blocking mode configuration for Queues */
#define OS_QUEUE_NONBLOCK   (1)

#define OS_SELF     0x0000ffff /**< \brief Self resource */


#include "osconfig.h"
/** The core API is declared in this header file */
#include "oscore.h"
#include "ostask.h"
#include "osmut.h"
#include "osqueue.h"
#include "ossem.h"
#include "ostime.h"
#include "ostimer.h"
#include "osint.h"
#include "ospool.h"
#include "osmemmgr.h"

/** The filesystem API is declared in this header file  */
#include "osapi-compiler-switches.h"
#include "oshwcore.h"
#include "osutils.h"

#if OS_FS_ENABLE_FILESYSTEM
#include "osfile.h"
#include "ospacket.h"
#endif
/*  #include "osapi-os-net.h"
 *  #include "osapi-hw-analog.h"
 *  #include "osapi-hw-pci.h"
 */


#ifdef __cplusplus
}
#endif

#endif  /*_OSAPI_H_*/



