idf_component_register(SRCS "HTTPServer.c"
    INCLUDE_DIRS "include"
    REQUIRES "esp_wifi" "FileSystem" "esp_http_server" "esp-tls"
)
