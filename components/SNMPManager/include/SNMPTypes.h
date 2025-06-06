#ifndef SNMP_TYPES_H__
#define SNMP_TYPES_H__

#include "lwip/sockets.h"

#define PORT 161

typedef enum TYPES_SNMP
{
    eTYPE_UNDEF = 0,
    eTYPE_INTEGER = 0x02,
    eTYPE_STRING = 0x04,
    eTYPE_NULL = 0x05,
    eTYPE_ODI = 0x06,
    eTYPE_SECUENCE = 0x30,
    eTYPE_NETWORK_ADDRESS = 0x40,
    eTYPE_COUNTER = 0x41,
    eTYPE_GAUGE = 0x42,
    eTYPE_TIMESTAMP = 0x43,
    eTYPE_OPAQUE = 0x44
} ENUM_TYPES_SNMP;

typedef enum QUERY_SNMP
{
    eGET_PDU = 0xA0,
    eGET_NEXT_REQUEST_PDU = 0xA1,
    eGET_RESPONSE_PDU = 0xA2,
    eSET_REQUEST_PDU = 0xA3,
    eTRAP_PDU = 0xA4,
} ENUM_QUERY_SNMP;

typedef enum ERROR_SNMP
{
    eSNMP_ERROR_OK = 0,
    eSNMP_ERROR_LONG,
    eSNMP_ERROR_ODI,
    eSNMP_ERROR_VALUE,
    eSNMP_ERROR_RONLY,
    eSNMP_ERROR_GENERIC
} ENUM_ERROR_SNMP;

typedef enum STATE_SNMP
{
    eSNMP_INIT,
    eSNMP_START,
    eSNMP_VERSION,
    eSNMP_COMUNITY,
    eSNMP_TYPE_PDU,
    eSNMP_REQUEST_ID,
    eSNMP_ERROR_,
    eSNMP_ERROR_INDEX,
    eSNMP_BIND_LIST,
    eSNMP_BIND,
    eSNMP_RESPONSE_ODI,
    eSNMP_MAX_STATE,
} ENUM_STATE_SNMP;

typedef enum RESMPONSE_SNMP
{
    eRESPONSE_STR,
    eRESPONSE_INT,
    eRESPONSE_NULL,
    eRESPONSE_MAX,
} ENUM_RESMPONSE_SNMP;

typedef struct TYPE_SNMP_QUERY
{
    uint8_t u8Header;
    uint8_t u8Len;
    uint16_t u16Type;
    uint8_t u8Version;
    uint8_t au8Buff[128];
    uint16_t u8Index;
    uint8_t u8StartOdi;
} TYPE_SNMP_QUERY;

typedef struct TYPE_RESPONSE_STR
{
    char acBuffer[23];
    uint8_t u8Len;
} TYPE_RESPONSE_STR;

typedef struct TYPE_RESPONSE_ODI
{
    uint8_t au8ODI[32];
    uint8_t u8Len;
} TYPE_RESPONSE_ODI;

typedef struct TYPE_RESPONSE
{
    ENUM_TYPES_SNMP eType;
    void *ptResponse;
    struct TYPE_RESPONSE *ptNext;
} TYPE_RESPONSE;

typedef struct TYPE_RESPONSE_QUERY
{
    TYPE_RESPONSE_ODI tODI;
    TYPE_RESPONSE *ptResponse;
} TYPE_RESPONSE_QUERY;

typedef struct TYPE_SNMP_SESION
{
    struct sockaddr_in tDestAddr;
    int iSocket;
} TYPE_SNMP_SESION;

#define HEADER_SIZE sizeof(uint8_t) + sizeof(uint8_t) + sizeof(uint16_t)
#define UINT32_SIZE sizeof(uint32_t)
#define UINT16_SIZE sizeof(uint16_t)
#define UINT8_SIZE sizeof(uint8_t)
#endif