
#include "contiki.h"
#include "net/ipv6/simple-udp.h"
#include "net/mac/tsch/tsch.h"
#include "net/mac/tsch/tsch-queue.h"
#include "lib/random.h"
#include "sys/node-id.h"

#include "sys/log.h"
#include "services/orchestra/orchestra.h"

#include "neg.h"

#define LOG_MODULE "App"
#define LOG_LEVEL LOG_LEVEL_INFO

#define UDP_PORT	8765
#define SEND_INTERVAL		  (2 * CLOCK_SECOND)


#include "qstable.c"

PROCESS(node_process, "TSCH Schedule Node");

# ifdef Q_STABLE
    // static struct simple_udp_connection server_neg_conn;  
    static struct simple_udp_connection neg_conn;
    bool q_unstable=false;

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
      }
      etimer_set(&periodic_timer, random_rand() % SEND_INTERVAL);
  
  }
  }
  PROCESS_END();
}
