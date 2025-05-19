#ifndef SNMP_RESPONSE_H__

#include <stdlib.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_system.h"
#include "esp_log.h"
#include "SNMPTypes.h"

void vInsertResponse(TYPE_RESPONSE *ptRespose, void *pvResponse, ENUM_TYPES_SNMP eType);
void vReadResponse(TYPE_RESPONSE *ptRespose);
void vFreeResponse(TYPE_RESPONSE *ptRespose);

#endif