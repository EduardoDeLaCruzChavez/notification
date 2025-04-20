#ifndef HTTP_SERVER_H___
#define HTTP_SERVER_H___
#include <esp_wifi.h>
#include <esp_event.h>
#include <esp_log.h>
#include <esp_system.h>
#include <sys/param.h>
#include "esp_netif.h"

#include <esp_http_server.h>
#include "esp_tls_crypto.h"
#include "sdkconfig.h"

int8_t i8StartServer(void);

#endif