#include <stdio.h>
#include "EspWifi.h"
#include "EspWifiTypes.h"
#include "FileSystem.h"
#include "Directory.h"

static const char *TAG_AP = "WiFi AP";
static const char *TAG_STA = "WiFi Sta";

static void vInitObjetWiFi(TYPE_WIFI *ptWiFi);
static void vInitWiFiAP(void);
static void vInitWiFiSta(TYPE_WIFI *ptWiFi);
static void WiFiEventHandler(void *vpArg, esp_event_base_t tEventBase,
                             int32_t i32EventID, void *vpEventData);

static void vInitWiFiAP(void)
{
    ESP_LOGI(TAG_AP, "ESP_WIFI_MODE_AP");
    esp_netif_create_default_wifi_ap();

    wifi_config_t tConfigAP = {
        .ap = {
            .ssid = ESP_WIFI_AP_SSID,
            .ssid_len = strlen(ESP_WIFI_AP_SSID),
            .channel = ESP_WIFI_AP_CHANNEL,
            .password = ESP_WIFI_AP_PASSD,
            .authmode = ESP_WIFI_AP_AUTH_MODE,
            .max_connection = ESP_WIFI_AP_MAX_STA,
            .pmf_cfg = {
                .required = false,
            },
        },
    };

    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_AP));
    ESP_ERROR_CHECK(esp_wifi_set_config(WIFI_IF_AP, &tConfigAP));
    ESP_LOGI(TAG_AP, "SSID: %s, Password: %s", ESP_WIFI_AP_SSID, ESP_WIFI_AP_PASSD);
    return;
}

static void vInitWiFiSta(TYPE_WIFI *ptWiFi)
{
    char acSSID[32] = {0};
    char acPSSD[64] = {0};
    char acHostName[16] = {0};
    esp_netif_t *tNetfit = NULL;
    wifi_config_t tConfigSta = {
        .sta = {
            .scan_method = WIFI_ALL_CHANNEL_SCAN,
            .failure_retry_cnt = ESP_WIFI_STA_RETRYS,
            .sae_pwe_h2e = WPA3_SAE_PWE_BOTH,
        },
    };

    ESP_LOGI(TAG_STA, "ESP_WIFI_MODE_STA");
    tNetfit = esp_netif_create_default_wifi_sta();

    vGetKey(KEY_SSID, acSSID, sizeof(acSSID));
    vGetKey(KEY_PSSD, acPSSD, sizeof(acPSSD));
    vGetKey(KEY_NAME, acHostName, sizeof(acHostName));

    snprintf((char *)tConfigSta.sta.ssid, sizeof(tConfigSta.sta.ssid), "%s", acSSID);
    snprintf((char *)tConfigSta.sta.password, sizeof(tConfigSta.sta.password), "%s", acPSSD);

    esp_netif_set_hostname(tNetfit, acHostName);
    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA));
    ESP_ERROR_CHECK(esp_wifi_set_config(WIFI_IF_STA, &tConfigSta));
    ESP_LOGI(TAG_STA, "vInitWiFiSta finished.");
    return;
}

static void vInitObjetWiFi(TYPE_WIFI *ptWiFi)
{
    if (ptWiFi == NULL)
    {
        vInitWiFiSta(ptWiFi);
        return;
    }
    ptWiFi->tState = eWIFI_UNDEF;
    ptWiFi->u8NumRetyr = 0;
    ptWiFi->tEventGroup = xEventGroupCreate();
}

void vInitWiFi(TYPE_WIFI *ptWiFi)
{
    wifi_init_config_t tWifiCfg = WIFI_INIT_CONFIG_DEFAULT();
    wifi_config_t tApInfo = {0};
    EventBits_t tBits = 0;
    bool bConfig = false;
    if (ptWiFi == NULL)
    {
        return;
    }

    vInitObjetWiFi(ptWiFi);

    ESP_ERROR_CHECK(esp_event_loop_create_default());
    ESP_ERROR_CHECK(esp_wifi_init(&tWifiCfg));
    ESP_ERROR_CHECK(esp_event_handler_instance_register(WIFI_EVENT,
                                                        ESP_EVENT_ANY_ID,
                                                        &WiFiEventHandler,
                                                        ptWiFi,
                                                        NULL));
    ESP_ERROR_CHECK(esp_event_handler_instance_register(IP_EVENT,
                                                        IP_EVENT_STA_GOT_IP,
                                                        &WiFiEventHandler,
                                                        ptWiFi,
                                                        NULL));

    vGetBlock(KEY_CFG, &bConfig, sizeof(bConfig));
    if (bConfig == false)
    {
        vInitWiFiAP();
    }
    else
    {
        vInitWiFiSta(ptWiFi);
    }

    ESP_ERROR_CHECK(esp_wifi_start());

    tBits = xEventGroupWaitBits(ptWiFi->tEventGroup,
                                WIFI_CONNECTED_BIT | WIFI_FAIL_BIT | WIFI_AP_START,
                                pdFALSE,
                                pdFALSE,
                                portMAX_DELAY);

    esp_wifi_get_config(WIFI_IF_STA, &tApInfo);

    if (tBits & WIFI_AP_START)
    {
        ptWiFi->tState = eWIFI_AP;
        ESP_LOGI(TAG_AP, "AP started");
    }
    else if (tBits & WIFI_CONNECTED_BIT)
    {
        ptWiFi->tState = eWIFI_STA;
        ESP_LOGI(TAG_STA, "Connected to SSID: %s", tApInfo.sta.ssid);
    }
    else if (tBits & WIFI_FAIL_BIT)
    {
        ESP_LOGI(TAG_STA, "Failed to connect to SSID:%s, password:%s",
                 tApInfo.sta.ssid, tApInfo.sta.password);
        bConfig = false;
        vSetBlock(KEY_CFG, &bConfig, sizeof(bConfig));
        vTaskDelay(1000 / portTICK_PERIOD_MS);
        esp_restart();
    }
    else
    {
        ESP_LOGE(TAG_STA, "UNEXPECTED EVENT");
        return;
    }
}

static void WiFiEventHandler(void *vpArg, esp_event_base_t tEventBase,
                             int32_t i32EventID, void *vpEventData)
{
    TYPE_WIFI *ptWiFi = (TYPE_WIFI *)vpArg;

    if (tEventBase == WIFI_EVENT && i32EventID == WIFI_EVENT_AP_START)
    {
        xEventGroupSetBits(ptWiFi->tEventGroup, WIFI_AP_START);
    }
    else if (tEventBase == WIFI_EVENT && i32EventID == WIFI_EVENT_AP_STACONNECTED)
    {
        wifi_event_ap_staconnected_t *event = (wifi_event_ap_staconnected_t *)vpEventData;
        ESP_LOGI(TAG_AP, "Station " MACSTR " joined, AID=%d",
                 MAC2STR(event->mac), event->aid);
    }
    else if (tEventBase == WIFI_EVENT && i32EventID == WIFI_EVENT_AP_STADISCONNECTED)
    {
        wifi_event_ap_stadisconnected_t *event = (wifi_event_ap_stadisconnected_t *)vpEventData;
        ESP_LOGI(TAG_AP, "Station " MACSTR " left, AID=%d, reason:%d",
                 MAC2STR(event->mac), event->aid, event->reason);
    }
    else if (tEventBase == WIFI_EVENT && i32EventID == WIFI_EVENT_STA_START)
    {
        esp_wifi_connect();
        ESP_LOGI(TAG_STA, "Station started");
    }
    else if (tEventBase == IP_EVENT && i32EventID == IP_EVENT_STA_GOT_IP)
    {
        xEventGroupSetBits(ptWiFi->tEventGroup, WIFI_CONNECTED_BIT);
    }
    else if (tEventBase == WIFI_EVENT && i32EventID == WIFI_EVENT_STA_DISCONNECTED)
    {
        ESP_LOGI(TAG_STA, "Connect to the AP fail");

        if (ptWiFi->u8NumRetyr < ESP_WIFI_STA_RETRYS)
        {
            esp_wifi_connect();
            ptWiFi->u8NumRetyr++;
            ESP_LOGI(TAG_STA, "Retry to connect to the AP");
        }
        else
        {
            xEventGroupSetBits(ptWiFi->tEventGroup, WIFI_FAIL_BIT);
        }
    }
}
