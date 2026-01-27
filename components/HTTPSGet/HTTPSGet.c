#include "HTTPSGet.h"

#define MAX_HTTP_RECV_BUFFER 512
#define MAX_HTTP_OUTPUT_BUFFER 2048
static const char *TAG = "HTTP_CLIENT";
static const char *pcFormatOn = "https://api.telegram.org/bot%s/sendMessage?chat_id=%lld&text=Se%c20conecto%c20%s";
static const char *pcFormatOff = "https://api.telegram.org/bot%s/sendMessage?chat_id=%lld&text=Se%c20desconecto%c20%s";

extern const char howsmyssl_com_root_cert_pem_start[] asm("_binary_howsmyssl_com_root_cert_pem_start");
extern const char howsmyssl_com_root_cert_pem_end[] asm("_binary_howsmyssl_com_root_cert_pem_end");

extern const char postman_root_cert_pem_start[] asm("_binary_postman_root_cert_pem_start");
extern const char postman_root_cert_pem_end[] asm("_binary_postman_root_cert_pem_end");

esp_err_t _http_event_handler(esp_http_client_event_t *evt)
{
    static char *output_buffer; // Buffer to store response of http request from event handler
    static int output_len;      // Stores number of bytes read
    switch (evt->event_id)
    {
    case HTTP_EVENT_ERROR:
        ESP_LOGD(TAG, "HTTP_EVENT_ERROR");
        break;
    case HTTP_EVENT_ON_CONNECTED:
        ESP_LOGD(TAG, "HTTP_EVENT_ON_CONNECTED");
        break;
    case HTTP_EVENT_HEADER_SENT:
        ESP_LOGD(TAG, "HTTP_EVENT_HEADER_SENT");
        break;
    case HTTP_EVENT_ON_HEADER:
        ESP_LOGD(TAG, "HTTP_EVENT_ON_HEADER, key=%s, value=%s", evt->header_key, evt->header_value);
        break;
    case HTTP_EVENT_ON_DATA:
        ESP_LOGD(TAG, "HTTP_EVENT_ON_DATA, len=%d", evt->data_len);
        // Clean the buffer in case of a new request
        if (output_len == 0 && evt->user_data)
        {
            // we are just starting to copy the output data into the use
            memset(evt->user_data, 0, MAX_HTTP_OUTPUT_BUFFER);
        }
        /*
         *  Check for chunked encoding is added as the URL for chunked encoding used in this example returns binary data.
         *  However, event handler can also be used in case chunked encoding is used.
         */
        if (!esp_http_client_is_chunked_response(evt->client))
        {
            // If user_data buffer is configured, copy the response into the buffer
            int copy_len = 0;
            if (evt->user_data)
            {
                // The last byte in evt->user_data is kept for the NULL character in case of out-of-bound access.
                copy_len = MIN(evt->data_len, (MAX_HTTP_OUTPUT_BUFFER - output_len));
                if (copy_len)
                {
                    memcpy(evt->user_data + output_len, evt->data, copy_len);
                }
            }
            else
            {
                int content_len = esp_http_client_get_content_length(evt->client);
                if (output_buffer == NULL)
                {
                    // We initialize output_buffer with 0 because it is used by strlen() and similar functions therefore should be null terminated.
                    output_buffer = (char *)calloc(content_len + 1, sizeof(char));
                    output_len = 0;
                    if (output_buffer == NULL)
                    {
                        ESP_LOGE(TAG, "Failed to allocate memory for output buffer");
                        return ESP_FAIL;
                    }
                }
                copy_len = MIN(evt->data_len, (content_len - output_len));
                if (copy_len)
                {
                    memcpy(output_buffer + output_len, evt->data, copy_len);
                }
            }
            output_len += copy_len;
        }

        break;
    case HTTP_EVENT_ON_FINISH:
        ESP_LOGD(TAG, "HTTP_EVENT_ON_FINISH");
        if (output_buffer != NULL)
        {
            // Response is accumulated in output_buffer. Uncomment the below line to print the accumulated response
            // ESP_LOG_BUFFER_HEX(TAG, output_buffer, output_len);
            free(output_buffer);
            output_buffer = NULL;
        }
        output_len = 0;
        break;
    case HTTP_EVENT_DISCONNECTED:
        ESP_LOGI(TAG, "HTTP_EVENT_DISCONNECTED");
        int mbedtls_err = 0;
        esp_err_t err = esp_tls_get_and_clear_last_error((esp_tls_error_handle_t)evt->data, &mbedtls_err, NULL);
        if (err != 0)
        {
            ESP_LOGI(TAG, "Last esp error code: 0x%x", err);
            ESP_LOGI(TAG, "Last mbedtls failure: 0x%x", mbedtls_err);
        }
        if (output_buffer != NULL)
        {
            free(output_buffer);
            output_buffer = NULL;
        }
        output_len = 0;
        break;
    case HTTP_EVENT_REDIRECT:
        ESP_LOGD(TAG, "HTTP_EVENT_REDIRECT");
        esp_http_client_set_header(evt->client, "From", "user@example.com");
        esp_http_client_set_header(evt->client, "Accept", "text/html");
        esp_http_client_set_redirection(evt->client);
        break;
    }
    return ESP_OK;
}

/*static void https_with_url(void)
{
    esp_http_client_config_t tConfig = {
        .url = "",
        .event_handler = _http_event_handler,
        .transport_type = HTTP_TRANSPORT_OVER_SSL,
        .crt_bundle_attach = esp_crt_bundle_attach,
    };
    esp_http_client_handle_t tClient = esp_http_client_init(&tConfig);
    // esp_http_client_set_method(tClient, HTTP);
    esp_err_t err = esp_http_client_perform(tClient);
    char output_buffer[1024] = {};
    char *pcTring = NULL;

    err = esp_http_client_open(tClient, 0);
    if (err == ESP_OK && esp_http_client_fetch_headers(tClient) >= 0)
    {
        int data_read = esp_http_client_read_response(tClient, output_buffer, MAX_HTTP_OUTPUT_BUFFER);

        if (data_read >= 0)
        {
            ESP_LOGI(TAG, "HTTP GET Status = %d, content_length = %" PRId64,
                     esp_http_client_get_status_code(tClient),
                     esp_http_client_get_content_length(tClient));
            ESP_LOG_BUFFER_HEX(TAG, output_buffer, data_read);

            pcTring = strstr(output_buffer, "\"hour\":");

            if (pcTring != NULL)
            {
                ESP_LOGI(TAG, "Info: %s", (pcTring + 7));
            }
        }
    }
    else
    {
        ESP_LOGE(TAG, "Error perform http request %s", esp_err_to_name(err));
    }
    esp_http_client_cleanup(tClient);
}*/

static void http_test_task(void *pvParameters)
{
    esp_http_client_config_t tConfig = {
        .url = "https://api.telegram.org/",
        .event_handler = _http_event_handler,
        .transport_type = HTTP_TRANSPORT_OVER_SSL,
        .crt_bundle_attach = esp_crt_bundle_attach,
    };
    esp_http_client_handle_t tClient = NULL;
    char acBuffer[512];
    TYPE_NOTIFY *ptNotify = NULL;
    TYPE_BOT_INFO *ptBotInfo = NULL;
    TYPE_CLIENTS *ptClients = NULL;
    TYPE_CLIENT_MAC *ptNextMac = NULL;

    if (pvParameters == NULL)
    {
        vTaskDelete(NULL);
    }

    bzero(acBuffer, sizeof(acBuffer));
    ptNotify = (TYPE_NOTIFY *)pvParameters;
    ptBotInfo = &ptNotify->tBot;
    ptClients = ptNotify->ptClients;

    while (1)
    {
        ulTaskNotifyTake(pdTRUE, portMAX_DELAY);

        tClient = esp_http_client_init(&tConfig);
        ptNextMac = ptClients->ptClient;
        while (ptNextMac != NULL && (ptClients->s8Reconnect > 0 || ptClients->s8Disconect > 0))
        {

            switch (ptNextMac->eClientState)
            {
            case eCLIENT_RECONNECT:
            {
                sniprintf(acBuffer, sizeof(acBuffer), pcFormatOn, ptBotInfo->acKey, ptBotInfo->s64ChatID, '%', '%', ptNextMac->acNombre);
                ptNextMac->eClientState = eCLIENT_ONLINE;
                ptClients->s8Reconnect--;
                break;
            }
            case eCLIENT_NEW_OFFLINE:
            {
                sniprintf(acBuffer, sizeof(acBuffer), pcFormatOff, ptBotInfo->acKey, ptBotInfo->s64ChatID, '%', '%', ptNextMac->acNombre);
                ptNextMac->eClientState = eCLIENT_OFFLINE;
                ptClients->s8Disconect--;
                break;
            }
            default:
            {
                ptNextMac = ptNextMac->ptNextClient;
                continue;
                break;
            }
            }

            esp_http_client_set_url(tClient, acBuffer);
            esp_http_client_perform(tClient);
            ESP_LOGI(TAG, "Name: %s, HTTP Status = %d", ptNextMac->acNombre, esp_http_client_get_status_code(tClient));
            ptNextMac = ptNextMac->ptNextClient;
            bzero(acBuffer, sizeof(acBuffer));
        }
        esp_http_client_cleanup(tClient);
    }
    vTaskDelete(NULL);
}

void vInitHTTPClient(void *pvParameters)
{
    static bool bInit = false;
    TYPE_NOTIFY *ptNotify = NULL;

    if (pvParameters == NULL)
    {
        return;
    }

    if (bInit == false)
    {
        ptNotify = (TYPE_NOTIFY *)pvParameters;
        xTaskCreate(&http_test_task, "http_test_task", 8192, pvParameters, 5, &ptNotify->tHandle);
        bInit = true;
    }
}
