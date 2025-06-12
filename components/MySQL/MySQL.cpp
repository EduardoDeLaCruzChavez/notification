#include <stdio.h>
#include "MySQL.h"
#include "Client.hpp"
#include "ESP32_MySQL_Query.hpp"
#include "ESP32_MySQL_Connection.hpp"
#ifdef __cplusplus
extern "C"
{
#endif

    void vClearDBInfo(TYPE_DB_INFO *ptDBInfo)
    {
        if (ptDBInfo == NULL)
        {
            return;
        }

        bzero(ptDBInfo->acHost, sizeof(ptDBInfo->acHost));
        bzero(ptDBInfo->acUser, sizeof(ptDBInfo->acUser));
        bzero(ptDBInfo->acPssd, sizeof(ptDBInfo->acPssd));
        ptDBInfo->u16Port = 0;
        ptDBInfo->pcDB = NULL;
    }

    void vInsertQuery(TYPE_DB_INFO *ptDBInfo, const char *query)
    {
        Client tClient = {};
        ESP32_MySQL_Connection tSQLConnect = ESP32_MySQL_Connection(&tClient);

        if (ptDBInfo == NULL)
        {
            return;
        }

        if (tSQLConnect.connect(ptDBInfo->acHost, ptDBInfo->u16Port, ptDBInfo->acUser, ptDBInfo->acPssd, ptDBInfo->pcDB) == true)
        {
            ESP32_MySQL_Query query_mem = ESP32_MySQL_Query(&tSQLConnect);

            if (tSQLConnect.connected())
            {
                // Execute the query
                if (!query_mem.execute(query))
                {
                    ESP_LOGI("MYSQL", "Error en conexion");
                }
                else
                {
                    ESP_LOGI("MYSQL", "Data Inserted.");
                }
            }
            tSQLConnect.close(); // close the connection
        }
    }

    void vReadSelect(TYPE_DB_INFO *ptDBInfo, char *pcQuery)
    {
        Client tClient = {};
        ESP32_MySQL_Connection tSQLConnect = ESP32_MySQL_Connection(&tClient);
        ESP32_MySQL_Query query_mem = ESP32_MySQL_Query(&tSQLConnect);

        const char *pFORMAT_VIEW_MAC = "SELECT * FROM VIEM_MAC_CLIENT";
        char acQuery[128] = {0};
        snprintf(acQuery, sizeof(acQuery), pFORMAT_VIEW_MAC, "");

        if (tSQLConnect.connect(ptDBInfo->acHost, ptDBInfo->u16Port, ptDBInfo->acUser, ptDBInfo->acPssd, ptDBInfo->pcDB) == false)
        {
            return;
        }

        if (!query_mem.execute(pFORMAT_VIEW_MAC))
        {
            return;
        }

        // Show the result
        // Fetch the columns and print them
        column_names *cols = query_mem.get_columns();

        for (int f = 0; f < cols->num_fields; f++)
        {
            printf("%s", cols->fields[f]->name);

            if (f < cols->num_fields - 1)
            {
                printf(",");
            }
        }
        printf("\n");
        // Read the rows and print them
        row_values *row = NULL;

        do
        {
            row = query_mem.get_next_row();

            if (row != NULL)
            {
                for (int f = 0; f < cols->num_fields; f++)
                {
                    printf("%s ", row->values[f]);
                    if (f < cols->num_fields - 1)
                    {
                        printf(",");
                    }
                }

                printf("\n");
            }
        } while (row != NULL);
        tSQLConnect.close(); // close the connection
    }

    void vReadClients(TYPE_DB_INFO *ptDBInfo, TYPE_CLIENTS *ptClients)
    {
        Client tClient = {};
        ESP32_MySQL_Connection tSQLConnect = ESP32_MySQL_Connection(&tClient);
        ESP32_MySQL_Query query_mem = ESP32_MySQL_Query(&tSQLConnect);

        const char *pFORMAT_VIEW_MAC = "SELECT * FROM VIEM_MAC_CLIENT";
        char acMac[13];
        char acNombre[16];
        if (tSQLConnect.connect(ptDBInfo->acHost, ptDBInfo->u16Port, ptDBInfo->acUser, ptDBInfo->acPssd, ptDBInfo->pcDB) == false)
        {
            return;
        }

        if (!query_mem.execute(pFORMAT_VIEW_MAC))
        {
            return;
        }

        // Show the result
        // Fetch the columns and print them
        column_names *cols = query_mem.get_columns();

        for (int f = 0; f < cols->num_fields; f++)
        {
            printf("%s", cols->fields[f]->name);

            if (f < cols->num_fields - 1)
            {
                printf(",");
            }
        }
        printf("\n");
        // Read the rows and print them
        row_values *row = NULL;

        do
        {
            row = query_mem.get_next_row();

            if (row != NULL)
            {
                bzero(acMac, sizeof(acMac));
                bzero(acNombre, sizeof(acNombre));

                for (int f = 0; f < cols->num_fields; f++)
                {
                    printf("%s ", row->values[f]);
                    if (f == 0)
                    {
                        snprintf(acMac, sizeof(acMac), "%s", row->values[f]);
                    }
                    else if (f == 1)
                    {
                        snprintf(acNombre, sizeof(acNombre), "%s", row->values[f]);
                    }
                }
                vInsertClient(ptClients, acMac, acNombre);
                printf("\n");
            }
        } while (row != NULL);
        tSQLConnect.close(); // close the connection
    }

    void vUpdateStateClient(TYPE_DB_INFO *ptDBInfo, TYPE_CLIENTS *ptClientList)
    {
        Client tClient = {};
        ESP32_MySQL_Connection tSQLConnect = ESP32_MySQL_Connection(&tClient);
        TYPE_CLIENT_MAC *ptNextClient = NULL;
        const char *pcFormatClient = "UPDATE Device SET DStatus='%s', DRssi = '%" PRId8 "' WHERE DName = '%s'";
        const char *pcONLINE = "Conectado";
        const char *pcOFFLINE = "Desconectado";
        const char *pcState = pcOFFLINE;
        char acBuff[128] = {0};

        if (ptDBInfo == NULL || ptClientList == NULL)
        {
            return;
        }

        if (tSQLConnect.connect(ptDBInfo->acHost, ptDBInfo->u16Port, ptDBInfo->acUser, ptDBInfo->acPssd, ptDBInfo->pcDB) == true)
        {
            ESP32_MySQL_Query query_mem = ESP32_MySQL_Query(&tSQLConnect);
            ptNextClient = ptClientList->ptClient;

            while (ptNextClient != NULL)
            {
                if (tSQLConnect.connected() == false)
                {
                    return;
                }

                if (ptNextClient->eClientState != eCLIENT_OFFLINE && ptNextClient->eClientState != eCLIENT_NEW_OFFLINE)
                {
                    pcState = pcONLINE;
                }

                if (ptNextClient->eClientState == eCLIENT_ONLINE && ptNextClient->s8RSSI == 0)
                {
                    ptNextClient = ptNextClient->ptNextClient;
                    continue;
                }

                snprintf(acBuff, sizeof(acBuff), pcFormatClient, pcState, ptNextClient->s8RSSI,
                         ptNextClient->acMAC);
                printf("%s\n", acBuff);
                query_mem.execute(acBuff);
                ptNextClient = ptNextClient->ptNextClient;
                pcState = pcOFFLINE;
            }
            tSQLConnect.close(); // close the connection
        }
    }

    void vReadNotifyInfo(TYPE_DB_INFO *ptDBInfo, TYPE_BOT_INFO *ptBotInfo)
    {
        Client tClient = {};
        ESP32_MySQL_Connection tSQLConnect = ESP32_MySQL_Connection(&tClient);
        ESP32_MySQL_Query query_mem = ESP32_MySQL_Query(&tSQLConnect);

        const char *pFORMAT_VIEW_CHAT_BOT = "SELECT * FROM `VIEW_CHAT_BOT`";

        if (tSQLConnect.connect(ptDBInfo->acHost, ptDBInfo->u16Port, ptDBInfo->acUser, ptDBInfo->acPssd, ptDBInfo->pcDB) == false)
        {
            return;
        }

        if (!query_mem.execute(pFORMAT_VIEW_CHAT_BOT))
        {
            return;
        }

        // Show the result
        // Fetch the columns and print them
        column_names *cols = query_mem.get_columns();

        for (int f = 0; f < cols->num_fields; f++)
        {
            printf("%s", cols->fields[f]->name);

            if (f < cols->num_fields - 1)
            {
                printf(",");
            }
        }
        printf("\n");
        // Read the rows and print them
        row_values *row = NULL;

        row = query_mem.get_next_row();

        if (row != NULL)
        {
            bzero(ptBotInfo->acKey, sizeof(ptBotInfo->acKey));
            printf("%s, %s\n", row->values[0], row->values[1]);

            snprintf(ptBotInfo->acKey, sizeof(ptBotInfo->acKey), "%s", row->values[0]);
            sscanf(row->values[1], "%lld", &ptBotInfo->s64ChatID);
            printf("%s, %lld\n", ptBotInfo->acKey, ptBotInfo->s64ChatID);
        }
        tSQLConnect.close(); // close the connection
    }

#ifdef __cplusplus
}
#endif

/*
CREATE TRIGGER `UpdateDevice` AFTER UPDATE ON `Device`
 FOR EACH ROW IF (NEW.DStatus != OLD.DStatus) AND EXISTS (
        SELECT 1 FROM User WHERE idUser = OLD.UidUser
    ) THEN
        INSERT INTO Event (DidUser, EState, datetime)
        VALUES (OLD.UidUser, NEW.DStatus, NOW());
    END IF
*/