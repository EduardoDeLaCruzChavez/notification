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
        char acRawMac[6] = {0};

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
                    if (strlen(row->values[f]) == 12)
                    {
                        vConverRawByte(row->values[f], acRawMac);
                        vInsertClient(ptClients, acRawMac);
                        bzero(acRawMac, sizeof(acRawMac));
                    }
                }

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
        const char *pcFormatClient = "UPDATE Device SET DStatus='%s' WHERE DName = '%02X%02X%02X%02X%02X%02X'";
        const char *pcONLINE = "ON";
        const char *pcOFFLINE = "OFF";
        const char *pcState = pcOFFLINE;
        char acBuff[128] = {0};

        if (ptDBInfo == NULL || ptClientList == NULL)
        {
            return;
        }

        if (tSQLConnect.connect(ptDBInfo->acHost, ptDBInfo->u16Port, ptDBInfo->acUser, ptDBInfo->acPssd, ptDBInfo->pcDB) == true)
        {
            ESP32_MySQL_Query query_mem = ESP32_MySQL_Query(&tSQLConnect);
            ptNextClient = &ptClientList->tClient;

            while (ptNextClient != NULL)
            {
                if (tSQLConnect.connected() == false)
                {
                    return;
                }

                if (ptNextClient->eClientState != eCLIENT_OFFLINE)
                {
                    pcState = pcONLINE;
                }

                snprintf(acBuff, sizeof(acBuff), pcFormatClient, pcState,
                         (unsigned char)ptNextClient->acMAC[0],
                         (unsigned char)ptNextClient->acMAC[1],
                         (unsigned char)ptNextClient->acMAC[2],
                         (unsigned char)ptNextClient->acMAC[3],
                         (unsigned char)ptNextClient->acMAC[4],
                         (unsigned char)ptNextClient->acMAC[5]);
                printf("%s\n", acBuff);
                query_mem.execute(acBuff);
                ptNextClient = ptNextClient->ptNextClient;
                pcState = pcOFFLINE;
            }
            tSQLConnect.close(); // close the connection
        }
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