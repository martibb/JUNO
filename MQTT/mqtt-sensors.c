#include "contiki.h"
#include "net/routing/routing.h"
#include "mqtt.h"
#include "net/ipv6/uip.h"
#include "sys/etimer.h"
#include "sys/log.h"
#include <string.h>

#include "../sensing/lidar.h"

#define LOG_MODULE "mqtt-sensors"
#define LOG_LEVEL LOG_LEVEL_DBG

#define MQTT_CLIENT_BROKER_IP_ADDR "fd00::1"
#define DEFAULT_BROKER_PORT 1883
#define DEFAULT_PUBLISH_INTERVAL (30 * CLOCK_SECOND)

#define MAX_TCP_SEGMENT_SIZE 32
#define BUFFER_SIZE 64
#define APP_BUFFER_SIZE 128

static char client_id[BUFFER_SIZE];
static char lidar_topic[BUFFER_SIZE] = "lidar";
static char lidar_buffer[APP_BUFFER_SIZE];

static struct mqtt_connection conn;
static struct etimer periodic_timer;

static uint8_t state;
#define STATE_INIT 0
#define STATE_NET_OK 1
#define STATE_CONNECTING 2
#define STATE_CONNECTED 3
#define STATE_SUBSCRIBED 4
#define STATE_DISCONNECTED 5

PROCESS(mqtt_client_process, "MQTT Client");
AUTOSTART_PROCESSES(&mqtt_client_process);

static void pub_handler(const char *topic, uint16_t topic_len, const uint8_t *chunk, uint16_t chunk_len) {
    LOG_INFO("Received message on topic: %s\n", topic);
    LOG_INFO("Payload: %.*s\n", chunk_len, (char *)chunk);
}

static void mqtt_event(struct mqtt_connection *m, mqtt_event_t event, void *data) {
    switch (event) {
        case MQTT_EVENT_CONNECTED:
            LOG_INFO("MQTT Connected\n");
            state = STATE_CONNECTED;
            break;
        case MQTT_EVENT_DISCONNECTED:
            LOG_ERR("MQTT Disconnected. Reason %u\n", *((mqtt_event_t *)data));
            state = STATE_DISCONNECTED;
            process_poll(&mqtt_client_process);
            break;
        case MQTT_EVENT_PUBLISH:
            pub_handler(((struct mqtt_message *)data)->topic, strlen(((struct mqtt_message *)data)->topic),
                        ((struct mqtt_message *)data)->payload_chunk, ((struct mqtt_message *)data)->payload_length);
            break;
        case MQTT_EVENT_SUBACK:
            LOG_INFO("Subscription successful\n");
            break;
        case MQTT_EVENT_PUBACK:
            LOG_INFO("Publishing complete.\n");
            break;
        default:
            LOG_WARN("Unhandled MQTT event: %d\n", event);
            break;
    }
}

static bool have_connectivity(void) {
    return (uip_ds6_get_global(ADDR_PREFERRED) != NULL) && (uip_ds6_defrt_choose() != NULL);
}

PROCESS_THREAD(mqtt_client_process, ev, data) {
    PROCESS_BEGIN();

    LOG_INFO("MQTT Sensors Process Started\n");

    snprintf(client_id, BUFFER_SIZE, "%02x%02x%02x%02x%02x%02x",
             linkaddr_node_addr.u8[0], linkaddr_node_addr.u8[1],
             linkaddr_node_addr.u8[2], linkaddr_node_addr.u8[5],
             linkaddr_node_addr.u8[6], linkaddr_node_addr.u8[7]);

    mqtt_register(&conn, &mqtt_client_process, client_id, mqtt_event, MAX_TCP_SEGMENT_SIZE);

    state = STATE_INIT;

    process_start(&lidar_sensor_process, NULL);
    process_post(&lidar_sensor_process, LIDAR_SUB_EVENT, &mqtt_client_process);

    etimer_set(&periodic_timer, CLOCK_SECOND * 5); // ogni 5 secondi

    while (1) {
        PROCESS_YIELD();

        if ((ev == PROCESS_EVENT_TIMER && data == &periodic_timer) || ev == PROCESS_EVENT_POLL) {
            if (state == STATE_INIT && have_connectivity()) {
                state = STATE_NET_OK;
            }

            if (state == STATE_NET_OK) {
                LOG_INFO("Connecting to MQTT broker\n");
                mqtt_connect(&conn, MQTT_CLIENT_BROKER_IP_ADDR, DEFAULT_BROKER_PORT,
                             (DEFAULT_PUBLISH_INTERVAL * 3) / CLOCK_SECOND, MQTT_CLEAN_SESSION_ON);
                state = STATE_CONNECTING;
            }

            if (state == STATE_CONNECTED) {
                strcpy(lidar_topic, "lidar");
                state = STATE_SUBSCRIBED;
            }
            if(state == STATE_DISCONNECTED){
		  	 	LOG_ERR("Disconnected from the MQTT broker...\nComing back to the initial state..\n");
		 	 	state = STATE_INIT;
		 	}
			etimer_set(&periodic_timer, CLOCK_SECOND * 5);
        }
        else if(state == STATE_SUBSCRIBED && ev == LIDAR_DISTANCE_EVENT) {
            LOG_INFO("New distance event\n");
            int distance = *((int *)data);
            snprintf(lidar_buffer, sizeof(lidar_buffer), "Published new distance value: %f", (float)distance); // Consegna questi messaggi al broker MQTT una volta che questo si è iscritto, ogni 5 sec
            mqtt_publish(&conn, NULL, lidar_topic, (uint8_t *)lidar_buffer, strlen(lidar_buffer), MQTT_QOS_LEVEL_0, MQTT_RETAIN_OFF);
        }
    }

    PROCESS_END();
}
