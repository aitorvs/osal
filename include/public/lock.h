/**
 *  \file   mil1553_lock.h
 *  \brief  This module implements read and write locks to produce thread-safe
 *  code.
 *
 *  The module implement the read_lock, read_unlock, write_lock and
 *  write_unlock functionalities to allow the user to implement thread-safe
 *  code.
 *  Those functionalities are implemted using the OSAL layer.
 *
 *  \author  Aitor Viana Sanchez (avs), Aitor.Viana.Sanchez@esa.int
 *
 *  \internal
 *    Created:  10/04/09
 *   Revision:  $Id: mil1553_lock.h 1.4 10/04/09 avs Exp $
 *   Compiler:  gcc/g++
 *    Company:  European Space Agency (ESA-ESTEC)
 *  Copyright:  Copyright (c) 2009, Aitor Viana Sanchez
 *
 *  This source code is released for free distribution under the terms of the
 *  GNU General Public License as published by the Free Software Foundation.
 * =====================================================================================
 */

#ifndef _LOCK_H
#define _LOCK_H

#ifdef __cplusplus
extern "C" {
#endif

typedef uint32_t lock_t;
typedef uint32_t lock_event_t;

#define lock(l) 		OS_MutSemTake((l))
#define lock_event_wait(l) 	OS_MutSemTake((l))
#define unlock(l)		OS_MutSemGive((l))
#define lock_event_send(l)	OS_MutSemGive((l))

/* IMPORTANT: use a standard (not simple) binary semaphore that may nest */
static __inline__ int
lock_create(lock_t *l)
{
	return OS_MutSemCreate(l, 0);
}

static __inline__ int
lock_event_create(lock_event_t *l)
{
	return OS_MutSemCreate(l, 0);
}

#define lock_destroy(l) OS_MutSemDelete((l))
#define lock_event_destroy(l) OS_MutSemDelete((l))


/**
 *  \class lock_rw_s
 *
 *  \brief This class structure declares a lock object.
 */
typedef struct {
    /** This field is the mutex which protects the resources inside this 
     * class */
	lock_t	mutex;
    /** This field accounts the number of readers accessing the resource  */
	unsigned	readers;
    /** This field accounts the number of tasks napping on the resource */
	lock_event_t	nap;
    /** This field accounts the number of writters waiting on the resource  */
	unsigned	sleeping_writers;
} lock_rw_s;

static lock_rw_s _rwlock={0};
#define __RLOCK()   read_lock(&_rwlock)
#define __RUNLOCK() read_unlock(&_rwlock)
#define __WLOCK()   write_lock(&_rwlock)
#define __WUNLOCK() write_unlock(&_rwlock);


static __inline__ int
lock_rw_init(lock_rw_s * pl)
{
    int status = 0;

	status = lock_create(&pl->mutex);
    if( status != 0 ) goto error_lock_create;

	status = lock_event_create(&pl->nap);
    if( status != 0 ) goto error_lock_event_create;

	pl->readers=0;
	pl->sleeping_writers=0;

    return 0;   /* SUCCESS  */

error_lock_event_create:
    lock_event_destroy(pl->nap);
error_lock_create:
    lock_destroy(pl->mutex);


    return status;  /*  ERROR   */
}

/* Readers / Writer lock implementation. This
 * should be a little more efficient for the common
 * case of a single reader which only has to 
 * acquire one mutex.
 * NOTE: A thread holding the write-lock may still
 *       do nested readLock() readUnlock() calls.
 *       However, a reader _MUST_NOT_ try to acquire
 *       the write lock!
 */
static __inline__  void
read_lock(lock_rw_s * l)
{
	lock(l->mutex);
	l->readers++;
	unlock(l->mutex);
}

static __inline__ void
read_unlock(lock_rw_s * l)
{
	lock(l->mutex);
	if (0 == --l->readers && l->sleeping_writers) {
		l->sleeping_writers--;
		lock_event_send(l->nap);
	}
	unlock(l->mutex);
}

static __inline__ void
write_lock(lock_rw_s * l)
{
	lock(l->mutex);
	while (l->readers) {
		l->sleeping_writers++;
		unlock(l->mutex);
		lock_event_wait(l->nap);
		lock(l->mutex);
	}
}

static __inline__ void
write_unlock(lock_rw_s * l)
{
	unlock(l->mutex);
}

#ifdef __cplusplus
}
#endif

#endif
