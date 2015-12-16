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

#include <pci.h>
//#include <rasta.h>

#define NODE_ADR_RX 33
#define NODE_ADR_TX 35

//#define SPW_DEVICE_1    "/dev/grspwrasta1"
#define SPW_DEVICE_1    "/dev/rastaio0/grspw1"

#define PKTSIZE 1024
static unsigned char rxpkt[PKTSIZE*10];


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
    int fd;

    printf("******** Initiating RASTA SPW test ********\n");

    fd = open(iface,O_RDWR);
    if ( fd < 0 ){
        printf("Failed to open %s driver (%d)\n",iface, errno);
        return -1;
    }
    else
        printf("Spw %s openned\n", iface);

    printf("Starting spacewire links\n");

    IOCTL(fd,SPACEWIRE_IOCTRL_SET_COREFREQ,30000); /* make driver calculate timings from 30MHz spacewire clock */
    IOCTL(fd,SPACEWIRE_IOCTRL_SET_NODEADDR,node_addr);
    IOCTL(fd,SPACEWIRE_IOCTRL_SET_RXBLOCK,0);
    IOCTL(fd,SPACEWIRE_IOCTRL_SET_TXBLOCK,0);
    IOCTL(fd,SPACEWIRE_IOCTRL_SET_TXBLOCK_ON_FULL,1);
    IOCTL(fd,SPACEWIRE_IOCTRL_SET_RM_PROT_ID,1); /* remove protocol id */

    spw_ioctl_packetsize packetsize;
    packetsize.rxsize = 8192;
    packetsize.txdsize = 8192;
    packetsize.txhsize = 10;
    IOCTL(fd,SPACEWIRE_IOCTRL_SET_PACKETSIZE,&packetsize);


    printf("Trying to bring link up\n");
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
    return fd;
}

struct packet_hdr {
    unsigned char addr;
    unsigned char protid;
    unsigned char dummy;
    unsigned char channel;
    unsigned char data[PKTSIZE];
};

void init_pkt(void *_p)
{
    struct packet_hdr *p = (struct packet_hdr*)_p;

    p->addr = NODE_ADR_RX;
    p->protid = 50;
    p->dummy = 0x01;
    p->channel = 0x01;

//    bzero( p->data, sizeof(p->data) );
//    for(i=0; i<sizeof(p->data); i++)
//        p->data[i] = i;
}

#define LOOPS_LOG2  9
#define SIZE_LOG2   10
void spw_task1(void *ignored) 
{ 
    int len;
    int fd;
    int size;
    int size_log2 = SIZE_LOG2;
    int loops = (1 << LOOPS_LOG2);
    int delay = 0;

    printf("SpaceWire TX Task started\n");

    init_pkt((void*)rxpkt);

    fd = spw_init(SPW_DEVICE_1, NODE_ADR_TX);

    printf("========== RX loop ===========\n");
    while ( 1 ) 
    {
        if( size_log2 == 14 ) break;
        size = (1 << size_log2);

        loops = (1 << LOOPS_LOG2);
        /*  Fill the packet size and the number of loops    */
        rxpkt[4] = size_log2++;
        rxpkt[5] = LOOPS_LOG2;
        printf("size = %d (%d)\n", rxpkt[4], size);
        printf("loops = %d\n", rxpkt[5]);

#if 0
        /*  Get the configuration to check the packet size  */
        spw_config config;
        IOCTL(fd, SPACEWIRE_IOCTRL_GET_CONFIG, &config);

        if( size < config.rxmaxlen ) 
        {
            printf("PKT size to small\n");
            continue;
        }
        else if( size > config.rxmaxlen )
        {
            printf("Disabling transmitter and receiver\n");
            IOCTL(fd, SPACEWIRE_IOCTRL_STOP, 0);

            spw_ioctl_packetsize packetsize;
            packetsize.rxsize = size;
            packetsize.txdsize = size;
            packetsize.txhsize = 10;
            IOCTL(fd,SPACEWIRE_IOCTRL_SET_PACKETSIZE,&packetsize);

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
        }
#endif

        if ( (len = write(fd, rxpkt, 32)) < 0 )
        {
            printf("Failed to write errno: (%d)\n", errno);
            exit(-1);
        }

        /*  Wait for the go ahead   */

        OS_Sleep(1000);
//        printf("Waiting for the go ahead\n");
//        while ( (len = read(fd,&rxpkt[0],sizeof(rxpkt))) < 1 )
//            OS_TaskYield();

        bzero( rxpkt, sizeof(rxpkt) );
        init_pkt( (struct packet_hdr*) rxpkt);
        printf("Go %d loops\n", loops);
        delay++;
        printf("Tx Delay %d\n", delay);
        while( loops-- )
        {
            OS_Sleep(delay);    // FIXME
            if ( (len = write(fd, rxpkt, size)) < 0 )
            {
                printf("Failed to write errno: (%d)\n", errno);
                exit(-1);
            }
        }

        printf("Done\n");
        OS_Sleep(1000);

//        printf("Waiting feedback data\n");
//        while ( (len = read(fd,&rxpkt[0],PKTSIZE*RXPKT_BUF)) < 1 )
//        {
//            OS_TaskYield();
//        }
    }

}

int main(void)
{
    static uint32_t t1;

    printf("Spacewire Echo Server\n");

    /*  init the RASTA HW   */
    //    init_pci();
    //    rasta_register();

    OS_Init();

    if( OS_TaskCreate (&t1,(void *)spw_task1, NULL, 2048, 90, 0, (void*)&t1) != OS_SUCCESS)
        printf("ERR: unable to create the tasks\n");

    OS_Start();


    return 0;


} /* end OS_Application Startup */
