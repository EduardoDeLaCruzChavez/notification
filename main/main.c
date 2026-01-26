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
#include "OutIn.h"

#define SLEEP_MS 200

char DATABASE[] = "mydb";
void vResetFactory();

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
    int32_t u32Mls = 60000;
    vInitNVSFlash();
    vInitGPIO();

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
    vWriteLed(true);

    while (1)
    {
        vResetFactory();

        if (u32Mls >= 60 * 1000)
        {
            i8SnmpGetNext(acGWIP, "public", "1.3.4.13.69.101", &tResponse);
            vReadResponse(&tResponse);

            vUpdateClient(&tClients, &tResponse);
            vFreeResponse(&tResponse);
            vUpdateStateClient(&tDBInfo, &tClients);
            if (tClients.s8Reconnect > 0 || tClients.s8Disconect > 0)
            {
                xTaskNotifyGive(tNotify.tHandle);
            }
            u32Mls = 0;
        }
        vTaskDelay(SLEEP_MS / portTICK_PERIOD_MS);
        u32Mls += SLEEP_MS;
    }
}

void vResetFactory()
{
    static int16_t s16Time = 10000;
    bool bReset = false;

    if (bReadPin(PIN_RESET) == 1)
    {
        s16Time -= SLEEP_MS;
        if (s16Time <= 0)
        {
            vSetBlock(KEY_CONNECT, &bReset, sizeof(bReset));
            vSetBlock(KEY_CFG, &bReset, sizeof(bReset));
            esp_restart();
        }
    }

    if (bReadPin(PIN_RESET) == 0)
    {
        s16Time = 10000;
    }
}