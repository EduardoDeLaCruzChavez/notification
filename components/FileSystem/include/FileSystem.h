#ifndef FILE_SYSTEM_H__
#define FILE_SYSTEM_H__
#include <stdio.h>
#include <string.h>
#include <sys/unistd.h>
#include <sys/stat.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_err.h"
#include "esp_log.h"
#include "esp_spiffs.h"

int8_t i8InitFileSystem(void);

#endif