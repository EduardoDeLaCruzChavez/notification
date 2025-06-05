#ifndef CLIENT_MAC_H__
#define CLIENT_MAC_H__
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"
#include "SNMPTypes.h"

typedef enum
{
    eCLIENT_UNDEF,
    eCLIENT_ONLINE,
    eCLIENT_OFFLINE,
    eCLIENT_RECONNECT,
    eCLIENT_MAX
} ENUM_CLIENT_STATE;

typedef struct TYPE_CLIENT_MAC
{
    char acMAC[6];
    ENUM_CLIENT_STATE eClientState;
    int8_t s8TimeOff;
    struct TYPE_CLIENT_MAC *ptNextClient;
} TYPE_CLIENT_MAC;

typedef struct TYPE_CLIENTS
{
    int8_t s8Reconnect;
    int8_t s8Clients;
    TYPE_CLIENT_MAC tClient;
} TYPE_CLIENTS;

void vInsertClient(TYPE_CLIENTS *ptClients, char *pcClietMac);
void vSetOffClient(TYPE_CLIENTS *ptClients);
int8_t s8SearchClient(TYPE_CLIENTS *ptClients, char *pcClietMac);
int8_t bReadMacClient(FILE *pFile, void *pvBuffer, int8_t s8Size);
void vAppendMacClient(TYPE_CLIENTS *ptClients, char *pcClientMac);
void vGetClientList(TYPE_CLIENTS *ptClients);
void vUpdateClient(TYPE_CLIENTS *ptClients, TYPE_RESPONSE *ptResponse);
void vConverRawByte(char *pcMac, char *pcRawMac);
#endif