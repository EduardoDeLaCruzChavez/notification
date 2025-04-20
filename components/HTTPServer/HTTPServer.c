#include <stdio.h>
#include "HTTPServer.h"
#include "Directory.h"

static const char *TAG = "HTTP Server";
static httpd_handle_t tServer = NULL;

static esp_err_t tRootGetHandler(httpd_req_t *ptReq);
static esp_err_t tRestartGetHandler(httpd_req_t *ptReq);
static esp_err_t tConfigPostHandler(httpd_req_t *ptReq);
static esp_err_t tFinalPostHandler(httpd_req_t *ptReq);
static esp_err_t tFinalGetHandler(httpd_req_t *ptReq);
static esp_err_t tError404Handler(httpd_req_t *ptReq, httpd_err_code_t code);
static esp_err_t tRedirectHandler(httpd_req_t *ptReq, char *pURL);

static const httpd_uri_t tRootGet = {
    .uri = "/",
    .method = HTTP_GET,
    .handler = tRootGetHandler,
};

static const httpd_uri_t tConfigPost = {
    .uri = "/Config",
    .method = HTTP_POST,
    .handler = tConfigPostHandler,
};

static const httpd_uri_t tConfigGet = {
    .uri = "/Config",
    .method = HTTP_GET,
    .handler = tConfigPostHandler,
};

static const httpd_uri_t tFinalPost = {
    .uri = "/Final",
    .method = HTTP_POST,
    .handler = tFinalPostHandler,
};

static const httpd_uri_t tFinalGet = {
    .uri = "/Final",
    .method = HTTP_GET,
    .handler = tFinalGetHandler,
};

static const httpd_uri_t tRestartPost = {
    .uri = "/Restart",
    .method = HTTP_GET,
    .handler = tRestartGetHandler,
};

static esp_err_t tRootGetHandler(httpd_req_t *ptReq)
{
    char acBuffer[256] = {0};
    FILE *pFile = NULL;
    pFile = fopen(ROOT_HTML_FILE, "r");

    if (pFile == NULL)
    {
        return ESP_FAIL;
    }

    httpd_resp_set_type(ptReq, "text/html");
    while (fgets(acBuffer, sizeof(acBuffer), pFile) != NULL)
    {
        httpd_resp_send_chunk(ptReq, acBuffer, HTTPD_RESP_USE_STRLEN);
    }
    httpd_resp_send_chunk(ptReq, NULL, 0);

    fclose(pFile);

    return ESP_OK;
}

static esp_err_t tRestartGetHandler(httpd_req_t *ptReq)
{
    char acBuffer[256] = {0};
    FILE *pFile = NULL;
    pFile = fopen(RESTART_HTML_FILE, "r");

    if (pFile == NULL)
    {
        return ESP_FAIL;
    }

    while (fgets(acBuffer, sizeof(acBuffer), pFile) != NULL)
    {
        httpd_resp_send_chunk(ptReq, acBuffer, HTTPD_RESP_USE_STRLEN);
    }
    httpd_resp_send_chunk(ptReq, NULL, 0);
    fclose(pFile);

    vTaskDelay(5000 / portTICK_PERIOD_MS);
    esp_restart();
    return ESP_OK;
}

static esp_err_t tConfigPostHandler(httpd_req_t *ptReq)
{
    char acBuffer[256] = {0};
    char acSSID[32] = {0}, acPSSD[64] = {0};
    char acHostName[16] = {0}, acMode[8] = {0};
    int iReceived = 0;
    int iRemaining = ptReq->content_len;
    uint8_t u8Keys = 0;
    FILE *pFileConfig = NULL;
    FILE *pFileMode = NULL;

    if (iRemaining == 0)
    {
        return tRedirectHandler(ptReq, "/");
    }

    while (iRemaining > 0)
    {
        iReceived = httpd_req_recv(ptReq, acBuffer + (ptReq->content_len - iRemaining), iRemaining);
        if (iReceived <= 0)
        {
            httpd_resp_send_err(ptReq, HTTPD_500_INTERNAL_SERVER_ERROR, "Error al recibir datos");
            return ESP_FAIL;
        }
        iRemaining -= iReceived;
    }
    acBuffer[ptReq->content_len] = 0;

    ESP_LOGI(TAG, "Found URL query => %s", acBuffer);
    if (httpd_query_key_value(acBuffer, "SSID", acSSID, sizeof(acSSID)) == ESP_OK)
    {
        u8Keys |= (1 << 0);
    }
    if (httpd_query_key_value(acBuffer, "PSSD", acPSSD, sizeof(acPSSD)) == ESP_OK)
    {
        u8Keys |= (1 << 1);
    }
    if (httpd_query_key_value(acBuffer, "name", acHostName, sizeof(acHostName)) == ESP_OK)
    {
        u8Keys |= (1 << 2);
    }
    if (httpd_query_key_value(acBuffer, "mode", acMode, sizeof(acMode)) == ESP_OK)
    {
        u8Keys |= (1 << 3);
    }

    pFileConfig = fopen(WIFI_CONFIG_FILE, "w");
    pFileMode = fopen(MODE_CONFIG_FILE, "w");

    if (pFileConfig == NULL || pFileMode == NULL)
    {
        return ESP_FAIL;
    }

    fprintf(pFileConfig, "SSID: %s\n", acSSID);
    fprintf(pFileConfig, "PSSD: %s\n", acPSSD);
    fprintf(pFileConfig, "Name: %s\n", acHostName);
    fprintf(pFileMode, "Mode: %s", acMode);

    fclose(pFileConfig);
    fclose(pFileMode);

    httpd_register_uri_handler(tServer, &tFinalGet);
    tRedirectHandler(ptReq, "/Final");
    return ESP_OK;
}

static esp_err_t tFinalGetHandler(httpd_req_t *ptReq)
{
    char acBuffer[256] = {0};

    FILE *pFileHtml = NULL;
    pFileHtml = fopen(FINAL_HTML_FILE, "r");

    if (pFileHtml == NULL)
    {
        return ESP_FAIL;
    }

    httpd_resp_set_type(ptReq, "text/html");
    while (fgets(acBuffer, sizeof(acBuffer), pFileHtml) != NULL)
    {
        httpd_resp_send_chunk(ptReq, acBuffer, HTTPD_RESP_USE_STRLEN);
    }

    httpd_resp_send_chunk(ptReq, NULL, 0);

    fclose(pFileHtml);
    httpd_register_uri_handler(tServer, &tFinalPost);
    return ESP_OK;
}

static esp_err_t tFinalPostHandler(httpd_req_t *ptReq)
{
    char acBuffer[64] = {0};
    char acAccion[16] = {0};
    int iReceived = 0;
    int iRemaining = ptReq->content_len;

    if (iRemaining == 0)
    {
        return tRedirectHandler(ptReq, "/");
    }

    while (iRemaining > 0)
    {
        iReceived = httpd_req_recv(ptReq, acBuffer + (ptReq->content_len - iRemaining), iRemaining);
        if (iReceived <= 0)
        {
            httpd_resp_send_err(ptReq, HTTPD_500_INTERNAL_SERVER_ERROR, "Error al recibir datos");
            return ESP_FAIL;
        }
        iRemaining -= iReceived;
    }
    acBuffer[ptReq->content_len] = 0;

    if (httpd_query_key_value(acBuffer, "action", acAccion, sizeof(acAccion)) == ESP_OK)
    {
        if (strcmp(acAccion, "modify") == 0)
        {
            httpd_unregister_uri(tServer, "/Final");
            return tRedirectHandler(ptReq, "/");
        }
        else if (strcmp(acAccion, "restart") == 0)
        {

            httpd_register_uri_handler(tServer, &tRestartPost);
            return tRedirectHandler(ptReq, "/Restart");
        }
    }
    return ESP_OK;
}

static esp_err_t tError404Handler(httpd_req_t *ptReq, httpd_err_code_t code)
{
    char acBuffer[256] = {0};
    FILE *pFile = NULL;

    pFile = fopen(ERROR_HTML_FILE, "r");

    if (pFile == NULL)
    {
        return ESP_FAIL;
    }

    httpd_resp_set_status(ptReq, "404 Not Found");
    httpd_resp_set_type(ptReq, "text/html");
    while (fgets(acBuffer, sizeof(acBuffer), pFile) != NULL)
    {
        httpd_resp_send_chunk(ptReq, acBuffer, HTTPD_RESP_USE_STRLEN);
    }
    httpd_resp_send_chunk(ptReq, NULL, 0);

    fclose(pFile);

    return ESP_OK;
}

static esp_err_t tRedirectHandler(httpd_req_t *ptReq, char *pURL)
{
    httpd_resp_set_status(ptReq, "302 Found");
    if (pURL != NULL)
    {
        httpd_resp_set_hdr(ptReq, "Location", pURL);
    }
    else
    {
        httpd_resp_set_hdr(ptReq, "Location", "/");
    }
    return httpd_resp_send(ptReq, NULL, 0);
}

int8_t i8StartServer(void)
{
    esp_err_t tRet = 0;

    ESP_LOGI(TAG, "Starting http server");

    httpd_config_t conf = HTTPD_DEFAULT_CONFIG();
    conf.lru_purge_enable = true;

    tRet = httpd_start(&tServer, &conf);

    if (ESP_OK != tRet)
    {
        ESP_LOGI(TAG, "Error starting server!");
        esp_restart();
    }

    ESP_LOGI(TAG, "Registering URI handlers");
    httpd_register_uri_handler(tServer, &tRootGet);
    httpd_register_uri_handler(tServer, &tConfigPost);
    httpd_register_uri_handler(tServer, &tConfigGet);
    httpd_register_err_handler(tServer, HTTPD_404_NOT_FOUND, tError404Handler);
    return -1; // Exit success
}