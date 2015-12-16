#include <gr_rasta_tmtc.h>
#include <drvmgr/ambapp_bus.h>

/* GR-RASTA-TMTC boards configuration example. Note that this is 
 * optional, we only override defaults. If defualt are ok, nothing
 * is need to be done.
 */

struct rtems_drvmgr_key rastatmtc0_esa_mctrl0_res[] = 
{
	/* SDRAM Configuration for a GR-RASTA-TMTC 50MHz board */
	{"mcfg1", KEY_TYPE_INT, {(unsigned int) 0x000002ff}},
	{"mcfg2", KEY_TYPE_INT, {(unsigned int) 0x82206000}},
	{"mcfg3", KEY_TYPE_INT, {(unsigned int) 0x000ff000}},
	KEY_EMPTY
};

/* Driver resources for GR-RASTA-TMTC 1 AMBA PnP bus */
struct rtems_drvmgr_drv_res gr_rasta_tmtc0_res[] = 
{
	{DRIVER_AMBAPP_ESA_MCTRL_ID, 0, &rastatmtc0_esa_mctrl0_res[0]},
	RES_EMPTY
};

/* Bus resources for all TMTC boards, only the first is configured */
struct rtems_drvmgr_drv_res *gr_rasta_tmtc_res[] = 
{
	&gr_rasta_tmtc0_res[0],		/* GR-RASTA-TMTC board 1 resources */
};
#define GR_RASTA_TMTC_NUM (sizeof(gr_rasta_tmtc_res)/sizeof(struct rtems_drvmgr_drv_res *))

void system_init_rastatmtc(void)
{
	/* Tell GR-RASTA-TMTC driver about the bus resources.
	 * Resources for one GR-RASTA-TMTC board are available.
	 * AMBAPP->PCI->GR-RASTA-TMTC->AMBAPP bus resources
	 *
	 * We do this initialization because we know that this
	 * driver will be used since we have the hardware present.
	 */
	gr_rasta_tmtc_set_resources(gr_rasta_tmtc_res, GR_RASTA_TMTC_NUM);
}
