idf_component_register(SRCS "FileSystem.c"
    PRIV_REQUIRES spiffs mbedtls
    INCLUDE_DIRS "include")

spiffs_create_partition_image(stora ../storage FLASH_IN_PROJECT)