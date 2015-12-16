/**
 *  \file   bsp_glue.c
 *  \brief  This file contains the glue routines between the app and the OS
 *  Board Support Package (BSP).
 *
 *  The functionality here allow the app to interface features that are board
 *  or OS specific and dont fit well in the OSAL layer.
 *
 *  \author  Aitor Viana Sanchez (avs), Aitor.Viana.Sanchez@esa.int
 *
 *  \internal
 *    Created:  17/03/09
 *   Revision:  $Id: bsp_glue.c 1.4 17/03/09 avs Exp $
 *   Compiler:  gcc/g++
 *    Company:  European Space Agency (ESA-ESTEC)
 *  Copyright:  Copyright (c) 2009, Aitor Viana Sanchez
 *
 *  This source code is released for free distribution under the terms of the
 *  GNU General Public License as published by the Free Software Foundation.
 * =====================================================================================
 */

#include <osal/osapi.h>
#include <osal/osdebug.h>


/*============================================= MACRO DEFINITIONS */

/*  Timers share the same decrementer. This means that the minimum allowed
 *  prescaler division factor is ntimers+1 (prescaler reload = ntimers)
 */
#define GPTIMER_NUMBER_OF_TIMERS    (2)
#define WDOG_MIN_TIME   (GPTIMER_NUMBER_OF_TIMERS)
#define WDOG_MAX_TIME   0xffffffff

/** Bits in the timer control register  */
#define GPTIMER_CONTROL_ENABLE      (0x1)
#define GPTIMER_CONTROL_RESTART     (0x2)
#define GPTIMER_CONTROL_LOAD        (0x4)
#define GPTIMER_CONTROL_IRQ_ENABLE  (0x8)
#define GPTIMER_CONTROL_IRQ_PENDING	(0x10)

struct timer_t
{
    uint32_t counter_value;
    uint32_t reload_value;
    uint32_t control;
    uint32_t dummy;
};

struct gptimer_t
{
	uint32_t scaler_value;                          /* 00 */
	uint32_t scaler_reload_value;                   /* 04 */
	uint32_t config_register;                       /* 08 */
	uint32_t unused;                                /* 0C */
	struct timer_t timer[GPTIMER_NUMBER_OF_TIMERS]; /* 10 */
};

/*  Defined in link.ld script file  */
//LOCAL volatile struct gptimer_t *gptimer = (struct gptimer_t*)0x80000300;
//LOCAL volatile uint32_t *prescaler = (uint32_t*)0x80000304;
//LOCAL uint32_t wdog_expire = 0;


/*============================================= PUBLIC INTERFACE  */
int OS_BSP_WdogInit(uint32_t timeout)
{
    os_return_minus_one_and_set_errno(OS_STATUS_NOT_SUPPORTED);
}

int OS_BSP_WdogEnable(void)
{
//    gptimer->timer[3].control |= GPTIMER_CONTROL_IRQ_ENABLE;
//    gptimer->timer[3].control |= GPTIMER_CONTROL_ENABLE;
    os_return_minus_one_and_set_errno(OS_STATUS_NOT_SUPPORTED);
}

int OS_BSP_WdogDisable(void)
{
//    gptimer->timer[3].control &= ~GPTIMER_CONTROL_ENABLE;
    os_return_minus_one_and_set_errno(OS_STATUS_NOT_SUPPORTED);
}

int OS_BSP_WdogReset(void)
{
    /*  Each GPTIMER can be reloaded with the value in its reload register at
     *  any time writing a '1' to the load bit in the control register
     */
//    gptimer->timer[3].control |= GPTIMER_CONTROL_LOAD;
    os_return_minus_one_and_set_errno(OS_STATUS_NOT_SUPPORTED);
}

