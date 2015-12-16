/**
 *  \file   bsp.h
 *  \brief  This header file declares all the BSP related features.
 *
 *  The functions declared here are specific for each BSP and may not be
 *  present in all OSAL supported BSPs
 *
 *  \author  Aitor Viana Sanchez (avs), Aitor.Viana.Sanchez@esa.int
 *
 *  \internal
 *    Created:  11/26/2009
 *   Revision:  $Id: bsp.h 1.4 11/26/2009 avs Exp $
 *   Compiler:  gcc/g++
 *    Company:  European Space Agency (ESA-ESTEC)
 *  Copyright:  Copyright (c) 2009, Aitor Viana Sanchez
 *
 *  This source code is released for free distribution under the terms of the
 *  GNU General Public License as published by the Free Software Foundation.
 * =====================================================================================
 */


#ifndef _OSAL_BSP_H_
#define _OSAL_BSP_H_

/**
 *  \brief This function initializes the watchdog.
 *
 *  The function configures the hardware watchdog (if some) to expire according
 *  to the 'timeout' parameter
 *
 *  \param  timeout This parameter is the watchdog timeout expressed in
 *  milliseconds
 *
 *  \return OS_ERR_NOT_SUPPORTED when the BSP does not support this feature
 *  \return OS_ERROR when the wdog initialization fails
 *  \return OS_SUCCESS when the watchdog is correctly configured.
 */
int OS_BSP_WdogInit(uint32_t timeout);

/**
 *  \brief This function enables the watchdog once it has been configured (see
 *  \ref OS_BSP_WdogInit())
 *
 *  \return OS_ERR_NOT_SUPPORTED when the BSP does not support this feature
 *  \return OS_ERROR when the function fails
 *  \return OS_SUCCESS when the watchdog is correctly enabled.
 */
int OS_BSP_WdogEnable(void);

/**
 *  \brief This function disables the watchdog once it has been configured (see
 *  \ref OS_BSP_WdogInit())
 *
 *  \return OS_ERR_NOT_SUPPORTED when the BSP does not support this feature
 *  \return OS_ERROR when the function fails
 *  \return OS_SUCCESS when the watchdog is correctly disabled.
 */
int OS_BSP_WdogDisable(void);

/**
 *  \brief This function resets a running watchdog once it has been configured (see
 *  \ref OS_BSP_WdogInit()) and enabled.
 *
 *  The function resets the wdog count to avoid the expiration. This function
 *  must be called periodically with a the period being less than the
 *  expiration time specified in the \ref OS_BSP_WdogInit() function, otherwise
 *  the wdog will expired.
 *
 *  \return OS_ERR_NOT_SUPPORTED when the BSP does not support this feature
 *  \return OS_ERROR when the function fails
 *  \return OS_SUCCESS when the watchdog is correctly reset.
 */
int OS_BSP_WdogReset(void);


#endif
