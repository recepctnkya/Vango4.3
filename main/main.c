/*
 * SPDX-FileCopyrightText: 2023-2024 Espressif Systems (Shanghai) CO LTD
 *
 * SPDX-License-Identifier: CC0-1.0
 */




 #include <stdio.h>
 #include "string.h"
 #include "sdkconfig.h"
 #include "freertos/FreeRTOS.h"
 #include "freertos/task.h"
 #include "freertos/semphr.h"
 #include "esp_timer.h"
 #include "esp_lcd_panel_ops.h"
 #include "esp_lcd_panel_rgb.h"
 #include "driver/gpio.h"
 #include "esp_err.h"
 #include "esp_log.h"
 #include "lvgl.h"
 #include <nvs_flash.h>
 //#include "demos/lv_demos.h"
 #include "ui_files/ui.h"
 
 #include "driver/i2c.h" 
 #include "esp_lcd_touch_gt911.h"
 
 #include "modbus_params.h"
 #include "mbcontroller.h"
 #include "hexnet_bluetooth.h"
 #include "hexnet_wifi.h"
 #include "hexnet_nvs.h"
 #include "display_manager.h"
 #include "data_manager.h"




void app_main()
{
    //data_manager_init();

    // Initialize NVS
    esp_err_t ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        ESP_ERROR_CHECK(nvs_flash_erase());
        ret = nvs_flash_init();
    }
    ESP_ERROR_CHECK(ret);

    display_manager_init(); // Display manager'ı başlat
    ble_init();
    //wifi_ini();
}