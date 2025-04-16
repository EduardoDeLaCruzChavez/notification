#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "CommonFuntions.h"
#include "FileSystem.h"

void app_main(void)
{
    vInitNVSFlash();
    if (i8InitFileSystem())
    {
        vTaskDelay(2000 / portTICK_PERIOD_MS);
        esp_restart();
        return;
    }
}
