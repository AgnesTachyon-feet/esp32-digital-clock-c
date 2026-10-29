#include "clock_screen.h"
#include <stdio.h>

bool clock_screen_show(clock_oled_t *oled, int mode, const clock_datetime_t *now,
                       bool valid, bool wifi, bool ntp, const clock_alarm_t *alarm,
                       const clock_stopwatch_t *sw, int64_t now_us) {
    if (!oled || !now || !alarm || !sw) return false;
    char line[32];
    clock_oled_clear(oled);
    if (mode == 0) {
        clock_oled_text(oled, 0, 0, "CLOCK");
        if (valid) {
            snprintf(line, sizeof(line), "%02d:%02d:%02d", now->hour, now->minute, now->second);
            clock_oled_text(oled, 0, 16, line);
            snprintf(line, sizeof(line), "%04d/%02d/%02d", now->year, now->month, now->day);
            clock_oled_text(oled, 0, 28, line);
        } else clock_oled_text(oled, 0, 16, "RTC NOT FOUND");
    } else if (mode == 1) {
        clock_oled_text(oled, 0, 0, "ALARM SET PLUS 5M");
        snprintf(line, sizeof(line), "%02d:%02d %s", alarm->hour, alarm->minute, alarm->armed ? "ON" : "OFF");
        clock_oled_text(oled, 0, 16, line);
        if (alarm->ringing) clock_oled_text(oled, 0, 30, "RINGING");
    } else if (mode == 2) {
        clock_oled_text(oled, 0, 0, "STOPWATCH");
        int64_t ms = clock_stopwatch_elapsed_ms(sw, now_us);
        snprintf(line, sizeof(line), "%02lld:%02lld.%02lld",
                 (long long)((ms / 60000) % 100), (long long)((ms / 1000) % 60),
                 (long long)((ms / 10) % 100));
        clock_oled_text(oled, 0, 16, line);
        snprintf(line, sizeof(line), "LAPS %u %s", (unsigned)sw->lap_count,
                 sw->running ? "RUN" : "STOP");
        clock_oled_text(oled, 0, 28, line);
    } else {
        clock_oled_text(oled, 0, 0, "SET RTC TIME");
        clock_oled_text(oled, 0, 16, "SET PLUS 1 MIN");
        if (valid) {
            snprintf(line, sizeof(line), "%02d:%02d:%02d", now->hour, now->minute, now->second);
            clock_oled_text(oled, 0, 28, line);
        }
    }
    clock_oled_text(oled, 0, 50, wifi ? (ntp ? "NTP SYNC OK" : "WIFI CONNECT") : "RTC OFFLINE MODE");
    return clock_oled_flush(oled);
}
