#pragma once
#include <stdbool.h>
#include <stdint.h>
#include "esp_event.h"
#include "freertos/FreeRTOS.h"
#include "freertos/event_groups.h"
#include "clock_rtc.h"

typedef struct {
    EventGroupHandle_t events;
    volatile bool connected;
    volatile bool synchronized;
    bool sntp_initialized;
} clock_wifi_time_t;

bool clock_wifi_init(clock_wifi_time_t *wifi);
bool clock_wifi_connect(clock_wifi_time_t *wifi, uint32_t timeout_ms);
bool clock_wifi_sync_rtc(clock_wifi_time_t *wifi, clock_rtc_t *rtc);
