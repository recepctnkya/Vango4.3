idf_component_register(

    SRCS 
    "src/hexnet_bluetooth.c"
    "src/hexnet_wifi.c"
    "src/hexnet_nvs.c"



    
    INCLUDE_DIRS 
    "inc" 
    "C:/Users/nakau/esp/v5.3/esp-idf/components/nvs_flash/include"
    "C:/Users/nakau/esp/v5.3/esp-idf/components/esp_partition/include"
    "C:/Users/nakau/esp/v5.3/esp-idf/components/app_update/include"
    "C:/Users/nakau/esp/v5.3/esp-idf/components/bootloader_support/include"
    "C:/Users/nakau/esp/v5.3/esp-idf/components/esp_netif/include"
    "C:/Users/nakau/esp/v5.3/esp-idf/components/esp_wifi/include"
    "C:/Users/nakau/esp/v5.3/esp-idf/components/spi_flash/include"    
    "C:/Users/nakau/esp/v5.3/esp-idf/components/spiffs/include"
    "C:/Users/nakau/esp/v5.3/esp-idf/components/bt/include/esp32c3/include"
    "C:/Users/nakau/esp/v5.3/esp-idf/components/bt/host/bluedroid/api/include/api"
    "C:/Users/nakau/esp/v5.3/esp-idf/components/bt/host/bluedroid/api/include"
    "C:/Users/nakau/esp/v5.3/esp-idf/components/esp_wifi/include"
    "C:/Users/nakau/esp/v5.3/esp-idf/components/esp_event/include"
    
    
  
    EMBED_FILES 
    "favicon.ico" "upload_script.html"
)
