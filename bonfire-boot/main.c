#include "bonfire.h"
#include <stdio.h>
#include <stdbool.h>


#include <time.h>
#include "pico_stack.h"
#include "pico_ipv4.h"
#include "pico_icmp4.h"
#include "pico_dhcp_client.h"
#include "pico_ethernet.h"
#include "pico_dns_client.h"
#include "monitor.h"
#include "console.h"
#include "uart.h"

#define NUM_PING 10

extern bool enable_debug;

static int finished = 0;



/* gets called when the ping receives a reply, or encounters a problem */
void cb_ping(struct pico_icmp4_stats *s)
{
    char host[30];
    pico_ipv4_to_string(host, s->dst.addr);
    if (s->err == 0) {
        /* if all is well, print some pretty info */
        dbg("%lu bytes from %s: icmp_req=%lu ttl=%lu time=%lu ms\n", s->size,
                host, s->seq, s->ttl, (long unsigned int)s->time);
        if (s->seq >= NUM_PING)
            finished = 1;
    } else {
        /* if something went wrong, print it and signal we want to stop */
        dbg("PING %lu to %s: Error %d\n", s->seq, host, s->err);
        finished = 1;
    }
}

extern struct pico_device *pico_eth_create(const char *name, const uint8_t *mac);

static uint8_t c_mac[] =  {0,0, 0x5e,0,0x0fa,0x0ce };

volatile bool  dhcp_succes =false;

void cb_dhcp(void *cli,int code)
{
struct pico_ip4 address, gw, netmask, dns;

char adr_b[16],gw_b[16],netmask_b[16],dns_b[16]; 

   dbg("DHCP callback %d\n",code);

   switch(code) {
     
     case PICO_DHCP_SUCCESS: 
        address=pico_dhcp_get_address(cli);
        gw=pico_dhcp_get_gateway(cli);
        netmask=pico_dhcp_get_netmask(cli);
        dns=pico_dhcp_get_nameserver(cli,0);

        pico_ipv4_to_string(adr_b,address.addr);
        pico_ipv4_to_string(gw_b,gw.addr);
        pico_ipv4_to_string(netmask_b,netmask.addr);
        pico_ipv4_to_string(dns_b,dns.addr);

        dbg("DHCP assigned  ip: %s mask: %s gw: %s dns: %s \n",
                adr_b,netmask_b,gw_b,dns_b);

        if (dns.addr) {
          pico_dns_client_nameserver(&dns,PICO_DNS_NS_ADD);
          struct pico_ip4 default_ns;
          if (pico_string_to_ipv4(PICO_DNS_NS_DEFAULT, (uint32_t *)&default_ns.addr) >= 0) {
              pico_dns_client_nameserver(&default_ns,PICO_DNS_NS_DEL);
          }        
        }
        dhcp_succes=true;

        break;
     case PICO_DHCP_ERROR: dbg("DHCP Error\n"); break;

   }

}

//extern void app_tcpecho(uint16_t source_port);
extern void tftp_server_main();

int main(void)
{
    int id;
    struct pico_ip4 ipaddr, netmask;
    struct pico_device* dev;
    uint32_t cid;
    unsigned long timeout;

    setBaudRate(BAUDRATE);
    #ifndef SIM
      wait(1000000);
    #endif

    /* initialise the stack. Super important if you don't want ugly stuff like
     * segfaults and such! */
    pico_stack_init();

    
    dev = pico_eth_create("eth0",c_mac);
    // if (!dev)
    //     return -1;
    

    /* assign the IP address to the  interface */
    pico_string_to_ipv4("0.0.0.0", &ipaddr.addr);
    pico_string_to_ipv4("255.255.255.0", &netmask.addr);
    pico_ipv4_link_add(dev, ipaddr, netmask);
   
    pico_dhcp_initiate_negotiation(dev,cb_dhcp,&cid);

    timeout = PICO_TIME() + 10; // Wait max. 10 seconds for DHCP
    while (!dhcp_succes && PICO_TIME()<timeout ){
       pico_stack_tick();
    }; // Wait until DHCP address assigend 
    if (!dhcp_succes) printk("DHCP no response, still trying\n");
    enable_debug=false;
    //dbg("starting ping\n");
    //id = pico_icmp4_ping("192.168.26.2", NUM_PING, 1000, 10000, 64, cb_ping);


    tftp_server_main();
    mon_main();
   
    return 0;
}