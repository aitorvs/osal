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

#define RXPKT_BUF 5
#define PKTSIZE 1000
static unsigned char rxpkt[PKTSIZE*RXPKT_BUF];


/* Start SSSPW driver */  
#define IOCTL(fd,num,arg) \
{ \
    int ret; \
    if ( (ret=ioctl(fd,num,arg)) != RTEMS_SUCCESSFUL ) { \
        printf("ioctl " #num " failed: errno: %d (%d,%d)\n",errno,ret,RTEMS_SUCCESSFUL); \
        return -1;\
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
    while(1){
        if (ioctl(fd0, SPACEWIRE_IOCTRL_START, 0) == -1) {
            sched_yield(); /*printf("ioctl failed: SPACEWIRE_IOCTRL_START\n");*/
        }else
            break;
    }
    printf("Link is up\n");

    spw_config config;
    IOCTL(fd0, SPACEWIRE_IOCTRL_GET_CONFIG, &config);
    spw_print_config(&config);
    return fd0;
}

#define RXPKT_BUF 5
#define PKTSIZE 1000

struct packet_hdr {
    unsigned char addr;
    unsigned char protid;
    unsigned char dummy;
    unsigned char channel;
    unsigned char data[PKTSIZE];
};

/* RX Task */
static struct packet_hdr txpkts[1];

void init_pkt(struct packet_hdr *p){
    int i;
    unsigned char j=0;

    p->addr = NODE_ADR_RX;
    p->protid = 50;
    p->dummy = 0x01;
    p->channel = 0x01;
    for(i=0; i<PKTSIZE; i++){
        p->data[i] = j;
        j++;
    }
}

void spw_task1(void *ignored) 
{ 

    int tx_bytes=0;
    int tx_pkts=0;

    unsigned char i;
    int j;
    int len,cnt=0;
    int loop;
    int fd0;

    printf("SpaceWire TX Task started\n");

    for(i=0; i<1; i++)
        init_pkt(&txpkts[i]);

    fd0 = spw_init(SPW_DEVICE_1, NODE_ADR_TX);

    i=0;
    loop=0;
    printf("========== RX loop ===========\n");
    while ( 1 ) 
    {
        for(j=0;j<PKTSIZE;j++)
        {
            txpkts[0].data[j] = i++;
        }

        if ( (len=write(fd0,txpkts,PKTSIZE+4)) < 0 ){
            printf("Failed to write errno:%d (%d)\n",errno,cnt);
            exit(-1);
        }
        tx_bytes+=len;
        tx_pkts++;
        /*    rtems_task_wake_after(1);*/
        printf("SPW TX: bytes: %d, packets: %d\n",tx_bytes,tx_pkts);

#if 1
        printf("Waiting feedback data\n");
        while ( (len = read(fd0,&rxpkt[0],PKTSIZE*RXPKT_BUF)) < 1 )
        {
//            printf("Failed read: len: %d, errno: %d (%d)\n",len,errno,cnt);
            OS_TaskYield();
        }
        printf("%d received\n", len);
#endif
        OS_Sleep(500);
        cnt++;
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

    if( OS_TaskCreate (&t1,(void *)spw_task1, 2048, 90, 0, (void*)&t1) != 0)
        printf("ERR: unable to create the tasks\n");

    OS_Start();


    return 0;


} /* end OS_Application Startup */
