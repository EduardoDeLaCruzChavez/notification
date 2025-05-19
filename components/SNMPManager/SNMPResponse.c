#include "SNMPResponse.h"

void vInsertResponse(TYPE_RESPONSE *ptRespose, void *pvResponse, ENUM_TYPES_SNMP eType)
{
    TYPE_RESPONSE *ptResNext = NULL;
    TYPE_RESPONSE *ptNewRes = NULL;

    if (ptRespose == NULL || pvResponse == NULL)
    {
        return;
    }
    ptResNext = ptRespose;

    while (ptResNext->ptNext != NULL)
    {
        ptResNext = ptResNext->ptNext;
    }

    ptResNext->eType = eType;
    ptResNext->ptResponse = pvResponse;

    ptNewRes = (TYPE_RESPONSE *)malloc(sizeof(TYPE_RESPONSE));

    if (ptNewRes == NULL)
    {
        return;
    }

    ptNewRes->ptNext = NULL;
    ptNewRes->ptResponse = NULL;
    ptNewRes->eType = 0;
    ptResNext->ptNext = ptNewRes;
    return;
}

void vReadResponse(TYPE_RESPONSE *ptRespose)
{
    TYPE_RESPONSE *ptResNext = NULL;

    if (ptRespose == NULL)
    {
        return;
    }
    ptResNext = ptRespose;
    while (ptResNext->ptResponse != NULL)
    {
        switch (ptResNext->eType)
        {
        case eTYPE_STRING:
        {
            TYPE_RESPONSE_STR *ptSTR = NULL;
            ptSTR = ptResNext->ptResponse;
            // printf("STR %p\n", ptSTR);
            // printf("RES %p\n", ptResNext);
            ESP_LOG_BUFFER_HEX("MAC", ptSTR->acBuffer, ptSTR->u8Len);
            break;
        }
        default:
            break;
        }
        ptResNext = ptResNext->ptNext;
    }
}

void vFreeResponse(TYPE_RESPONSE *ptRespose)
{
    TYPE_RESPONSE *ptResNext = NULL;
    TYPE_RESPONSE *ptResAnt = NULL;

    if (ptRespose == NULL)
    {
        return;
    }
    free(ptRespose->ptResponse);

    // printf("STR %p\n", ptRespose->ptResponse);
    // printf("RES %p\n", ptRespose);
    ptResNext = ptRespose->ptNext;
    ptRespose->ptNext = NULL;

    while (ptResNext->ptNext != NULL)
    {
        ptResAnt = ptResNext;
        ptResNext = ptResNext->ptNext;
        // printf("STR %p\n", ptResAnt->ptResponse);
        // printf("RES %p\n", ptResAnt);
        free(ptResAnt->ptResponse);
        free(ptResAnt);
    }
}