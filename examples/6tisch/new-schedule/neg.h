uip_ipaddr_t * overflow_ip;

extern struct tsch_slotframe *sf_unicast;
extern struct tsch_slotframe *sf_common;

struct neg_pack
{
  uint16_t slotframe;
  bool isRX;
  bool isAdd;
  bool is_broadcast;
};
