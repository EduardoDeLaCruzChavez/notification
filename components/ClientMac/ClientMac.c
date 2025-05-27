#include "ClientMac.h"
#include <string.h>
#include <stdlib.h>
#include "Directory.h"

#define MAX_TIME_OFF 5

void vInsertClient(TYPE_CLIENTS *ptClients, char *pcClietMac)
{
    TYPE_CLIENT_MAC *ptNext = NULL;
    TYPE_CLIENT_MAC *ptNewClient = NULL;

    if (ptClients == NULL)
    {
        return;
    }

    ptNext = &ptClients->tClient;
    while (ptNext->ptNextClient != NULL)
    {
        ptNext = ptNext->ptNextClient;
    }

    ptNewClient = (TYPE_CLIENT_MAC *)malloc(sizeof(TYPE_CLIENT_MAC));

    if (ptNewClient == NULL)
    {
        ESP_LOGE("", "Error al insertar nuevo cliente");
        return;
    }

    ptClients->s8Clients++;
    ptNewClient->ptNextClient = NULL;
    ptNewClient->s8TimeOff = 0;
    ptNewClient->eClientState = eCLIENT_UNDEF;
    memcpy(ptNewClient->acMAC, pcClietMac, sizeof(ptNewClient->acMAC));
    ptNext->ptNextClient = ptNewClient;
}

void vSetOffClient(TYPE_CLIENTS *ptClients)
{
    TYPE_CLIENT_MAC *ptNext = NULL;

    if (ptClients == NULL)
    {
        return;
    }

    ptNext = &ptClients->tClient;
    while (ptNext != NULL)
    {
        ptNext->eClientState = eCLIENT_OFFLINE;
        if (ptNext->s8TimeOff < MAX_TIME_OFF)
        {
            ptNext->s8TimeOff++;
        }
        ptNext = ptNext->ptNextClient;
    }
}

int8_t s8SearchClient(TYPE_CLIENTS *ptClients, char *pcClietMac)
{
    TYPE_CLIENT_MAC *ptNext = NULL;
    int8_t s8Index = 0;

    if (ptClients == NULL)
    {
        return -1;
    }

    ptNext = &ptClients->tClient;
    while (ptNext != NULL)
    {
        if (memcmp(ptNext->acMAC, pcClietMac, sizeof(ptNext->acMAC)) == 0)
        {
            return s8Index;
        }
        s8Index++;
        ptNext = ptNext->ptNextClient;
    }
    return -1;
}

int8_t bReadMacClient(FILE *pFile, void *pvBuffer, int8_t s8Size)
{
    size_t tSize = 0;

    tSize = fread(pvBuffer, sizeof(char), s8Size, pFile);
    return tSize == s8Size;
}

void vAppendMacClient(TYPE_CLIENTS *ptClients, char *pcClientMac)
{
    FILE *pFile = NULL;
    TYPE_CLIENT_MAC tMac;

    if (pcClientMac == NULL || pcClientMac == NULL)
    {
        return;
    }

    if (s8SearchClient(ptClients, pcClientMac) >= 0)
    {
        printf("Cliente existe\n");
        return;
    }

    pFile = fopen(CLIENT_LIST, "ab+");

    if (pFile == NULL)
    {
        printf("File NULL aapend\n");
        return;
    }

    fwrite(pcClientMac, sizeof(char), sizeof(tMac.acMAC), pFile);

    fclose(pFile);
    return;
}

void vGetClientList(TYPE_CLIENTS *ptClients)
{

    FILE *pFile = NULL;
    char acMACBuff[6] = {0};

    if (ptClients == NULL)
    {
        return;
    }

    pFile = fopen(CLIENT_LIST, "rb");
    if (pFile == NULL)
    {
        printf("File NULL list\n");
        return;
    }

    while (bReadMacClient(pFile, acMACBuff, sizeof(acMACBuff)))
    {
        ESP_LOG_BUFFER_HEX("MAC", acMACBuff, sizeof(acMACBuff));
        vInsertClient(ptClients, acMACBuff);
        ptClients->s8Clients++;
    }

    fclose(pFile);
}

void vSetOnlineClient(TYPE_CLIENTS *ptClients, int8_t s8Pos)
{
    TYPE_CLIENT_MAC *ptNexMac = NULL;
    int8_t s8Index = 0;

    if (ptClients == NULL)
    {
        return;
    }

    if (s8Pos < 0 || s8Pos > ptClients->s8Clients)
    {
        return;
    }

    ptNexMac = &ptClients->tClient;
    while (ptNexMac != NULL)
    {
        if (s8Index == s8Pos)
        {
            break;
        }
        ptNexMac = ptNexMac->ptNextClient;
        s8Index++;
    }

    ptNexMac->eClientState = eCLIENT_ONLINE;
    if (ptNexMac->s8TimeOff >= MAX_TIME_OFF)
    {
        ESP_LOGI("Cliente", "Cliente reconectado");
        ptNexMac->eClientState = eCLIENT_RECONNECT;
        ptClients->s8Reconnect++;
    }
    ESP_LOG_BUFFER_HEX("Cliente", ptNexMac->acMAC, sizeof(ptNexMac->acMAC));
    ptNexMac->s8TimeOff = 0;
}

void vUpdateClient(TYPE_CLIENTS *ptClients, TYPE_RESPONSE *ptResponse)
{
    TYPE_RESPONSE *ptNextRes = NULL;
    int8_t s8Index = 0;

    if (ptClients == NULL || ptResponse == NULL)
    {
        return;
    }
    ptNextRes = ptResponse;
    vSetOffClient(ptClients);

    while (ptNextRes != NULL)
    {

        if (ptNextRes->eType == eTYPE_STRING)
        {
            s8Index = s8SearchClient(ptClients, ptNextRes->ptResponse);
            vSetOnlineClient(ptClients, s8Index);
        }
        ptNextRes = ptNextRes->ptNext;
    }
    return;
}