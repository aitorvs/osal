
#include <drvmgr/ambapp_bus_grlib.h>
#include <drvmgr/ambapp_bus.h>
#include <amba.h>

struct ambapp_dev_hdr *ambapp_root = NULL;

/* B1553RT driver configuration (optional) */
struct rtems_drvmgr_key grlib_drv_res_b1553rt0[] =
{
#ifdef SET_B1553RT_FREQ
	{"coreFreq", KEY_TYPE_INT, {(unsigned int)SET_B1553RT_FREQ}},
#endif
	KEY_EMPTY
};

/* GRPCI driver configuration (optional) */
struct rtems_drvmgr_key grlib_drv_res_grpci0[] =
{
/*	{"byteTwisting", KEY_TYPE_INT, {(unsigned int)1}}, */
	KEY_EMPTY
};

/* GRGPIO0 driver configuration (optional) */
struct rtems_drvmgr_key grlib_drv_res_grgpio0[] =
{
	{"nBits", KEY_TYPE_INT, {(unsigned int)24}},
	KEY_EMPTY
};

/* GRGPIO1 driver configuration (optional) */
struct rtems_drvmgr_key grlib_drv_res_grgpio1[] =
{
	{"nBits", KEY_TYPE_INT, {(unsigned int)8}},
	KEY_EMPTY
};

/* GRGPIO1 driver configuration (optional) */
struct rtems_drvmgr_key grlib_drv_res_spictrl0[] =
{
#ifdef SPICTRL_SLV_SEL_FUNC
	{"slvSelFunc", KEY_TYPE_POINTER, {(unsigned int)SPICTRL_SLV_SEL_FUNC}},
#endif
	KEY_EMPTY
};

struct rtems_drvmgr_drv_res grlib_drv_resources[] =
{
	{DRIVER_AMBAPP_GAISLER_B1553RT_ID, 0, &grlib_drv_res_b1553rt0[0]},
	{DRIVER_AMBAPP_GAISLER_GRPCI_ID, 0, &grlib_drv_res_grpci0[0]},
	{DRIVER_AMBAPP_GAISLER_SPICTRL_ID, 0, &grlib_drv_res_spictrl0[0]},
	RES_EMPTY
};

struct ambapp_bus grlib_amba_bus =
{
	.mmaps = NULL,
	.root = NULL
};

struct grlib_config grlib_bus_config = 
{
	&grlib_amba_bus,		/* AMBAPP bus setup */
	&grlib_drv_resources[0],	/* Driver configuration */
};

/* LEON3 specific system init */
void system_init2(void)
{
/************ DIFFER FROM 4.10 ************/
  /* Scan with new amba routines, RTEMS-4.8 still uses the legacy AMBA routines */
	ambapp_root = NULL;
  ambapp_scan(LEON3_IO_AREA, NULL, NULL, NULL, &ambapp_root, NULL);
/******************************************/

	/* LEON3 AMBA PnP bus */
	grlib_amba_bus.mmaps = NULL;
	grlib_amba_bus.root = ambapp_root;

	/* Register GRLIB root bus */
	drv_mgr_grlib_init(&grlib_bus_config);
}
