idf_component_register(SRCS "MySQL.cpp" "ESP32_MySQL_Connection.cpp" "ESP32_MySQL_AES.cpp" "ESP32_MySQL_Packet.cpp"
    "ESP32_MySQL_Query.cpp" "ESP32_MySQL_Encrypt_Sha1.cpp"
    INCLUDE_DIRS "include"
    REQUIRES "esp-tls" "ClientMac")
