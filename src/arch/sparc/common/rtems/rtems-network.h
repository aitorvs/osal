#ifndef _RTEMS_NETWORKCONFIG_H_
#define _RTEMS_NETWORKCONFIG_H_


#warning "Compiling old version of networking"

#include <public/osal_config.h>

#ifdef CONFIG_OS_ENABLE_NETWORKING

//#define RTEMS_USE_BOOTP
//#define RTEMS_IP_ADDRESS    CONFIG_OS_NETWORK_IPADDR
extern char osal_ip_addr[];
#define RTEMS_IP_MASK       "255.255.255.0"

#include <bsp.h>
#include <rtems/rtems_bsdnet.h> // it must be included before network.h


/*
 *  The following will normally be set by the BSP if it supports
 *  a single network device driver.  In the event, it supports
 *  multiple network device drivers, then the user's default
 *  network device driver will have to be selected by a BSP
 *  specific mechanism.
 */
 
/* #define RTEMS_BSP_NETWORK_DRIVER_NAME "open_eth1"    */

#ifndef RTEMS_BSP_NETWORK_DRIVER_NAME
#warning "RTEMS_BSP_NETWORK_DRIVER_NAME is not defined"
#define RTEMS_BSP_NETWORK_DRIVER_NAME "no_network1"
#endif

#ifndef RTEMS_BSP_NETWORK_DRIVER_ATTACH
#ifdef RTEMS_BSP_NETWORK_DRIVER_ATTACH_SMC91111 
#define RTEMS_BSP_NETWORK_DRIVER_ATTACH RTEMS_BSP_NETWORK_DRIVER_ATTACH_SMC91111
#else 
#warning "RTEMS_BSP_NETWORK_DRIVER_ATTACH is not defined"
#define RTEMS_BSP_NETWORK_DRIVER_ATTACH 0
#endif
#endif

//#define RTEMS_USE_BOOTP

/*
 * Define RTEMS_SET_ETHERNET_ADDRESS if you want to specify the
 * Ethernet address here.  If RTEMS_SET_ETHERNET_ADDRESS is not
 * defined the driver will choose an address (usually 12:34:56:78:90:12).
 */
/* #define RTEMS_SET_ETHERNET_ADDRESS */
#if (defined (RTEMS_SET_ETHERNET_ADDRESS))
/* static char ethernet_address[6] = { 0x08, 0x00, 0x3e, 0x12, 0x28, 0xb1 }; */
static char ethernet_address[6] = { 0x00, 0x80, 0x7F, 0x22, 0x61, 0x79 };

#endif

#ifdef RTEMS_USE_LOOPBACK 
/*
 * Loopback interface
 */
extern void rtems_bsdnet_loopattach();
static struct rtems_bsdnet_ifconfig loopback_config = {
	"lo0",				/* name */
	rtems_bsdnet_loopattach,	/* attach function */

	NULL,				/* link to next interface */

	"131.176.4.103",		/* IP address */
	"255.255.255.0",		/* IP net mask */
};
#endif

/*
 * Default network interface
 */
static struct rtems_bsdnet_ifconfig netdriver_config = {
	RTEMS_BSP_NETWORK_DRIVER_NAME,		/* name */
	RTEMS_BSP_NETWORK_DRIVER_ATTACH,	/* attach function */

#ifdef RTEMS_USE_LOOPBACK 
	&loopback_config,		/* link to next interface */
#else
	NULL,				/* No more interfaces */
#endif


#if (defined (RTEMS_USE_BOOTP))
	NULL,				/* BOOTP supplies IP address */
	NULL,				/* BOOTP supplies IP net mask */
#else
	osal_ip_addr,	/* IP address */
	RTEMS_IP_MASK,		/* IP net mask */
#endif /* !RTEMS_USE_BOOTP */

#if (defined (RTEMS_SET_ETHERNET_ADDRESS))
	ethernet_address,               /* Ethernet hardware address */
#else
	NULL,                           /* Driver supplies hardware address */
#endif
	0				/* Use default driver parameters */
};


/*
 * Network configuration
 */
struct rtems_bsdnet_config rtems_bsdnet_config = {
	&netdriver_config,

#if (defined (RTEMS_USE_BOOTP))
	rtems_bsdnet_do_bootp,
#else
	NULL,
#endif

	100,			/* Default network task priority */
	128*1024,		/* Default mbuf capacity */
	256*1024,		/* Default mbuf cluster capacity */

#if (!defined (RTEMS_USE_BOOTP))
	"rtems_host",		/* Host name */
	"localnet",		/* Domain name */
	"131.176.4.250",	/* Gateway */
	"192.168.0.1",		/* Log host */
	{"131.176.24.47" },	/* Name server(s) */
	{"131.176.4.102" },	/* NTP server(s) */

#endif /* !RTEMS_USE_BOOTP */
};

#else   /*  CONFIG_OS_ENABLE_NETWORK    */
#endif



#endif /* _RTEMS_NETWORKCONFIG_H_ */
