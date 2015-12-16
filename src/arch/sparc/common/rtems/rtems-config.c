//#ifndef _RTEMS_CONFIG_
//#define _RTEMS_CONFIG_


#include "osal/osconfig.h"   // The OS configuration is placed
#include "osal/osapi-compiler-switches.h"
#include "osal/osdebug.h"

//#include <rtems.h>
//#include <rtems/score/cpuopts.h>
#include <rtems/ramdisk.h>
#include <pci.h>
#include <stdio.h>
#include <stdlib.h>

#ifndef RTEMS_VERSION_LATER_THAN
#define RTEMS_VERSION_LATER_THAN(ma,mi,re) \
	(    __RTEMS_MAJOR__  > (ma)	\
	 || (__RTEMS_MAJOR__ == (ma) && __RTEMS_MINOR__  > (mi))	\
	 || (__RTEMS_MAJOR__ == (ma) && __RTEMS_MINOR__ == (mi) && __RTEMS_REVISION__ > (re)) \
    )
#endif
#ifndef RTEMS_VERSION_ATLEAST
#define RTEMS_VERSION_ATLEAST(ma,mi,re) \
	(    __RTEMS_MAJOR__  > (ma)	\
	|| (__RTEMS_MAJOR__ == (ma) && __RTEMS_MINOR__  > (mi))	\
	|| (__RTEMS_MAJOR__ == (ma) && __RTEMS_MINOR__ == (mi) && __RTEMS_REVISION__ >= (re)) \
	)
#endif

#define CONFIGURE_INIT

#include <bsp.h> /* for device driver prototypes */

rtems_task Init( rtems_task_argument argument);

/* configuration information */

#define CONFIGURE_INIT_TASK_ATTRIBUTES  (RTEMS_DEFAULT_ATTRIBUTES | RTEMS_FLOATING_POINT)

/*-----------------------------------------------------------------------------
 *  DRIVER table configuration
 *-----------------------------------------------------------------------------*/
/*  Enable the RTEMS Stack Checker. Comment it out to disable the stack checker.
 *  Enabling this feature may slow the system down a bit  
 */
#define STACK_CHECKER_ON

#define CONFIGURE_APPLICATION_NEEDS_CONSOLE_DRIVER
#define CONFIGURE_APPLICATION_NEEDS_CLOCK_DRIVER
#define CONFIGURE_APPLICATION_NEEDS_TIMER_DRIVER
#define CONFIGURE_MAXIMUM_DRIVERS 16

//#if defined(CONFIG_OS_ENABLE_FILESYSTEM)
#if 0

#define CONFIGURE_HAS_OWN_DEVICE_DRIVER_TABLE
rtems_driver_address_table Device_drivers[] = {
    CONSOLE_DRIVER_TABLE_ENTRY,
    CLOCK_DRIVER_TABLE_ENTRY,
    RAMDISK_DRIVER_TABLE_ENTRY,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
};
#define CONFIGURE_NUMBER_OF_DRIVERS \
    ( (sizeof(Device_drivers) / sizeof(rtems_driver_address_table)) )

#else
#define CONFIGURE_APPLICATION_NEEDS_NULL_DRIVER 1
#endif

/** Is defined if the user wants to use Classic API Initialization Tasks Table.
 * The applicatio may want to use initialization tasks or thread table from
 * another API. */
#define CONFIGURE_RTEMS_INIT_TASKS_TABLE
/** This is the stack size of the single initialization task defined by the
 * Classic API Initialization Tasks Table. By default is, if not defined, is
 * RTEMS_MINIMUM_STACK_SIZE */
#define CONFIGURE_INIT_TASK_STACK_SIZE      (OS_MAX_TASKS * RTEMS_MINIMUM_STACK_SIZE)
/** Is set to the number of bytes the applications wishes to add to the task
 * stack requirements calculated by RTEMS. If the application creates tasks
 * with stacks greater than the minimum, then that memory is NOT accounted for
 * by RTEMS. If not defined, the default value is '0'*/
#define CONFIGURE_EXTRA_TASK_STACKS         (OS_EXTRA_STACK_OVERHEAD)
/** Is set to the number of kilobytes the application wishes to add to the
 * requirements calculated by RTEMS. Default value is '0' */
#define CONFIGURE_MEMORY_OVERHEAD   (OS_EXTRA_MEMORY_OVERHEAD)


/*-----------------------------------------------------------------------------
 *  User extensions
 *-----------------------------------------------------------------------------*/
#define CONFIGURE_MAXIMUM_USER_EXTENSIONS 2

/*-----------------------------------------------------------------------------
 *  File system configuration
 *-----------------------------------------------------------------------------*/
//#if defined(CONFIG_OS_ENABLE_FILESYSTEM)
#if 0
/* 
 * The application wishes to use the full functionality of the IMFS
 */
    #define CONFIGURE_USE_IMFS_AS_BASE_FILESYSTEM
#else
/*
 * RTEMS now has a device-only file system which is smaller in size than
 * miniIMFS and IMFS and enables to use the filesystem only for the file
 * devices
 */
#if ( RTEMS_VERSION_LATER_THAN(4, 6, 5) )
#else
    #define CONFIGURE_MAXIMUM_DEVICES    16
#endif
    #define CONFIGURE_USE_DEVFS_AS_BASE_FILESYSTEM
#endif

#define CONFIGURE_LIBIO_MAXIMUM_FILE_DESCRIPTORS 32

/*  Base address of the RTEMS RAM workspace for RASTA
 *  FIXME -- This is trying to solve the problem with the bsp_start() RTEMS
 *  function. Some times prints "Not enough RAM!!!" message
 */
//#ifdef CONFIG_RASTA
//#warning "Configurint the RTEMS RAM work area"
//#define CONFIGURE_EXECUTIVE_RAM_WORK_AREA   (0x60000000)
//#endif


/*-----------------------------------------------------------------------------
 *  OS configuration
 *-----------------------------------------------------------------------------*/
/** Is the maximum number of Classic API tasks that can be concurrently active */
#define CONFIGURE_MAXIMUM_TASKS             OS_MAX_TASKS 
/** Is the maximum number of Classic API timers that can be concurrently active */
#define CONFIGURE_MAXIMUM_TIMERS            OS_MAX_TIMERS
/** Is the maximum number of Classic API semaphores that can be concurrently active */
#define CONFIGURE_MAXIMUM_SEMAPHORES        (OS_MAX_SEMAPHORES + OS_MAX_QUEUES)

/** Is the maximum number of Classic API mutexes that can be concurrently active */
#define CONFIGURE_MAXIMUM_MUTEXES           (OS_MAX_MUTEXES)
/** Is the maximum number of Classic API queues that can be concurrently active */
/*  
 *  FIXME: It is comment as the priority message queue is implementing not using
 *  any OS resource
 */
//#define CONFIGURE_MAXIMUM_MESSAGE_QUEUES    OS_MAX_QUEUES

/** Is the maximum number of Classic API rate monotonic periods that can be 
 * concurrently active */
#define CONFIGURE_MAXIMUM_PERIODS           OS_MAX_MONOTONIC_TASKS

/** Is the initial priority of the single initialization task defined by the
 * Classic API Initialization Task Table. The default value is '1' which is the
 * highest RTEMS priority */
#define CONFIGURE_INIT_TASK_PRIORITY      1

/** 
 * Is set to the number of bytes the application whises to be reserved for
 * pending message queue buffers.
 * If not set, the default value is '0'
 *
 *  FIXME: It is commented out as the priority message queue is implementing not using
 *  any OS resource
 */
//#define CONFIGURE_MESSAGE_BUFFER_MEMORY (1024 * OS_MAX_QUEUES)


/*-----------------------------------------------------------------------------
 *  CLOCK configuration
 *-----------------------------------------------------------------------------*/
//#define CONFIGURE_MICROSECONDS_PER_TICK RTEMS_MILLISECONDS_TO_MICROSECONDS(100)
/**
 * RTEMS time functionality operates in multiple of clock ticks, i.e. a task
 * can only sleep for an integer number of ticks. This macro defines the number
 * of microseconds per tick, which is the actual resolution of our system
 */
#define CONFIGURE_MICROSECONDS_PER_TICK 1000

/**
 *  This parameters is related to the hardware clock freq. In our case the
 *  hardware clock frequency is 80Mhz
 */
#ifdef CONFIG_HARDWARE_CLOCK_FREQ
#define CONFIGURE_CLICKS_PER_MICROSECOND CONFIG_HARDWARE_CLOCK_FREQ
#else
#error "The CONFIG_HARDWARE_CLOCK_FREQ is needed for some BSP-related OSAL functions"
#endif

/*
 * In case we are using RTEMS 4.8 or later the drivers configuration is different.
 */
#if ( RTEMS_VERSION_LATER_THAN(4, 6, 5) )
    #include <rtems/confdefs.h>

#if defined (CONFIG_RASTA)
    #include "drv_mgr_config.h"
    #include "rasta-rtems4.8/config.c"
#else

    #ifdef CONFIG_OS_ENABLE_NETWORKING
    #include "rtems-network.h"
    #endif

#endif // CONFIG_RASTA

#else
    #include <confdefs.h>
#endif // RTEMS_VERSION_LATER_THAN


/*-----------------------------------------------------------------------------
 *  NETWORK configuration
 *-----------------------------------------------------------------------------*/
//#if defined (CONFIG_OS_ENABLE_NETWORKING)
//
//#if ( RTEMS_VERSION_LATER_THAN(4, 6, 5) )
//
//#if defined (CONFIG_RASTA)
//    #include "rtems4.10-network.c"
//#else
//    #include "rtems-network.c"
//#endif // CONFIG_RASTA
//
//#else
//    #include "rtems-network.c"
//#endif //RTEMS_VERSION_LATER_THAN(4, 6, 5) 
//#endif // CONFIG_OS_ENABLE_NETWORK

/**
 *  \brief This function calculates the RTEMS RAM workspace size according to
 *  RTEMS SPARC Application Supplement document
 *  \param  none
 *  \return none
 */
#define RTEMS_DATA_SPACE_SIZE           (9059)
#define RTEMS_MAX_CONFIGURATION_SIZE    (50432)
uint32_t rtems_ram_workspace = 0;
static void print_rtems_ram_workspace(void)
{
    extern int _endtext, text_start;
    extern int data_start, edata;
    extern int _end, bss_start;

    rtems_ram_workspace = 
        CONFIGURE_MAXIMUM_TASKS * 488 + 
        CONFIGURE_MAXIMUM_TIMERS * 68 + 
        CONFIGURE_MAXIMUM_SEMAPHORES * 124 + 
        CONFIGURE_MAXIMUM_MUTEXES * 124 + 
        CONFIGURE_MAXIMUM_MESSAGE_QUEUES * 148 + 
        CONFIGURE_MAXIMUM_REGIONS * 144 + 
        CONFIGURE_MAXIMUM_PARTITIONS * 56 + 
        CONFIGURE_MAXIMUM_PORTS * 36 + 
        CONFIGURE_MAXIMUM_PERIODS * 36 +
        CONFIGURE_MAXIMUM_USER_EXTENSIONS * 64 + 
        CONFIGURE_MAXIMUM_TASKS * 136  +    // FIXME -- this is worst case
        CONFIGURE_MESSAGE_BUFFER_MEMORY + 
        CONFIGURE_MEMORY_OVERHEAD + 
        RTEMS_DATA_SPACE_SIZE + 
        RTEMS_MAX_CONFIGURATION_SIZE;

//    TRACE(CONFIGURE_MAXIMUM_TASKS, "d");
//    TRACE(CONFIGURE_MAXIMUM_TIMERS, "d");
//    TRACE(CONFIGURE_MAXIMUM_SEMAPHORES, "d");
//    TRACE(CONFIGURE_MAXIMUM_MUTEXES, "d");
//    TRACE(CONFIGURE_MAXIMUM_MESSAGE_QUEUES, "d");
//    TRACE(CONFIGURE_MAXIMUM_REGIONS, "d");
//    TRACE(CONFIGURE_MAXIMUM_PARTITIONS, "d");
//    TRACE(CONFIGURE_MAXIMUM_PORTS, "d");
//    TRACE(CONFIGURE_MAXIMUM_PERIODS, "d");
//    TRACE(CONFIGURE_MAXIMUM_USER_EXTENSIONS, "d");
//    TRACE(CONFIGURE_MESSAGE_BUFFER_MEMORY, "d");
//    TRACE(CONFIGURE_MEMORY_OVERHEAD, "d");

    printf("======================================\n");
    printf("RTEMS Memory Requirements = %.3f KB\n", (double)rtems_ram_workspace/1024);
    printf("APP Memory Requirements\n");
    printf("\t.text: %d bytes\n", (&_endtext-&text_start)*4);
    printf("\t.data: %d bytes\n", (&edata-&data_start)*4);
    printf("\t.bss: %d bytes\n", (&_end-&bss_start)*4);
    printf("======================================\n");

        
}

/*  
 *  The OSAL implementation for RTEMS call directly to the main function in the
 *  application SW. That means that the application SW is exactly the same for
 *  every platforms
 */
extern int main(void);
extern void OS_TaskJoin(void);
rtems_task Init( rtems_task_argument argument)
{
    int status;

    /*  parameter unused    */
    argument = argument;

#if ( RTEMS_VERSION_LATER_THAN(4, 6, 5) && CONFIG_RASTA)
    if( system_init() == 0 )
    {
//        rtems_drvmgr_print_topo();
        pci_print();
    }
#else
#endif

    print_rtems_ram_workspace();
    status = main();
    ASSERT( 0 == status );

    OS_TaskJoin();
    exit(3);
    rtems_task_delete(RTEMS_SELF);
}

//#endif
