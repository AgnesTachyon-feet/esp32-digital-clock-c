#pragma once
#include <stdbool.h>
#include <stdint.h>
#include "driver/gpio.h"
#include "freertos/FreeRTOS.h"
#include "freertos/queue.h"

typedef enum { CLOCK_BTN_MODE, CLOCK_BTN_SET, CLOCK_BTN_ALARM, CLOCK_BTN_LAP } clock_button_id_t;
typedef struct { clock_button_id_t button; bool long_press; } clock_button_event_t;

typedef struct { uint32_t pin; } clock_pin_event_t;
struct clock_buttons;
typedef struct { struct clock_buttons *owner; uint32_t pin; } clock_isr_context_t;
typedef struct clock_buttons {
    clock_isr_context_t contexts[4];
    QueueHandle_t isr_queue;
    QueueHandle_t destination;
    volatile uint32_t dropped_interrupts;
} clock_buttons_t;

bool clock_buttons_start(clock_buttons_t *buttons, QueueHandle_t destination);
