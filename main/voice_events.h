#ifndef VOICE_EVENTS_H
#define VOICE_EVENTS_H

#include "freertos/FreeRTOS.h"
#include "freertos/event_groups.h"

// Event group handle
extern EventGroupHandle_t voice_events;

// Event bit definitions
#define EVT_ON  (1 << 0)
#define EVT_OFF (1 << 1)

#endif // VOICE_EVENTS_H