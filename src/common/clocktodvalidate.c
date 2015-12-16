/*
 *  Time of Day (TOD) Handler -- Validate Classic TOD
 *
 *
 *  COPYRIGHT (c) 1989-2007.
 *  On-Line Applications Research Corporation (OAR).
 *
 *  The license and distribution terms for this file may be
 *  found in the file LICENSE in this distribution or at
 *  http://www.rtems.com/license/LICENSE.
 *
 *  $Id: clocktodvalidate.c,v 1.9 2007/04/02 21:51:52 joel Exp $
 */

#include <osal/osapi.h>
#include <glue/tod.h>

/*
 *  The following array contains the number of days in all months.
 *  The first dimension should be 1 for leap years, and 0 otherwise.
 *  The second dimension should range from 1 to 12 for January to
 *  February, respectively.
 */
const uint32_t   TOD_Days_per_month[ 2 ][ 13 ] = {
    { 0, 31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31 },
    { 0, 31, 29, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31 }
};

int32_t OS_tod_validate(
        OS_time_of_day_t *the_tod
        )
{
    uint32_t   days_in_month;

    if ((!the_tod)                                  ||
            (the_tod->mul_MicroSeconds  >= TOD_MICROSECONDS_PER_SECOND) ||
            (the_tod->mul_Seconds >= TOD_SECONDS_PER_MINUTE) ||
            (the_tod->mul_Minute >= TOD_MINUTES_PER_HOUR)   ||
            (the_tod->mul_Hour   >= TOD_HOURS_PER_DAY)      ||
            (the_tod->mul_Month  == 0)                      ||
            (the_tod->mul_Month  >  TOD_MONTHS_PER_YEAR)    ||
            (the_tod->mul_Year   <  TOD_BASE_YEAR)          ||
            (the_tod->mul_Day    == 0) )

        return 0;

    if ( (the_tod->mul_Year % 4) == 0 )
        days_in_month = TOD_Days_per_month[ 1 ][ the_tod->mul_Month ];
    else
        days_in_month = TOD_Days_per_month[ 0 ][ the_tod->mul_Month ];

    if ( the_tod->mul_Day > days_in_month )
        return 0;

    return 1;
}
