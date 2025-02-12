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

#define GYRO_CRITICAL_ANGLE 45
#define GYROSCOPE_SAMPLING_INTERVAL 10   // Measurement interval (seconds)

int gyro_publishing_enabled = 1; // 1 = Publish, 0 = Pause publishing
int gyro_test_running = 0; // 0 = explore session, 1 = test session

typedef struct {
    int gyro_x;
    int gyro_y;
    int gyro_z;
} gyroscope_data_t;

int generate_random_gyro_value() {
    int random_val = rand() % 100;

    if (random_val < 40) {
        return (rand() % 91) + 45;
    } else {
        return (rand() % 91) - 45;
    }

    return 40;
}

void gyro_update_leds(int slope) {
    if (abs(slope) > 25 && abs(slope) <= 70) {
        LOG_INFO("Set led as green.\n");
        leds_off(LEDS_BLUE);
        leds_on(LEDS_GREEN);
        leds_off(LEDS_RED);
    } else if (abs(slope) > 70) {
        LOG_INFO("Set led as red.\n");
        leds_off(LEDS_BLUE);
        leds_off(LEDS_GREEN);
        leds_on(LEDS_RED);
    } else {
        LOG_INFO("Set led as blue.\n");
        leds_on(LEDS_BLUE);
        leds_off(LEDS_GREEN);
        leds_off(LEDS_RED);
    }
}

void gyro_reset_leds() {
    LOG_INFO("Reset all leds to off.\n");
    leds_off(LEDS_BLUE);
    leds_off(LEDS_GREEN);
    leds_off(LEDS_RED);
}

PROCESS(gyroscope_sensor_process, "Gyroscope sensor process");

PROCESS_THREAD(gyroscope_sensor_process, ev, data) {
    static struct etimer et;
    static struct process *subscriber;
    static gyroscope_data_t gyroscope_data;
    static clock_time_t press_time;

    PROCESS_BEGIN();

    srand(clock_time());

    subscriber = (struct process*)data;

    button_hal_init(); // Inizializza i pulsanti

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

            if (gyro_test_running && ev == button_hal_press_event) {
                press_time = clock_time();
            }
            else if (gyro_test_running && ev == button_hal_release_event) {
                clock_time_t release_time = clock_time();
                clock_time_t duration = release_time - press_time;

                if (duration >= CLOCK_SECOND * 2) {
                    if (gyro_publishing_enabled) {
                        gyro_publishing_enabled = 0;
                        LOG_INFO("Gyroscope paused, stopping data publishing.\n");

                        do {
                            PROCESS_WAIT_EVENT_UNTIL(ev == button_hal_press_event);
                            press_time = clock_time();
                            PROCESS_WAIT_EVENT_UNTIL(ev == button_hal_release_event);
                            release_time = clock_time();
                            duration = release_time - press_time;
                        } while (duration < CLOCK_SECOND * 2);

                        gyro_publishing_enabled = 1;
                        LOG_INFO("Gyroscope resumed, publishing data...\n");
                        etimer_reset(&et);
                    }
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

                LOG_INFO("New Gyroscope data: X=%d, Y=%d, Z=%d\n",
                         gyroscope_data.gyro_x, gyroscope_data.gyro_y, gyroscope_data.gyro_z);

                if(gyro_test_running == 1) {
                    int max_sensed;
                    if(gyroscope_data.gyro_x > gyroscope_data.gyro_y) {
                        max_sensed = gyroscope_data.gyro_x;
                    }
                    else {
                        max_sensed = gyroscope_data.gyro_y;
                    }
                    gyro_update_leds(max_sensed);
                }

                process_post(subscriber, GYROSCOPE_SLOPE_EVENT, &gyroscope_data);
                etimer_reset(&et);

            }
        }
    }

    PROCESS_END();
}