#include <stdio.h>
#include "FileSystem.h"
#include "Directory.h"

static const char *TAG = "File System";

int8_t i8InitFileSystem(void)
{
    ESP_LOGI(TAG, "Initializing SPIFFS");
    size_t tTotal = 0, tUsed = 0;
    esp_err_t tRet = 0;

    esp_vfs_spiffs_conf_t conf = {
        .base_path = ROOT_DIR,
        .partition_label = NULL,
        .max_files = 5,
        .format_if_mount_failed = true,
    };

    tRet = esp_vfs_spiffs_register(&conf);

    if (tRet != ESP_OK)
    {
        if (tRet == ESP_FAIL)
        {
            ESP_LOGE(TAG, "Failed to mount or format filesystem");
        }
        else if (tRet == ESP_ERR_NOT_FOUND)
        {
            ESP_LOGE(TAG, "Failed to find SPIFFS partition");
        }
        else
        {
            ESP_LOGE(TAG, "Failed to initialize SPIFFS (%s)", esp_err_to_name(tRet));
        }
        return -1;
    }

    ESP_LOGI(TAG, "Performing SPIFFS_check().");
    tRet = esp_spiffs_check(conf.partition_label);

    if (tRet != ESP_OK)
    {
        ESP_LOGE(TAG, "SPIFFS_check() failed (%s)", esp_err_to_name(tRet));
        return -1;
    }

    ESP_LOGI(TAG, "SPIFFS_check() successful");

    tRet = esp_spiffs_info(conf.partition_label, &tTotal, &tUsed);
    if (tRet != ESP_OK)
    {
        ESP_LOGE(TAG, "Failed to get SPIFFS partition information (%s). Formatting...", esp_err_to_name(tRet));
        esp_spiffs_format(conf.partition_label);
        return -1;
    }
    ESP_LOGI(TAG, "Partition size: Total: %d, Used: %d", tTotal, tUsed);

    // Check consistency of reported partition size info.
    if (tUsed > tTotal)
    {
        ESP_LOGW(TAG, "Number of tUsed bytes cannot be larger than tTotal. Performing SPIFFS_check().");
        tRet = esp_spiffs_check(conf.partition_label);

        if (tRet != ESP_OK)
        {
            ESP_LOGE(TAG, "SPIFFS_check() failed (%s)", esp_err_to_name(tRet));
            return -1;
        }
    }
    return 0;
}

int8_t i8FileExist(const char *pacFilename)
{
    struct stat tFile = {0};

    if (pacFilename == NULL)
    {
        return -1;
    }

    if (stat(pacFilename, &tFile) == 0)
    {
        return 0;
    }

    return -1;
}