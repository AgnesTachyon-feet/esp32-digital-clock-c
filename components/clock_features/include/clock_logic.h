#pragma once
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

typedef struct {
    bool running;
    int64_t started_us;
    int64_t saved_us;
    int64_t laps_ms[5];
    size_t lap_count;
} clock_stopwatch_t;

void clock_stopwatch_toggle(clock_stopwatch_t *sw, int64_t now_us);
void clock_stopwatch_reset(clock_stopwatch_t *sw);
int64_t clock_stopwatch_elapsed_ms(const clock_stopwatch_t *sw, int64_t now_us);
bool clock_stopwatch_lap(clock_stopwatch_t *sw, int64_t now_us);

typedef struct {
    bool armed;
    bool ringing;
    int hour;
    int minute;
    int64_t last_trigger_key;
} clock_alarm_t;

void clock_alarm_init(clock_alarm_t *alarm);
void clock_alarm_toggle(clock_alarm_t *alarm);
void clock_alarm_set_time(clock_alarm_t *alarm, int hour, int minute);
void clock_alarm_increment_minutes(clock_alarm_t *alarm, int increment);
bool clock_alarm_check(clock_alarm_t *alarm, int date_key, int hour, int minute);
void clock_alarm_dismiss(clock_alarm_t *alarm);
