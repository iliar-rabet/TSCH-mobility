#include "contiki.h"
#include "net/ipv6/simple-udp.h"
#include "net/mac/tsch/tsch.h"
#include "net/mac/tsch/tsch-queue.h"
#include "lib/random.h"
#include "sys/node-id.h"
// #include "routing/rpl-classic/rpl-dag.h"
#include "routing/rpl-lite/rpl-dag-root.h"
#include "sys/log.h"
#include "services/orchestra/orchestra.h"

#include "sys/energest.h"

#define LOG_MODULE "App"
#define LOG_LEVEL LOG_LEVEL_INFO

#define UDP_PORT	8765


PROCESS(node_process, "TSCH Schedule Node");

# if Q_STABLE
#include "neg.h"
    // static struct simple_udp_connection server_neg_conn;  
    static struct simple_udp_connection neg_conn;
    static uint16_t neg_counter;
    bool q_unstable=false;
    bool bc_unstable=false;
    int scheduled_slots[40];
    #if BC_STABLE
      static bool enable_bc;
      static struct ctimer ack_timer;
    #endif
    int tree[NUM_NODES];
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

// #define Q_STABLE
# if Q_STABLE

void send_bc(){
  uip_sr_node_t *link;
  // rpl_dag_root_print_links(" ");

  link = uip_sr_node_head();
  while(link != NULL) {
    // char buf[100];
    uip_ipaddr_t child_ipaddr;
    uip_ipaddr_t parent_ipaddr;

    NETSTACK_ROUTING.get_sr_node_ipaddr(&child_ipaddr, link);
    NETSTACK_ROUTING.get_sr_node_ipaddr(&parent_ipaddr, link->parent);


    
    if(link->acked==false){
      // LOG_INFO("not acked:");
      // LOG_INFO_6ADDR(&child_ipaddr);
      // LOG_INFO("\n");
      struct neg_pack p;
      p.is_broadcast=1;
      p.slotframe = ORCHESTRA_UNICAST_PERIOD/2 + neg_counter;  
      p.isAdd = true;
      simple_udp_sendto(&neg_conn, (char *)&p, sizeof(p), &child_ipaddr);  
    }
    else{
      // LOG_INFO("acked ");
      // LOG_INFO_6ADDR(&child_ipaddr);
      // LOG_INFO("\n");
    }
    
    link = uip_sr_node_next(link);
  }
  LOG_INFO("links: end of list\n");

}

static void
neg_callback(struct simple_udp_connection *c,
          const uip_ipaddr_t *sender_addr,
          uint16_t sender_port,
          const uip_ipaddr_t *receiver_addr,
          uint16_t receiver_port,
          const uint8_t *data,
          uint16_t datalen)
{
    neg_counter++;
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

#if BC_STABLE
    if(data[0]=='b' && data[1]=='c'){ 
      LOG_INFO("Server NEG BC\n");
      send_bc();
      ctimer_set(&ack_timer,CLOCK_SECOND * 0.1, send_bc, NULL);
      return;
    }
    if(data[0]=='A' && data[1]=='C' && data[2]=='K' && data[3]=='B'){ 
      // set_ack_true(sender_addr);
      return;
    }
#endif

    LOG_INFO("Server NEG UC\n");
    uip_ipaddr_t requested_parent;
    memcpy(&requested_parent, data,sizeof(uip_ipaddr_t));
    LOG_INFO_6ADDR(&requested_parent);

    struct neg_pack p;
    p.slotframe = ORCHESTRA_UNICAST_PERIOD/2 + neg_counter;
    p.isRX = false;
    p.isAdd = true;
    p.is_broadcast=false;
    simple_udp_sendto(&neg_conn, (char *)&p, sizeof(p), sender_addr);  

    struct neg_pack p2;
    p2.slotframe = ORCHESTRA_UNICAST_PERIOD - neg_counter;
    p2.isRX = true;
    p2.isAdd = true;
    p.is_broadcast=false;
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
  
  etimer_set(&neg_timer, START_AFTER * CLOCK_SECOND);
  
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
  static struct etimer bc_timer;


  PROCESS_BEGIN();
  NETSTACK_ROUTING.root_start(); 

  #if BC_STABLE
  enable_bc=false;
  #endif
  simple_udp_register(&udp_conn, UDP_PORT, NULL, UDP_PORT, data_rx_callback);  
  
  etimer_set(&bc_timer, START_AFTER * CLOCK_SECOND);

  while(1) {
    PROCESS_WAIT_EVENT_UNTIL(etimer_expired(&bc_timer));

    etimer_set(&bc_timer, CLOCK_SECOND);

  } 
  PROCESS_END();
}


// static unsigned long
// to_seconds(uint64_t time)
// {
//   return (unsigned long)(time / ENERGEST_SECOND);
// }

// PROCESS_THREAD(energest_example_process, ev, data)
// {
//   static struct etimer periodic_timer;
//   PROCESS_BEGIN();

//   etimer_set(&periodic_timer, CLOCK_SECOND * 60);
//   while(1) {
//     PROCESS_WAIT_EVENT_UNTIL(etimer_expired(&periodic_timer));
//     etimer_reset(&periodic_timer);

//     /* Update all energest times. */
//     energest_flush();

//     printf("\nEnergest:\n");
//     printf(" CPU          %4lus LPM      %4lus DEEP LPM %4lus  Total time %lus\n",
//            to_seconds(energest_type_time(ENERGEST_TYPE_CPU)),
//            to_seconds(energest_type_time(ENERGEST_TYPE_LPM)),
//            to_seconds(energest_type_time(ENERGEST_TYPE_DEEP_LPM)),
//            to_seconds(ENERGEST_GET_TOTAL_TIME()));
//     printf(" Radio LISTEN %4lus TRANSMIT %4lus OFF      %4lus\n",
//            to_seconds(energest_type_time(ENERGEST_TYPE_LISTEN)),
//            to_seconds(energest_type_time(ENERGEST_TYPE_TRANSMIT)),
//            to_seconds(ENERGEST_GET_TOTAL_TIME()
//                       - energest_type_time(ENERGEST_TYPE_TRANSMIT)
//                       - energest_type_time(ENERGEST_TYPE_LISTEN)));
//   }
//   PROCESS_END();
// }