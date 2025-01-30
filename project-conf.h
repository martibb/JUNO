#ifndef PROJECT_CONF_H_
#define PROJECT_CONF_H_

// Abilitazione del supporto IPv6
#define UIP_CONF_IPV6 1

// Abilitazione 6LoWPAN
#define NETSTACK_CONF_NETWORK rpl_border_router_network
#define NETSTACK_CONF_RDC nullrdc_driver
#define NETSTACK_CONF_MAC nullmac_driver
#define NETSTACK_CONF_PHY cc2420_driver

// Impostazioni per la comunicazione MQTT
#define MQTT_MAX_PAYLOAD_LEN 100
#define MQTT_PUBLISH_INTERVAL 5

// Tempo di connessione MQTT
#define MQTT_KEEP_ALIVE_INTERVAL 60

// Abilitazione dei LED per il debug
#define LEDS_CONF_LED1 1
#define LEDS_CONF_LED2 1
#define LEDS_CONF_LED3 1
#define LEDS_CONF_LED4 1

// Altri parametri di configurazione
#define UIP_CONF_DS6_DEFAULT_PREFIX   0xaaaa
#define UIP_CONF_DS6_PREFIX_LEN       64

#endif /* PROJECT_CONF_H_ */
