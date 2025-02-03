#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include "net/ipv6/uip.h"
#include "net/ipv6/uip-ds6.h"
#include "contiki.h"
#include "sys/etimer.h"
#include "sys/log.h"
#include "./lidar.h"

#define LOG_MODULE "LiDAR"
#define LOG_LEVEL LOG_LEVEL_DBG

process_event_t LIDAR_DISTANCE_EVENT;
process_event_t LIDAR_SUB_EVENT;
process_event_t LIDAR_ALARM_EVENT;
process_event_t LIDAR_STOP_EVENT;

#define LIDAR_LOWER_BOUND 10   // Minimum measurable distance (cm)
#define LIDAR_UPPER_BOUND 300  // Maximum measurable distance (cm)
#define LIDAR_OBSTACLE_THRESHOLD 50 // Obstacle threshold (cm)
#define LIDAR_SAMPLING_INTERVAL 5   // Measurement interval (seconds)

int publishing_enabled = 1; // 1 = Publish, 0 = Pause publication

float generate_random_value(float min, float max) {
    return ((float)rand() / (float)RAND_MAX) * (max - min) + min;
}

PROCESS(lidar_sensor_process, "LiDAR sensor process");

PROCESS_THREAD(lidar_sensor_process, ev, data) {
    static struct etimer et;
    static struct process *subscriber;
    static int distance;

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

            if (ev == LIDAR_ALARM_EVENT) {

                publishing_enabled = !publishing_enabled;
                if (publishing_enabled) {
                    LOG_INFO("LiDAR resumed, publishing data...\n");
                    etimer_reset(&et);
                } else {
                    LOG_INFO("LiDAR paused, stopping data publishing.\n");
                }
            }

            if(ev == LIDAR_STOP_EVENT) {
                break;
            }

            if (ev == PROCESS_EVENT_TIMER && etimer_expired(&et) && publishing_enabled) {
                distance = (int)generate_random_value(LIDAR_LOWER_BOUND, LIDAR_UPPER_BOUND);
                LOG_INFO("New LiDAR distance: %d cm\n", distance);
                process_post(subscriber, LIDAR_DISTANCE_EVENT, &distance);
                etimer_reset(&et);
            }
        }

    }

    PROCESS_END();
}