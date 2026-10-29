#include "clock_logic.h"

void clock_stopwatch_toggle(clock_stopwatch_t *sw, int64_t now_us) {
    if (sw->running) sw->saved_us += now_us - sw->started_us;
    else sw->started_us = now_us;
    sw->running = !sw->running;
}
void clock_stopwatch_reset(clock_stopwatch_t *sw) {
    *sw = (clock_stopwatch_t){0};
}
int64_t clock_stopwatch_elapsed_ms(const clock_stopwatch_t *sw, int64_t now_us) {
    int64_t elapsed = sw->saved_us + (sw->running ? now_us - sw->started_us : 0);
    return (elapsed > 0 ? elapsed : 0) / 1000;
}
bool clock_stopwatch_lap(clock_stopwatch_t *sw, int64_t now_us) {
    if (!sw->running || sw->lap_count >= 5) return false;
    sw->laps_ms[sw->lap_count++] = clock_stopwatch_elapsed_ms(sw, now_us);
    return true;
}
void clock_alarm_init(clock_alarm_t *alarm) {
    *alarm = (clock_alarm_t){.hour = 7, .minute = 30, .last_trigger_key = -1};
}
void clock_alarm_toggle(clock_alarm_t *alarm) {
    alarm->armed = !alarm->armed;
    alarm->ringing = false;
}
void clock_alarm_set_time(clock_alarm_t *alarm, int hour, int minute) {
    if (hour >= 0 && hour < 24 && minute >= 0 && minute < 60) {
        alarm->hour = hour;
        alarm->minute = minute;
        alarm->ringing = false;
    }
}
void clock_alarm_increment_minutes(clock_alarm_t *alarm, int increment) {
    int n = ((alarm->hour * 60 + alarm->minute + increment) % 1440 + 1440) % 1440;
    clock_alarm_set_time(alarm, n / 60, n % 60);
}
bool clock_alarm_check(clock_alarm_t *alarm, int date_key, int hour, int minute) {
    int64_t key = (int64_t)date_key * 1440 + hour * 60 + minute;
    if (alarm->armed && hour == alarm->hour && minute == alarm->minute && key != alarm->last_trigger_key) {
        alarm->last_trigger_key = key;
        alarm->ringing = true;
        return true;
    }
    return false;
}
void clock_alarm_dismiss(clock_alarm_t *alarm) { alarm->ringing = false; }
