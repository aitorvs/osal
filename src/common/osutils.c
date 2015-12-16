/*
 * ============================================================================
 *
 *       Filename:  osutils.c
 *
 *    Description:  This file implements some utility functions
 *
 *        Version:  1.0
 *        Created:  09/09/2009 11:01:27 AM
 *       Modified:  09/09/2009 11:01:27 AM
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  Aitor Viana Sanchez (avs), aitor.viana.sanchez@esa.int
 *        Company:  European Space Agency (ESA-ESTEC)
 *
 * ============================================================================
 */

#include <osal/osapi.h>
#include <osal/osdebug.h>
#include <time.h>

#define MAX_SEC_IN_USEC 4293

void timespec_to_micros(struct timespec time_spec, uint32_t *usecs)
{
    if( (*usecs) < 1000000 )
    {
        *usecs = time_spec.tv_nsec / 1000;
    }
    else
    {
        if( time_spec.tv_sec > MAX_SEC_IN_USEC )
        {
            time_spec.tv_sec = MAX_SEC_IN_USEC;
        }
        *usecs = (time_spec.tv_sec * 1000000) + (time_spec.tv_nsec / 1000) ;
    }
}

void micros_to_timespec(uint32_t usecs, struct timespec *time_spec)
{
    if( usecs < 1000000 )
    {
        time_spec->tv_sec = 0;
        time_spec->tv_nsec = (usecs * 1000);
    }
    else
    {
        time_spec->tv_sec = usecs / 1000000;
        time_spec->tv_nsec = (usecs % 1000000) * 1000;
    }
}



void timespec_add_us(struct timespec *t, uint64_t us)
{
    uint64_t ns = us*1000;
    while( ns >= 1000000000 )
    {
        ns -= 1000000000;
        t->tv_sec += 1;
    }
    t->tv_nsec += ns;
    while( t->tv_nsec >= 1000000000 )
    {
        t->tv_nsec -= 1000000000;
        ASSERT(t->tv_nsec > 0 );
        t->tv_sec += 1;
    }
}

int32_t timespec_cmp(struct timespec *a, struct timespec *b)
{
    if (a->tv_sec > b->tv_sec) return 1;
    else if (a->tv_sec < b->tv_sec) return -1;
    else if (a->tv_sec == b->tv_sec) {
        if (a->tv_nsec > b->tv_nsec) return 1;
        else if (a->tv_nsec == b->tv_nsec) return 0;
        else return -1;
    }

    return -1;
}


