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
#include "nvs_flash.h"
#include "nvs.h"

int8_t i8InitFileSystem(void);
int8_t i8FileExist(const char *pacFilename);

void vClearKey(char *pcKey);
void vSetKey(char *pckey, char *pcValue);
void vGetKey(char *pckey, char *pcValue, int iSize);
void vGetBlock(char *pckey, void *pvBuff, int iSize);
void vSetBlock(char *pckey, void *pvBuff, int iSize);
void vClearAllNVS();
#endif