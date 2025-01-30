#include <stdbool.h>
#include <stdio.h>
#include "contiki.h"
#include "sys/etimer.h"
#include "sys/log.h"
#include "./lidar.h"
#include "../../utils/utils.h"

#define LOG_MODULE "LiDAR"
#define LOG_LEVEL LOG_LEVEL_APP

process_event_t LIDAR_SAMPLE_EVENT;
process_event_t LIDAR_SUB_EVENT;
process_event_t LIDAR_ALARM_EVENT;

#define LIDAR_LOWER_BOUND 10   // Minimum measurable distance (cm)
#define LIDAR_UPPER_BOUND 300  // Maximum measurable distance (cm)
#define LIDAR_OBSTACLE_THRESHOLD 50 // Obstacle threshold (cm)
#define LIDAR_SAMPLING_INTERVAL 2   // Measurement interval (seconds) // FixMe: Change to 5

PROCESS(lidar_sensor_process, "LiDAR sensor process");

PROCESS_THREAD(lidar_sensor_process, ev, data) {
    static struct etimer et;
    static struct process *subscriber;
    static int distance;

    PROCESS_BEGIN();

    subscriber = (struct process *)data;

    LOG_INFO("LiDAR sensor process started...\n");

    LIDAR_SAMPLE_EVENT = process_alloc_event();
    LIDAR_ALARM_EVENT = process_alloc_event();

    PROCESS_WAIT_EVENT_UNTIL(ev == LIDAR_SUB_EVENT);

    etimer_set(&et, CLOCK_SECOND * LIDAR_SAMPLING_INTERVAL);

    while (true) {
        PROCESS_YIELD();

        if (etimer_expired(&et)) {

            distance = (int)generate_random_value(LIDAR_LOWER_BOUND, LIDAR_UPPER_BOUND);
            process_post(subscriber, LIDAR_SAMPLE_EVENT, &distance);
            etimer_reset(&et);
        }
    }

    PROCESS_END();
}
