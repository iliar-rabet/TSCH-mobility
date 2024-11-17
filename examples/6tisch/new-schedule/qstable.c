
# if Q_STABLE
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
  
  etimer_set(&neg_timer, random_rand() % SEND_INTERVAL);
  
 while(1) {
    PROCESS_WAIT_EVENT_UNTIL(etimer_expired(&neg_timer));
    if(NETSTACK_ROUTING.node_is_reachable() && NETSTACK_ROUTING.get_root_ipaddr(&root_ip)) {
      /* Send network uptime timestamp to the network root node */  
      
      if(q_unstable == true){
        LOG_INFO("IF Q_UNSTABLE\n");
        simple_udp_sendto(&neg_conn, (void *)overflow_ip, sizeof(uip_ipaddr_t), &root_ip);
        LOG_INFO("Asking for extra ");
        LOG_INFO_6ADDR(&root_ip); 
        q_unstable=false;
        // memcpy(payload, overflows_node_ll, LINKADDR_SIZE);

      }
    }
    etimer_set(&neg_timer, SEND_INTERVAL * 2);
  }

  PROCESS_END();

}
#endif