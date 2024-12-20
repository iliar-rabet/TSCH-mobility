
#ifndef PROJECT_CONF_H_
#define PROJECT_CONF_H_

/* Set to enable TSCH security */
#ifndef WITH_SECURITY
#define WITH_SECURITY 0
#endif /* WITH_SECURITY */

#define ENERGEST_CONF_ON 1

/* USB serial takes space, free more space elsewhere */
#define SICSLOWPAN_CONF_FRAG 0
//#define UIP_CONF_BUFFER_SIZE 160

/*******************************************************/
/******************* Configure TSCH ********************/
/*******************************************************/

/* Disable the 6TiSCH minimal schedule */
#define TSCH_SCHEDULE_CONF_WITH_6TISCH_MINIMAL 0

#define ORCHESTRA_CONF_COMMON_SHARED_PERIOD 627
#define ORCHESTRA_CONF_UNICAST_PERIOD 17 
#define ORCHESTRA_CONF_EBSF_PERIOD 397

#define RPL_CONF_WITH_PROBING 1
#define RPL_CONF_WITH_DAO_ACK 0


#define START_AFTER 600
#define NUM_NODES 8

#define Q_STABLE 0
#define BC_STABLE 0

#define M_SECONDS 10
#define S_SECONDS 20

#define M_SEND_INTERVAL (M_SECONDS*CLOCK_SECOND)
#define SEND_INTERVAL (S_SECONDS*CLOCK_SECOND)

#if WITH_SECURITY

/* Enable security */
#define LLSEC802154_CONF_ENABLED 1

#endif /* WITH_SECURITY */

/*******************************************************/
/************* Other system configuration **************/
/*******************************************************/

/* Logging */
#define LOG_CONF_LEVEL_RPL                         LOG_LEVEL_WARN
#define LOG_CONF_LEVEL_TCPIP                       LOG_LEVEL_WARN
#define LOG_CONF_LEVEL_IPV6                        LOG_LEVEL_WARN
#define LOG_CONF_LEVEL_6LOWPAN                     LOG_LEVEL_WARN
#define LOG_CONF_LEVEL_MAC                         LOG_LEVEL_INFO
#define LOG_CONF_LEVEL_FRAMER                      LOG_LEVEL_NONE
#define TSCH_LOG_CONF_PER_SLOT                     1

#endif /* PROJECT_CONF_H_ */

