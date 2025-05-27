#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "CommonFuntions.h"
#include "FileSystem.h"
#include "EspWifi.h"
#include "HTTPServer.h"
#include "SNMPManager.h"
#include "SNMPResponse.h"
#include "ClientMac.h"
#include "Directory.h"

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
    TYPE_RESPONSE tResponse = {0};
    TYPE_CLIENTS tClients = {0};
    remove(CLIENT_LIST);
    vTaskDelay(3000 / portTICK_PERIOD_MS);
    if (i8FileExist(CLIENT_LIST))
    {
        char Mac1[] = {
            0xC8,
            0x58,
        };
        char Mac2[] = {
            0x16,
            0x84,
        };
        char Mac3[] = {
            0xD4,
            0x8A,
        };
        vAppendMacClient(&tClients, Mac1);
        vAppendMacClient(&tClients, Mac2);
        vAppendMacClient(&tClients, Mac3);
    }

    vGetClientList(&tClients);

    while (1)
    {
        i8SnmpGetNext("192.168.8.1", "public", "1.3.6.1.2.1.4.22.1.2", &tResponse);
        vReadResponse(&tResponse);

        vUpdateClient(&tClients, &tResponse);
        vFreeResponse(&tResponse);
        vTaskDelay(60000 / portTICK_PERIOD_MS);
    }
}
