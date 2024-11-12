
#include "contiki.h"
#include "net/ipv6/simple-udp.h"
#include "net/mac/tsch/tsch.h"
#include "net/mac/tsch/tsch-queue.h"
#include "lib/random.h"
#include "sys/node-id.h"

#include "sys/log.h"
#include "services/orchestra/orchestra.h"

#include "neg.h"

// #include "net/routing/rpl-classic/"
#define LOG_MODULE "App"
#define LOG_LEVEL LOG_LEVEL_INFO

#define UDP_PORT	8765
#define SEND_INTERVAL		  (20 * CLOCK_SECOND)


PROCESS(node_process, "TSCH Schedule Node");

# ifdef Q_STABLE
#include "net/routing/rpl-lite/rpl-timers.h"
    // static struct simple_udp_connection server_neg_conn;  
    static struct simple_udp_connection neg_conn;
    bool q_unstable=false;
    #if BC_STABLE
    bool bc_unstable=false;
    #endif
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
  
    struct  neg_pack p;
    memcpy(&p, data, sizeof(p));
    LOG_INFO("NEG received from ");
    LOG_INFO_6ADDR(sender_addr);
    LOG_INFO(" %u \n", p.slotframe);

    if(p.is_broadcast){
      LOG_INFO("adding BC\n");
      rpl_timers_dio_reset("TRICKLE RESET");
      tsch_schedule_add_link(sf_common,
      LINK_OPTION_TX | LINK_OPTION_RX  | LINK_OPTION_SHARED,
          LINK_TYPE_ADVERTISING, &tsch_broadcast_address,
          p.slotframe, 0, 1);
      // simple_udp_sendto(&neg_conn, "ACKBC", 5, sender_addr);

    }

    if(p.isRX){
      LOG_INFO("adding rx\n");
      tsch_schedule_add_link(sf_unicast,
          LINK_OPTION_RX | LINK_OPTION_SHARED,
          LINK_TYPE_ADVERTISING, &tsch_broadcast_address,
          p.slotframe, 0, 1);
    
    }
    else 
    {
        LOG_INFO("adding tx\n");
        tsch_schedule_add_link(sf_unicast,
          LINK_OPTION_TX | LINK_OPTION_SHARED,
          LINK_TYPE_ADVERTISING, &tsch_broadcast_address,
          p.slotframe, 0, 1);
    }

}


PROCESS_THREAD(neg_process, ev, data)
{
  static struct etimer neg_timer;

  // char payload[LINKADDR_SIZE];
  uip_ipaddr_t root_ip;

  PROCESS_BEGIN();

  if (simple_udp_register(&neg_conn, NEG_PORT, NULL, NEG_PORT, neg_callback) == 0)
    LOG_INFO("NEG UDP not OK\n");
  
  etimer_set(&neg_timer, START_AFTER * CLOCK_SECOND);
  #ifdef BC_STABLE
  bc_unstable=false;
  #endif
  while(1) {
    PROCESS_WAIT_EVENT_UNTIL(etimer_expired(&neg_timer));
    if(NETSTACK_ROUTING.node_is_reachable() && NETSTACK_ROUTING.get_root_ipaddr(&root_ip)) {
      /* Send network uptime timestamp to the network root node */  
      
      #ifdef BC_STABLE
      if(bc_unstable==true ){
          // LOG_INFO("Asking for extra BC\n");
          // simple_udp_sendto(&neg_conn, "bc\n", strlen("bc\n"), &root_ip);
          // bc_unstable=false;
      }
      else {
      #endif
      if(q_unstable == true){
        simple_udp_sendto(&neg_conn, (void *)overflow_ip, sizeof(uip_ipaddr_t), &root_ip);
        LOG_INFO("Asking for extra ");
        LOG_INFO_6ADDR(&root_ip); 
        q_unstable=false;
        // memcpy(payload, overflows_node_ll, LINKADDR_SIZE);

      }
      #ifdef BC_STABLE
      }
      #endif
    }
    etimer_set(&neg_timer, CLOCK_SECOND);
  }

  PROCESS_END();

}
#endif

PROCESS_THREAD(node_process, ev, data)
{
  static struct simple_udp_connection udp_conn;
  static struct etimer periodic_timer;
  static uint32_t seqnum;
  // char str[20];
  uip_ipaddr_t dst;

  PROCESS_BEGIN();
  simple_udp_register(&udp_conn, UDP_PORT, NULL, UDP_PORT, data_rx_callback);  

  if(node_id == 1) {  /* Running on the root? */
    NETSTACK_ROUTING.root_start();
  }
  else
  {
      /* Initialization; `rx_packet` is the function for packet reception */
      etimer_set(&periodic_timer, CLOCK_SECOND * 60);

      /* Main loop */
      while(1) {
        PROCESS_WAIT_EVENT_UNTIL(etimer_expired(&periodic_timer));
        if(NETSTACK_ROUTING.node_is_reachable()
          && NETSTACK_ROUTING.get_root_ipaddr(&dst)) {
          /* Send network uptime timestamp to the network root node */
          seqnum++;
          LOG_INFO("Send to ");
          LOG_INFO_6ADDR(&dst);
          LOG_INFO_(", seqnum %" PRIu32 " ;\n", seqnum);
          simple_udp_sendto(&udp_conn, &seqnum, sizeof(seqnum), &dst);

          // LOG_INFO("DIO INTERVAL %d\n",rpl_get_any_dag()->dio_intcurrent);
      }
      etimer_set(&periodic_timer, random_rand() % SEND_INTERVAL);
  
  }
  }
  PROCESS_END();
}
