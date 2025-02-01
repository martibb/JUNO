#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "contiki.h"
#include "coap-engine.h"

/* Log configuration */
#include "sys/log.h"
#define LOG_MODULE "legs servo motors"
#define LOG_LEVEL LOG_LEVEL_APP

#define SERVER "coap://[fd00::1]:5683"

extern coap_resource_t  movement;

PROCESS(legs_servo_motors, "Robotic legs Servo Motors Control");
AUTOSTART_PROCESSES(&legs_servo_motors);

PROCESS_THREAD(legs_servo_motors, ev, data)
{
  PROCESS_BEGIN();

  PROCESS_PAUSE();

  LOG_INFO("Starting Robotic Legs Servo Motors Controller\n");

  coap_activate_resource(&movement, "movement");

  PROCESS_END();
}