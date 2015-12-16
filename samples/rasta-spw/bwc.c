#include <osal/osapi.h>

#include <rtems.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <bsp.h> /* for device driver prototypes */
#include <stdio.h>
#include <stdlib.h>
#include <sys/ioctl.h>
#include <sched.h>
#include <string.h>

#if (__GNUC__ > 3)
#include <ambapp.h>
#include <grspw.h>
#else
#endif

//#include <rasta.h>
#include <pci.h>

#define NODE_ADR_RX 33
#define NODE_ADR_TX 35

//#define SPW_DEVICE_2    "/dev/grspwrasta1"
#define SPW_DEVICE_2    "/dev/rastaio0/grspw1"


/* Start SSSPW driver */  
#define IOCTL(fd,num,arg) \
{ \
    int ret; \
    if ( (ret=ioctl(fd,num,arg)) != RTEMS_SUCCESSFUL ) { \
        printf("ioctl " #num " failed: errno: %d (%d,%d)\n",errno,ret,RTEMS_SUCCESSFUL); \
        exit(0);\
    } \
} 


static void spw_print_statistics(spw_stats *stats);
static void spw_print_config(spw_config *cnf);



static void spw_print_statistics(spw_stats *stats) 
{
    printf("\n");
    printf(" ******** STATISTICS ********  \n");
    printf("Transmit link errors: %i\n", stats->tx_link_err);
    printf("Receiver RMAP header CRC errors: %i\n", stats->rx_rmap_header_crc_err);
    printf("Receiver RMAP data CRC errors: %i\n", stats->rx_rmap_data_crc_err);
    printf("Receiver EEP errors: %i\n", stats->rx_eep_err);
    printf("Receiver truncation errors: %i\n", stats->rx_truncated);
    printf("Parity errors: %i\n", stats->parity_err);
    printf("Escape errors: %i\n", stats->escape_err);
    printf("Credit errors: %i\n", stats->credit_err);
    printf("Disconnect errors: %i\n", stats->disconnect_err);
    printf("Write synchronization errors: %i\n", stats->write_sync_err);
    printf("Early EOP/EEP: %i\n", stats->early_ep);
    printf("Invalid Node Address: %i\n", stats->invalid_address);
    printf("Packets transmitted: %i\n", stats->packets_sent);
    printf("Packets received: %i\n", stats->packets_received);
}



static void spw_print_config(spw_config *cnf)
{
    printf("\n");
    printf(" ******** CONFIG ********  \n");
    printf("Node Address: %i\n", (int)cnf->nodeaddr);
    printf("Destination Key: %i\n", (int)cnf->destkey);
    printf("Clock Divider: %i\n", (int)cnf->clkdiv);
    printf("Rx Maximum Packet: %i\n", (int)cnf->rxmaxlen);
    printf("Timer: %i\n", (int)cnf->timer);
    printf("Linkdisabled: %i\n", (int)cnf->linkdisabled);
    printf("Linkstart: %i\n", (int)cnf->linkstart);
    printf("Disconnect: %i\n", (int)cnf->disconnect);
    printf("Promiscuous: %i\n", (int)cnf->promiscuous);
    printf("RMAP Enable: %i\n", (int)cnf->rmapen);
    printf("RMAP Buffer Disable: %i\n", (int)cnf->rmapbufdis);
    printf("Check Rmap Error: %i\n", (int)cnf->check_rmap_err);
    printf("Remove Protocol ID: %i\n", (int)cnf->rm_prot_id);
    printf("Blocking Transmit: %i\n", (int)cnf->tx_blocking);
    printf("Disable when Link Error: %i\n", (int)cnf->disable_err);
    printf("Link Error IRQ Enabled: %i\n", (int)cnf->link_err_irq);
    printf("Link Error Event Task ID: %i\n", (int)cnf->event_id);
    printf("RMAP Available: %i\n", (int)cnf->is_rmap);
    printf("RMAP CRC Available: %i\n", (int)cnf->is_rmapcrc);
    printf("Unaligned Receive Buffer Support: %i\n", (int)cnf->is_rxunaligned);
    printf("\n");
}


/* ========================================================= 
   initialisation */



int spw_init(char *iface, int node_addr)
{ 
    int fd0;

    printf("******** Initiating RASTA SPW test ********\n");

    fd0 = open(iface,O_RDWR);
    if ( fd0 < 0 ){
        printf("Failed to open %s driver (%d)\n",iface, errno);
        return -1;
    }
    else
        printf("Spw %s openned\n", iface);

    printf("Starting spacewire links\n");

    IOCTL(fd0,SPACEWIRE_IOCTRL_SET_COREFREQ,30000); /* make driver calculate timings from 30MHz spacewire clock */
    IOCTL(fd0,SPACEWIRE_IOCTRL_SET_NODEADDR,node_addr);
    IOCTL(fd0,SPACEWIRE_IOCTRL_SET_RXBLOCK,0);
    IOCTL(fd0,SPACEWIRE_IOCTRL_SET_TXBLOCK,0);
    IOCTL(fd0,SPACEWIRE_IOCTRL_SET_TXBLOCK_ON_FULL,1);
    IOCTL(fd0,SPACEWIRE_IOCTRL_SET_RM_PROT_ID,1); /* remove protocol id */

    printf("Trying to bring link up\n");
    while(1)
    {
        if (ioctl(fd0, SPACEWIRE_IOCTRL_START, 0) == -1) 
        {
            OS_TaskYield();
        }else
            break;
    }
    printf("Link is up\n");

    spw_config config;
    IOCTL(fd0, SPACEWIRE_IOCTRL_GET_CONFIG, &config);
    spw_print_config(&config);

    return fd0;
}

#define RXPKT_BUF 10
#define PKTSIZE 1024

struct packet_hdr {
    unsigned char addr;
    unsigned char protid;
    unsigned char dummy;
    unsigned char channel;
    unsigned char data[PKTSIZE];
};

/* RX Task */
static unsigned char rxpkt[PKTSIZE*RXPKT_BUF];

void spw_task2(void *ignored) 
{
    int len;
    int cnt=0;
    int j;
    int fd;
    int pkt_size = 0;
    int loops = 0;
    spw_ioctl_packetsize packetsize;
    spw_config config;
    int total;
    OS_time_t t1, t2;
    uint32_t tdelta = 0;

    printf("SpaceWire RX Task started\n");
    fd = spw_init(SPW_DEVICE_2, NODE_ADR_RX);


    printf("========== RX loop ===========\n");
    while(1)
    {
        printf("Waiting for data to come...\n");
        while ( (len = read(fd,&rxpkt[0],sizeof(rxpkt))) < 1 )
        {
            OS_TaskYield();
        }
        printf("%d received\n", len);

        /* skip first 2bytes (vchan and dummy) */
        if ( (rxpkt[0]==1) && (rxpkt[1]==1) ){
            j=2; /* strip virtual channel protocol, non-ssspw device */
        }else{
            j=0; /* hardware uses virtual channel protocol, hw already stripped it */
        }

        /*  We should receive the packet size and the number of loops   */
        pkt_size = (1 << rxpkt[j]);
        loops = (1 << rxpkt[j+1]);

        printf("pkt_size/loops %d/%d\n", pkt_size, loops);

        printf("Reconfiguring the Packet size to %d bytes\n", pkt_size);

        IOCTL(fd, SPACEWIRE_IOCTRL_GET_CONFIG, &config);

        if( pkt_size < config.rxmaxlen ) 
        {
            printf("PKT size to small\n");
            continue;
        }
        else if( pkt_size > config.rxmaxlen )
        {
            printf("Disabling transmitter and receiver\n");
            IOCTL(fd, SPACEWIRE_IOCTRL_STOP, 0);

            packetsize.rxsize = pkt_size;
            packetsize.txdsize = pkt_size;
            packetsize.txhsize = 10;
            IOCTL(fd,SPACEWIRE_IOCTRL_SET_PACKETSIZE,&packetsize);

        }
        else
        {
            printf("Disabling transmitter and receiver\n");
            IOCTL(fd, SPACEWIRE_IOCTRL_STOP, 0);
        }

        printf("Trying to bring link back up\n");
        while(1)
        {
            if (ioctl(fd, SPACEWIRE_IOCTRL_START, 0) == -1) 
            {
                OS_TaskYield();
            }else
                break;
        }
        printf("Link is up\n");

        spw_config config;
        IOCTL(fd, SPACEWIRE_IOCTRL_GET_CONFIG, &config);
        spw_print_config(&config);

        /*  Return the go ahead */
//        printf("Return the go ahead\n");
//        rxpkt[0] = NODE_ADR_TX;
//        if ( (len = write(fd, &rxpkt[0], len)) < 0 ){
//            printf("Failed to write errno: %d\n",errno);
//            exit(-1);
//        }

        printf("Starting (%d loops)............\n", loops);
        total = 0;
        OS_GetLocalTime(&t1);
        while( loops-- )
        {
            while ( (len = read(fd,&rxpkt[0],sizeof(rxpkt))) < 1 )
            {
                OS_TaskYield();
            }
            total += len;
        }
        OS_GetLocalTime(&t2);

        tdelta = (t2.seconds-t1.seconds)*1000000+(t2.microseconds-t1.microseconds);
        printf("Results for test_block_write on Network Remote Storage device: %ld usec, %.3lf Mbps.\n", tdelta, (double)total*8/(double)tdelta );
    }
}

int main(void)
{
    static uint32_t t1;

    printf("Spacewire Echo Client\n");

    /*  init the RASTA HW   */
    //    init_pci();
    //    rasta_register();

    OS_Init();

    if( OS_TaskCreate (&t1,(void *)spw_task2, NULL, 2048, 100, 0, (void*)&t1) != OS_SUCCESS)
        printf("ERR: unable to create the tasks\n");

    OS_Start();


    return 0;


} /* end OS_Application Startup */
