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
#include "HTTPSGet.h"

char DATABASE[] = "mydb";

void app_main(void)
{

    static TYPE_WIFI tWiFi = {0};
    TYPE_CLIENTS tClients = {
        .ptClient = NULL,
        .s8Clients = 0,
        .s8Disconect = 0,
        .s8Reconnect = 0,
    };
    TYPE_RESPONSE tResponse = {0};
    TYPE_NOTIFY tNotify = {
        .ptClients = &tClients,
    };
    char acGWIP[16] = {0};
    int8_t s8Sec = 60;
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

    vReadNotifyInfo(&tDBInfo, &tNotify.tBot);
    vReadClients(&tDBInfo, &tClients);
    vInitHTTPClient(&tNotify);
    vGetApIp(acGWIP);

    while (1)
    {
        if (s8Sec >= 60)
        {
            i8SnmpGetNext(acGWIP, "public", "1.3.4.13.69.101", &tResponse);
            vReadResponse(&tResponse);

            vUpdateClient(&tClients, &tResponse);
            vFreeResponse(&tResponse);
            vUpdateStateClient(&tDBInfo, &tClients);
            if (tClients.s8Reconnect > 0 || tClients.s8Disconect > 0)
            {
                vTaskResume(tNotify.tHandle);
            }
            s8Sec = 0;
        }
        vTaskDelay(1000 / portTICK_PERIOD_MS);
        s8Sec++;
    }
}