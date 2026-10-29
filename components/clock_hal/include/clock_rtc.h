#pragma once
#include <stdbool.h>
#include <stdint.h>
#include "clock_i2c.h"

typedef struct {
    int year, month, day, weekday;
    int hour, minute, second;
} clock_datetime_t;

typedef struct {
    clock_i2c_t *bus;
    i2c_master_dev_handle_t device;
} clock_rtc_t;

bool clock_rtc_init(clock_rtc_t *rtc, clock_i2c_t *bus);
bool clock_rtc_read(clock_rtc_t *rtc, clock_datetime_t *out);
bool clock_rtc_write(clock_rtc_t *rtc, const clock_datetime_t *datetime);
