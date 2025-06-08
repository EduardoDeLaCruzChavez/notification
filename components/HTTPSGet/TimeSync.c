#include <string.h>
#include <stdlib.h>
#include <time.h>
#include <sys/time.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/event_groups.h"
#include "esp_log.h"
#include "esp_system.h"
#include "nvs_flash.h"
#include "nvs.h"
#include "esp_netif.h"
#include "esp_netif_sntp.h"

#include "lwip/err.h"
#include "lwip/sockets.h"
#include "lwip/sys.h"
#include "lwip/netdb.h"
#include "lwip/dns.h"
#include "TimeSync.h"

static const char *TAG = "TimeSync";

#define STORAGE_NAMESPACE "storage"

#define TIME_PERIOD (86400000000ULL)

void initialize_sntp(void)
{
    ESP_LOGI(TAG, "Initializing SNTP");
    esp_sntp_config_t config = ESP_NETIF_SNTP_DEFAULT_CONFIG_MULTIPLE(2,
                                                                      ESP_SNTP_SERVER_LIST("time.windows.com", "pool.ntp.org"));
    esp_netif_sntp_init(&config);
}

static esp_err_t obtain_time(void)
{
    // wait for time to be set
    int retry = 0;
    const int retry_count = 10;
    while (esp_netif_sntp_sync_wait(pdMS_TO_TICKS(2000)) != ESP_OK && ++retry < retry_count)
    {
        ESP_LOGI(TAG, "Waiting for system time to be set... (%d/%d)", retry, retry_count);
    }
    if (retry == retry_count)
    {
        return ESP_FAIL;
    }
    return ESP_OK;
}

esp_err_t fetch_and_store_time_in_nvs(void *args)
{
    nvs_handle_t tNVSHandle = 0;
    esp_err_t tError = 0;

    initialize_sntp();
    if (obtain_time() != ESP_OK)
    {
        tError = ESP_FAIL;
        goto exit;
    }

    time_t tTime;
    time(&tTime);

    // Open
    tError = nvs_open(STORAGE_NAMESPACE, NVS_READWRITE, &tNVSHandle);
    if (tError != ESP_OK)
    {
        goto exit;
    }

    // Write
    tError = nvs_set_i64(tNVSHandle, "timestamp", tTime);
    if (tError != ESP_OK)
    {
        goto exit;
    }

    tError = nvs_commit(tNVSHandle);
    if (tError != ESP_OK)
    {
        goto exit;
    }

exit:
    if (tNVSHandle != 0)
    {
        nvs_close(tNVSHandle);
    }
    esp_netif_sntp_deinit();

    if (tError != ESP_OK)
    {
        ESP_LOGE(TAG, "Error updating time in nvs");
    }
    else
    {
        ESP_LOGI(TAG, "Updated time in NVS");
    }
    return tError;
}

esp_err_t update_time_from_nvs(void)
{
    nvs_handle_t tNVSHandle = 0;
    esp_err_t tError;

    tError = nvs_open(STORAGE_NAMESPACE, NVS_READWRITE, &tNVSHandle);
    if (tError != ESP_OK)
    {
        ESP_LOGE(TAG, "Error opening NVS");
        goto exit;
    }

    int64_t tTimestamp = 0;

    tError = nvs_get_i64(tNVSHandle, "timestamp", &tTimestamp);
    if (tError == ESP_ERR_NVS_NOT_FOUND)
    {
        ESP_LOGI(TAG, "Time not found in NVS. Syncing time from SNTP server.");
        if (fetch_and_store_time_in_nvs(NULL) != ESP_OK)
        {
            tError = ESP_FAIL;
        }
        else
        {
            tError = ESP_OK;
        }
    }
    else if (tError == ESP_OK)
    {
        struct timeval get_nvs_time;
        get_nvs_time.tv_sec = tTimestamp;
        settimeofday(&get_nvs_time, NULL);
    }

exit:
    if (tNVSHandle != 0)
    {
        nvs_close(tNVSHandle);
    }
    return tError;
}

void vInitGetTime(void)
{

    if (esp_reset_reason() == ESP_RST_POWERON)
    {
        ESP_LOGI(TAG, "Updating time from NVS");
        setenv("TZ", "PST8PDT,M3.2.0/2:00:00,M11.1.0/2:00:00", 1);
        tzset();
        ESP_ERROR_CHECK(fetch_and_store_time_in_nvs(NULL));
    }

    const esp_timer_create_args_t nvs_update_timer_args = {
        .callback = (void *)&fetch_and_store_time_in_nvs,
    };

    esp_timer_handle_t nvs_update_timer;
    ESP_ERROR_CHECK(esp_timer_create(&nvs_update_timer_args, &nvs_update_timer));
    ESP_ERROR_CHECK(esp_timer_start_periodic(nvs_update_timer, TIME_PERIOD));
}