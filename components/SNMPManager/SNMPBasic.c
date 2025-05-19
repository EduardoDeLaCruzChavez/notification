#include <stdio.h>
#include <string.h>
#include "SNMPBasic.h"

int8_t i8SetString(TYPE_SNMP_QUERY *ptSNMP, const char *pcString)
{

    if (ptSNMP == NULL || pcString == NULL)
    {
        return -1;
    }

    ptSNMP->au8Buff[ptSNMP->u8Index++] = eTYPE_STRING;
    ptSNMP->au8Buff[ptSNMP->u8Index++] = strlen(pcString);

    while (*pcString != '\0')
    {
        ptSNMP->au8Buff[ptSNMP->u8Index++] = *pcString++;
    }

    return 0;
}

int8_t i8GetString(TYPE_SNMP_QUERY *ptSNMP, char *pcString, uint8_t u8Size)
{
    uint8_t u8Index = 0;
    uint8_t u8Max = 0;

    if (ptSNMP == NULL || pcString == NULL)
    {
        return -1;
    }

    u8Max = ptSNMP->au8Buff[ptSNMP->u8Index++];
    for (; u8Index < u8Max && u8Index < u8Size; u8Index++, pcString++)
    {
        *pcString = ptSNMP->au8Buff[ptSNMP->u8Index++];
    }

    if (u8Index < u8Size)
    {
        *pcString = 0;
    }

    return u8Index;
}

int8_t i8SetInteger(TYPE_SNMP_QUERY *ptSNMP, uint32_t u32Value, int8_t i8Size)
{
    int8_t i8Byte = 0;

    if (ptSNMP == NULL || i8Size > 4)
    {
        return -1;
    }

    ptSNMP->au8Buff[ptSNMP->u8Index++] = eTYPE_INTEGER;
    ptSNMP->au8Buff[ptSNMP->u8Index++] = i8Size;

    for (i8Byte = i8Size - 1; i8Byte >= 0; i8Byte--)
    {
        ptSNMP->au8Buff[ptSNMP->u8Index++] = (uint8_t)(u32Value >> (8 * i8Byte)) & (0xFF);
    }

    return 0;
}

uint32_t u32GetInteger(TYPE_SNMP_QUERY *ptSNMP)
{
    uint32_t u32Result = 0;
    int8_t i8Size = 0;

    if (ptSNMP == NULL)
    {
        return 0;
    }

    i8Size = ptSNMP->au8Buff[ptSNMP->u8Index++] - 1;

    for (; i8Size >= 0; i8Size--)
    {
        u32Result |= (ptSNMP->au8Buff[ptSNMP->u8Index++] << (8 * i8Size));
    }

    return u32Result;
}

int8_t i8SetODI(TYPE_SNMP_QUERY *ptSNMP, const char *pcODI)
{
    uint8_t u8IxVarList = 0,
            u8IxVarBind = 0,
            u8IxODI = 0;
    uint8_t u8Index = 0, u8InitODI = 0,
            u8Max = 0;
    char acCopy[32] = {0};
    uint16_t au8Values[24] = {0};
    char *pToken = NULL;

    if (ptSNMP == NULL || pcODI == NULL)
    {
        return -1;
    }

    ptSNMP->au8Buff[ptSNMP->u8Index++] = eTYPE_SECUENCE;
    u8IxVarList = ptSNMP->u8Index++;

    ptSNMP->au8Buff[ptSNMP->u8Index++] = eTYPE_SECUENCE;
    u8IxVarBind = ptSNMP->u8Index++;

    ptSNMP->au8Buff[ptSNMP->u8Index++] = eTYPE_ODI;
    u8IxODI = ptSNMP->u8Index++;

    snprintf(acCopy, sizeof(acCopy), "%s", pcODI);
    pToken = strtok(acCopy, ".");
    while (pToken != NULL)
    {

        au8Values[u8Max++] = atoi(pToken);

        pToken = strtok(NULL, ".");
    }

    if (u8Max > 2)
    {
        u8InitODI = (au8Values[0]) * 40;
        u8InitODI += (au8Values[1]);
    }

    ptSNMP->u8StartOdi = ptSNMP->u8Index;
    ptSNMP->au8Buff[ptSNMP->u8Index++] = u8InitODI;
    for (u8Index = 2; u8Index < u8Max; u8Index++)
    {
        ptSNMP->au8Buff[ptSNMP->u8Index++] = au8Values[u8Index];
    }
    // Set Len ODI
    ptSNMP->au8Buff[u8IxODI] = ptSNMP->u8Index - u8IxODI - 1;

    // Set Null Query
    ptSNMP->au8Buff[ptSNMP->u8Index++] = eTYPE_NULL;
    ptSNMP->au8Buff[ptSNMP->u8Index++] = 0;

    ptSNMP->au8Buff[u8IxVarList] = ptSNMP->u8Index - u8IxVarList - 1;
    ptSNMP->au8Buff[u8IxVarBind] = ptSNMP->u8Index - u8IxVarBind - 1;
    return 0;
}

int8_t i8SetODITypeODI(TYPE_SNMP_QUERY *ptSNMP, TYPE_RESPONSE_ODI *ptODI)
{
    uint8_t u8IxVarList = 0,
            u8IxVarBind = 0,
            u8IxODI = 0;
    uint8_t u8Index = 0;

    if (ptSNMP == NULL || ptODI == NULL)
    {
        return -1;
    }

    ptSNMP->au8Buff[ptSNMP->u8Index++] = eTYPE_SECUENCE;
    u8IxVarList = ptSNMP->u8Index++;

    ptSNMP->au8Buff[ptSNMP->u8Index++] = eTYPE_SECUENCE;
    u8IxVarBind = ptSNMP->u8Index++;

    ptSNMP->au8Buff[ptSNMP->u8Index++] = eTYPE_ODI;
    u8IxODI = ptSNMP->u8Index++;

    ptSNMP->u8StartOdi = ptSNMP->u8Index;

    for (u8Index = 0; u8Index < ptODI->u8Len; u8Index++)
    {
        ptSNMP->au8Buff[ptSNMP->u8Index++] = ptODI->au8ODI[u8Index];
    }
    // Set Len ODI
    ptSNMP->au8Buff[u8IxODI] = ptSNMP->u8Index - u8IxODI - 1;

    // Set Null Query
    ptSNMP->au8Buff[ptSNMP->u8Index++] = eTYPE_NULL;
    ptSNMP->au8Buff[ptSNMP->u8Index++] = 0;

    ptSNMP->au8Buff[u8IxVarList] = ptSNMP->u8Index - u8IxVarList - 1;
    ptSNMP->au8Buff[u8IxVarBind] = ptSNMP->u8Index - u8IxVarBind - 1;
    return 0;
}

int8_t i8GetODI(TYPE_SNMP_QUERY *ptSNMP, TYPE_RESPONSE_ODI *ptODI)
{
    uint8_t u8Index = 0;
    uint8_t u8Max = 0;

    if (ptSNMP == NULL || ptODI == NULL)
    {
        return -1;
    }

    u8Max = ptSNMP->au8Buff[ptSNMP->u8Index++];
    for (; u8Index < u8Max && u8Index < sizeof(ptODI->au8ODI); u8Index++)
    {
        ptODI->au8ODI[u8Index] = ptSNMP->au8Buff[ptSNMP->u8Index++];
    }

    ptODI->u8Len = u8Index;
    return 0;
}
