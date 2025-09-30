idf_component_register(

    SRCS 
    "src/hexnet_bluetooth.c"
    "src/hexnet_wifi.c"
    "src/hexnet_nvs.c"
    "src/hexnet_canbus.c"

    REQUIRES
    esp_timer
    driver
    freertos

    
    INCLUDE_DIRS 
    "inc" 
    "${IDF_PATH}/components/nvs_flash/include"
    "${IDF_PATH}/components/esp_partition/include"
    "${IDF_PATH}/components/app_update/include"
    "${IDF_PATH}/components/bootloader_support/include"
    "${IDF_PATH}/components/esp_netif/include"
    "${IDF_PATH}/components/esp_wifi/include"
    "${IDF_PATH}/components/spi_flash/include"    
    "${IDF_PATH}/components/spiffs/include"
    "${IDF_PATH}/components/bt/include/esp32c3/include"
    "${IDF_PATH}/components/bt/host/bluedroid/api/include/api"
    "${IDF_PATH}/components/bt/host/bluedroid/api/include"
    "${IDF_PATH}/components/esp_wifi/include"
    "${IDF_PATH}/components/esp_event/include"
    "${IDF_PATH}/components/driver/twai/include/driver"
    "${IDF_PATH}/components/driver/twai/include"
    "${IDF_PATH}/components/esp_driver_gpio/include/driver"
    "${IDF_PATH}/components/esp_driver_gpio/include"
    
    
  
    EMBED_FILES 
    "favicon.ico" "upload_script.html"
)
