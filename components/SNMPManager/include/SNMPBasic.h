#ifndef SNMP_BASIC_H__
#define SNMP_BASIC_H__
#include "SNMPTYPES.h"
#include <inttypes.h>

int8_t i8SetString(TYPE_SNMP_QUERY *ptSNMP, const char *pcString);

int8_t i8GetString(TYPE_SNMP_QUERY *ptSNMP, char *pcString, uint8_t u8Size);

int8_t i8SetInteger(TYPE_SNMP_QUERY *ptSNMP, uint32_t u32Value, int8_t i8Size);

uint32_t u32GetInteger(TYPE_SNMP_QUERY *ptSNMP);

int8_t i8SetODI(TYPE_SNMP_QUERY *ptSNMP, const char *pcODI);
int8_t i8SetODITypeODI(TYPE_SNMP_QUERY *ptSNMP, TYPE_RESPONSE_ODI *ptODI);
int8_t i8GetODI(TYPE_SNMP_QUERY *ptSNMP, TYPE_RESPONSE_ODI *ptODI);
#endif