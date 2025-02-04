#include "contiki.h"
#include "net/ipv6/uip.h"
#include "net/ipv6/uip-ds6.h"
#include "sys/etimer.h"
#include "sys/log.h"
#include "stdlib.h"
#include "stdbool.h"
#include "./lidar.h"
#include "os/dev/button-hal.h"
#include "os/dev/leds.h"

#define LOG_MODULE "LiDAR"
#define LOG_LEVEL LOG_LEVEL_DBG

process_event_t LIDAR_DISTANCE_EVENT;
process_event_t LIDAR_SUB_EVENT;
process_event_t LIDAR_ALARM_EVENT;
process_event_t LIDAR_STOP_EVENT;

#define LIDAR_MAX_DISTANCE 100  // Maximum distance (meters)
#define LIDAR_OBSTACLE_PROBABILITY 0.2 // Obstacle probability
#define LIDAR_SAMPLING_INTERVAL 5   // Measurement interval (seconds)

int publishing_enabled = 1; // 1 = Publish, 0 = Pause publishing
int test_running = 0; // 0 = explore session, 1 = test session

typedef struct {
    int distance_front;
    int distance_right;
    int distance_left;
} lidar_data_t;

int generate_lidar_value() {
    return ((float)rand() / (float)RAND_MAX) < LIDAR_OBSTACLE_PROBABILITY ? (rand() % LIDAR_MAX_DISTANCE) : LIDAR_MAX_DISTANCE;
}

void update_leds(int distance) {
    if (distance== 100) {
        leds_on(LEDS_GREEN);
        leds_off(LEDS_YELLOW);
        leds_off(LEDS_RED);
    } else if (distance < 100 && distance > 20) {
        leds_off(LEDS_GREEN);
        leds_on(LEDS_YELLOW);
        leds_off(LEDS_RED);
    } else if (distance <= 20) {
        leds_off(LEDS_GREEN);
        leds_off(LEDS_YELLOW);
        leds_on(LEDS_RED);
    }
}

void reset_leds() {
    leds_off(LEDS_GREEN);
    leds_off(LEDS_YELLOW);
    leds_off(LEDS_RED);
}

PROCESS(lidar_sensor_process, "LiDAR sensor process");

PROCESS_THREAD(lidar_sensor_process, ev, data) {
    static struct etimer et;
    static struct process *subscriber;
    static lidar_data_t lidar_data;

    PROCESS_BEGIN();

    subscriber = (struct process*)data;

    LOG_INFO("LiDAR sensor process started...\n");

    LIDAR_DISTANCE_EVENT = process_alloc_event();
    LIDAR_SUB_EVENT = process_alloc_event();
    LIDAR_ALARM_EVENT = process_alloc_event();
    LIDAR_STOP_EVENT = process_alloc_event();

    while(true) {
        PROCESS_WAIT_EVENT_UNTIL(ev == LIDAR_SUB_EVENT);
        etimer_set(&et, CLOCK_SECOND * LIDAR_SAMPLING_INTERVAL);

        while (true) {
            PROCESS_YIELD();

            if (ev == button_hal_press_event && test_running == 1) {
                publishing_enabled = !publishing_enabled;
                if (publishing_enabled) {
                    LOG_INFO("LiDAR resumed, publishing data...\n");
                    etimer_reset(&et);
                } else {
                    LOG_INFO("Simulating LiDAR out of order. LiDAR paused, stopping data publishing.\n");
                }
            }

            if(ev == LIDAR_STOP_EVENT) {

                if(test_running==1) {
                    test_running = 0;
                    reset_leds();
                }

                break;
            }

            if (ev == PROCESS_EVENT_TIMER && etimer_expired(&et) && publishing_enabled) {
                lidar_data.distance_front = generate_lidar_value();

                if(test_running==1) {
                    update_leds(lidar_data.distance_front);
                }

                lidar_data.distance_right = generate_lidar_value();
                lidar_data.distance_left = generate_lidar_value();

                LOG_INFO("New LiDAR distances - Front: %d m, Right: %d m, Left: %d m\n",
                         lidar_data.distance_front, lidar_data.distance_right, lidar_data.distance_left);

                process_post(subscriber, LIDAR_DISTANCE_EVENT, &lidar_data);
                etimer_reset(&et);
            }
        }
    }

    PROCESS_END();
}