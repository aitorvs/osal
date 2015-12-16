/* Configuration file for LEON2, GRLIB-LEON2 and LEON3 systems 
 * 
 * Defines driver resources in separate files,
 *  - LEON3         - leon3_drv_config.c
 *  - LEON2         - leon2_drv_config.c
 *  - LEON2-GRLIB   - leon2_grlib_drv_config.c
 *
 * Initializes,
 *   - Driver manager
 *   - Networking if ENABLE_NETWORK is set
 * 
 */

/* Define for GRLIB LEON2 systems, when a AMBA PnP bus is available */
/*#define LEON2_GRLIB*/

#ifdef CONFIG_OS_ENABLE_NETWORKING
#include "networkconfig.h"
#endif

/* Include the Driver resource configuration for the different systems */
#if defined(LEON3)
  /* GRLIB-LEON3 */
  #include "leon3_drv_config.c"
#elif defined(LEON2)
  #ifdef LEON2_GRLIB
    /* GRLIB-LEON2 */
    #include "leon2_grlib_drv_config.c"
  #else
    /* Standard LEON2 */
    #include "leon2_drv_config.c"
  #endif
#endif

/* Include the GR-RASTA-TMTC configuration only if the GR-RASTA-TMTC driver is 
 * included.
 */
#ifdef CONFIG_RASTA
#include "config_gr_rasta_io.c"
#ifdef CONFIG_RASTA_GAISLER_TMTC_ENABLE
    #include "config_gr_rasta_tmtc.c"
#endif
#endif

#include <stdlib.h>
#include <stdio.h>

#ifdef CONFIG_OS_ENABLE_NETWORKING
#include <network_interface_add.h>
#include <rtems/rtems_bsdnet.h>
struct rtems_bsdnet_ifconfig smcconfig;
#endif

int system_init(void)
{
/************ DIFFER FROM 4.10 ************/
	/* RTEMS prior to 4.10 does not set up the shared interrupt handling automatically */
#if defined(LEON3)
#if (__RTEMS_MAJOR__ == 4) && (__RTEMS_MINOR__ < 10)
	/* Initialize shared interrupt handling, must be done after extended
	 * interrupt controller has been registered.
	 */
	BSP_shared_interrupt_init();
#endif
#endif

#ifdef CONFIG_RASTA_GAISLER_TMTC_ENABLE
    system_init_rastatmtc();
#endif

#ifdef CONFIG_OS_ENABLE_NETWORKING
	/* Registering SMC driver first, this way the first entry in
	 * interface_configs will reflect the SMC network settings.
	 */
	smcconfig.name = "smc1";
	smcconfig.drv_ctrl = NULL;
	smcconfig.attach = (void *)RTEMS_BSP_NETWORK_DRIVER_ATTACH_SMC91111;
	network_interface_add(&smcconfig);
#endif

    system_init2();


	/* Initializing Driver Manager */
	printf("Initializing manager\n");
	if ( rtems_drvmgr_init() ) {
		printf("Driver manager Failed to initialize\n");
		return -1;
	}

	/* Print Driver manager drivers and their assigned devices */
	rtems_drvmgr_print_drvs(1);
//	drv_mgr_print_drvs(1);

#ifdef CONFIG_OS_ENABLE_NETWORKING
	/* Init network */
	printf("Initializing network\n");
	rtems_bsdnet_initialize_network ();
	printf("Initializing network DONE\n\n");
	rtems_bsdnet_show_inet_routes();
	printf("\n");
	rtems_bsdnet_show_if_stats();
	printf("\n\n");
#endif

    printf("=================================================================\n");

    return 0;

}

