#include "clock_wifi_time.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "esp_log.h"
#include "esp_netif.h"
#include "esp_netif_sntp.h"
#include "esp_wifi.h"
#include "nvs_flash.h"

#define GOT_IP BIT0
static const char *TAG = "CLOCK_TIME";
static void wifi_handler(void *arg, esp_event_base_t base, int32_t event_id, void *data) {
    (void)data;
    clock_wifi_time_t *wifi = (clock_wifi_time_t *)arg;
    if (base == WIFI_EVENT &&
        (event_id == WIFI_EVENT_STA_START || event_id == WIFI_EVENT_STA_DISCONNECTED)) {
        if (event_id == WIFI_EVENT_STA_DISCONNECTED) {
            wifi->connected = false;
            xEventGroupClearBits(wifi->events, GOT_IP);
        }
        (void)esp_wifi_connect();
    } else if (base == IP_EVENT && event_id == IP_EVENT_STA_GOT_IP) {
        wifi->connected = true;
        xEventGroupSetBits(wifi->events, GOT_IP);
    }
}
bool clock_wifi_init(clock_wifi_time_t *wifi) {
    if (!wifi) return false;
    esp_err_t err = nvs_flash_init();
    if (err == ESP_ERR_NVS_NO_FREE_PAGES || err == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        if (nvs_flash_erase() != ESP_OK) return false;
        err = nvs_flash_init();
    }
    if (err != ESP_OK) return false;
    wifi->events = xEventGroupCreate();
    if (!wifi->events) return false;
    if (esp_netif_init() != ESP_OK || esp_event_loop_create_default() != ESP_OK) return false;
    if (!esp_netif_create_default_wifi_sta()) return false;
    wifi_init_config_t init = WIFI_INIT_CONFIG_DEFAULT();
    if (esp_wifi_init(&init) != ESP_OK) return false;
    if (esp_event_handler_register(WIFI_EVENT, ESP_EVENT_ANY_ID, wifi_handler, wifi) != ESP_OK ||
        esp_event_handler_register(IP_EVENT, IP_EVENT_STA_GOT_IP, wifi_handler, wifi) != ESP_OK) return false;
    wifi_config_t network = {0};
    snprintf((char *)network.sta.ssid, sizeof(network.sta.ssid), "%s", "Wokwi-GUEST");
    network.sta.channel = 6;
    network.sta.threshold.authmode = WIFI_AUTH_OPEN;
    return esp_wifi_set_mode(WIFI_MODE_STA) == ESP_OK &&
           esp_wifi_set_config(WIFI_IF_STA, &network) == ESP_OK &&
           esp_wifi_start() == ESP_OK;
}
bool clock_wifi_connect(clock_wifi_time_t *wifi, uint32_t timeout_ms) {
    if (!wifi || !wifi->events) return false;
    if (wifi->connected) return true;
    EventBits_t value = xEventGroupWaitBits(wifi->events, GOT_IP, pdFALSE, pdTRUE, pdMS_TO_TICKS(timeout_ms));
    return (value & GOT_IP) != 0;
}
bool clock_wifi_sync_rtc(clock_wifi_time_t *wifi, clock_rtc_t *rtc) {
    if (!wifi || !rtc || !wifi->connected) return false;
    if (!wifi->sntp_initialized) {
        esp_sntp_config_t config = ESP_NETIF_SNTP_DEFAULT_CONFIG("pool.ntp.org");
        if (esp_netif_sntp_init(&config) != ESP_OK) return false;
        wifi->sntp_initialized = true;
    }
    if (esp_netif_sntp_sync_wait(pdMS_TO_TICKS(12000)) != ESP_OK) {
        ESP_LOGW(TAG, "NTP unavailable; RTC fallback remains");
        return false;
    }
    setenv("TZ", "ICT-7", 1);
    tzset();
    time_t epoch = time(NULL);
    struct tm local;
    if (!localtime_r(&epoch, &local)) return false;
    clock_datetime_t datetime = {
        .year = local.tm_year + 1900, .month = local.tm_mon + 1,
        .day = local.tm_mday, .weekday = local.tm_wday + 1,
        .hour = local.tm_hour, .minute = local.tm_min, .second = local.tm_sec
    };
    wifi->synchronized = clock_rtc_write(rtc, &datetime);
    ESP_LOGI(TAG, "NTP -> RTC: %s", wifi->synchronized ? "synced" : "not synced");
    return wifi->synchronized;
}
