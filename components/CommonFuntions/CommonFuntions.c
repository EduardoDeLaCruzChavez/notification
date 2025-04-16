#include <stdio.h>
#include "CommonFuntions.h"
#include "nvs.h"

void vInitNVSFlash()
{
    esp_err_t tRet = 0;

    ESP_ERROR_CHECK(esp_netif_init());

    tRet = nvs_flash_init();
    if (tRet == ESP_ERR_NVS_NO_FREE_PAGES || tRet == ESP_ERR_NVS_NEW_VERSION_FOUND)
    {
        ESP_ERROR_CHECK(nvs_flash_erase());
        tRet = nvs_flash_init();
    }

    ESP_ERROR_CHECK(tRet);
}