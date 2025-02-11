#include <stdlib.h>
#include <string.h>
#include "coap-engine.h"
#include <stdio.h>
#include "dev/leds.h"
#include "sys/log.h"

#define LOG_MODULE "anchoring"
#define LOG_LEVEL LOG_LEVEL_DBG

int activated = 0; // 1 = harpoons activated, 0 = not actiove

static void res_post_move_handler(coap_message_t *request, coap_message_t *response,
                                   uint8_t *buffer, uint16_t preferred_size, int32_t *offset);

// Resource definition
RESOURCE(anchoring,
         "title=\"Harpoons anchoring \";rt=\"Control\"",
         NULL,
         res_post_move_handler,
         NULL,
         NULL);

// Handler to activate or deactivate harpoons
static void res_post_move_handler(coap_message_t *request, coap_message_t *response,
                                   uint8_t *buffer, uint16_t preferred_size, int32_t *offset) {
    const uint8_t *payload;
    int len = coap_get_payload(request, &payload);

    if(len > 0) {
        int request;
        sscanf((char *)payload, "{ \"request\": %d }", &request);
        LOG_INFO("Payload: %s\n", (char*)payload);

        if(request == activated) {
            LOG_INFO("Harpoons manteined in state: %d\n", request);
        }
        else {
            LOG_INFO("Harpoons: transition from state %d to %d\n", activated, request);
            activated = request;
        }
    }

    coap_set_status_code(response, CHANGED_2_04);
}