#include <string.h>
#include <strings.h>

#include "contiki.h"
#include "net/routing/routing.h"
#include "mqtt.h"
#include "net/ipv6/uip.h"
#include "net/ipv6/uip-icmp6.h"
#include "net/ipv6/sicslowpan.h"
#include "sys/etimer.h"
#include "sys/ctimer.h"
#include "lib/sensors.h"
#include "dev/button-hal.h"
#include "dev/leds.h"
#include "os/sys/log.h"

#include "mqtt-sensor.h"
#include "../sensing/lidar/lidar.h"  // Inclusione del sensore LiDAR
#include "../utils/utils.h"

#define LOG_MODULE "mqtt-sensors"
#ifdef MQTT_SENSOR_CONF_LOG_LEVEL
#define LOG_LEVEL MQTT_SENSOR_CONF_LOG_LEVEL
#else
#define LOG_LEVEL LOG_LEVEL_DBG
#endif

#define MQTT_LIDAR "1"
#define MQTT_LIDAR_ERROR "-1"

static bool lidar_error;

// Assumption: broker does not require authentication
#define MQTT_CLIENT_BROKER_IP_ADDR "fd00::1" // MQTT broker address
static const char *broker_ip = MQTT_CLIENT_BROKER_IP_ADDR;

#define DEFAULT_BROKER_PORT         1883
#define DEFAULT_PUBLISH_INTERVAL    (30 * CLOCK_SECOND)

#define MAX_TCP_SEGMENT_SIZE    32
#define CONFIG_IP_ADDR_STR_LEN   64

static struct mqtt_message *msg_ptr = 0;
static struct mqtt_connection conn;

#define BUFFER_SIZE 64
static char client_id[BUFFER_SIZE];
static char lidar_topic[BUFFER_SIZE];
static char sub_topic[BUFFER_SIZE];

#define APP_BUFFER_SIZE 128
static char lidar_buffer[APP_BUFFER_SIZE];

#define STATE_MACHINE_PERIODIC (CLOCK_SECOND >> 1)
static struct etimer periodic_timer;

#define STATE_INIT    	    0
#define STATE_NET_OK        1
#define STATE_CONNECTING    2
#define STATE_CONNECTED     3
#define STATE_SUBSCRIBED    4
#define STATE_DISCONNECTED  5
static uint8_t state;

static int node_id;

PROCESS_NAME(mqtt_sensor_process);
AUTOSTART_PROCESSES(&mqtt_sensor_process);
PROCESS(mqtt_sensor_process, "MQTT Sensor");

static void publish(char* topic, char* buffer){
	int status = mqtt_publish(&conn, NULL, topic, (uint8_t *)buffer, strlen(buffer), MQTT_QOS_LEVEL_0, MQTT_RETAIN_OFF);
	switch(status) {
	    case MQTT_STATUS_OK:
	        return;
	    case MQTT_STATUS_NOT_CONNECTED_ERROR: {
	        LOG_ERR("Publishing failed. Error: MQTT_STATUS_NOT_CONNECTED_ERROR.\n");
	        state = STATE_DISCONNECTED;
	        return;
	    }
	    case MQTT_STATUS_OUT_QUEUE_FULL: {
	        mqtt_disconnect(&conn);
    		state = STATE_DISCONNECTED;
	        return;
	    }
	    default:
	        LOG_ERR("Publishing failed. Error: unknown.\n");
	        return;
	}
}

static void mqtt_event(struct mqtt_connection *m, mqtt_event_t event, void *data){
 	switch(event) {
  		case MQTT_EVENT_CONNECTED: {
    		LOG_INFO("Connection completed..\n");
    		state = STATE_CONNECTED;
			leds_off(LEDS_ALL);
    		break;
  		}
  		case MQTT_EVENT_DISCONNECTED: {
    		LOG_ERR("MQTT Disconnect. Reason %u\n", *((mqtt_event_t *)data));
    		state = STATE_DISCONNECTED;
    		process_poll(&mqtt_sensor_process);
    		break;
  		}
  		case MQTT_EVENT_PUBLISH: {
    		msg_ptr = data;
    		process_post(&mqtt_sensor_process, LIDAR_OBSTACLE_EVENT, &distance);
    		break;
  		}
 		case MQTT_EVENT_SUBACK: {
    		LOG_INFO("Application is subscribed to topic successfully\n");
    		break;
  		}
  		case MQTT_EVENT_UNSUBACK: {
    		LOG_INFO("Application is unsubscribed to topic successfully\n");
    		break;
  		}
  		case MQTT_EVENT_PUBACK: {
    		LOG_INFO("Publishing complete.\n");
    		break;
  		}
  		default:
    		LOG_WARN("Unhandled MQTT event: %i\n", event);
    		break;
    }
}

static bool have_connectivity(void){
  	if(uip_ds6_get_global(ADDR_PREFERRED) == NULL || uip_ds6_defrt_choose() == NULL) {
    		return false;
  	}
 	return true;
}

PROCESS_THREAD(mqtt_sensor_process, ev, data){
	PROCESS_BEGIN();
	mqtt_status_t status;
	char broker_address[CONFIG_IP_ADDR_STR_LEN];
  	node_id = linkaddr_node_addr.u8[7];
	LOG_INFO("MQTT Sensor Process\nID: %d", node_id);

  	snprintf(client_id, BUFFER_SIZE, "%02x%02x%02x%02x%02x%02x",
                     linkaddr_node_addr.u8[0], linkaddr_node_addr.u8[1],
                     linkaddr_node_addr.u8[2], linkaddr_node_addr.u8[5],
                     linkaddr_node_addr.u8[6], linkaddr_node_addr.u8[7]);

  	mqtt_register(&conn, &mqtt_sensor_process, client_id, mqtt_event, MAX_TCP_SEGMENT_SIZE);

  	state = STATE_INIT;

  	// Avvia il processo del LiDAR e invia evento di sottoscrizione
  	process_start(&lidar_sensor_process, NULL);
  	process_post(&lidar_sensor_process, LIDAR_SUB_EVENT, &mqtt_sensor_process);

	leds_on(LEDS_ALL);
  	etimer_set(&periodic_timer, STATE_MACHINE_PERIODIC);

  	while(1) {
		PROCESS_YIELD();
		if((ev == PROCESS_EVENT_TIMER && data == &periodic_timer) || ev == PROCESS_EVENT_POLL){
		  	if(state == STATE_INIT && have_connectivity()){
				 	state = STATE_NET_OK;
					LOG_INFO("Connectivity found, going towards STATE_NET_OK\n");
		  	}
			if(state == STATE_NET_OK) {
				LOG_INFO("Connecting to the MQTT server...\n");
				memcpy(broker_address, broker_ip, strlen(broker_ip));
				mqtt_connect(&conn, broker_address, DEFAULT_BROKER_PORT, (DEFAULT_PUBLISH_INTERVAL * 3) / CLOCK_SECOND, MQTT_CLEAN_SESSION_ON);
				state = STATE_CONNECTING;
			}
			if(state == STATE_CONNECTED) {
				sprintf(sub_topic, "alarm/%d", node_id);
				status = mqtt_subscribe(&conn, NULL, sub_topic, MQTT_QOS_LEVEL_0);
				LOG_INFO("Subscribing to alarm/%d...\n", node_id);
				strcpy(lidar_topic, "lidar");
				state = STATE_SUBSCRIBED;
		 	}
		 	if(state == STATE_DISCONNECTED){
		  	 	LOG_ERR("Disconnected from MQTT broker...\n Restarting...\n");
		 	 	state = STATE_INIT;
		 	}
			etimer_set(&periodic_timer, STATE_MACHINE_PERIODIC);
    	} else if (state == STATE_SUBSCRIBED && ev == LIDAR_DISTANCE_EVENT) {
            int distance = *((int *)data); // Estrai la distanza

                LOG_INFO("New LiDAR sample: %d cm\n", distance);

                // Formatta il messaggio JSON
                buffer_json_message(lidar_buffer, APP_BUFFER_SIZE, node_id, "lidar", distance, "cm");
                publish(lidar_topic, lidar_buffer);

                // Controllo ostacolo
                if (distance <= LIDAR_OBSTACLE_THRESHOLD) {
                    LOG_WARN("Obstacle detected at %d cm!\n", distance);
                    process_post(&mqtt_sensor_process, LIDAR_OBSTACLE_EVENT, &distance);
                }
    	}
	}
	PROCESS_END();
}