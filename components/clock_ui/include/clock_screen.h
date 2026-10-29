#pragma once
#include <stdbool.h>
#include <stdint.h>
#include "clock_oled.h"
#include "clock_rtc.h"
#include "clock_logic.h"

bool clock_screen_show(clock_oled_t *oled, int mode, const clock_datetime_t *now,
                       bool time_valid, bool wifi, bool ntp,
                       const clock_alarm_t *alarm, const clock_stopwatch_t *sw, int64_t now_us);
