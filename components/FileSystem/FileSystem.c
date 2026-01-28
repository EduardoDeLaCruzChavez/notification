#include <stdio.h>
#include "FileSystem.h"
#include "Directory.h"

static const char *TAG = "File System";
static const char *TAG_NVS = "NVS";

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

    ESP_LOGD(TAG, "Performing SPIFFS_check().");
    tRet = esp_spiffs_check(conf.partition_label);

    if (tRet != ESP_OK)
    {
        ESP_LOGE(TAG, "SPIFFS_check() failed (%s)", esp_err_to_name(tRet));
        return -1;
    }

    ESP_LOGD(TAG, "SPIFFS_check() successful");

    tRet = esp_spiffs_info(conf.partition_label, &tTotal, &tUsed);
    if (tRet != ESP_OK)
    {
        ESP_LOGE(TAG, "Failed to get SPIFFS partition information (%s). Formatting...", esp_err_to_name(tRet));
        esp_spiffs_format(conf.partition_label);
        return -1;
    }
    ESP_LOGD(TAG, "Partition size: Total: %d, Used: %d", tTotal, tUsed);

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

void vClearKey(char *pcKey)
{
    nvs_handle_t tHandle;
    esp_err_t tError = 0;

    if (pcKey == NULL)
    {
        return;
    }

    tError = nvs_open("storages", NVS_READWRITE, &tHandle);

    if (tError != ESP_OK)
    {
        ESP_LOGE(TAG_NVS, "Error (%s) opening NVS handle!", esp_err_to_name(tError));
        return;
    }

    tError = nvs_erase_key(tHandle, pcKey);
    if (tError != ESP_OK)
    {
        ESP_LOGE(TAG_NVS, "Failed to write string!");
    }

    ESP_LOGD(TAG_NVS, "Committing updates in NVS...");
    tError = nvs_commit(tHandle);

    if (tError != ESP_OK)
    {
        ESP_LOGE(TAG_NVS, "Failed to commit NVS changes!");
    }

    // Close
    nvs_close(tHandle);
    ESP_LOGD(TAG_NVS, "NVS handle closed.");
}

void vSetKey(char *pckey, char *pcValue)
{
    nvs_handle_t tHandle;
    esp_err_t tError = 0;

    if (pckey == NULL || pcValue == NULL)
    {
        return;
    }

    tError = nvs_open("storages", NVS_READWRITE, &tHandle);

    if (tError != ESP_OK)
    {
        ESP_LOGE(TAG_NVS, "Error (%s) opening NVS handle!", esp_err_to_name(tError));
        return;
    }

    tError = nvs_set_str(tHandle, pckey, pcValue);
    if (tError != ESP_OK)
    {
        ESP_LOGE(TAG_NVS, "Failed to write string!");
    }

    ESP_LOGD(TAG_NVS, "Committing updates in NVS...");
    tError = nvs_commit(tHandle);

    if (tError != ESP_OK)
    {
        ESP_LOGE(TAG_NVS, "Failed to commit NVS changes!");
    }

    // Close
    nvs_close(tHandle);
    ESP_LOGD(TAG_NVS, "NVS handle closed.");
}

void vGetKey(char *pckey, char *pcValue, int iSize)
{
    nvs_handle_t tHandle;
    esp_err_t tErro = 0;
    size_t tRequiredSize = 0;

    if (pckey == NULL || pcValue == NULL)
    {
        return;
    }

    tErro = nvs_open("storages", NVS_READWRITE, &tHandle);

    if (tErro != ESP_OK)
    {
        ESP_LOGE(TAG_NVS, "Error (%s) opening NVS handle!", esp_err_to_name(tErro));
        return;
    }

    ESP_LOGD(TAG_NVS, "Reading string from NVS...");
    tErro = nvs_get_str(tHandle, pckey, NULL, &tRequiredSize);

    if (tErro == ESP_OK && tRequiredSize < iSize)
    {
        tErro = nvs_get_str(tHandle, pckey, pcValue, &tRequiredSize);
        if (tErro == ESP_OK)
        {
            ESP_LOGD(TAG_NVS, "Read string: %s", pcValue);
        }
    }

    // Close
    nvs_close(tHandle);
    ESP_LOGD(TAG_NVS, "NVS handle closed.");
}

void vGetBlock(char *pckey, void *pvBuff, int iSize)
{
    nvs_handle_t tHandle;
    esp_err_t tErro = 0;
    size_t tRequiredSize = 0;

    if (pckey == NULL || pvBuff == NULL)
    {
        return;
    }

    tErro = nvs_open("storages", NVS_READWRITE, &tHandle);

    if (tErro != ESP_OK)
    {
        ESP_LOGE(TAG_NVS, "Error (%s) opening NVS handle!", esp_err_to_name(tErro));
        return;
    }

    ESP_LOGD(TAG_NVS, "Reading string from NVS...");
    tErro = nvs_get_blob(tHandle, pckey, NULL, &tRequiredSize);

    if (tErro == ESP_OK && tRequiredSize == iSize)
    {
        tErro = nvs_get_blob(tHandle, pckey, pvBuff, &tRequiredSize);
    }

    // Close
    nvs_close(tHandle);
    ESP_LOGD(TAG_NVS, "NVS handle closed.");
}

void vSetBlock(char *pckey, void *pvBuff, int iSize)
{
    nvs_handle_t tHandle;
    esp_err_t tError = 0;

    if (pckey == NULL || pvBuff == NULL)
    {
        return;
    }

    tError = nvs_open("storages", NVS_READWRITE, &tHandle);

    if (tError != ESP_OK)
    {
        ESP_LOGE(TAG_NVS, "Error (%s) opening NVS handle!", esp_err_to_name(tError));
        return;
    }

    tError = nvs_set_blob(tHandle, pckey, pvBuff, iSize);
    if (tError != ESP_OK)
    {
        ESP_LOGE(TAG_NVS, "Failed to write string!");
    }

    ESP_LOGD(TAG_NVS, "Committing updates in NVS...");
    tError = nvs_commit(tHandle);

    if (tError != ESP_OK)
    {
        ESP_LOGE(TAG_NVS, "Failed to commit NVS changes!");
    }

    // Close
    nvs_close(tHandle);
    ESP_LOGD(TAG_NVS, "NVS handle closed.");
}

void vClearAllNVS()
{
    nvs_handle_t my_handle;
    esp_err_t err = nvs_open("storages", NVS_READWRITE, &my_handle);
    if (err == ESP_OK)
    {

        if (err == ESP_OK)
        {
            nvs_commit(my_handle); // Confirma los cambios
        }
        nvs_close(my_handle); // Cierra la sesión de NVS
    }
    nvs_handle_t tHandle;
    esp_err_t tError = 0;

    tError = nvs_open("storages", NVS_READWRITE, &tHandle);

    if (tError != ESP_OK)
    {
        ESP_LOGE(TAG_NVS, "Error (%s) opening NVS handle!", esp_err_to_name(tError));
        return;
    }

    err = nvs_erase_all(my_handle);
    if (tError != ESP_OK)
    {
        ESP_LOGE(TAG_NVS, "Failed to clear nvs");
    }

    ESP_LOGD(TAG_NVS, "Committing updates in NVS...");
    tError = nvs_commit(tHandle);

    if (tError != ESP_OK)
    {
        ESP_LOGE(TAG_NVS, "Failed to commit NVS changes!");
    }

    nvs_close(tHandle);
    ESP_LOGD(TAG_NVS, "NVS handle closed.");
}