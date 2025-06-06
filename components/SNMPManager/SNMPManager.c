#include <stdio.h>
#include "SNMPManager.h"
#include "SNMPBasic.h"
#include "SNMPResponse.h"

const char *TAG = "SNMP Manager";

static int8_t i8SnmpQueryChar(TYPE_SNMP_QUERY *ptSNMPQuery, const char *pcType, const char *pcODI, ENUM_QUERY_SNMP tQueryType);
static uint8_t u8SnmpHeader(TYPE_SNMP_QUERY *ptSNMPQuery, const char *pcType, ENUM_QUERY_SNMP tQueryType);
static void vSnmpFooter(TYPE_SNMP_QUERY *ptSNMPQuery, uint8_t u8IndexPDU);

void vInitSNMP(char *pcIPHost, TYPE_SNMP_SESION *ptSesion)
{

    struct timeval tTimeOut = {0};

    if (pcIPHost == NULL || ptSesion == NULL)
    {
        return;
    }
    bzero(&ptSesion->tDestAddr, sizeof(ptSesion->tDestAddr));

    ptSesion->tDestAddr.sin_addr.s_addr = inet_addr(pcIPHost);
    ptSesion->tDestAddr.sin_family = AF_INET;
    ptSesion->tDestAddr.sin_port = htons(PORT);

    ptSesion->iSocket = socket(AF_INET, SOCK_DGRAM, IPPROTO_IP);

    if (ptSesion->iSocket < 0)
    {
        ESP_LOGE(TAG, "Unable to create socket: errno %d", errno);
        return;
    }

    tTimeOut.tv_sec = 5;
    tTimeOut.tv_usec = 0;

    setsockopt(ptSesion->iSocket, SOL_SOCKET, SO_RCVTIMEO, &tTimeOut, sizeof(tTimeOut));

    ESP_LOGI(TAG, "Socket created, sending to %s:%d", pcIPHost, PORT);
}

void vDeInitSNMP(TYPE_SNMP_SESION *ptSesion)
{
    if (ptSesion == NULL || ptSesion->iSocket < 0)
    {
        return;
    }
    shutdown(ptSesion->iSocket, SHUT_RDWR);
    close(ptSesion->iSocket);
}
void vPrintQuery(TYPE_SNMP_QUERY *ptSNMPQuery)
{
    char *pc = NULL;
    uint16_t u8Index = 0;

    if (ptSNMPQuery == NULL)
    {
        return;
    }

    pc = (char *)ptSNMPQuery;

    for (; u8Index <= ptSNMPQuery->u8Len + 1; u8Index++)
    {
        printf("%02X ", (char)*pc);
        pc++;
    }
    printf("\n");
}

void vSendQuery(TYPE_SNMP_SESION *ptSesion, TYPE_SNMP_QUERY *ptQuery)
{
    int iError = 0;

    if (ptSesion == NULL || ptQuery == NULL)
    {
        return;
    }

    iError = sendto(ptSesion->iSocket, ptQuery, ptQuery->u8Len + 2, 0, (struct sockaddr *)&ptSesion->tDestAddr, sizeof(ptSesion->tDestAddr));
    // ESP_LOGE(TAG, "Len %0X", (int)ptQuery->u8Len + 1);
    if (iError < 0)
    {
        ESP_LOGE(TAG, "Error occurred during sending: errno %d", errno);
        return;
    }
}

int iRecvQuery(TYPE_SNMP_SESION *ptSesion, TYPE_SNMP_QUERY *ptQueryRecv)
{
    struct sockaddr_storage tSourceAddr = {0};
    socklen_t tSockLen = sizeof(tSourceAddr);
    int iLen = -1;

    if (ptSesion == NULL || ptQueryRecv == NULL)
    {
        return iLen;
    }

    iLen = recvfrom(ptSesion->iSocket, ptQueryRecv, sizeof(*ptQueryRecv), 0, (struct sockaddr *)&tSourceAddr, &tSockLen);

    if (iLen > 0)
    {
        ESP_LOGI(TAG, "Received %d bytes", iLen);
        // vPrintQuery(ptQueryRecv);
    }

    return iLen;
}

static uint8_t u8SnmpHeader(TYPE_SNMP_QUERY *ptSNMPQuery, const char *pcType, ENUM_QUERY_SNMP tQueryType)
{
    static uint32_t u32ResquestID = 1;
    uint8_t u8IndexPDU = 0;

    ptSNMPQuery->u8Header = 0x30;
    ptSNMPQuery->u16Type = 0x0102;
    ptSNMPQuery->u8Version = 0;
    ptSNMPQuery->u8Index = 0;

    i8SetString(ptSNMPQuery, pcType);
    ptSNMPQuery->au8Buff[ptSNMPQuery->u8Index++] = tQueryType;
    // Index Len Request
    u8IndexPDU = ptSNMPQuery->u8Index++;
    i8SetInteger(ptSNMPQuery, u32ResquestID, UINT32_SIZE);
    // Set ERROR
    i8SetInteger(ptSNMPQuery, 0, UINT8_SIZE);
    // Set ERROR INDEX
    i8SetInteger(ptSNMPQuery, 0, UINT8_SIZE);
    u32ResquestID++;

    return u8IndexPDU;
}

static void vSnmpFooter(TYPE_SNMP_QUERY *ptSNMPQuery, uint8_t u8IndexPDU)
{
    // Set Len PDU
    ptSNMPQuery->au8Buff[u8IndexPDU] = ptSNMPQuery->u8Index - u8IndexPDU - 1;
    ptSNMPQuery->u8Len = HEADER_SIZE + ptSNMPQuery->u8Index - 1;
    return;
}

static int8_t i8SnmpQueryChar(TYPE_SNMP_QUERY *ptSNMPQuery, const char *pcType, const char *pcODI, ENUM_QUERY_SNMP tQueryType)
{

    uint8_t u8IndexPDU = 0;

    if (ptSNMPQuery == NULL || pcType == NULL || pcODI == NULL)
    {
        return -1;
    }
    u8IndexPDU = u8SnmpHeader(ptSNMPQuery, pcType, tQueryType);

    i8SetODI(ptSNMPQuery, pcODI);
    vSnmpFooter(ptSNMPQuery, u8IndexPDU);

    // vPrintQuery(ptSNMPQuery);
    return 0;
}

static int8_t i8SnmpQueryTypeODI(TYPE_SNMP_QUERY *ptSNMPQuery, const char *pcType, TYPE_RESPONSE_ODI *ptODI, ENUM_QUERY_SNMP tQueryType)
{

    uint16_t u8IndexPDU = 0;

    if (ptSNMPQuery == NULL || pcType == NULL || ptODI == NULL)
    {
        return -1;
    }
    u8IndexPDU = u8SnmpHeader(ptSNMPQuery, pcType, tQueryType);

    i8SetODITypeODI(ptSNMPQuery, ptODI);
    vSnmpFooter(ptSNMPQuery, u8IndexPDU);
    // vPrintQuery(ptSNMPQuery);
    return 0;
}

int8_t i8ReadResponse(TYPE_SNMP_QUERY *ptSNMPRes, TYPE_RESPONSE_QUERY *ptQueryRes)
{
    ENUM_STATE_SNMP eState = eSNMP_START;
    // uint16_t u16MaxIndex = 0;
    uint8_t u8Index = 0;
    uint32_t u32Buffer = 0;
    char acBuffer[64] = {0};

    if (ptSNMPRes == NULL || ptQueryRes == NULL)
    {
        return -1;
    }

    // u8MaxIndex = ptSNMPRes->u8Index + 2;
    ptSNMPRes->u8Index = 0;
    while (eState != eSNMP_MAX_STATE)
    {
        switch (eState)
        {
        case eSNMP_START:
        {
            if (ptSNMPRes->u8Header != eTYPE_SECUENCE)
            {
                return eState;
            }
            u8Index += 2;
            break;
        }
        case eSNMP_VERSION:
        {
            if (ptSNMPRes->u16Type != 0x0102 || ptSNMPRes->u8Version != 0x00)
            {
                return eState;
            }
            u8Index += 3;
            break;
        }
        case eSNMP_COMUNITY:
        {
            if (ptSNMPRes->au8Buff[ptSNMPRes->u8Index++] != eTYPE_STRING)
            {
                return eState;
            }
            i8GetString(ptSNMPRes, acBuffer, sizeof(acBuffer));
            // ESP_LOGI(TAG, "Comunity: %s", acBuffer);
            break;
        }
        case eSNMP_TYPE_PDU:
        {
            if (ptSNMPRes->au8Buff[ptSNMPRes->u8Index++] != eGET_RESPONSE_PDU)
            {
                return eState;
            }
            ptSNMPRes->u8Index++;
            break;
        }
        case eSNMP_REQUEST_ID:
        {
            if (ptSNMPRes->au8Buff[ptSNMPRes->u8Index++] != eTYPE_INTEGER)
            {
                return eState;
            }
            u32Buffer = u32GetInteger(ptSNMPRes);
            // ESP_LOGI(TAG, "Request ID: %" PRIu32 "", u32Buffer);
            break;
        }
        case eSNMP_ERROR_:
        {
            if (ptSNMPRes->au8Buff[ptSNMPRes->u8Index++] != eTYPE_INTEGER)
            {
                return eState;
            }
            u32Buffer = u32GetInteger(ptSNMPRes);
            // ESP_LOGI(TAG, "Error: %" PRIu32 "", u32Buffer);
            break;
        }
        case eSNMP_ERROR_INDEX:
        {
            if (ptSNMPRes->au8Buff[ptSNMPRes->u8Index++] != eTYPE_INTEGER)
            {
                return eState;
            }
            u32Buffer = u32GetInteger(ptSNMPRes);
            if (u32Buffer != 0)
            {
                // ESP_LOGI(TAG, "Error Index: %" PRIu32 "", u32Buffer);
                return -1;
            }
            break;
        }
        case eSNMP_BIND_LIST:
        case eSNMP_BIND:
        {
            if (ptSNMPRes->au8Buff[ptSNMPRes->u8Index++] != eTYPE_SECUENCE)
            {
                return eState;
            }
            ptSNMPRes->u8Index++;
            break;
        }
        case eSNMP_RESPONSE_ODI:
        {
            if (ptSNMPRes->au8Buff[ptSNMPRes->u8Index++] != eTYPE_ODI)
            {
                return eState;
            }
            i8GetODI(ptSNMPRes, &ptQueryRes->tODI);

            /*uint8_t u8Index = 0;
            for (; u8Index < ptQueryRes->tODI.u8Len; u8Index++)
            {
                printf("%02X ", (char)ptQueryRes->tODI.au8ODI[u8Index]);
            }
            printf("\n");*/
            break;
        }
        default:
            break;
        }
        eState++;
    }

    switch (ptSNMPRes->au8Buff[ptSNMPRes->u8Index++])
    {

    case eTYPE_STRING:
    {
        TYPE_RESPONSE_STR *ptSTR = NULL;
        ptSTR = calloc(1, sizeof(TYPE_RESPONSE_STR));

        if (ptSTR == NULL)
        {
            break;
        }

        ptSTR->u8Len = i8GetString(ptSNMPRes, ptSTR->acBuffer, sizeof(ptSTR->acBuffer));
        vInsertResponse(ptQueryRes->ptResponse, ptSTR, eTYPE_STRING);
        break;
    }
    default:
        return -1;
        break;
    }
    return 0;
}

int8_t i8SnmpGetNext(char *pcIPHost, const char *pcType, const char *pcODI, TYPE_RESPONSE *ptResponse)
{
    TYPE_SNMP_SESION tSesion = {0};
    TYPE_SNMP_QUERY tQueryRecv = {0};
    TYPE_SNMP_QUERY tQuerySend = {0};
    TYPE_RESPONSE_QUERY tQueryRes = {0};

    uint8_t u8Error = 0;
    if (pcIPHost == NULL || pcType == NULL || pcODI == NULL || ptResponse == NULL)
    {
        return -1;
    }

    ptResponse->eType = eTYPE_UNDEF;
    ptResponse->ptNext = NULL;
    ptResponse->ptResponse = NULL;

    tQueryRes.ptResponse = ptResponse;
    vInitSNMP(pcIPHost, &tSesion);

    i8SnmpQueryChar(&tQuerySend, pcType, pcODI, (ENUM_QUERY_SNMP)eGET_NEXT_REQUEST_PDU);

    do
    {
        vSendQuery(&tSesion, &tQuerySend);
        iRecvQuery(&tSesion, &tQueryRecv);
        u8Error = i8ReadResponse(&tQueryRecv, &tQueryRes);
        if (u8Error != 0)
        {
            ESP_LOGI(TAG, "Error %d", (int)u8Error);
            break;
        }

        i8SnmpQueryTypeODI(&tQuerySend, pcType, &tQueryRes.tODI, (ENUM_QUERY_SNMP)eGET_NEXT_REQUEST_PDU);
    } while (1);
    vDeInitSNMP(&tSesion);
    return 1;
}

int8_t i8SnmpGet(const char *pcType, const char *pcODI)
{
    return i8SnmpQueryChar(NULL, pcType, pcODI, (ENUM_QUERY_SNMP)eGET_PDU);
}