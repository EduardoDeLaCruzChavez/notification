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
#include "MySQL.h"

char DATABASE[] = "mydb";

void app_main(void)
{

    static TYPE_WIFI tWiFi = {0};
    TYPE_CLIENTS tClients = {0};
    TYPE_RESPONSE tResponse = {0};

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

    TYPE_DB_INFO tDBInfo = {
        .acHost = "raspberrycz",
        .acUser = "writer",
        .acPssd = "m7p1l1n0gr4nd3",
        .u16Port = 3306,
        .pcDB = DATABASE,
    };

    vReadClients(&tDBInfo, &tClients);

    while (1)
    {
        i8SnmpGetNext("192.168.8.1", "public", "1.3.4.13.69.101", &tResponse);
        vReadResponse(&tResponse);

        vUpdateClient(&tClients, &tResponse);
        vFreeResponse(&tResponse);
        vUpdateStateClient(&tDBInfo, &tClients);
        vTaskDelay(60000 / portTICK_PERIOD_MS);
    }
}