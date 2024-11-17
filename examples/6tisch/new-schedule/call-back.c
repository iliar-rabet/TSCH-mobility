#include "contiki.h"
#include "net/mac/tsch/tsch.h"
#include "services/orchestra/orchestra.h"
#include "sys/log.h"
#include "net/ipv6/simple-udp.h"
#include "neg.h"
#include "net/routing/rpl-lite/rpl-timers.h"

#define LOG_MODULE "App"
#define LOG_LEVEL LOG_LEVEL_INFO

int added_tx=0;
int added_rx=0;
int dc = 100 * (1/ORCHESTRA_UNICAST_PERIOD);
#ifdef Q_STABLE

void neg_callback(struct simple_udp_connection *c,
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

    #if BC_STABLE
    if(p.is_broadcast){
      LOG_INFO("adding BC\n");
      rpl_timers_dio_reset("TRICKLE RESET");
      tsch_schedule_add_link(sf_common,
      LINK_OPTION_TX | LINK_OPTION_RX  | LINK_OPTION_SHARED,
          LINK_TYPE_ADVERTISING, &tsch_broadcast_address,
          p.slotframe, 0, 1);

    }
    #endif

    if(p.isRX){
      added_rx++;
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


            // #ifdef Q_STABLE
            //   tsch_queue_count_all_queues();
            // #endif

#endif