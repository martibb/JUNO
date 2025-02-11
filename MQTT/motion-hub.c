#include "contiki.h"
#include "net/routing/routing.h"
#include "mqtt.h"
#include "net/ipv6/uip.h"
#include "sys/etimer.h"
#include "sys/log.h"
#include <string.h>

#include "../lidar/lidar.h"
#include "../gyroscope/gyroscope.h"
#include <locale.h>

#define LOG_MODULE "motion-hub"
#define LOG_LEVEL LOG_LEVEL_DBG

#define MQTT_CLIENT_BROKER_IP_ADDR "fd00::1"
#define DEFAULT_BROKER_PORT 1883
#define DEFAULT_PUBLISH_INTERVAL (30 * CLOCK_SECOND)

#define MAX_TCP_SEGMENT_SIZE 32
#define BUFFER_SIZE 64
#define APP_BUFFER_SIZE 256
#define QUEUE_SIZE 10

static char client_id[BUFFER_SIZE];
static char lidar_topic[BUFFER_SIZE] = "lidar";
static char control_topic[BUFFER_SIZE] = "sensor/control";
static char lidar_buffer[APP_BUFFER_SIZE];
static char gyroscope_topic[BUFFER_SIZE] = "gyroscope";
static char gyroscope_buffer[APP_BUFFER_SIZE];

static struct mqtt_connection conn;
static struct etimer periodic_timer;
static struct etimer publish_timer;

static uint8_t state;
#define STATE_INIT 0
#define STATE_NET_OK 1
#define STATE_CONNECTING 2
#define STATE_CONNECTED 3
#define STATE_SUBSCRIBED 4
#define STATE_DISCONNECTED 5

typedef struct {
    int distance_front;
    int distance_right;
    int distance_left;
} lidar_data_t;

typedef struct {
    float gyro_x;
    float gyro_y;
    float gyro_z;
} gyroscope_data_t;

// Coda di eventi
typedef struct {
    char topic[BUFFER_SIZE];
    char message[APP_BUFFER_SIZE];
} mqtt_queue_item_t;

static mqtt_queue_item_t mqtt_queue[QUEUE_SIZE];
static uint8_t queue_head = 0;
static uint8_t queue_tail = 0;

PROCESS(mqtt_client_process, "MQTT Client");
AUTOSTART_PROCESSES(&mqtt_client_process);

static void pub_handler(const char *topic, uint16_t topic_len, const uint8_t *chunk, uint16_t chunk_len) {
    LOG_INFO("Received message on topic: %s\n", topic);
    LOG_INFO("Payload: %.*s\n", chunk_len, (char *)chunk);

    if (strncmp(topic, control_topic, topic_len) == 0) {
        if (strncmp((char *)chunk, "start", chunk_len) == 0) {
            test_running = 0;
            gyro_test_running = 0;
            process_post(&lidar_sensor_process, LIDAR_SUB_EVENT, &mqtt_client_process);
            process_post(&gyroscope_sensor_process, GYROSCOPE_SUB_EVENT, &mqtt_client_process);
            LOG_INFO("Start sensing command received...\n");
        } else if (strncmp((char*)chunk, "stop", chunk_len) == 0) {
            process_post(&lidar_sensor_process, LIDAR_STOP_EVENT, NULL);
            process_post(&gyroscope_sensor_process, GYROSCOPE_STOP_EVENT, NULL);
            LOG_INFO("Stop sensing command received...\n");
        } else if (strncmp((char*)chunk, "test-session", chunk_len) == 0) {
            test_running = 1;
            gyro_test_running = 1;
            process_post(&lidar_sensor_process, LIDAR_SUB_EVENT, &mqtt_client_process);
            process_post(&gyroscope_sensor_process, GYROSCOPE_SUB_EVENT, &mqtt_client_process);
            LOG_INFO("Test session started.\n");
        }
    }
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

static void evaluate_publishing(int status) {
    switch(status) {
        case MQTT_STATUS_OK:
            break;
        case MQTT_STATUS_NOT_CONNECTED_ERROR:
            LOG_ERR("Publishing failed. Error: MQTT_STATUS_NOT_CONNECTED_ERROR.\n");
            state = STATE_DISCONNECTED;
            break;
        case MQTT_STATUS_OUT_QUEUE_FULL:
            LOG_ERR("Publishing failed. Error: MQTT_STATUS_OUT_QUEUE_FULL.\n");
            mqtt_disconnect(&conn);
            state = STATE_DISCONNECTED;
            break;
        default:
            LOG_ERR("Publishing failed. Error: unknown.\n");
    }
}

static void enqueue_publish(const char *topic, const char *message) {
    if ((queue_tail + 1) % QUEUE_SIZE != queue_head) {
        strncpy(mqtt_queue[queue_tail].topic, topic, BUFFER_SIZE);
        strncpy(mqtt_queue[queue_tail].message, message, APP_BUFFER_SIZE);
        queue_tail = (queue_tail + 1) % QUEUE_SIZE;
    } else {
        LOG_WARN("Queue is full. Dropping message.\n");
    }
}

static void process_queue(void) {
    if (queue_head != queue_tail) {
        mqtt_queue_item_t *item = &mqtt_queue[queue_head];
        int status = mqtt_publish(&conn, NULL, item->topic, (uint8_t *)item->message, strlen(item->message), MQTT_QOS_LEVEL_0, MQTT_RETAIN_OFF);
        evaluate_publishing(status);
        queue_head = (queue_head + 1) % QUEUE_SIZE;
    }
}

PROCESS_THREAD(mqtt_client_process, ev, data) {

    setlocale(LC_NUMERIC, "C");

    PROCESS_BEGIN();

    LOG_INFO("MQTT Sensors Process Started\n");

    snprintf(client_id, BUFFER_SIZE, "%02x%02x%02x%02x%02x%02x",
             linkaddr_node_addr.u8[0], linkaddr_node_addr.u8[1],
             linkaddr_node_addr.u8[2], linkaddr_node_addr.u8[5],
             linkaddr_node_addr.u8[6], linkaddr_node_addr.u8[7]);

    mqtt_register(&conn, &mqtt_client_process, client_id, mqtt_event, MAX_TCP_SEGMENT_SIZE);

    state = STATE_INIT;

    process_start(&lidar_sensor_process, NULL);
    process_start(&gyroscope_sensor_process, NULL);
    etimer_set(&periodic_timer, CLOCK_SECOND * 5);
    etimer_set(&publish_timer, CLOCK_SECOND * 2);

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
                mqtt_subscribe(&conn, NULL, control_topic, MQTT_QOS_LEVEL_0);
                strcpy(lidar_topic, "lidar");
                strcpy(gyroscope_topic, "gyroscope");
                state = STATE_SUBSCRIBED;
            }
            if(state == STATE_DISCONNECTED){
                LOG_ERR("Disconnected from the MQTT broker...\nComing back to the initial state..\n");
                state = STATE_INIT;
            }
            etimer_set(&periodic_timer, CLOCK_SECOND * 5);
        }

        if (ev == LIDAR_DISTANCE_EVENT && state == STATE_SUBSCRIBED) {
            LOG_INFO("New distance event\n");
            lidar_data_t *lidar_data = (lidar_data_t *)data;
            sprintf(lidar_buffer, "{\"distance_front\": %d, \"distance_right\": %d, \"distance_left\": %d}",
                    lidar_data->distance_front, lidar_data->distance_right, lidar_data->distance_left);
            enqueue_publish(lidar_topic, lidar_buffer);
        }

        if (ev == GYROSCOPE_SLOPE_EVENT && state == STATE_SUBSCRIBED) {
            LOG_INFO("New slope event\n");
            gyroscope_data_t *gyroscope_data = (gyroscope_data_t *)data;
            snprintf(gyroscope_buffer, APP_BUFFER_SIZE, "{\"gyro_x\": %.2f, \"gyro_y\": %.2f, \"gyro_z\": %.2f}",
                        gyroscope_data->gyro_x, gyroscope_data->gyro_y, gyroscope_data->gyro_z);
            enqueue_publish(gyroscope_topic, gyroscope_buffer);
        }

        if (ev == PROCESS_EVENT_TIMER && data == &publish_timer) {
            process_queue();
            etimer_reset(&publish_timer);
        }
    }

    PROCESS_END();
}
