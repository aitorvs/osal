/**
 *  \file   osapi-compiler-switches.h
 *  \brief  This file defines all the needed flags for the debug capabilities..
 *
 *  Detailed description starts here.
 *
 *  \author  Aitor Viana Sanchez (avs), Aitor.Viana.Sanchez@esa.int
 *
 *  \internal
 *    Created:  29/01/08
 *   Revision:  $Id: osapi-compiler-switches.h 1.4 29/01/08 avs Exp $
 *   Compiler:  gcc/g++
 *    Company:  European Space Agency (ESA-ESTEC)
 *  Copyright:  Copyright (c) 2008, Aitor Viana Sanchez
 *
 *  This source code is released for free distribution under the terms of the
 *  GNU General Public License as published by the Free Software Foundation.
 * =====================================================================================
 */

#ifndef OSAPICOMPILERSWITCHES_H_
#define OSAPICOMPILERSWITCHES_H_

/*  The NDEBUG symbol controls whether assertions are switched on or off
 *  under the Borland C++Builder. If the variable is defined, then assertions
 *  are switched off. If it is not defined, then assertions are switched on.
 */
#undef NDEBUG       /* assertions are switched on   */
/* #define NDEBUG   */ 

/*  The USE_SYSTEM_ASSERT symbol controls whether the assert() macro defined in
 *  the system-wide header file <code>assert.h</code> or the one defined in <code>DebugSupport.h</code> is used.
 *  If the symbol is defined, the definition in <code>assert.h</code> is taken.
 *  Otherwise the definition in <code>DebugSupport.h</code> is used.
 */
/* #define USE_SYSTEM_ASSERT
 */
 #undef USE_SYSTEM_ASSERT

#endif /*OSAPICOMPILERSWITCHES_H_*/
