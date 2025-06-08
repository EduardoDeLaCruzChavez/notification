#ifndef ESP_HTTPS_CLIENT_H__
#define ESP_HTTPS_CLIENT_H__
#include <string.h>
#include <sys/param.h>
#include <stdlib.h>
#include <ctype.h>
#include "esp_log.h"
#include "nvs_flash.h"
#include "esp_event.h"
#include "esp_netif.h"
#include "esp_tls.h"
#include <string.h>

#if CONFIG_MBEDTLS_CERTIFICATE_BUNDLE
#include "esp_crt_bundle.h"
#endif

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_system.h"

#include "esp_http_client.h"
#include "ClientMac.h"

typedef struct TYPE_NOTIFY
{
    TYPE_BOT_INFO tBot;
    TYPE_CLIENTS *ptClients;
    TaskHandle_t tHandle;
} TYPE_NOTIFY;

void vInitHTTPClient(void *pvParameters);

#endif