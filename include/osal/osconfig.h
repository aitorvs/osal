/**
 *  \file   osconfig.h
 *  \brief  Operating System configuration items.
 *
 *  This file defines the configuration for the OSAPI.
 *  <p>
 *  The OSAPI layer configure statically the number of tasks,
 *  queues, semaphores, mutex and timers.
 *  The application shall define the CONFIG_MAX_NUMBER_XXX labels
 *  in order to configure the OSAPI layer.
 *
 *  \author  Aitor Viana Sanchez (avs), Aitor.Viana.Sanchez@esa.int
 *
 *  \internal
 *    Created:  29/01/08
 *   Revision:  $Id: osconfig.h 1.4 29/01/08 avs Exp $
 *   Compiler:  gcc/g++
 *    Company:  European Space Agency (ESA-ESTEC)
 *  Copyright:  Copyright (c) 2008, Aitor Viana Sanchez
 *
 *  This source code is released for free distribution under the terms of the
 *  GNU General Public License as published by the Free Software Foundation.
 * =====================================================================================
 */

#ifndef _OSCONFIG_
#define _OSCONFIG_

#include <public/osal_config.h>

/** Internal mutex used to make some resources thread-safe.
 * The internal resources are: tasks, bin semaphores, mutex, counting
 * semaphores, timers, pools and each resource takes 2 mutex to make it
 * thread-safe.
 * In total n_of_resources x 2
 */
#define INTERNAL_MUTEX  14

/*  Times the minimum stack size for the task's stack   */
#define OS_MIN_STACK_TIMES      CONFIG_OS_MIN_STACK_TIMES
/** Is the maximum number of tasks that can be concurrently active */
#define OS_MAX_TASKS            (CONFIG_MAX_NUMBER_OF_TASKS + CONFIG_MAX_NUMBER_OF_MONOTONIC_TASKS)
/** Is the maximum number of rate monotonic periods that can be 
 * concurrently active */
#define OS_MAX_MONOTONIC_TASKS  CONFIG_MAX_NUMBER_OF_MONOTONIC_TASKS
/** Is the maximum number of queues that can be concurrently active */
#define OS_MAX_QUEUES           CONFIG_MAX_NUMBER_OF_QUEUES
/** Maximum number of queues in the OS  */
#define OS_MAX_POOLS            CONFIG_MAX_NUMBER_OF_POOLS
/** Maximum number of binary sempahores in the OS  */
#define OS_MAX_BIN_SEMAPHORES   CONFIG_MAX_NUMBER_OF_SEMAPHORES
/** Maximum number of counting sempahores in the OS  */
#define OS_MAX_COUNT_SEMAPHORES	CONFIG_MAX_NUMBER_OF_SEMAPHORES
/** Is the maximum number of semaphores that can be concurrently active */
#define OS_MAX_SEMAPHORES		OS_MAX_BIN_SEMAPHORES
/** Is the maximum number of mutexes that can be concurrently active */
#define OS_MAX_MUTEXES          (CONFIG_MAX_NUMBER_OF_MUTEX + INTERNAL_MUTEX)
/** Is the maximum number of timers that can be concurrently active */
#define OS_MAX_TIMERS           CONFIG_MAX_NUMBER_OF_TIMERS

/** Is set to the number of bytes the applications wishes to add to the task
 * stack requirements calculated by the OS. If the application creates tasks
 * with stacks greater than the minimum, then that memory is NOT accounted for
 * by the OS. If not defined, the default value is '0'*/
#define OS_EXTRA_STACK_OVERHEAD CONFIG_EXTRA_STACK_OVERHEAD
/** Is set to the number of kilobytes the application wishes to add to the
 * requirements calculated by the OS. Default value is '0' */
#define OS_EXTRA_MEMORY_OVERHEAD CONFIG_EXTRA_MEMORY_OVERHEAD

#endif  /*OSCONFIG*/

