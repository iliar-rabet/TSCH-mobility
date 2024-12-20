  #include "contiki.h"
#include "net/ipv6/simple-udp.h"
#include "net/mac/tsch/tsch.h"
#include "net/mac/tsch/tsch-queue.h"
#include "lib/random.h"
#include "sys/node-id.h"

#include "sys/log.h"
#include "services/orchestra/orchestra.h"
#include "net/routing/rpl-lite/rpl-neighbor.h"
#include "net/link-stats.h"
#include "net/packetbuf.h"

#include "neg.h"
#include "call-back.h"

uip_ipaddr_t * overflow_ip;

#define LOG_MODULE "App"
#define LOG_LEVEL LOG_LEVEL_INFO

#define UDP_PORT	8765


PROCESS(node_process, "TSCH Schedule Node");

#if Q_STABLE
  #include "net/routing/rpl-lite/rpl-timers.h"
  static struct simple_udp_connection neg_conn;
  bool q_unstable=false;
  #if BC_STABLE
    bool bc_unstable=false;
    bool mobility_mode=false;
  #endif
  PROCESS(neg_process, "NEG process");
  AUTOSTART_PROCESSES(&node_process, &neg_process);

#else
  AUTOSTART_PROCESSES(&node_process);
#endif

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


PROCESS_THREAD(node_process, ev, data)
{
  static struct simple_udp_connection udp_conn;
  static struct etimer periodic_timer;
  static uint32_t seqnum;
  // char str[20];
  uip_ipaddr_t dst;

  PROCESS_BEGIN();
  simple_udp_register(&udp_conn, UDP_PORT, NULL, UDP_PORT, data_rx_callback);  

      /* Initialization; `rx_packet` is the function for packet reception */
      etimer_set(&periodic_timer, CLOCK_SECOND * START_AFTER);

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

        etimer_set(&periodic_timer, random_rand() % M_SEND_INTERVAL);
    }
    etimer_set(&periodic_timer, random_rand() % M_SEND_INTERVAL);
  }
  PROCESS_END();
}

#if Q_STABLE
PROCESS_THREAD(neg_process, ev, data)
{
  static struct etimer neg_timer;

  // char payload[LINKADDR_SIZE];
  uip_ipaddr_t root_ip;
  #if BC_STABLE
    uip_ipaddr_t bc_ip;
  #endif

  PROCESS_BEGIN();

  if (simple_udp_register(&neg_conn, NEG_PORT, NULL, NEG_PORT, neg_callback) == 0)
    LOG_INFO("NEG UDP not OK\n");
  
  etimer_set(&neg_timer, START_AFTER * CLOCK_SECOND);
  PROCESS_WAIT_EVENT_UNTIL(etimer_expired(&neg_timer));
  #if BC_STABLE
  bc_unstable=false;
  #endif
  etimer_set(&neg_timer, CLOCK_SECOND);

 while(1) {
    PROCESS_WAIT_EVENT_UNTIL(etimer_expired(&neg_timer));
    if(NETSTACK_ROUTING.node_is_reachable() && NETSTACK_ROUTING.get_root_ipaddr(&root_ip)) {
      /* Send network uptime timestamp to the network root node */  
      
      #if BC_STABLE
      if(bc_unstable==false){
        //stop sending
        mobility_mode=false;
      }
      else if(bc_unstable==true ){
          LOG_INFO("Asking for extra BC\n");
          struct neg_pack p;
          p.is_broadcast=true;
          // simple_udp_sendto(&neg_conn, "bc\n", strlen("bc\n"), &root_ip);

          uip_create_linklocal_allnodes_mcast(&bc_ip);
          simple_udp_sendto(&neg_conn, &p, sizeof(struct neg_pack), &bc_ip);

          bc_unstable=false;
          mobility_mode=true;

          // rpl_timers_dio_reset("TRICKLE RESET MN");

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
      #if BC_STABLE
      }
      #endif
      #if ENERGEST_CONF_ON
          printf(" Radio LISTEN %4lus TRANSMIT %4lus OFF      %4lus\n",
           to_seconds(energest_type_time(ENERGEST_TYPE_LISTEN)),
           to_seconds(energest_type_time(ENERGEST_TYPE_TRANSMIT)),
           to_seconds(ENERGEST_GET_TOTAL_TIME()
                      - energest_type_time(ENERGEST_TYPE_TRANSMIT)
                      - energest_type_time(ENERGEST_TYPE_LISTEN)));
      #endif
    }
    etimer_set(&neg_timer, 2*CLOCK_SECOND);
  }

  PROCESS_END();

}
#endif