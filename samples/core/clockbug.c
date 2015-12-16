/**
 *  \file   clockbug.c
 *  \brief  This file implements a test regarding a bug discovered in
 *  30/03/09 in the ERC32's get microseconds since last tick handler.
 *
 *  The test consist in checking if the time given since las tick handler is
 *  always coherent.
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
#include <stdio.h>

static void task1(void)
{
    OS_time_t old, new;
    uint32_t errors = 0;
    uint32_t iterations = 0;

    printf("Application task %d running\n", (int)OS_TaskGetId());

    OS_GetTimeSinceBoot(&old);

    while(1)
    {
        OS_GetTimeSinceBoot(&new);

        if( new.mul_Seconds < old.mul_Seconds )
        {
            printf("ERROR: mul_Seconds decrease (%d < %d)\n", (int)new.mul_Seconds, (int)old.mul_Seconds);
        }
        else if( new.mul_Seconds == old.mul_Seconds )
        {
            iterations++;
            if( new.mul_MicroSeconds < old.mul_MicroSeconds )
            {
                /*  This is an error    */
//                printf("\t new = %d sec, %d mul_MicroSeconds\n", new.mul_Seconds, new.mul_MicroSeconds);
//                printf("\t old = %d sec, %d mul_MicroSeconds\n", old.mul_Seconds, old.mul_MicroSeconds);
                errors++;
            }
                
        }

        new.mul_Seconds -= old.mul_Seconds;

        /*  Stop after 10 mul_Seconds   */
        if( new.mul_Seconds > 10 )
        {
            printf("%d Time Acquisitions - %d errors\n", (int)iterations, (int)errors);
            OS_TaskExit();
        }
    }
}

int main(void)
{
    uint32_t t1;

	printf("RTEMS Bug. The rtems_clock_get_uptime() function can retun \
            wrong time values if a clock tick interrupt occurs when acquiring the \
            uptime (30/3/09 Test)\n");
    
    OS_Init();
    
    if( OS_TaskCreate (&t1,(void *)task1, 2048, 99, 0, (void*)NULL) != 0)
    	printf("ERR: unable to create the tasks\n");
    
    OS_Start();


    return 0;

} /* end OS_Application Startup */



