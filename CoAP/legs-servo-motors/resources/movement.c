#include <stdlib.h>
#include <string.h>
#include "coap-engine.h"
#include <stdio.h>
#include "dev/leds.h"
#include "sys/log.h"

#define LOG_MODULE "movement"
#define LOG_LEVEL LOG_LEVEL_DBG

static void res_post_move_handler(coap_message_t *request, coap_message_t *response,
                                   uint8_t *buffer, uint16_t preferred_size, int32_t *offset);

// Resource definition
RESOURCE(movement,
         "title=\"Robotic Legs Movement\";rt=\"Control\"",
         NULL,
         res_post_move_handler,
         NULL,
         NULL);

// Handler to move in a specific direction activating servo motors on robotic legs
static void res_post_move_handler(coap_message_t *request, coap_message_t *response,
                                   uint8_t *buffer, uint16_t preferred_size, int32_t *offset) {
    const uint8_t *payload;
    int len = coap_get_payload(request, &payload);

    if(len > 0) {
        int direction; // 1=forward; 2=right; 3=backward; 4=left
        int angle;
        sscanf((char *)payload, "{ \"direction\": %d, \"angle\": %d }", &direction, &angle);

        LOG_INFO("Activated servo motors to move %d degrees in direction %d\n", angle, direction);
    }

    coap_set_status_code(response, CHANGED_2_04);
}