#ifndef COMMON_ESP_H__
#define COMMON_ESP_H__
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_netif.h"
#include "nvs_flash.h"
#include "esp_mac.h"
#include "esp_log.h"

void vInitNVSFlash();

#endif