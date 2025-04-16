#ifndef ESP_WIFI_H__
#define ESP_WIFI_H__
#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/event_groups.h"
#include "esp_mac.h"
#include "esp_wifi.h"
#include "esp_event.h"
#include "esp_log.h"
#include "nvs_flash.h"

#include "lwip/err.h"
#include "lwip/sys.h"
typedef enum
{
    eWIFI_UNDEF,
    eWIFI_STA,
    eWIFI_AP,
    eWIFI_MAX
} ENUM_WIFI_STATE;
typedef struct
{
    EventGroupHandle_t tEventGroup;
    ENUM_WIFI_STATE tState;
    uint8_t u8NumRetyr;
} TYPE_WIFI;

void vInitWiFi(TYPE_WIFI *ptWiFi);
#endif
