#pragma once
#include <stdbool.h>
#include <stdint.h>
#include "clock_i2c.h"

typedef struct {
    clock_i2c_t *bus;
    i2c_master_dev_handle_t device;
    uint8_t buffer[1024];
} clock_oled_t;

bool clock_oled_init(clock_oled_t *oled, clock_i2c_t *bus);
void clock_oled_clear(clock_oled_t *oled);
void clock_oled_text(clock_oled_t *oled, int x, int y, const char *text);
bool clock_oled_flush(clock_oled_t *oled);
