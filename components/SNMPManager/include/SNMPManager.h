#ifndef SNMP_MANAGER_H__
#define SNMP_MANAGER_H__

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_system.h"
#include "esp_log.h"

#include "lwip/err.h"
#include "lwip/sockets.h"
#include "lwip/sys.h"
#include "SNMPTypes.h"

void vInitSNMP(char *pcIPHost, TYPE_SNMP_SESION *ptSesion);
int8_t i8SnmpGet(const char *pcType, const char *pcODI);
int8_t i8SnmpGetNext(char *pcIPHost, const char *pcType, const char *pcODI, TYPE_RESPONSE *ptRespons);

#endif