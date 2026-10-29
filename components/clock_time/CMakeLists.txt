idf_component_register(SRCS "clock_wifi_time.c" INCLUDE_DIRS "include"
  REQUIRES clock_hal esp_wifi esp_event esp_netif nvs_flash lwip freertos)
