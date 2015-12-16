/**
 *  \file   oshandlers.c
 *  \brief  This file implement all the handlers for the RTEMS layer
 *
 *  \author  Aitor Viana Sanchez (avs), Aitor.Viana.Sanchez@esa.int
 *
 *  \internal
 *    Created:  09/04/09
 *   Revision:  $Id: oshandlers.c 1.4 09/04/09 avs Exp $
 *   Compiler:  gcc/g++
 *    Company:  European Space Agency (ESA-ESTEC)
 *  Copyright:  Copyright (c) 2009, Aitor Viana Sanchez
 *
 *  This source code is released for free distribution under the terms of the
 *  GNU General Public License as published by the Free Software Foundation.
 * =====================================================================================
 */

/*****************************************************************************
  PRIVATE DATA
 *****************************************************************************/

LOCAL uint32_t extension_task_creation_log = 0;
/* LOCAL uint32_t extension_task_deletion_log = 0;
 * LOCAL uint32_t extension_task_restart_log = 0;
 * LOCAL uint32_t extension_task_start_log = 0;
 * LOCAL uint32_t extension_task_exit_log = 0;
 */

LOCAL BOOLEAN extension_task_create_handler(
    rtems_tcb *current_task,
    rtems_tcb *new_task
    );
LOCAL rtems_extension extension_fatal_error_handler(
        uint32_t    the_source,
        BOOLEAN   is_internal,
        uint32_t    the_error
        );

LOCAL rtems_extensions_table table_extensions = 
{
    extension_task_create_handler,  /*  task creation extension */
    NULL,                           /*  task start extension */
    NULL,                           /*  task restart extension */
    NULL,                           /*  task delete extension */
    NULL,                           /*  task switch extension */
    NULL,                           /*  task begin extension */
    NULL,                           /*  task extted extension */
    extension_fatal_error_handler   /*  fatal error extension   */
};




/*****************************************************************************
  PRIVATE INTERFACE
 *****************************************************************************/

LOCAL int rtems_init_extension_handlers(void)
{
    rtems_name table_name;
    rtems_status_code rtems_status;
    rtems_id    table_id;

    /*  Add user extension table    */
    table_name = rtems_build_name('U','S','E','R');
    rtems_status = rtems_extension_create (table_name, &table_extensions, &table_id);

    assert( RTEMS_SUCCESSFUL == rtems_status );

    if( RTEMS_SUCCESSFUL == rtems_status ) return OS_SUCCESS;
    else return OS_ERROR;

}

LOCAL BOOLEAN extension_task_create_handler(
    rtems_tcb *current_task,
    rtems_tcb *new_task
    )
{

    extension_task_creation_log++;

    return TRUE;
}


/* 
 * ===  FUNCTION  ======================================================================
 *         Name:  extension_fatal_error_handler
 *  Description:  This function it is used to find out whether a halt has
 *  occurred during the SW execution.
 * =====================================================================================
 */
LOCAL rtems_extension extension_fatal_error_handler(
        uint32_t    the_source,
        BOOLEAN   is_internal,
        uint32_t    the_error
        )
{
    OS_printf("FATAL ERROR has occurred!!!\n");
    OS_printf("Source: ");

    switch(the_source)
    {
        case INTERNAL_ERROR_CORE:
            OS_printf("INTERNAL_ERROR_CORE\n");
            break;
        case INTERNAL_ERROR_RTEMS_API:
            OS_printf("INTERNAL_ERROR_RTEMS_API\n");
            break;
        case INTERNAL_ERROR_POSIX_API:
            OS_printf("INTERNAL_ERROR_POSIX_API\n");
            break;
        case INTERNAL_ERROR_ITRON_API:
            OS_printf("INTERNAL_ERROR_ITRON_API\n");
            break;
    }

    OS_printf("\n");

    if( is_internal == TRUE )
    {
        OS_printf("It is an INTERNAL error\n");

        OS_printf("Error: ");

        switch( the_error )
        {
            case INTERNAL_ERROR_NO_CONFIGURATION_TABLE:
                OS_printf ("INTERNAL_ERROR_NO_CONFIGURATION_TABLE");
                break;
            case INTERNAL_ERROR_NO_CPU_TABLE:
                OS_printf ("INTERNAL_ERROR_NO_CPU_TABLE");
                break;
            case INTERNAL_ERROR_INVALID_WORKSPACE_ADDRESS:
                OS_printf ("INTERNAL_ERROR_INVALID_WORKSPACE_ADDRESS");
                break;
            case INTERNAL_ERROR_TOO_LITTLE_WORKSPACE:
                OS_printf ("INTERNAL_ERROR_TOO_LITTLE_WORKSPACE");
                break;
            case INTERNAL_ERROR_WORKSPACE_ALLOCATION:
                OS_printf ("INTERNAL_ERROR_WORKSPACE_ALLOCATION");
                break;
            case INTERNAL_ERROR_INTERRUPT_STACK_TOO_SMALL:
                OS_printf ("INTERNAL_ERROR_INTERRUPT_STACK_TOO_SMALL");
                break;
            case INTERNAL_ERROR_THREAD_EXITTED:
                OS_printf ("INTERNAL_ERROR_THREAD_EXITTED");
                break;
            case INTERNAL_ERROR_INCONSISTENT_MP_INFORMATION:
                OS_printf ("INTERNAL_ERROR_INCONSISTENT_MP_INFORMATION");
                break;
            case INTERNAL_ERROR_INVALID_NODE:
                OS_printf ("INTERNAL_ERROR_INVALID_NODE");
                break;
            case INTERNAL_ERROR_NO_MPCI:
                OS_printf ("INTERNAL_ERROR_NO_MPCI");
                break;
            case INTERNAL_ERROR_BAD_PACKET:
                OS_printf ("INTERNAL_ERROR_BAD_PACKET");
                break;
            case INTERNAL_ERROR_OUT_OF_PACKETS:
                OS_printf ("INTERNAL_ERROR_OUT_OF_PACKETS");
                break;
            case INTERNAL_ERROR_OUT_OF_GLOBAL_OBJECTS:
                OS_printf ("INTERNAL_ERROR_OUT_OF_GLOBAL_OBJECTS");
                break;
            case INTERNAL_ERROR_OUT_OF_PROXIES:
                OS_printf ("INTERNAL_ERROR_OUT_OF_PROXIES");
                break;
            case INTERNAL_ERROR_INVALID_GLOBAL_ID:
                OS_printf ("INTERNAL_ERROR_INVALID_GLOBAL_ID");
                break;
            case INTERNAL_ERROR_BAD_STACK_HOOK:
                OS_printf ("INTERNAL_ERROR_BAD_STACK_HOOK");
                break;
            case INTERNAL_ERROR_BAD_ATTRIBUTES:
                OS_printf ("INTERNAL_ERROR_BAD_ATTRIBUTES");
                break;
            default:
                OS_printf ("UNKNOWN (%d)", the_error);
        }
        OS_printf ("\n");

    }
    else
    {
        OS_printf ("It is NOT an internal error.\n");
        /* Assume an RTEMS Classic API error... */
        show_error_code (the_error);
    }

}


