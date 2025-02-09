#include "contiki.h"
#include "net/ipv6/uip.h"
#include "net/ipv6/uip-ds6.h"
#include "sys/etimer.h"
#include "sys/log.h"
#include "stdlib.h"
#include "stdbool.h"
#include "./gyroscope.h"
#include "os/dev/button-hal.h"
#include "os/dev/leds.h"
#include "math.h"

#define LOG_MODULE "Gyroscope"
#define LOG_LEVEL LOG_LEVEL_DBG

process_event_t GYROSCOPE_SLOPE_EVENT;
process_event_t GYROSCOPE_SUB_EVENT;
process_event_t GYROSCOPE_ALARM_EVENT;
process_event_t GYROSCOPE_STOP_EVENT;

#define GYRO_CRITICAL_ANGLE 45.0
#define GYROSCOPE_SAMPLING_INTERVAL 12   // Measurement interval (seconds)

int gyro_publishing_enabled = 1; // 1 = Publish, 0 = Pause publishing
int gyro_test_running = 0; // 0 = explore session, 1 = test session

typedef struct {
    float gyro_x;
    float gyro_y;
    float gyro_z;
} gyroscope_data_t;

float generate_random_gyro_value() {
    float random_val = (float)rand() / (float)RAND_MAX; // Genera un numero tra 0 e 1

    if (random_val < 0.4) {
        // Con il 40% di probabilità, genera un valore critico oltre 45°
        return (rand() % 90) + 45.0; // Angoli tra 45 e 135 gradi
    } else {
        // Con l'60% di probabilità, genera un valore normale tra -45° e 45°
        return ((float)rand() / (float)RAND_MAX) * 90.0 - 45.0;
    }
}

void set_all_green() {
    leds_on(LEDS_GREEN);
    leds_off(LEDS_YELLOW);
    leds_off(LEDS_RED);
}

void gyro_update_leds(float slope) {
    if (fabs(slope) > 25 && fabs(slope) <= 45) {
        leds_off(LEDS_GREEN);
        leds_on(LEDS_YELLOW);
        leds_off(LEDS_RED);
    } else if (fabs(slope) > 45) {
        leds_off(LEDS_GREEN);
        leds_off(LEDS_YELLOW);
        leds_on(LEDS_RED);
    } else { // Se torna alla normalità, accendi verde
        set_all_green();
    }
}

void gyro_reset_leds() {
    leds_off(LEDS_GREEN);
    leds_off(LEDS_YELLOW);
    leds_off(LEDS_RED);
}

PROCESS(gyroscope_sensor_process, "Gyroscope sensor process");

PROCESS_THREAD(gyroscope_sensor_process, ev, data) {
    static struct etimer et;
    static struct process *subscriber;
    static gyroscope_data_t gyroscope_data;

    PROCESS_BEGIN();

    subscriber = (struct process*)data;
    button_hal_init(); // Inizializza i pulsanti
    set_all_green();   // Inizializza LED verdi all'avvio

    LOG_INFO("Gyroscope sensor process started...\n");

    GYROSCOPE_SLOPE_EVENT = process_alloc_event();
    GYROSCOPE_SUB_EVENT = process_alloc_event();
    GYROSCOPE_ALARM_EVENT = process_alloc_event();
    GYROSCOPE_STOP_EVENT = process_alloc_event();

    while(true) {
        PROCESS_WAIT_EVENT_UNTIL(ev == GYROSCOPE_SUB_EVENT);
        etimer_set(&et, CLOCK_SECOND * GYROSCOPE_SAMPLING_INTERVAL);

        while (true) {
            PROCESS_YIELD();

            if (ev == button_hal_press_event && gyro_test_running == 1) {
                gyro_publishing_enabled = !gyro_publishing_enabled;
                if (gyro_publishing_enabled) {
                    LOG_INFO("Gyroscope resumed, publishing data...\n");
                    etimer_reset(&et);
                } else {
                    LOG_INFO("Simulating gyroscope out of order. Gyroscope paused, stopping data publishing.\n");
                }
            }

            if (ev == GYROSCOPE_STOP_EVENT) {
                if(gyro_test_running == 1) {
                    gyro_test_running = 0;
                    gyro_reset_leds();
                }
                break;
            }

            if (ev == PROCESS_EVENT_TIMER && etimer_expired(&et) && gyro_publishing_enabled) {

                gyroscope_data.gyro_x = generate_random_gyro_value();
                gyroscope_data.gyro_y = generate_random_gyro_value();
                gyroscope_data.gyro_z = generate_random_gyro_value();

                LOG_INFO("New Gyroscope data: X=%.2f, Y=%.2f, Z=%.2f\n",
                         gyroscope_data.gyro_x, gyroscope_data.gyro_y, gyroscope_data.gyro_z);

                if (fabs(gyroscope_data.gyro_x) > GYRO_CRITICAL_ANGLE || fabs(gyroscope_data.gyro_y) > GYRO_CRITICAL_ANGLE) {
                    LOG_WARN("WARNING: Critical inclination detected! Activating actuators...\n");

                    if (gyro_test_running == 1) {
                        if (fabs(gyroscope_data.gyro_x) > fabs(gyroscope_data.gyro_y)) {
                            gyro_update_leds(gyroscope_data.gyro_x);
                        } else {
                            gyro_update_leds(gyroscope_data.gyro_y);
                        }
                    }

                    // Prova qui:
                    process_post(subscriber, GYROSCOPE_SLOPE_EVENT, &gyroscope_data);

                }

                //process_post(subscriber, GYROSCOPE_SLOPE_EVENT, &gyroscope_data);
                etimer_reset(&et);
            } else if (gyro_test_running == 1) {
                set_all_green();
            }
        }
    }

    PROCESS_END();
}
