/*
 *  Time of Day (TOD) Handler - Classic TOD to Seconds
 *
 *  COPYRIGHT (c) 1989-2007.
 *  On-Line Applications Research Corporation (OAR).
 *
 *  The license and distribution terms for this file may be
 *  found in the file LICENSE in this distribution or at
 *  http://www.rtems.com/license/LICENSE.
 *
 *  $Id: clocktodtoseconds.c,v 1.8 2007/04/05 21:17:26 joel Exp $
 */

#include <osal/osapi.h>
#include <glue/tod.h>

/*
 *  The following array contains the number of days in all months
 *  up to the month indicated by the index of the second dimension.
 *  The first dimension should be 1 for leap years, and 0 otherwise.
 */
static const uint16_t   TOD_Days_to_date[2][13] = {
    { 0, 0, 31, 59, 90, 120, 151, 181, 212, 243, 273, 304, 334 },
    { 0, 0, 31, 60, 91, 121, 152, 182, 213, 244, 274, 305, 335 }
};

/*
 *  The following array contains the number of days in the years
 *  since the last leap year.  The index should be 0 for leap
 *  years, and the number of years since the beginning of a leap
 *  year otherwise.
 */
static const uint16_t   TOD_Days_since_last_leap_year[4] = { 0, 366, 731, 1096 };



int32_t   OS_tod_to_seconds(
        OS_time_of_day_t *the_tod
        )
{
    uint32_t   time;
    uint32_t   year_mod_4;

    time = the_tod->mul_Day - 1;
    year_mod_4 = the_tod->mul_Year & 3;

    if ( year_mod_4 == 0 )
        time += TOD_Days_to_date[ 1 ][ the_tod->mul_Month ];
    else
        time += TOD_Days_to_date[ 0 ][ the_tod->mul_Month ];

    time += ( (the_tod->mul_Year - TOD_BASE_YEAR) / 4 ) *
        ( (TOD_DAYS_PER_YEAR * 4) + 1);

    time += TOD_Days_since_last_leap_year[ year_mod_4 ];

    time *= TOD_SECONDS_PER_DAY;

    time += ((the_tod->mul_Hour * TOD_MINUTES_PER_HOUR) + the_tod->mul_Minute)
        * TOD_SECONDS_PER_MINUTE;

    time += the_tod->mul_Seconds;

    time += TOD_SECONDS_1970_THROUGH_1988;

    return( time );
}
