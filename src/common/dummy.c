/**
 *  \file   dummy.c
 *  \brief  This file defines all the undefined symbols needed that the user
 *  does not define.
 *
 *  \author  Aitor Viana Sanchez (avs), Aitor.Viana.Sanchez@esa.int
 *
 *  \internal
 *    Created:  12/11/2009
 *   Revision:  $Id: dummy.c 1.4 12/11/2009 avs Exp $
 *   Compiler:  gcc/g++
 *    Company:  European Space Agency (ESA-ESTEC)
 *  Copyright:  Copyright (c) 2009, Aitor Viana Sanchez
 *
 *  This source code is released for free distribution under the terms of the
 *  GNU General Public License as published by the Free Software Foundation.
 * =====================================================================================
 */

#include <public/osal_config.h>

#if defined (CONFIG_OS_ENABLE_NETWORKING)
char osal_ip_addr[] = "192.168.1.25";
#endif

