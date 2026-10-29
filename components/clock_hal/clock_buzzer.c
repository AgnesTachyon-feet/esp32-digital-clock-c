#include "clock_buzzer.h"
#include "driver/ledc.h"
#include "esp_err.h"

bool clock_buzzer_init(void) {
    ledc_timer_config_t timer = {
        .speed_mode = LEDC_LOW_SPEED_MODE, .timer_num = LEDC_TIMER_0,
        .duty_resolution = LEDC_TIMER_10_BIT, .freq_hz = 2000,
        .clk_cfg = LEDC_AUTO_CLK
    };
    if (ledc_timer_config(&timer) != ESP_OK) return false;
    ledc_channel_config_t channel = {
        .gpio_num = 25, .speed_mode = LEDC_LOW_SPEED_MODE,
        .channel = LEDC_CHANNEL_0, .timer_sel = LEDC_TIMER_0,
        .duty = 0, .hpoint = 0
    };
    return ledc_channel_config(&channel) == ESP_OK;
}
void clock_buzzer_set(bool on) {
    ledc_set_duty(LEDC_LOW_SPEED_MODE, LEDC_CHANNEL_0, on ? 512 : 0);
    ledc_update_duty(LEDC_LOW_SPEED_MODE, LEDC_CHANNEL_0);
}
