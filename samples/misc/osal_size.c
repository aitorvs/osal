/**
 *  \file   hello.c
 *  \brief  This file calculates the number of TROS resources needed by the 
 *  OSAL library.
 *
 *  OSAL has to be compiled with the DEBUG options enabled to print-out the
 *  resources
 *
 *  \author  Aitor Viana Sanchez (avs), Aitor.Viana.Sanchez@esa.int
 *
 *  \internal
 *    Created:  28/01/08
 *   Revision:  $Id: doxygen.templates.example,v 1.4 2007/08/02 14:35:24 mehner Exp $
 *   Compiler:  gcc/g++
 *    Company:  European Space Agency (ESA-ESTEC)
 *  Copyright:  Copyright (c) 2008, Aitor Viana Sanchez
 *
 *  This source code is released for free distribution under the terms of the
 *  GNU General Public License as published by the Free Software Foundation.
 * =====================================================================================
 */

/*-----------------------------------------------------------------------------
 * Changelog:
 * 29/01/08 10:16:44
 * - Some include header files has been taken out.
 *-----------------------------------------------------------------------------*/

#include <osal/osapi.h>

int main(void)
{
    extern int _endtext, text_start;
    extern int data_start, edata;
    extern int end, bss_start;

    printf(".text: %d bytes\n", (&_endtext-&text_start)*4);
    printf(".data: %d bytes\n", (&edata-&data_start)*4);
    printf(".bss: %d bytes\n", (&end-&bss_start)*4);
} /* end OS_Application Startup */



