#include <stdbool.h>
#include <stdint.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"
#include "esp_log.h"
#include "esp_timer.h"
#include "clock_i2c.h"
#include "clock_rtc.h"
#include "clock_oled.h"
#include "clock_buttons.h"
#include "clock_buzzer.h"
#include "clock_wifi_time.h"
#include "clock_logic.h"
#include "clock_screen.h"

static const char *TAG = "CLOCK_APP";

typedef struct {
    clock_i2c_t bus;
    clock_rtc_t rtc;
    clock_oled_t oled;
    clock_buttons_t buttons;
    clock_alarm_t alarm;
    clock_stopwatch_t stopwatch;
    clock_wifi_time_t wifi;
    QueueHandle_t commands;
    clock_datetime_t now;
    bool rtc_ok, display_ok, buzzer_ok;
    int mode;
    int64_t ring_started_us;
} clock_app_t;

static clock_app_t app;

static void ntp_task(void *arg) {
    clock_app_t *a = (clock_app_t *)arg;
    if (!clock_wifi_init(&a->wifi)) {
        ESP_LOGW(TAG, "WiFi initialization failed: RTC fallback active");
    } else {
        for (;;) {
            if (clock_wifi_connect(&a->wifi, 12000)) {
                if (clock_wifi_sync_rtc(&a->wifi, &a->rtc)) ESP_LOGI(TAG, "NTP -> RTC synchronized");
            } else ESP_LOGW(TAG, "WiFi unavailable; using DS1307");
            vTaskDelay(pdMS_TO_TICKS(3600000));
        }
    }
    vTaskDelete(NULL);
}

static void handle_command(clock_app_t *a, const clock_button_event_t *event) {
    ESP_LOGI(TAG, "button=%u long=%d", (unsigned)event->button, (int)event->long_press);
    if (event->button == CLOCK_BTN_MODE) {
        a->mode = (a->mode + 1) % 4;
    } else if (event->button == CLOCK_BTN_SET) {
        if (a->mode == 1) clock_alarm_increment_minutes(&a->alarm, 5);
        else if (a->mode == 3 && a->rtc_ok) {
            clock_datetime_t datetime = a->now;
            if (++datetime.minute == 60) { datetime.minute = 0; datetime.hour = (datetime.hour + 1) % 24; }
            datetime.second = 0;
            if (clock_rtc_write(&a->rtc, &datetime)) a->now = datetime;
        } else if (a->mode == 2 && a->stopwatch.running) {
            (void)clock_stopwatch_lap(&a->stopwatch, esp_timer_get_time());
        }
    } else if (event->button == CLOCK_BTN_ALARM) {
        if (a->alarm.ringing) {
            clock_alarm_dismiss(&a->alarm);
            if (a->buzzer_ok) clock_buzzer_set(false);
        } else clock_alarm_toggle(&a->alarm);
    } else if (event->button == CLOCK_BTN_LAP && a->mode == 2) {
        if (event->long_press) clock_stopwatch_reset(&a->stopwatch);
        else clock_stopwatch_toggle(&a->stopwatch, esp_timer_get_time());
    }
}

static void display_task(void *arg) {
    clock_app_t *a = (clock_app_t *)arg;
    int64_t last_rtc_read_us = -500000;
    for (;;) {
        clock_button_event_t event;
        while (xQueueReceive(a->commands, &event, 0) == pdTRUE) handle_command(a, &event);
        int64_t us = esp_timer_get_time();
        if (us - last_rtc_read_us >= 500000) {
            last_rtc_read_us = us;
            clock_datetime_t candidate;
            if (a->rtc_ok && clock_rtc_read(&a->rtc, &candidate)) {
                a->now = candidate;
            } else a->rtc_ok = false;
            if (a->rtc_ok && clock_alarm_check(&a->alarm,
                a->now.year * 10000 + a->now.month * 100 + a->now.day,
                a->now.hour, a->now.minute)) {
                a->ring_started_us = us;
                if (a->buzzer_ok) clock_buzzer_set(true);
                ESP_LOGI(TAG, "ALARM ringing");
            }
        }
        if (a->alarm.ringing && us - a->ring_started_us > 10000000) {
            clock_alarm_dismiss(&a->alarm);
            if (a->buzzer_ok) clock_buzzer_set(false);
        }
        if (a->display_ok && !clock_screen_show(&a->oled, a->mode, &a->now,
             a->rtc_ok, a->wifi.connected, a->wifi.synchronized,
             &a->alarm, &a->stopwatch, us)) ESP_LOGW(TAG, "OLED update failed");
        vTaskDelay(pdMS_TO_TICKS(200));
    }
}

void app_main(void) {
    ESP_LOGI(TAG, "ESP-IDF C Digital Clock starting");
    clock_alarm_init(&app.alarm);
    if (!clock_i2c_init(&app.bus)) { ESP_LOGE(TAG, "I2C bus failed"); return; }
    app.rtc_ok = clock_rtc_init(&app.rtc, &app.bus);
    app.display_ok = clock_oled_init(&app.oled, &app.bus);
    app.buzzer_ok = clock_buzzer_init();
    ESP_LOGI(TAG, "RTC=%d OLED=%d Buzzer=%d", app.rtc_ok, app.display_ok, app.buzzer_ok);
    app.commands = xQueueCreate(16, sizeof(clock_button_event_t));
    if (!app.commands || !clock_buttons_start(&app.buttons, app.commands)) {
        ESP_LOGE(TAG, "button system failed"); return;
    }
    if (xTaskCreate(display_task, "clock_display", 8192, &app, 5, NULL) != pdPASS ||
        xTaskCreate(ntp_task, "clock_ntp", 6144, &app, 4, NULL) != pdPASS) {
        ESP_LOGE(TAG, "task creation failed"); return;
    }
    ESP_LOGI(TAG, "READY: MODE=18 SET=19 ALARM=4 LAP=5");
}
