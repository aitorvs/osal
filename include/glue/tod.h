/** 
 *  @file  rtems/score/tod.h
 *
 *  This include file contains all the constants and structures associated
 *  with the Time of Day Handler.
 */

/*
 *  COPYRIGHT (c) 1989-2007.
 *  On-Line Applications Research Corporation (OAR).
 *
 *  The license and distribution terms for this file may be
 *  found in the file LICENSE in this distribution or at
 *  http://www.rtems.com/license/LICENSE.
 *
 *  $Id: tod.h,v 1.30 2007/04/05 22:13:08 joel Exp $
 */

#ifndef _RTEMS_SCORE_TOD_H
#define _RTEMS_SCORE_TOD_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>

/**
 *  This constant represents the number of seconds in a minute.
 */
#define TOD_SECONDS_PER_MINUTE (uint32_t)60

/**
 *  This constant represents the number of minutes per hour.
 */
#define TOD_MINUTES_PER_HOUR   (uint32_t)60

/**
 *  This constant represents the number of months in a year.
 */
#define TOD_MONTHS_PER_YEAR    (uint32_t)12

/**
 *  This constant represents the number of days in a non-leap year.
 */
#define TOD_DAYS_PER_YEAR      (uint32_t)365

/**
 *  This constant represents the number of hours per day.
 */
#define TOD_HOURS_PER_DAY      (uint32_t)24

/**
 *  This constant represents the number of seconds in a day which does
 *  not include a leap second.
 */
#define TOD_SECONDS_PER_DAY    (uint32_t) (TOD_SECONDS_PER_MINUTE * \
                                TOD_MINUTES_PER_HOUR   * \
                                TOD_HOURS_PER_DAY)

/**
 *  This constant represents the number of seconds in a non-leap year.
 */
#define TOD_SECONDS_PER_NON_LEAP_YEAR (365 * TOD_SECONDS_PER_DAY)

/**
 *  This constant represents the number of seconds in a millisecond.
 */
#define TOD_MILLISECONDS_PER_SECOND     (uint32_t)1000

/**
 *  This constant represents the number of microseconds in a second.
 */
#define TOD_MICROSECONDS_PER_SECOND     (uint32_t)1000000

/**
 *  This constant represents the number of nanoseconds in a second.
 */
#define TOD_NANOSECONDS_PER_SECOND      (uint32_t)1000000000

/**
 *  This constant represents the number of nanoseconds in a second.
 */
#define TOD_NANOSECONDS_PER_MICROSECOND (uint32_t)1000

/*
 *  Seconds from January 1, 1970 to January 1, 1988.  Used to account for
 *  differences between POSIX API and RTEMS core. The timespec format time
 *  is kept in POSIX compliant form.
 */
#define TOD_SECONDS_1970_THROUGH_1988 \
  (((TOD_BASE_YEAR - 1970)  * TOD_SECONDS_PER_NON_LEAP_YEAR) + \
  (4 * TOD_SECONDS_PER_DAY))

/** @brief RTEMS Epoch Year
 *
 *  The following constant define the earliest year to which an
 *  time of day can be initialized.  This is considered the
 *  epoch.
 */
#define TOD_BASE_YEAR 1988

/** @brief TOD_MILLISECONDS_TO_MICROSECONDS
 *
 *  This routine converts an interval expressed in milliseconds to microseconds.
 *
 *  @note This must be a macro so it can be used in "static" tables.
 */
#define TOD_MILLISECONDS_TO_MICROSECONDS(_ms) ((uint32_t)(_ms) * 1000L)

/**
 *  \brief This routine returns the seconds from the epoch until the current
 *  date and time.
 *
 *  \param  the_tod pointer to the time and date structure
 *
 *  \return seconds since epoch until the_tod
 */
int32_t   OS_tod_to_seconds(OS_time_of_day_t *the_tod);

/**
 *  \brief This function checks the validity of a date and time structure.
 *
 *  \param  the_tod pointer to a time and date structure
 *
 *  \return TRUE if the date, time and nanosecond are valid
 *  \return FALSE if the 'the_tod' is invalid
 */
int32_t OS_tod_validate(OS_time_of_day_t *the_tod);

#ifdef __cplusplus
}
#endif

/**@}*/

#endif
/* end of include file */
