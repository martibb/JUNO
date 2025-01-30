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

#define LIDAR_LOWER_BOUND 10   // Minima distanza misurabile (cm)
#define LIDAR_UPPER_BOUND 300  // Massima distanza misurabile (cm)
#define LIDAR_OBSTACLE_THRESHOLD 50 // Soglia per ostacolo (cm)
#define LIDAR_SAMPLING_INTERVAL 5   // Tempo tra le misurazioni (secondi)
#define LIDAR_MAX_VARIATION 10      // Variazione casuale tra misurazioni

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
    distance = (int)rand_sample_range(LIDAR_LOWER_BOUND, LIDAR_UPPER_BOUND);

    while (true) {
        PROCESS_YIELD();

        if (etimer_expired(&et)) {
            // Genera una nuova lettura casuale con variazione limitata
            distance = (int)rand_sample_variation_range(distance,
                         LIDAR_MAX_VARIATION, LIDAR_LOWER_BOUND, LIDAR_UPPER_BOUND);

            // Controlla se c'è un ostacolo
            if (distance <= LIDAR_OBSTACLE_THRESHOLD) {
                LOG_WARN("Ostacolo rilevato a %d cm!\n", distance);
                process_post(subscriber, LIDAR_ALARM_EVENT, &distance);
            } else {
                LOG_INFO("Nessun ostacolo, distanza: %d cm\n", distance);
            }

            // Invia il valore del sensore
            process_post(subscriber, LIDAR_SAMPLE_EVENT, &distance);
            etimer_reset(&et);
        }
    }

    PROCESS_END();
}
