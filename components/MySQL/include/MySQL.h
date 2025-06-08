#ifndef MY_SQL_H__
#define MY_SQL_H__
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"

typedef struct
{
    char acHost[64];
    char acUser[32];
    char acPssd[32];
    char *pcDB;
    uint16_t u16Port;
} TYPE_DB_INFO;

#ifdef __cplusplus
extern "C"
{
#endif

#include "ClientMac.h"
    void vClearDBInfo(TYPE_DB_INFO *ptDBInfo);
    void vInsertQuery(TYPE_DB_INFO *ptDBInfo, const char *query);
    void vReadSelect(TYPE_DB_INFO *ptDBInfo, char *pcQuery);
    void vReadClients(TYPE_DB_INFO *ptDBInfo, TYPE_CLIENTS *ptClients);
    void vUpdateStateClient(TYPE_DB_INFO *ptDBInfo, TYPE_CLIENTS *ptClientList);
    void vReadNotifyInfo(TYPE_DB_INFO *ptDBInfo, TYPE_BOT_INFO *ptBotInfo);
#ifdef __cplusplus
}
#endif
#endif
