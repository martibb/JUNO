#include <stdlib.h>
#include <string.h>
#include "coap-engine.h"
#include <stdio.h>
#include "dev/leds.h"

// Actuator status
// TODO: servono almeno 4 robotic legs. Gli attuatori sono i motori.
// Nella vita reale ne servono due per gamba. Per semplificare, supponiamo
// di attivare un servo motore per gamba.
static int position = 0; // posizione complessiva del rover rispetto alla partenza
static int moving = 0;
static int direction; // 1=forward; 2=right; 3=backward; 4=right;

static void gate_post_handler(coap_message_t *request, coap_message_t *response,
                                uint8_t *buffer, uint16_t preferred_size, int32_t *offset);

RESOURCE(auto_gate,
         "title=\"Robotic Legs Movement\";rt=\"Control\"",
         NULL,
         movement_post_move_handler,
         NULL,
         NULL);

// Handler per muovere la zampa
static void res_post_move_handler(coap_message_t *request, coap_message_t *response,
                                    uint8_t *buffer, uint16_t preferred_size, int32_t *offset) {
    const uint8_t *payload;
    int len = coap_get_payload(request, &payload);

    if(len > 0) {
        int direction, angle;
        sscanf((char *)payload, "{ \"direction\": %d, \"angle\": %d }", &direction, &angle);

        // Distinguere 4 casi di direzione:
        position += angle; // The rover moves
        moving = 1;
        LOG_INFO("Moving leg to %d degrees at speed %d\n", position, speed);
    }
    coap_set_status_code(response, CHANGED_2_04);
}