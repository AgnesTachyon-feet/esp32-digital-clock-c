#include "clock_rtc.h"

static uint8_t bcd(uint8_t n) { return (uint8_t)((n / 10) * 16 + n % 10); }
static int dec(uint8_t n) { return (n >> 4) * 10 + (n & 15); }

bool clock_rtc_init(clock_rtc_t *rtc, clock_i2c_t *bus) {
    if (!rtc || !bus) return false;
    rtc->bus = bus;
    if (!clock_i2c_add_device(bus, 0x68, &rtc->device)) return false;
    uint8_t seconds;
    if (!clock_i2c_read_registers(bus, rtc->device, 0, &seconds, 1)) return false;
    if (seconds & 0x80) {
        uint8_t command[2] = {0, (uint8_t)(seconds & 0x7f)};
        return clock_i2c_write(bus, rtc->device, command, sizeof(command));
    }
    return true;
}
bool clock_rtc_read(clock_rtc_t *rtc, clock_datetime_t *dt) {
    if (!rtc || !rtc->bus || !dt) return false;
    uint8_t bytes[7] = {0};
    if (!clock_i2c_read_registers(rtc->bus, rtc->device, 0, bytes, sizeof(bytes))) return false;
    if (bytes[0] & 0x80) return false;
    dt->second = dec(bytes[0] & 0x7f);
    dt->minute = dec(bytes[1] & 0x7f);
    if (bytes[2] & 0x40) {
        int hour12 = dec(bytes[2] & 0x1f);
        dt->hour = (hour12 % 12) + ((bytes[2] & 0x20) ? 12 : 0);
    } else dt->hour = dec(bytes[2] & 0x3f);
    dt->weekday = dec(bytes[3] & 0x07);
    dt->day = dec(bytes[4] & 0x3f);
    dt->month = dec(bytes[5] & 0x1f);
    dt->year = 2000 + dec(bytes[6]);
    return dt->second < 60 && dt->minute < 60 && dt->hour < 24 && dt->day >= 1 && dt->day <= 31 &&
           dt->month >= 1 && dt->month <= 12 && dt->year >= 2000 && dt->year <= 2099;
}
bool clock_rtc_write(clock_rtc_t *rtc, const clock_datetime_t *dt) {
    if (!rtc || !rtc->bus || !dt) return false;
    if (dt->year < 2000 || dt->year > 2099 || dt->month < 1 || dt->month > 12 ||
        dt->day < 1 || dt->day > 31 || dt->hour < 0 || dt->hour > 23 ||
        dt->minute < 0 || dt->minute > 59 || dt->second < 0 || dt->second > 59) return false;
    uint8_t bytes[8] = {0x00, bcd(dt->second), bcd(dt->minute), bcd(dt->hour),
                        bcd(dt->weekday >= 1 && dt->weekday <= 7 ? dt->weekday : 1),
                        bcd(dt->day), bcd(dt->month), bcd(dt->year - 2000)};
    return clock_i2c_write(rtc->bus, rtc->device, bytes, sizeof(bytes));
}
