idf_component_register(SRCS "HTTPSGet.c" "TimeSync.c"
    INCLUDE_DIRS "include"
    REQUIRES esp-tls esp_wifi esp_timer nvs_flash esp_http_client esp_event ClientMac
    EMBED_TXTFILES howsmyssl_com_root_cert.pem
    postman_root_cert.pem)