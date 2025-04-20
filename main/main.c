#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "CommonFuntions.h"
#include "FileSystem.h"
#include "EspWifi.h"
#include "HTTPServer.h"

void app_main(void)
{

    static TYPE_WIFI tWiFi = {0};
    vInitNVSFlash();
    if (i8InitFileSystem())
    {
        vTaskDelay(2000 / portTICK_PERIOD_MS);
        esp_restart();
        return;
    }

    vInitWiFi(&tWiFi);
    if (tWiFi.tState == eWIFI_AP && i8StartServer())
    {
        return;
    }
}
