#pragma once
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include "driver/i2c_master.h"
#include "freertos/FreeRTOS.h"
#include "freertos/semphr.h"

typedef struct {
    i2c_master_bus_handle_t handle;
    SemaphoreHandle_t mutex;
} clock_i2c_t;

bool clock_i2c_init(clock_i2c_t *bus);
bool clock_i2c_add_device(clock_i2c_t *bus, uint8_t address, i2c_master_dev_handle_t *device);
bool clock_i2c_write(clock_i2c_t *bus, i2c_master_dev_handle_t dev, const uint8_t *data, size_t length);
bool clock_i2c_read_registers(clock_i2c_t *bus, i2c_master_dev_handle_t dev, uint8_t reg, uint8_t *out, size_t length);
