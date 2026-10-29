#include "clock_buttons.h"
#include <stddef.h>
#include "freertos/task.h"
#include "esp_log.h"

static const gpio_num_t BUTTON_PINS[4] = {GPIO_NUM_18, GPIO_NUM_19, GPIO_NUM_4, GPIO_NUM_5};

static void IRAM_ATTR button_isr(void *arg) {
    clock_isr_context_t *ctx = (clock_isr_context_t *)arg;
    clock_buttons_t *buttons = ctx->owner;
    clock_pin_event_t event = {.pin = ctx->pin};
    BaseType_t woken = pdFALSE;
    if (xQueueSendFromISR(buttons->isr_queue, &event, &woken) != pdTRUE) ++buttons->dropped_interrupts;
    if (woken) portYIELD_FROM_ISR();
}
static void button_worker(void *arg) {
    clock_buttons_t *buttons = (clock_buttons_t *)arg;
    clock_pin_event_t raw;
    TickType_t last_tick[4] = {0};
    for (;;) {
        if (xQueueReceive(buttons->isr_queue, &raw, portMAX_DELAY) != pdTRUE) continue;
        int index = -1;
        for (int i = 0; i < 4; ++i) if ((uint32_t)BUTTON_PINS[i] == raw.pin) index = i;
        if (index < 0) continue;
        TickType_t now = xTaskGetTickCount();
        if (last_tick[index] && now - last_tick[index] < pdMS_TO_TICKS(60)) continue;
        vTaskDelay(pdMS_TO_TICKS(25));
        if (gpio_get_level(BUTTON_PINS[index]) != 0) continue;
        TickType_t pressed_at = xTaskGetTickCount();
        while (gpio_get_level(BUTTON_PINS[index]) == 0) vTaskDelay(pdMS_TO_TICKS(10));
        last_tick[index] = xTaskGetTickCount();
        clock_button_event_t evt = {
            .button = (clock_button_id_t)index,
            .long_press = (last_tick[index] - pressed_at) >= pdMS_TO_TICKS(800)
        };
        if (xQueueSend(buttons->destination, &evt, pdMS_TO_TICKS(50)) != pdTRUE)
            ESP_LOGW("BUTTON", "command queue full");
    }
}
bool clock_buttons_start(clock_buttons_t *buttons, QueueHandle_t destination) {
    if (!buttons || !destination) return false;
    buttons->destination = destination;
    buttons->isr_queue = xQueueCreate(32, sizeof(clock_pin_event_t));
    if (!buttons->isr_queue) return false;
    gpio_config_t config = {
        .pin_bit_mask = (1ULL << GPIO_NUM_18) | (1ULL << GPIO_NUM_19) |
                        (1ULL << GPIO_NUM_4) | (1ULL << GPIO_NUM_5),
        .mode = GPIO_MODE_INPUT,
        .pull_up_en = GPIO_PULLUP_ENABLE,
        .pull_down_en = GPIO_PULLDOWN_DISABLE,
        .intr_type = GPIO_INTR_NEGEDGE
    };
    if (gpio_config(&config) != ESP_OK || gpio_install_isr_service(0) != ESP_OK) return false;
    for (int i = 0; i < 4; ++i) {
        buttons->contexts[i] = (clock_isr_context_t){.owner = buttons, .pin = (uint32_t)BUTTON_PINS[i]};
        if (gpio_isr_handler_add(BUTTON_PINS[i], button_isr, &buttons->contexts[i]) != ESP_OK) return false;
    }
    return xTaskCreate(button_worker, "clock_buttons", 3072, buttons, 8, NULL) == pdPASS;
}
