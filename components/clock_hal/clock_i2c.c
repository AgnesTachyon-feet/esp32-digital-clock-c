#include "clock_i2c.h"
#include "esp_log.h"

bool clock_i2c_init(clock_i2c_t *bus) {
    if (!bus) return false;
    bus->mutex = xSemaphoreCreateMutex();
    if (!bus->mutex) return false;
    i2c_master_bus_config_t config = {
        .clk_source = I2C_CLK_SRC_DEFAULT,
        .i2c_port = I2C_NUM_0,
        .sda_io_num = GPIO_NUM_21,
        .scl_io_num = GPIO_NUM_22,
        .glitch_ignore_cnt = 7,
        .flags.enable_internal_pullup = true
    };
    esp_err_t err = i2c_new_master_bus(&config, &bus->handle);
    if (err != ESP_OK) ESP_LOGE("I2C", "I2C init failed: %s", esp_err_to_name(err));
    return err == ESP_OK;
}
bool clock_i2c_add_device(clock_i2c_t *bus, uint8_t address, i2c_master_dev_handle_t *device) {
    if (!bus || !bus->handle || !device) return false;
    i2c_device_config_t config = {
        .dev_addr_length = I2C_ADDR_BIT_LEN_7,
        .device_address = address,
        .scl_speed_hz = 100000
    };
    return i2c_master_bus_add_device(bus->handle, &config, device) == ESP_OK;
}
bool clock_i2c_write(clock_i2c_t *bus, i2c_master_dev_handle_t dev, const uint8_t *data, size_t length) {
    if (!bus || !bus->mutex || !dev || !data || !length) return false;
    if (xSemaphoreTake(bus->mutex, pdMS_TO_TICKS(200)) != pdTRUE) return false;
    esp_err_t err = i2c_master_transmit(dev, data, length, 100);
    xSemaphoreGive(bus->mutex);
    return err == ESP_OK;
}
bool clock_i2c_read_registers(clock_i2c_t *bus, i2c_master_dev_handle_t dev, uint8_t reg, uint8_t *out, size_t length) {
    if (!bus || !bus->mutex || !dev || !out || !length) return false;
    if (xSemaphoreTake(bus->mutex, pdMS_TO_TICKS(200)) != pdTRUE) return false;
    esp_err_t err = i2c_master_transmit_receive(dev, &reg, 1, out, length, 100);
    xSemaphoreGive(bus->mutex);
    return err == ESP_OK;
}
