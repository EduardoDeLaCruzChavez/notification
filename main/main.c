#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "EspWifi.h"

void app_main(void)
{
    static TYPE_WIFI tWiFi;

    vInitWiFi(&tWiFi);
}
