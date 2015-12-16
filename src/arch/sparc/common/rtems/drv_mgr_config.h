/**
 *  \file   drv_mgr_config.h
 *  \brief  This file selects the drivers for LEON systems in RTEMS v4.10
 *
 *  The drivers are selected defining the array drv_mgr_drivers, it contains
 *  one function pointer per driver responsible to register one or more
 *  drivers.
 *  The drv_mgr_drivers can be set-up defining CONFIGURE_INIT, selecting the
 *  appropiate drivers and including drv_mgr/drv_manager_confdefs.h header
 *  file.
 *  This approach is similar to configurin standard RTEMS project using
 *  rtems/confdefs.h. This file performs such a configuration.
 *
 *  LEON2 and LEON3 labels are defined in <bsp.h> file
 *
 *  \author  Aitor Viana Sanchez (avs), Aitor.Viana.Sanchez@esa.int
 *
 *  \internal
 *    Created:  23/02/09
 *   Revision:  $Id: drv_mgr_config.h 1.4 23/02/09 avs Exp $
 *   Compiler:  gcc/g++
 *    Company:  European Space Agency (ESA-ESTEC)
 *  Copyright:  Copyright (c) 2009, Aitor Viana Sanchez
 *
 *  This source code is released for free distribution under the terms of the
 *  GNU General Public License as published by the Free Software Foundation.
 * =====================================================================================
 */

#ifndef _DRV_MGR_CONFIG_H_
#define _DRV_MGR_CONFIG_H_

#include <public/osal_config.h>

/* PCI */
#define CONFIGURE_DRIVER_AMBAPP_GAISLER_PCIF
#define CONFIGURE_DRIVER_AMBAPP_GAISLER_GRPCI
#define CONFIGURE_DRIVER_PCI_GR_RASTA_IO
#define CONFIGURE_DRIVER_PCI_GR_RASTA_ADCDAC
#define CONFIGURE_DRIVER_PCI_GR_701
//#define CONFIGURE_DRIVER_AMBAPP_MCTRL
/*#define CONFIGURE_DRIVER_AMBAPP_GAISLER_PCITRACE*/


/* Select drivers used by the driver manager */
#ifdef CONFIG_RASTA_GAISLER_GRETH_ENABLE
    #define CONFIGURE_DRIVER_AMBAPP_GAISLER_GRETH
#endif

#ifdef CONFIG_RASTA_GAISLER_GRSPW_ENABLE
#define CONFIGURE_DRIVER_AMBAPP_GAISLER_GRSPW
#endif

#ifdef CONFIG_RASTA_GAISLER_GRCAN_ENABLE
#define CONFIGURE_DRIVER_AMBAPP_GAISLER_GRCAN
#endif

#ifdef CONFIG_RASTA_GAISLER_GROCCAN_ENABLE
#define CONFIGURE_DRIVER_AMBAPP_GAISLER_OCCAN
#endif

#ifdef CONFIG_RASTA_GAISLER_B1553BRM_ENABLE
#define CONFIGURE_DRIVER_AMBAPP_GAISLER_B1553BRM
#endif

#ifdef CONFIG_RASTA_GAISLER_B1553RT_ENABLE
#define CONFIGURE_DRIVER_AMBAPP_GAISLER_B1553RT
#endif

#ifdef CONFIG_RASTA_GAISLER_APBUART_ENABLE
    #define CONFIGURE_DRIVER_AMBAPP_GAISLER_APBUART
#endif

#ifdef CONFIG_RASTA_GAISLER_TMTC_ENABLE
    #define CONFIGURE_DRIVER_AMBAPP_GAISLER_GRTM
    #define CONFIGURE_DRIVER_AMBAPP_GAISLER_GRTC
    #define CONFIGURE_DRIVER_PCI_GR_RASTA_TMTC
#endif

#ifdef LEON2
  /* PCI support for AT697 */
  #define CONFIGURE_DRIVER_LEON2_AT697PCI
  /* AMBA PnP Support for GRLIB-LEON2 */
  #define CONFIGURE_DRIVER_LEON2_AMBAPP
#endif

#include <drvmgr/drvmgr_confdefs.h>
//#include <drv_mgr/drv_manager_confdefs.h>

#endif

