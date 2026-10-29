idf_component_register(SRCS "clock_i2c.c" "clock_rtc.c" "clock_oled.c" "clock_buttons.c" "clock_buzzer.c"
  INCLUDE_DIRS "include"
  REQUIRES esp_driver_i2c esp_driver_gpio esp_driver_ledc freertos esp_timer)
