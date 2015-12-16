/*
 * Network configuration
 * 
 ************************************************************
 * EDIT THIS FILE TO REFLECT YOUR NETWORK CONFIGURATION     *
 * BEFORE RUNNING ANY RTEMS PROGRAMS WHICH USE THE NETWORK! * 
 ************************************************************
 *
 *  networkconfig.h,v 1.8 2001/08/31 18:11:43 joel Exp
 */

#ifndef _NETWORKCONFIG_H_
#define _NETWORKCONFIG_H_

#include <rtems/rtems_bsdnet.h>
#include "network_interface_add.h"

//#define RTEMS_USE_BOOTP
//#define RTEMS_USE_DHCP

#ifdef RTEMS_USE_DHCP
void rtems_bsdnet_do_dhcp(void);
#endif

/* #include <bsp.h> */

extern char osal_ip_addr[];

#ifdef RTEMS_USE_LOOPBACK 
/*
 * Loopback interface
 */
extern void rtems_bsdnet_loopattach();
static struct rtems_bsdnet_ifconfig loopback_config = {
	"lo0",				/* name */
	rtems_bsdnet_loopattach,	/* attach function */

	NULL,				/* link to next interface */

	"127.0.0.1",			/* IP address */
	"255.0.0.0",			/* IP net mask */
};
#endif


/*
 * Network configuration
 */
struct rtems_bsdnet_config rtems_bsdnet_config = {
#ifdef RTEMS_USE_LOOPBACK 
	&loopback_config,		/* link to next interface */
#else
	NULL,				/* No more interfaces */
#endif

#if (defined (RTEMS_USE_BOOTP))
	rtems_bsdnet_do_bootp,
#elif (defined (RTEMS_USE_DHCP))
	rtems_bsdnet_do_dhcp,
#else
	NULL,
#endif

	100,			/* Default network task priority */
	128*1024,		/* Default mbuf capacity */
	256*1024,		/* Default mbuf cluster capacity */

#if (!defined (RTEMS_USE_BOOTP))
	"rtems_host",		/* Host name */
	"localnet",		/* Domain name */
	"192.168.1.1",		/* Gateway */
	"192.168.1.1",		/* Log host */
	{"192.168.1.1" },	/* Name server(s) */
	{"192.168.1.1" },	/* NTP server(s) */

#endif /* !RTEMS_USE_BOOTP */

};

/* Table used by network interfaces that register themselves using the
 * network_interface_add routine. From this table the IP address, netmask 
 * and Ethernet MAC address of an interface is taken.
 *
 * The network_interface_add routine puts the interface into the
 * rtems_bsnet_config.ifconfig list.
 *
 * Set IP Address and Netmask to NULL to select BOOTP.
 */
struct ethernet_config interface_configs[] =
{
	{ osal_ip_addr, "255.255.255.0", {0x00, 0x80, 0x7F, 0x22, 0x61, 0x79}},
	{ osal_ip_addr, "255.255.255.0", {0x00, 0x80, 0x7F, 0x22, 0x61, 0x7A}},
	{ "192.168.1.212", "255.255.255.0", {0x00, 0x80, 0x7F, 0x22, 0x61, 0x7B}},
	{ "192.168.1.213", "255.255.255.0", {0x00, 0x80, 0x7F, 0x22, 0x61, 0x7C}},
	{ "192.168.1.214", "255.255.255.0", {0x00, 0x80, 0x7F, 0x22, 0x61, 0x7D}},
	{ "192.168.1.215", "255.255.255.0", {0x00, 0x80, 0x7F, 0x22, 0x61, 0x7E}},
	{ "192.168.1.216", "255.255.255.0", {0x00, 0x80, 0x7F, 0x22, 0x61, 0x7F}},
	{NULL, NULL, {0,0,0,0,0,0}}
};
#define INTERFACE_CONFIG_CNT (sizeof(interface_configs)/sizeof(struct ethernet_config) - 1)

/*
 * For TFTP test application
 */
#if (defined (RTEMS_USE_BOOTP))
#define RTEMS_TFTP_TEST_HOST_NAME "BOOTP_HOST"
#define RTEMS_TFTP_TEST_FILE_NAME "BOOTP_FILE"
#else
#define RTEMS_TFTP_TEST_HOST_NAME "XXX.YYY.ZZZ.XYZ"
#define RTEMS_TFTP_TEST_FILE_NAME "tftptest"
#endif

#endif
