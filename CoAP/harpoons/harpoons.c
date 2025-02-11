#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "contiki.h"
#include "coap-engine.h"

#include "sys/log.h"
#define LOG_MODULE "harpoons"
#define LOG_LEVEL LOG_LEVEL_DBG

#define SERVER "coap://[fd00::1]:5683"

PROCESS(harpoons, "Harpoons Anchoring Control");
AUTOSTART_PROCESSES(&harpoons);

extern coap_resource_t anchoring;

PROCESS_THREAD(harpoons, ev, data)
{
  PROCESS_BEGIN();

  PROCESS_PAUSE();

  LOG_INFO("Starting Harpoons Anchoring Controller\n");

  coap_activate_resource(&anchoring, "anchoring");

  PROCESS_END();
}