#include "contiki.h"
#include "net/ipv6/simple-udp.h"
#include "net/mac/tsch/tsch.h"
#include "net/mac/tsch/tsch-queue.h"
#include "lib/random.h"
#include "sys/node-id.h"
#include "routing/rpl-lite/rpl-dag-root.h"
#include "sys/log.h"
#include "services/orchestra/orchestra.h"

// #include "schedule_ll.c"
#include "neg.h"

#define LOG_MODULE "App"
#define LOG_LEVEL LOG_LEVEL_INFO

#define UDP_PORT	8765
#define SEND_INTERVAL		  (10 * CLOCK_SECOND)


PROCESS(node_process, "TSCH Schedule Node");

# ifdef Q_STABLE
    // static struct simple_udp_connection server_neg_conn;  
    static struct simple_udp_connection neg_conn;
    static uint16_t neg_counter;
    bool q_unstable=false;
    int scheduled_slots[40];

PROCESS(neg_process, "NEG process");
AUTOSTART_PROCESSES(&node_process, &neg_process);

#else
AUTOSTART_PROCESSES(&node_process);
#endif


/* Put all cells on the same slotframe */
#define APP_SLOTFRAME_HANDLE 1
/* Put all unicast cells on the same timeslot (for demonstration purposes only) */
#define APP_UNICAST_TIMESLOT 1



static void
data_rx_callback(struct simple_udp_connection *c,
          const uip_ipaddr_t *sender_addr,
          uint16_t sender_port,
          const uip_ipaddr_t *receiver_addr,
          uint16_t receiver_port,
          const uint8_t *data,
          uint16_t datalen)
{
  uint32_t seqnum;

  if(datalen >= sizeof(seqnum)) {
    memcpy(&seqnum, data, sizeof(seqnum));

    LOG_INFO("Received from ");
    LOG_INFO_6ADDR(sender_addr);
    LOG_INFO_(", seqnum %" PRIu32 " ;\n", seqnum);
  }
}

# ifdef Q_STABLE

static void
neg_callback(struct simple_udp_connection *c,
          const uip_ipaddr_t *sender_addr,
          uint16_t sender_port,
          const uip_ipaddr_t *receiver_addr,
          uint16_t receiver_port,
          const uint8_t *data,
          uint16_t datalen)
{
    
    LOG_INFO("Server NEG  ");

    neg_counter++;

    // rpl_dag_root_print_links("");
    // used_orchestra_links("");
    // for (i=0;i<40;i++)
    // {
    //   if (scheduled_slots[i] == 0)
    //     break;
    //   LOG_INFO("scheduled_slots[i] %d", scheduled_slots[i])
    // }
    
    // if(find_parent(sender_addr)==NULL){
    //   LOG_INFO("CAN NOT FIND PARENT\n");
    //   return;
    // }

    uip_ipaddr_t requested_parent;
    memcpy(&requested_parent, data,sizeof(uip_ipaddr_t));
    LOG_INFO_6ADDR(&requested_parent);

    struct neg_pack p;
    p.slotframe = ORCHESTRA_UNICAST_PERIOD/2 + neg_counter;
    p.isRX = false;
    p.isAdd = true;
    simple_udp_sendto(&neg_conn, (char *)&p, sizeof(p), sender_addr);  

    struct neg_pack p2;
    p2.slotframe = ORCHESTRA_UNICAST_PERIOD - neg_counter;
    p2.isRX = true;
    p2.isAdd = true;
    simple_udp_sendto(&neg_conn, (char *)&p2, sizeof(p2), &requested_parent);  

}


PROCESS_THREAD(neg_process, ev, data)
{
  static struct etimer neg_timer;

  // char payload[LINKADDR_SIZE];
  uip_ipaddr_t root_ip;
  char str[10];
 

  PROCESS_BEGIN();
  neg_counter=0;
  if (simple_udp_register(&neg_conn, NEG_PORT, NULL, NEG_PORT, neg_callback) == 0)
    LOG_INFO("NEG UDP not OK\n");
  
  etimer_set(&neg_timer, random_rand() % SEND_INTERVAL);
  
 while(1) {
    PROCESS_WAIT_EVENT_UNTIL(etimer_expired(&neg_timer));
    if(NETSTACK_ROUTING.node_is_reachable() && NETSTACK_ROUTING.get_root_ipaddr(&root_ip)) {
      /* Send network uptime timestamp to the network root node */  
      
      if(q_unstable == true){
        LOG_INFO("IF Q_UNSTABLE\n");
        sprintf(str,"hello");
        simple_udp_sendto(&neg_conn, str, strlen(str), &root_ip);
        LOG_INFO("Asking for extra ");
        LOG_INFO_6ADDR(&root_ip);       
        q_unstable=false;

      }
    }
    etimer_set(&neg_timer, SEND_INTERVAL * 2);
  }

  PROCESS_END();

}
#endif

PROCESS_THREAD(node_process, ev, data)
{
  static struct simple_udp_connection udp_conn;
  

  PROCESS_BEGIN();
  
    simple_udp_register(&udp_conn, UDP_PORT, NULL, UDP_PORT, data_rx_callback);  
    NETSTACK_ROUTING.root_start(); 
  
  PROCESS_END();
}