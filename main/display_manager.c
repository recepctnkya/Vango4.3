// display_manager.c

#include "display_manager.h"
#include "data_manager.h"
#include "definitions.h"
#include "ui_files/ui.h"
#include "hexnet_bluetooth.h"
#include "hexnet_nvs.h"

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
#include "cJson.h"

#include "driver/i2c.h" 
#include "esp_lcd_touch_gt911.h"
#include "waveshare_rgb_lcd_port.h"



static const char *TAG = "DISPLAY_MANAGER";



extern lv_obj_t * ui_scrTheme;
extern lv_obj_t * ui_scrWallpaper;

extern lv_obj_t * ui_lblVangoText;
extern lv_obj_t *ui_imgDevice;
extern lv_obj_t *ui_imgsconnected;
extern lv_obj_t *ui_imgsnotconnected;
extern lv_obj_t *ui_imgBluetoothNotConnected;
extern lv_obj_t *ui_imgBluetoothConnected;
extern lv_obj_t *ui_arcGrup1;
extern lv_obj_t *ui_arcGrup2;
extern lv_obj_t *ui_swO1; 
extern lv_obj_t *ui_swO2; 
extern lv_obj_t *ui_swO3; 
extern lv_obj_t *ui_swO4; 
extern lv_obj_t *ui_swO5; 
extern lv_obj_t *ui_swO6; 
extern lv_obj_t *ui_swO7; 
extern lv_obj_t *ui_swO8; 
extern lv_obj_t *ui_swO9; 
extern lv_obj_t *ui_swO10;
extern lv_obj_t *ui_swO11;
extern lv_obj_t *ui_swO12;
extern lv_obj_t *ui_swO13;
extern lv_obj_t *ui_swO14;
extern lv_obj_t *ui_swO15;
extern lv_obj_t *ui_swO16;

extern lv_obj_t* ui_cbxO1;
extern lv_obj_t* ui_cbxO2;
extern lv_obj_t* ui_cbxO3;
extern lv_obj_t* ui_cbxO4;
extern lv_obj_t* ui_cbxO5;
extern lv_obj_t* ui_cbxO6;
extern lv_obj_t* ui_cbxO7;
extern lv_obj_t* ui_cbxO8;
extern lv_obj_t* ui_cbxO9;
extern lv_obj_t* ui_cbxO10;
extern lv_obj_t* ui_cbxO11;
extern lv_obj_t* ui_cbxO12;
extern lv_obj_t* ui_cbxO13;
extern lv_obj_t* ui_cbxO14;
extern lv_obj_t* ui_cbxO15;
extern lv_obj_t* ui_cbxO16;



extern lv_obj_t* ui_swDim1;
extern lv_obj_t* ui_swDim2;
extern lv_obj_t* ui_swDim3;
extern lv_obj_t* ui_swDim4;

extern lv_obj_t* ui_cbxDim1;
extern lv_obj_t* ui_cbxDim2;
extern lv_obj_t* ui_cbxDim3;
extern lv_obj_t* ui_cbxDim4;

extern lv_obj_t* ui_Checkbox1;
extern lv_obj_t* ui_Checkbox2;
extern lv_obj_t* ui_Checkbox3;
extern lv_obj_t* ui_Checkbox4;
extern lv_obj_t* ui_Checkbox5;


// Declare the panels
extern lv_obj_t* ui_pnlGrup1;
extern lv_obj_t* ui_pnlGrup2;
extern lv_obj_t* ui_pnlGrup3;
extern lv_obj_t* ui_pnlOutputs;
extern lv_obj_t* ui_pnlConnectionLost;

extern lv_obj_t *ui_imgWForecast;
extern lv_obj_t *ui_lblDateAndTime;
lv_obj_t * ui_btnIOGot;


extern lv_obj_t *ui_lblSelectTheme;
extern lv_obj_t *ui_lblWallpaper; 
extern lv_obj_t *ui_lblRolllerTime;
extern lv_obj_t *ui_swEnableWallpaper;
extern lv_obj_t *ui_rlrTime;

extern lv_obj_t * ui_scrInit;
extern lv_obj_t *ui_scrRules;
extern lv_obj_t *ui_scrPanelSettings;
extern lv_obj_t *ui_lblPanelSettings;
extern lv_obj_t *ui_lblSensors;
extern lv_obj_t *ui_lblDimmableOutputs;
extern lv_obj_t *ui_pnlSensors;
extern lv_obj_t *ui_lblWeather;
extern lv_obj_t *ui_lblSettingsB;

extern lv_obj_t *ui_Colorwheel1;
extern lv_obj_t *ui_btnRGBColor;
extern lv_obj_t *ui_brInit;

// Slave ID 
#define SLAVE_ID 50

// General CID
#define CID_MIDITHREE 0

// Voltage and Current
#define VOLTAGE_INDIS 1
#define CURRENT_INDIS 2

// Control Registers
#define RESET_INDIS 3
#define BLUETOOTH_INDIS 4



// Inputs
#define INPUT_1_INDIS 21
#define INPUT_2_INDIS 22
#define INPUT_3_INDIS 23
#define INPUT_4_INDIS 24
#define INPUT_5_INDIS 25
#define INPUT_6_INDIS 26
#define INPUT_7_INDIS 27
#define INPUT_8_INDIS 28
#define INPUT_9_INDIS 29
#define INPUT_10_INDIS 30
#define INPUT_11_INDIS 31
#define INPUT_12_INDIS 32
#define INPUT_13_INDIS 33
#define INPUT_14_INDIS 34
#define INPUT_15_INDIS 35
#define INPUT_16_INDIS 36



// Define weather conditions
#define WEATHER_SUNNY         1
#define WEATHER_PARTLY_SUNNY  2
#define WEATHER_THUNDER       3
#define WEATHER_RAINY         4
#define WEATHER_SNOWY         5
#define WEATHER_CLOUDY        6

// Define weather icon positions
#define ICON_SUNNY            0
#define ICON_PARTLY_SUNNY     34
#define ICON_THUNDER          75
#define ICON_RAINY            115
#define ICON_SNOWY            150
#define ICON_CLOUDY           190

// Analog Inputs
#define ANALOG_INPUT_1_INDIS 37
#define ANALOG_INPUT_2_INDIS 38
#define ANALOG_INPUT_3_INDIS 39
#define ANALOG_INPUT_4_INDIS 40

// Dimmable Outputs
#define DIMMABLE_OUTPUT_1_INDIS 41
#define DIMMABLE_OUTPUT_2_INDIS 42

// RGB Outputs
#define RGB_R_INDIS 42
#define RGB_G_INDIS 43
#define RGB_B_INDIS 44



#define I2C_MASTER_SCL_IO           9       /*!< GPIO number used for I2C master clock */
#define I2C_MASTER_SDA_IO           8       /*!< GPIO number used for I2C master data  */
#define I2C_MASTER_NUM              0       /*!< I2C master i2c port number, the number of i2c peripheral interfaces available will depend on the chip */
#define I2C_MASTER_FREQ_HZ          400000                     /*!< I2C master clock frequency */
#define I2C_MASTER_TX_BUF_DISABLE   0                          /*!< I2C master doesn't need buffer */
#define I2C_MASTER_RX_BUF_DISABLE   0                          /*!< I2C master doesn't need buffer */
#define I2C_MASTER_TIMEOUT_MS       1000

#define GPIO_INPUT_IO_4    4
#define GPIO_INPUT_PIN_SEL  1ULL<<GPIO_INPUT_IO_4


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////// Please update the following configuration according to your LCD spec //////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#define EXAMPLE_LCD_PIXEL_CLOCK_HZ     (18 * 1000 * 1000)
#define EXAMPLE_LCD_BK_LIGHT_ON_LEVEL  1
#define EXAMPLE_LCD_BK_LIGHT_OFF_LEVEL !EXAMPLE_LCD_BK_LIGHT_ON_LEVEL
#define EXAMPLE_PIN_NUM_BK_LIGHT       -1
#define EXAMPLE_PIN_NUM_HSYNC          46
#define EXAMPLE_PIN_NUM_VSYNC          3
#define EXAMPLE_PIN_NUM_DE             5
#define EXAMPLE_PIN_NUM_PCLK           7
#define EXAMPLE_PIN_NUM_DATA0          14 // B3
#define EXAMPLE_PIN_NUM_DATA1          38 // B4
#define EXAMPLE_PIN_NUM_DATA2          18 // B5
#define EXAMPLE_PIN_NUM_DATA3          17 // B6
#define EXAMPLE_PIN_NUM_DATA4          10 // B7
#define EXAMPLE_PIN_NUM_DATA5          39 // G2
#define EXAMPLE_PIN_NUM_DATA6          0 // G3
#define EXAMPLE_PIN_NUM_DATA7          45 // G4
#define EXAMPLE_PIN_NUM_DATA8          48 // G5
#define EXAMPLE_PIN_NUM_DATA9          47 // G6
#define EXAMPLE_PIN_NUM_DATA10         21 // G7
#define EXAMPLE_PIN_NUM_DATA11         1  // R3
#define EXAMPLE_PIN_NUM_DATA12         2  // R4
#define EXAMPLE_PIN_NUM_DATA13         42 // R5
#define EXAMPLE_PIN_NUM_DATA14         41 // R6
#define EXAMPLE_PIN_NUM_DATA15         40 // R7
#define EXAMPLE_PIN_NUM_DISP_EN        -1

// The pixel number in horizontal and vertical
#define EXAMPLE_LCD_H_RES              800
#define EXAMPLE_LCD_V_RES              480

#if CONFIG_EXAMPLE_DOUBLE_FB
#define EXAMPLE_LCD_NUM_FB             2
#else
#define EXAMPLE_LCD_NUM_FB             1
#endif // CONFIG_EXAMPLE_DOUBLE_FB

#define EXAMPLE_LVGL_TICK_PERIOD_MS    2
#define EXAMPLE_LVGL_TASK_MAX_DELAY_MS 500
#define EXAMPLE_LVGL_TASK_MIN_DELAY_MS 1
#define EXAMPLE_LVGL_TASK_STACK_SIZE   (8 * 1024)
#define EXAMPLE_LVGL_TASK_PRIORITY     2

static SemaphoreHandle_t lvgl_mux = NULL;

// we use two semaphores to sync the VSYNC event and the LVGL task, to avoid potential tearing effect
#if CONFIG_EXAMPLE_AVOID_TEAR_EFFECT_WITH_SEM
SemaphoreHandle_t sem_vsync_end;
SemaphoreHandle_t sem_gui_ready;
#endif

// Define the maximum number of outputs
#define MAX_OUTPUTS 16
// extern lv_obj_t* ui_img_lamp_png;
// extern lv_obj_t* ui_img_water_png;
// extern lv_obj_t* ui_img_outlet_png;
// extern lv_obj_t* ui_img_oven_png;
// extern lv_obj_t* ui_img_tv_png;
// extern lv_obj_t* ui_img_refrigerator_png;
// extern lv_obj_t* ui_img_toilet_png;
// extern lv_obj_t* ui_img_usb_png;
// extern lv_obj_t* ui_img_ac_png;
// extern lv_obj_t* ui_img_readinglamp_png;
// extern lv_obj_t* ui_img_heater_png;

// Declare button, label, and image objects
lv_obj_t* btnIO[MAX_OUTPUTS] = {NULL};
lv_obj_t* lblIO[MAX_OUTPUTS] = {NULL};
lv_obj_t* imgIO[MAX_OUTPUTS] = {NULL};
lv_obj_t* sldDims[4] = {NULL};
lv_obj_t* lblDims[4] = {NULL};
// Define the button names and icons
const char* lblBtnNames[18] = {
    "LAMP", "TOILET", "KITCHEN", "BEDROOM", "CORRIDOR", "STEP", "AC", "USB", "REGRIGE.", "WATER P.", "OUTLET", "OVEN", "TV", "EX.LIGHT", "EX.OUTLET", "HEATER", "SPOT", "READING L."
};



// Example data to save
int numOfOutputs = 16;
int numOfDims = 4;
int numOfSensors = 5;
bool slaveConnectionStatus = true;
int panelThemeType = 0;
int panelWallpaperEnable = false;
int panelWallpaperTime = 0;
int numberOfNotifications = 0;
cJSON* notifications = NULL;
// Declare the global outputsBuffer
int outputsBuffer[16] = {0};
int sensorsBuffer[5] = {0};
int dimsBuffer[4] = {0};
int btn_index = 0;

int panelWallpaperEnableCounter = 1;






// extern lv_obj_t *scr;
static void example_lvgl_touch_cb(lv_indev_drv_t * drv, lv_indev_data_t * data)
{
    uint16_t touchpad_x[1] = {0};
    uint16_t touchpad_y[1] = {0};
    uint8_t touchpad_cnt = 0;

    /* Read touch controller data */
    esp_lcd_touch_read_data(drv->user_data);

    /* Get coordinates */
    bool touchpad_pressed = esp_lcd_touch_get_coordinates(drv->user_data, touchpad_x, touchpad_y, NULL, &touchpad_cnt, 1);

    if (touchpad_pressed && touchpad_cnt > 0) {
        data->point.x = touchpad_x[0];
        data->point.y = touchpad_y[0];
        data->state = LV_INDEV_STATE_PR;
        panelWallpaperEnableCounter = 0;
    } else {
        data->state = LV_INDEV_STATE_REL;
    }
}




void my_btnThemeWhiteFunc(void)
{
    panelThemeType = 0;
    lv_obj_set_style_bg_color(ui_scrMain, lv_color_hex(0xFFFFFF), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_color(ui_scrTheme, lv_color_hex(0xFFFFFF), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_color(ui_scrSettings, lv_color_hex(0xFFFFFF), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_color(ui_scrRules, lv_color_hex(0xFFFFFF), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_color(ui_scrPanelSettings, lv_color_hex(0xFFFFFF), LV_PART_MAIN | LV_STATE_DEFAULT);

    
    lv_obj_set_style_text_color(ui_lblPanelSettings, lv_color_hex(0x000000), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_color(ui_lblSensors, lv_color_hex(0x000000), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_color(ui_lblDimmableOutputs, lv_color_hex(0x000000), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_color(ui_Checkbox1, lv_color_hex(0x000000), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_color(ui_Checkbox2, lv_color_hex(0x000000), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_color(ui_Checkbox3, lv_color_hex(0x000000), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_color(ui_Checkbox4, lv_color_hex(0x000000), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_color(ui_Checkbox5, lv_color_hex(0x000000), LV_PART_MAIN | LV_STATE_DEFAULT);


    lv_obj_set_style_text_color(ui_lblPnlGrup1Sicaklik1, lv_color_hex(0x000000), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_color(ui_lblPnlGrup1Sicaklik2, lv_color_hex(0x000000), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_color(ui_lblPnlGrup1SicaklikDeger1, lv_color_hex(0x000000), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_color(ui_lblPnlGrup1SicaklikDeger2, lv_color_hex(0x000000), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_color(ui_pnlGrupSicaklik1, lv_color_hex(0xFFFFFF), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_color(ui_pnlGrupSicaklik2, lv_color_hex(0xFFFFFF), LV_PART_MAIN | LV_STATE_DEFAULT);


    lv_obj_set_style_bg_color(ui_pnlGrup1, lv_color_hex(0xFFFFFF), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_color(ui_lblGrup1Oran1, lv_color_hex(0x000000), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_color(ui_lblGrup1, lv_color_hex(0x000000), LV_PART_MAIN | LV_STATE_DEFAULT);

    lv_obj_set_style_bg_color(ui_pnlGrup2, lv_color_hex(0xFFFFFF), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_color(ui_lblGrup1Oran2, lv_color_hex(0x000000), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_color(ui_lblGrup2, lv_color_hex(0x000000), LV_PART_MAIN | LV_STATE_DEFAULT);

    lv_obj_set_style_bg_color(ui_pnlGrup3, lv_color_hex(0xFFFFFF), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_color(ui_lblGrup1Oran3, lv_color_hex(0x000000), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_color(ui_lblGrup3, lv_color_hex(0x000000), LV_PART_MAIN | LV_STATE_DEFAULT);


    lv_obj_set_style_bg_color(ui_pnlSensors, lv_color_hex(0xFFFFFF), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_color(ui_pnlOutputs, lv_color_hex(0xFFFFFF), LV_PART_MAIN | LV_STATE_DEFAULT);

    lv_obj_set_style_text_color(ui_lblVangoText, lv_color_hex(0x000000), LV_PART_MAIN | LV_STATE_DEFAULT);


    lv_obj_set_style_text_color(ui_lblSelectTheme, lv_color_hex(0x000000), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_color(ui_lblWallpaper, lv_color_hex(0x000000), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_color(ui_lblRolllerTime, lv_color_hex(0x000000), LV_PART_MAIN | LV_STATE_DEFAULT);

    lv_obj_set_style_text_color(ui_lblRolllerTime, lv_color_hex(0x000000), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_color(ui_lblRolllerTime, lv_color_hex(0x000000), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_color(ui_lblRolllerTime, lv_color_hex(0x000000), LV_PART_MAIN | LV_STATE_DEFAULT);

    lv_obj_set_style_text_color(ui_lblWeather, lv_color_hex(0x000000), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_color(ui_lblDateAndTime, lv_color_hex(0x000000), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_color(ui_lblSettingsB, lv_color_hex(0x000000), LV_PART_MAIN | LV_STATE_DEFAULT);
    

    for (int i = 0; i < numOfDims; i++) {
        lv_obj_set_style_text_color(lblDims[i], lv_color_hex(0x000000), LV_PART_MAIN | LV_STATE_DEFAULT);
    }

}




// Function to get the image name based on the button name
const void* get_image_for_button(int outputBufferIndex) {
    if (outputBufferIndex < 0 || outputBufferIndex >= 19) {
        return NULL; // Invalid index
    }

    const char* btnName = lblBtnNames[outputBufferIndex];

    if (strcmp(btnName, "KITCHEN") == 0 || strcmp(btnName, "LAMP") == 0 || strcmp(btnName, "EX.LIGHT") == 0 || strcmp(btnName, "SPOT") == 0 || strcmp(btnName, "BEDROOM") == 0 || strcmp(btnName, "CORRIDOR") == 0 || strcmp(btnName, "STEP") == 0) {
        return &ui_img_lamp_png;
    } else if (strcmp(btnName, "TOILET") == 0) {
        return &ui_img_toilet_png;
    } else if (strcmp(btnName, "OUTLET") == 0 || strcmp(btnName, "EX.OUTLET") == 0 || strcmp(btnName, "HEATER") == 0) {
        return &ui_img_outlet_png;
    } else if (strcmp(btnName, "USB") == 0) {
        return &ui_img_usb_png;
    } else if (strcmp(btnName, "REGRIGE.") == 0) {
        return &ui_img_refrigerator_png;
    } else if (strcmp(btnName, "WATER P.") == 0) {
        return &ui_img_water_png;
    } else if (strcmp(btnName, "OVEN") == 0) {
        return &ui_img_oven_png;
    } else if (strcmp(btnName, "TV") == 0) {
        return &ui_img_tv_png;
    } else if (strcmp(btnName, "AC") == 0) {
        return &ui_img_ac_png;
    } else if (strcmp(btnName, "HEATER") == 0) {
        return &ui_img_heater_png;
    } else if (strcmp(btnName, "READING L.") == 0) {
        return &ui_img_readinglamp_png;
    } else {
        return NULL; // No matching image found
    }
}


// Function to set panel coordinates dynamically
void set_sensor_panels_coordinates(int numOfSensors, int sensorsBuffer[5]) {
    lv_obj_t* panels[3] = {ui_pnlGrup1, ui_pnlGrup2, ui_pnlGrup3};
    lv_obj_t* panels2[2] = {ui_pnlGrupSicaklik1, ui_pnlGrupSicaklik2};

    int x_coords[3] = {-320, -210, -101};
    int y_coords[3] = {60, 61, 59};
    int x2_coords[2] = {-320, -208};
    int y2_coords[2] = {147, 151};


    lv_obj_clear_flag(ui_pnlGrup1, LV_OBJ_FLAG_HIDDEN);     /// Flags
    lv_obj_clear_flag(ui_pnlGrup2, LV_OBJ_FLAG_HIDDEN);     /// Flags
    lv_obj_clear_flag(ui_pnlGrup3, LV_OBJ_FLAG_HIDDEN);     /// Flags
    lv_obj_clear_flag(ui_pnlGrupSicaklik1, LV_OBJ_FLAG_HIDDEN);     /// Flags
    lv_obj_clear_flag(ui_pnlGrupSicaklik2, LV_OBJ_FLAG_HIDDEN);     /// Flags

    int current_index = 0;
    for (int i = 0; i < 3; i++) {
        if (sensorsBuffer[i + 2] == 1) {
            lv_obj_set_x(panels[i], x_coords[current_index]);
            lv_obj_set_y(panels[i], y_coords[current_index]);
            current_index++;
        }
        else {
            lv_obj_set_x(panels[i], LV_COORD_MAX);
            lv_obj_set_y(panels[i], LV_COORD_MAX);
        }
    }
    current_index = 0;
    for (int i = 0; i < 2; i++) {
        if (sensorsBuffer[i] == 1) {
            lv_obj_set_x(panels2[i], x2_coords[current_index]);
            lv_obj_set_y(panels2[i], y2_coords[current_index]);
            current_index++;
        }
        else {
            lv_obj_set_x(panels2[i], LV_COORD_MAX);
            lv_obj_set_y(panels2[i], LV_COORD_MAX);
        }
    }
}

// Function to toggle button color based on regs_data
void button_events(lv_event_t* e) {
    lv_obj_t* btn = lv_event_get_target(e);
    btn_index = (int)lv_event_get_user_data(e);
    toggle_regs_data(btn_index + OUTPUT_1_INDIS);
    ESP_LOGI(TAG, "Button index: %d", btn_index);
}

// Function to create the UI dynamically based on numOfOutputs
void create_dynamic_ui(lv_obj_t* parent) {

    int btn_width = 105;
    int btn_height = 190;
    int btn_x_offset = 106; // btn_width + 1 for spacing
    int btn_y_offset = 191; // btn_height + 1 for spacing
    int x_start = 14;
    int y_start = -102;

    // Apply numOfOutputs and outputsBuffer to swO1-swO16 and cbxO1-cbxO16
    lv_obj_t* switches[16] = {ui_swO1, ui_swO2, ui_swO3, ui_swO4, ui_swO5, ui_swO6, ui_swO7, ui_swO8, ui_swO9, ui_swO10, ui_swO11, ui_swO12, ui_swO13, ui_swO14, ui_swO15, ui_swO16};
    lv_obj_t* dropdowns[16] = {ui_cbxO1, ui_cbxO2, ui_cbxO3, ui_cbxO4, ui_cbxO5, ui_cbxO6, ui_cbxO7, ui_cbxO8, ui_cbxO9, ui_cbxO10, ui_cbxO11, ui_cbxO12, ui_cbxO13, ui_cbxO14, ui_cbxO15, ui_cbxO16};
    lv_obj_t* checkboxes[5] = {ui_Checkbox1, ui_Checkbox2, ui_Checkbox3, ui_Checkbox4, ui_Checkbox5};

    lv_obj_t* dimcheckboxes[4] = {ui_swDim1, ui_swDim2, ui_swDim3, ui_swDim4};
    lv_obj_t* dimdropdowns[4] = {ui_cbxDim1, ui_cbxDim2, ui_cbxDim3, ui_cbxDim4};

    // Adjust button size and spacing if numOfOutputs is greater than 8
    if (numOfOutputs > 8) {
        btn_width = 105;
        btn_height = 95;
        btn_x_offset = 106; // btn_width + 1 for spacing
        btn_y_offset = 96; // btn_height + 1 for spacing
        y_start = -150;
    }

    for (int i = 0; i < numOfOutputs; i++) {
        int row = i / 4;
        int col = i % 4;


        btnIO[i] = lv_btn_create(parent);
        lv_obj_set_width(btnIO[i], btn_width);
        lv_obj_set_height(btnIO[i], btn_height);
        lv_obj_set_x(btnIO[i], x_start + col * btn_x_offset);
        lv_obj_set_y(btnIO[i], y_start + row * btn_y_offset);
        lv_obj_set_align(btnIO[i], LV_ALIGN_CENTER);
        lv_obj_add_flag(btnIO[i], LV_OBJ_FLAG_SCROLL_ON_FOCUS);     /// Flags
        lv_obj_clear_flag(btnIO[i], LV_OBJ_FLAG_SCROLLABLE);      /// Flags
        lv_obj_set_style_radius(btnIO[i], 0, LV_PART_MAIN | LV_STATE_DEFAULT);
        lv_obj_set_style_bg_color(btnIO[i], lv_color_hex(0x5A5A5A), LV_PART_MAIN | LV_STATE_DEFAULT);
        lv_obj_set_style_bg_opa(btnIO[i], 255, LV_PART_MAIN | LV_STATE_DEFAULT);

        lblIO[i] = lv_label_create(btnIO[i]);
        lv_obj_set_width(lblIO[i], LV_SIZE_CONTENT);   /// 1
        lv_obj_set_height(lblIO[i], LV_SIZE_CONTENT);    /// 1
        lv_obj_set_x(lblIO[i], 0);

        if (numOfOutputs > 8) {
            lv_obj_set_align(lblIO[i], LV_ALIGN_BOTTOM_MID);
            lv_obj_set_y(lblIO[i], btn_height / 2 - 35); // Adjust y position to align at the bottom mid
        }
        else {

            lv_obj_set_y(lblIO[i], 0); // Adjust y position to align at the bottom mid
            lv_obj_set_align(lblIO[i], LV_ALIGN_CENTER);
        }
        lv_obj_set_align(lblIO[i], LV_ALIGN_BOTTOM_MID);
        lv_label_set_text(lblIO[i], lblBtnNames[outputsBuffer[i] - 1]);
        lv_obj_set_style_text_color(lblIO[i], lv_color_hex(0xFFFFFF), LV_PART_MAIN | LV_STATE_DEFAULT);
        lv_obj_set_style_text_opa(lblIO[i], 255, LV_PART_MAIN | LV_STATE_DEFAULT);
        lv_obj_set_style_text_font(lblIO[i], &lv_font_montserrat_18, LV_PART_MAIN | LV_STATE_DEFAULT);

        imgIO[i] = lv_img_create(btnIO[i]);
        lv_img_set_src(imgIO[i], get_image_for_button(outputsBuffer[i] - 1));
        lv_obj_set_width(imgIO[i], LV_SIZE_CONTENT);   /// 1
        lv_obj_set_height(imgIO[i], LV_SIZE_CONTENT);    /// 1
        lv_obj_set_x(imgIO[i], 0);
        lv_obj_set_y(imgIO[i], 5);
        if(numOfOutputs > 8) {
            lv_obj_set_align(imgIO[i], LV_ALIGN_TOP_MID);
        }
        else {
            lv_obj_set_align(imgIO[i], LV_ALIGN_CENTER);
        }
        
        lv_obj_add_flag(imgIO[i], LV_OBJ_FLAG_ADV_HITTEST);     /// Flags
        lv_obj_clear_flag(imgIO[i], LV_OBJ_FLAG_SCROLLABLE);      /// Flags
        // Add event callback for button click
        lv_obj_add_event_cb(btnIO[i], button_events, LV_EVENT_CLICKED, (void*)i);
    }


        for (int i = 0; i < numOfDims; i++) {
        // Create the slider
        sldDims[i] = lv_slider_create(ui_pnlSensors);
        //lv_slider_set_value(sldDims[i], dimsBuffer[i], LV_ANIM_OFF);
        if (lv_slider_get_mode(sldDims[i]) == LV_SLIDER_MODE_RANGE) {
            lv_slider_set_left_value(sldDims[i], 0, LV_ANIM_OFF);
        }
        lv_obj_set_width(sldDims[i], 305);
        lv_obj_set_height(sldDims[i], 14);
        lv_obj_set_x(sldDims[i], -6);
        lv_obj_set_y(sldDims[i], -56 + i * 40); // Adjust y position dynamically
        lv_obj_set_align(sldDims[i], LV_ALIGN_CENTER);

        lv_obj_set_style_pad_left(sldDims[i], 15, LV_PART_KNOB | LV_STATE_PRESSED);
        lv_obj_set_style_pad_right(sldDims[i], 15, LV_PART_KNOB | LV_STATE_PRESSED);
        lv_obj_set_style_pad_top(sldDims[i], 15, LV_PART_KNOB | LV_STATE_PRESSED);
        lv_obj_set_style_pad_bottom(sldDims[i], 15, LV_PART_KNOB | LV_STATE_PRESSED);

        // Create the label
        lblDims[i] = lv_label_create(ui_pnlSensors);
        lv_obj_set_width(lblDims[i], LV_SIZE_CONTENT);
        lv_obj_set_height(lblDims[i], LV_SIZE_CONTENT);
        lv_obj_set_x(lblDims[i], -135);
        lv_obj_set_y(lblDims[i], -77 + i * 40); // Adjust y position dynamically
        lv_obj_set_align(lblDims[i], LV_ALIGN_RIGHT_MID);
        lv_label_set_text_fmt(lblDims[i], "%s:", lblBtnNames[dimsBuffer[i] - 1]);
    }





    for (int i = 0; i < 16; i++) {
        if (i < numOfOutputs) {
            lv_obj_add_state(switches[i], LV_STATE_CHECKED); // Check the switch
            lv_dropdown_set_selected(dropdowns[i], outputsBuffer[i] - 1); // Set the dropdown value
            lv_obj_add_flag(dropdowns[i], LV_OBJ_FLAG_CLICKABLE); // Make the dropdown clickable
        } else{
            lv_obj_clear_state(switches[i], LV_STATE_CHECKED); // Uncheck the switch
            lv_obj_clear_flag(switches[i], LV_OBJ_FLAG_CLICKABLE); // Make the switch non-clickable
            lv_dropdown_set_selected(dropdowns[i], 0); // Reset the dropdown value
            lv_obj_clear_flag(dropdowns[i], LV_OBJ_FLAG_CLICKABLE); // Make the dropdown non-clickable
        }
    }
    
    // Ensure only the last checkbox is clickable
    for (int i = 0; i < 16; i++) {
        if (i == numOfOutputs - 1) {
            lv_obj_add_flag(switches[i], LV_OBJ_FLAG_CLICKABLE); // Make the last switch clickable
            lv_obj_add_flag(switches[i + 1], LV_OBJ_FLAG_CLICKABLE);
            break;
        } else {
            lv_obj_clear_flag(switches[i], LV_OBJ_FLAG_CLICKABLE);
        }
    }


    //Function to apply sensorsBuffer to checkboxes
    for (int i = 0; i < 5; i++) {
        if (sensorsBuffer[i] == 1) {
            lv_obj_add_state(checkboxes[i], LV_STATE_CHECKED); // Check the checkbox
        } else {
            lv_obj_clear_state(checkboxes[i], LV_STATE_CHECKED); // Uncheck the checkbox
        }
    }



    //Dims buffer apply to panel settings
    for (int i = 0; i < 4; i++) {
        if (i < numOfDims) {
            lv_obj_add_state(dimcheckboxes[i], LV_STATE_CHECKED); // Check the switch
            lv_dropdown_set_selected(dimdropdowns[i], dimsBuffer[i] - 1); // Set the dropdown value
            lv_obj_add_flag(dimdropdowns[i], LV_OBJ_FLAG_CLICKABLE); // Make the dropdown clickable
        } else {
            lv_obj_clear_state(dimcheckboxes[i], LV_STATE_CHECKED); // Uncheck the switch
            lv_obj_clear_flag(dimcheckboxes[i], LV_OBJ_FLAG_CLICKABLE); // Make the switch non-clickable
            lv_dropdown_set_selected(dimdropdowns[i], 0); // Reset the dropdown value
            lv_obj_clear_flag(dimdropdowns[i], LV_OBJ_FLAG_CLICKABLE); // Make the dropdown non-clickable
        }
    }
    
    // Ensure only the last checkbox is clickable
    for (int i = 0; i < 4; i++) {
        if (i == numOfDims - 1) {
            lv_obj_add_flag(dimcheckboxes[i], LV_OBJ_FLAG_CLICKABLE); // Make the last switch clickable
            lv_obj_add_flag(dimcheckboxes[i + 1], LV_OBJ_FLAG_CLICKABLE);
            break;
        } else {
            lv_obj_clear_flag(dimcheckboxes[i], LV_OBJ_FLAG_CLICKABLE);
        }
    }

    if (numOfDims == 0) {
        lv_obj_add_flag(dimcheckboxes[0], LV_OBJ_FLAG_CLICKABLE);
    }
    if (numOfOutputs == 0) {
        lv_obj_add_flag(switches[0], LV_OBJ_FLAG_CLICKABLE);
    }

    set_sensor_panels_coordinates(3, sensorsBuffer);
    if (panelThemeType){
        my_btnBlackThemeFunc();
    }
    else{
        my_btnThemeWhiteFunc();
    }
    apply_theme_settings();

}
void my_btnBlackThemeFunc(void)
{
    panelThemeType = 1;
    lv_obj_set_style_bg_color(ui_scrMain, lv_color_hex(0x000000), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_color(ui_scrTheme, lv_color_hex(0x000000), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_color(ui_scrSettings, lv_color_hex(0x000000), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_color(ui_scrRules, lv_color_hex(0x000000), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_color(ui_scrPanelSettings, lv_color_hex(0x000000), LV_PART_MAIN | LV_STATE_DEFAULT);

    
    lv_obj_set_style_text_color(ui_lblPanelSettings, lv_color_hex(0xFFFFFF), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_color(ui_lblSensors, lv_color_hex(0xFFFFFF), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_color(ui_lblDimmableOutputs, lv_color_hex(0xFFFFFF), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_color(ui_Checkbox1, lv_color_hex(0xFFFFFF), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_color(ui_Checkbox2, lv_color_hex(0xFFFFFF), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_color(ui_Checkbox3, lv_color_hex(0xFFFFFF), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_color(ui_Checkbox4, lv_color_hex(0xFFFFFF), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_color(ui_Checkbox5, lv_color_hex(0xFFFFFF), LV_PART_MAIN | LV_STATE_DEFAULT);


    lv_obj_set_style_text_color(ui_lblPnlGrup1Sicaklik1, lv_color_hex(0xFFFFFF), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_color(ui_lblPnlGrup1Sicaklik2, lv_color_hex(0xFFFFFF), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_color(ui_lblPnlGrup1SicaklikDeger1, lv_color_hex(0xFFFFFF), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_color(ui_lblPnlGrup1SicaklikDeger2, lv_color_hex(0xFFFFFF), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_color(ui_pnlGrupSicaklik1, lv_color_hex(0x000000), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_color(ui_pnlGrupSicaklik2, lv_color_hex(0x000000), LV_PART_MAIN | LV_STATE_DEFAULT);


    lv_obj_set_style_bg_color(ui_pnlGrup1, lv_color_hex(0x000000), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_color(ui_lblGrup1Oran1, lv_color_hex(0xFFFFFF), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_color(ui_lblGrup1, lv_color_hex(0xFFFFFF), LV_PART_MAIN | LV_STATE_DEFAULT);

    lv_obj_set_style_bg_color(ui_pnlGrup2, lv_color_hex(0x000000), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_color(ui_lblGrup1Oran2, lv_color_hex(0xFFFFFF), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_color(ui_lblGrup2, lv_color_hex(0xFFFFFF), LV_PART_MAIN | LV_STATE_DEFAULT);

    lv_obj_set_style_bg_color(ui_pnlGrup3, lv_color_hex(0x000000), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_color(ui_lblGrup1Oran3, lv_color_hex(0xFFFFFF), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_color(ui_lblGrup3, lv_color_hex(0xFFFFFF), LV_PART_MAIN | LV_STATE_DEFAULT);


    lv_obj_set_style_bg_color(ui_pnlSensors, lv_color_hex(0x000000), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_color(ui_pnlOutputs, lv_color_hex(0x000000), LV_PART_MAIN | LV_STATE_DEFAULT);

    lv_obj_set_style_text_color(ui_lblVangoText, lv_color_hex(0xFFFFFF), LV_PART_MAIN | LV_STATE_DEFAULT);

    lv_obj_set_style_text_color(ui_lblSelectTheme, lv_color_hex(0xFFFFFF), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_color(ui_lblWallpaper, lv_color_hex(0xFFFFFF), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_color(ui_lblRolllerTime, lv_color_hex(0xFFFFFF), LV_PART_MAIN | LV_STATE_DEFAULT);

    lv_obj_set_style_text_color(ui_lblWeather, lv_color_hex(0xFFFFFF), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_color(ui_lblDateAndTime, lv_color_hex(0xFFFFFF), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_color(ui_lblSettingsB, lv_color_hex(0xFFFFFF), LV_PART_MAIN | LV_STATE_DEFAULT);

    for (int i = 0; i < numOfDims; i++) {
        lv_obj_set_style_text_color(lblDims[i], lv_color_hex(0xFFFFFF), LV_PART_MAIN | LV_STATE_DEFAULT);
    }
}



// Helper function to create a notification
cJSON* create_notification(const char* type, const char* time, const char* date) {
    cJSON *notification = cJSON_CreateObject();
    cJSON_AddStringToObject(notification, "NotificationType", type);
    cJSON_AddStringToObject(notification, "NotificationTime", time);
    cJSON_AddStringToObject(notification, "NotificationDate", date);
    return notification;
}

// Function to create example notifications and add them to the JSON object
void add_example_notifications(cJSON *json, int numberOfNotifications) {
    cJSON *notifications = cJSON_CreateArray();

    for (int i = 0; i < numberOfNotifications && i < 10; i++) {
        char type[16];
        char time[6];
        char date[11];

        // Example data for notifications
        snprintf(type, sizeof(type), "Type%d", i + 1);
        snprintf(time, sizeof(time), "%02d:00", i + 8); // Example times from 08:00 to 17:00
        snprintf(date, sizeof(date), "2023-10-%02d", i + 1); // Example dates from 2023-10-01 to 2023-10-10

        cJSON_AddItemToArray(notifications, create_notification(type, time, date));
    }

    cJSON_AddItemToObject(json, "notifications", notifications);
}


// Example function to create JSON data packet as a C string
char* create_json_data_packet(const uint16_t* regs_data, int numOfOutputs, int numOfDims, int numOfSensors, bool slaveConnectionStatus, const char* themeType, int numberOfNotifications, cJSON* notifications) {
    // Create a JSON object
    cJSON *json = cJSON_CreateObject();

    // Add number of outputs, dims, sensors, slave connection status, and theme type to the JSON object
    cJSON_AddNumberToObject(json, "numOfOutputs", numOfOutputs);
    cJSON_AddNumberToObject(json, "numOfDims", numOfDims);
    cJSON_AddNumberToObject(json, "numOfSensors", numOfSensors);
    cJSON_AddBoolToObject(json, "slaveConnectionStatus", slaveConnectionStatus);
    cJSON_AddStringToObject(json, "themeType", "Hello");

    // Add outputs to the JSON object
    cJSON *outputs = cJSON_CreateObject();
    cJSON_AddItemToObject(json, "outputs", outputs);
    cJSON_AddNumberToObject(outputs, "output_1", regs_data[OUTPUT_1_INDIS]);
    cJSON_AddNumberToObject(outputs, "output_2", regs_data[OUTPUT_2_INDIS]);
    cJSON_AddNumberToObject(outputs, "output_3", regs_data[OUTPUT_3_INDIS]);
    cJSON_AddNumberToObject(outputs, "output_4", regs_data[OUTPUT_4_INDIS]);
    cJSON_AddNumberToObject(outputs, "output_5", regs_data[OUTPUT_5_INDIS]);
    cJSON_AddNumberToObject(outputs, "output_6", regs_data[OUTPUT_6_INDIS]);
    cJSON_AddNumberToObject(outputs, "output_7", regs_data[OUTPUT_7_INDIS]);
    cJSON_AddNumberToObject(outputs, "output_8", regs_data[OUTPUT_8_INDIS]);

    // Add analog sensors to the JSON object
    cJSON *analog_sensors = cJSON_CreateObject();
    cJSON_AddItemToObject(json, "analog_sensors", analog_sensors);
    cJSON_AddNumberToObject(analog_sensors, "analog_input_1", regs_data[ANALOG_INPUT_1_INDIS]);
    cJSON_AddNumberToObject(analog_sensors, "analog_input_2", regs_data[ANALOG_INPUT_2_INDIS]);
    cJSON_AddNumberToObject(analog_sensors, "analog_input_3", regs_data[ANALOG_INPUT_3_INDIS]);
    cJSON_AddNumberToObject(analog_sensors, "analog_input_4", regs_data[ANALOG_INPUT_4_INDIS]);


    // Add dims to the JSON object
    cJSON *dims = cJSON_CreateObject();
    cJSON_AddItemToObject(json, "dims", dims);
    cJSON_AddNumberToObject(dims, "dim_1", regs_data[DIMMABLE_OUTPUT_1_INDIS]);
    cJSON_AddNumberToObject(dims, "dim_2", regs_data[DIMMABLE_OUTPUT_1_INDIS]);

    // Add voltage and current to the JSON object
    cJSON_AddNumberToObject(json, "voltage", regs_data[VOLTAGE_INDIS] / 100.0);
    cJSON_AddNumberToObject(json, "current", regs_data[CURRENT_INDIS] / 100.0);

    // Add notifications to the JSON object
    cJSON_AddNumberToObject(json, "numberOfNotifications", numberOfNotifications);
    add_example_notifications(json, numberOfNotifications);


    // Convert JSON object to string
    char *json_str = cJSON_PrintUnformatted(json);

    // Free the JSON object
    cJSON_Delete(json);

    return json_str; // Caller is responsible for freeing the returned string
}



void show_weather_icon(int index) {
    lv_img_set_offset_x(ui_imgWForecast, index );
}


void set_weather_icon(int weather) {
    switch (weather) {
        case WEATHER_SUNNY:
            show_weather_icon(ICON_SUNNY);
            break;
        case WEATHER_PARTLY_SUNNY:
            show_weather_icon(ICON_PARTLY_SUNNY);
            break;
        case WEATHER_THUNDER:
            show_weather_icon(ICON_THUNDER);
            break;
        case WEATHER_RAINY:
            show_weather_icon(ICON_RAINY);
            break;
        case WEATHER_SNOWY:
            show_weather_icon(ICON_SNOWY);
            break;
        case WEATHER_CLOUDY:
            show_weather_icon(ICON_CLOUDY);
            break;
        default:
            ESP_LOGI(TAG, "Unknown weather condition: %d", weather);
            break;
    }
}



typedef struct {
    uint16_t output_buffer[16];
    char time[10];
    char date[11];
    char weather[20];
    int number_of_notifications;
    char notifications_buffer[10][50];
} mobile_app_stream_t;
mobile_app_stream_t stream;



void parse_json_data(const char* json_data, mobile_app_stream_t* stream) {
    cJSON *json = cJSON_Parse(json_data);
    if (json == NULL) {
        //ESP_LOGI(TAG, "Error parsing JSON data");
        return;
    }

    cJSON *stream_data = cJSON_GetObjectItem(json, "stream_data");
    if (stream_data == NULL) {
        ESP_LOGI(TAG, "Error getting stream_data");
        cJSON_Delete(json);
        return;
    }

    // Parse outputs
    cJSON *outputs = cJSON_GetObjectItem(stream_data, "outputs");
    if (outputs != NULL) {
        for (int i = 0; i < 16; i++) {
            char output_name[10];
            snprintf(output_name, sizeof(output_name), "output_%d", i + 1);
            cJSON *output = cJSON_GetObjectItem(outputs, output_name);
            if (output != NULL) {
                stream->output_buffer[i] = (uint16_t)output->valueint;
            }
        }
    }


    // Parse time and date
    cJSON *time_data = cJSON_GetObjectItem(stream_data, "time_data");
    if (time_data != NULL) {
        cJSON *time = cJSON_GetObjectItem(time_data, "time");
        cJSON *date = cJSON_GetObjectItem(time_data, "date");
        if (time != NULL && date != NULL) {
            strncpy(stream->time, time->valuestring, sizeof(stream->time) - 1);
            strncpy(stream->date, date->valuestring, sizeof(stream->date) - 1);

            // Merge time and date
            char date_and_time[22]; // Adjust size as needed
            snprintf(date_and_time, sizeof(date_and_time), "%s %s", stream->date, stream->time);

            // Set the merged time and date to the label
            lv_label_set_text(ui_lblDateAndTime, date_and_time);
        }
    }

    // Parse weather
    cJSON *weather = cJSON_GetObjectItem(stream_data, "weather");
    if (weather != NULL) {
        strncpy(stream->weather, weather->valuestring, sizeof(stream->weather) - 1);
        if (strcmp(weather->valuestring, "Sunny") == 0) {
            set_weather_icon(WEATHER_SUNNY);
        } else if (strcmp(weather->valuestring, "Partly Sunny") == 0) {
            set_weather_icon(WEATHER_PARTLY_SUNNY);
        } else if (strcmp(weather->valuestring, "Thunder") == 0) {
            set_weather_icon(WEATHER_THUNDER);
        } else if (strcmp(weather->valuestring, "Rainy") == 0) {
            set_weather_icon(WEATHER_RAINY);
        } else if (strcmp(weather->valuestring, "Snowy") == 0) {
            set_weather_icon(WEATHER_SNOWY);
        } else if (strcmp(weather->valuestring, "Cloudy") == 0) {
            set_weather_icon(WEATHER_CLOUDY);
        } else {
            ESP_LOGI(TAG, "Unknown weather condition: %s", weather->valuestring);
        }
    }

    // Parse notifications
    cJSON *notifications = cJSON_GetObjectItem(stream_data, "notifications");
    if (notifications != NULL) {
        cJSON *number_of_notifications = cJSON_GetObjectItem(notifications, "number_of_notifications");
        if (number_of_notifications != NULL) {
            stream->number_of_notifications = number_of_notifications->valueint;
        }

        for (int i = 0; i < stream->number_of_notifications && i < 10; i++) {
            char notification_name[20];
            snprintf(notification_name, sizeof(notification_name), "notification_%d", i + 1);
            cJSON *notification = cJSON_GetObjectItem(notifications, notification_name);
            if (notification != NULL) {
                strncpy(stream->notifications_buffer[i], notification->valuestring, sizeof(stream->notifications_buffer[i]) - 1);
            }
        }
    }

    cJSON_Delete(json);

    // Log parsed data
    ESP_LOGI(TAG, "Outputs:");
    for (int i = 0; i < 16; i++) {
        ESP_LOGI(TAG, "output_%d: %d", i + 1, stream->output_buffer[i]);
    }
    ESP_LOGI(TAG, "Time: %s", stream->time);
    ESP_LOGI(TAG, "Date: %s", stream->date);
    ESP_LOGI(TAG, "Weather: %s", stream->weather);
    ESP_LOGI(TAG, "Number of Notifications: %d", stream->number_of_notifications);
    for (int i = 0; i < stream->number_of_notifications; i++) {
        ESP_LOGI(TAG, "Notification %d: %s", i + 1, stream->notifications_buffer[i]);
    }
}

//-------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------
//COOOOOOOOOOOOOOOOOOOOOK Onemli bilgi. Mutlaka Bu Videoyu izlemen lazim.
//https://www.youtube.com/watch?v=gCxBAK9EByA

//https://www.youtube.com/@usefulelectronics/videos
//-------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------


// Function to set the image source based on connection status
void set_device_image(bool connected) {
    static bool condition = false;
    if (connected) {
        lv_obj_clear_flag(ui_imgsconnected, LV_OBJ_FLAG_HIDDEN);
        lv_obj_add_flag(ui_imgsnotconnected, LV_OBJ_FLAG_HIDDEN);
        condition = true;
    } else {
        lv_obj_clear_flag(ui_imgsnotconnected, LV_OBJ_FLAG_HIDDEN);
        lv_obj_add_flag(ui_imgsconnected, LV_OBJ_FLAG_HIDDEN);
        if (condition && !connected)
        {
            condition = false;
            lv_obj_clear_flag(ui_pnlConnectionLost, LV_OBJ_FLAG_HIDDEN);     /// Flags
            lv_obj_move_foreground(ui_pnlConnectionLost);
        }
        
    }  
}


// Function to set the image source based on connection status
void set_bluetooth_icon(bool connected) {
    if (!connected) {
        lv_obj_clear_flag(ui_imgBluetoothNotConnected, LV_OBJ_FLAG_HIDDEN);
        lv_obj_add_flag(ui_imgBluetoothConnected, LV_OBJ_FLAG_HIDDEN);
    } else {
        lv_obj_add_flag(ui_imgBluetoothNotConnected, LV_OBJ_FLAG_HIDDEN);
        lv_obj_clear_flag(ui_imgBluetoothConnected, LV_OBJ_FLAG_HIDDEN);
    }
}


int initBarCounter = 0;
int initCounter = 0;
int scrMode = 0;
static void init_timer(lv_timer_t * timer) {
    if (initBarCounter < 10) {
        lv_bar_set_value(ui_brInit, initBarCounter * 10, LV_ANIM_OFF);   
        
    }
    initBarCounter++;


    
    if (panelThemeType == 0) {
        lv_obj_set_style_bg_color(ui_scrMain, lv_color_hex(0xFFFFFF), LV_PART_MAIN | LV_STATE_DEFAULT);
    } else {
        lv_obj_set_style_bg_color(ui_scrMain, lv_color_hex(0x000000), LV_PART_MAIN | LV_STATE_DEFAULT);
    } 


}

// Timer callback function
static void timer_updateTimer_callback(lv_timer_t * timer) {
    if (initCounter < 3) {
        if (scrMode == 0) {
            scrMode = 1;
        }
    }
    else {
        if (scrMode == 1) {
            lv_scr_load(ui_scrMain);
            scrMode = 0;
        }
        initCounter = 11;
        // Your code here, e.g., update display with new data
        const uint16_t* regs_data = getSlavesRegsData();
        update_display_with_data((const uint8_t*)regs_data, 70);
    }
    initCounter++;

}


static void wallpaper_update_timer_callback(lv_timer_t * timer) {
    if (panelWallpaperEnable) {
        if (panelWallpaperEnableCounter == panelWallpaperTime) {
            lv_scr_load(ui_scrWallpaper);

        }
        panelWallpaperEnableCounter++;
    }
    else {
        panelWallpaperEnableCounter = 0;
    }
    
}

// Function to set the button color based on the value
void set_button_color(lv_obj_t *btn, uint16_t value, int connected) {
    if (connected == 0) {
        lv_obj_set_style_bg_color(btn, lv_color_hex(0x5F5F5F), LV_PART_MAIN | LV_STATE_DEFAULT); // Gray
        return;
    }
    if (value == 1) {
        lv_obj_set_style_bg_color(btn, lv_color_hex(0x37C600), LV_PART_MAIN | LV_STATE_DEFAULT); // Green
    } else if (value == 0) {
        lv_obj_set_style_bg_color(btn, lv_color_hex(0x5A5A5A), LV_PART_MAIN | LV_STATE_DEFAULT); // Gray
    } else if (value == 2) {
        lv_obj_set_style_bg_color(btn, lv_color_hex(0xC60000), LV_PART_MAIN | LV_STATE_DEFAULT); // Red
    }
}

void get_data_json_format(const uint16_t* regs_data, int txPacketType, char** json_str)  {

    // Call create_json_data_packet function
    *json_str = create_json_data_packet(regs_data, numOfOutputs, numOfDims, numOfSensors, slaveConnectionStatus, panelThemeType, numberOfNotifications, notifications);
    cJSON_Delete(notifications);
} 





// Function to update display with new data
void update_display_with_data(const uint8_t *data, int length) {
    const uint16_t* regs_data = (const uint16_t*)data;

    // Fetch data from registers
    uint16_t analog_input_1 = regs_data[ANALOG_INPUT_1_INDIS];
    uint16_t analog_input_2 = regs_data[ANALOG_INPUT_2_INDIS];
    uint16_t analog_input_3 = regs_data[ANALOG_INPUT_3_INDIS];
    uint16_t analog_input_4 = regs_data[ANALOG_INPUT_4_INDIS];
    float batarya_volt = regs_data[VOLTAGE_INDIS] / 100.0;
    float amper = regs_data[CURRENT_INDIS] / 100.0;

    // Convert voltage to string with comma
    char batarya_volt_str[10];
    if (batarya_volt < 7.0 || batarya_volt > 16.0) {
        ESP_LOGW(TAG, "Voltage out of range: %.2fV", batarya_volt);
        snprintf(batarya_volt_str, sizeof(batarya_volt_str), "Fail");
    } else {
        int before_comma_volt = (int)batarya_volt;
        int after_comma_volt = (int)((batarya_volt - before_comma_volt) * 100);
        snprintf(batarya_volt_str, sizeof(batarya_volt_str), "%d,%02dV", before_comma_volt, after_comma_volt);
    }

    // Convert current to string with comma
    char amper_str[10];
    if (amper < 0 || amper > 100.0) {
        ESP_LOGW(TAG, "Current out of range: %.2fA", amper);
        snprintf(amper_str, sizeof(amper_str), "Fail");
    } else {
        int before_comma_amper = (int)amper;
        int after_comma_amper = (int)((amper - before_comma_amper) * 100);
        snprintf(amper_str, sizeof(amper_str), "%d,%02dA", before_comma_amper, after_comma_amper);
    }


    // Update the display labels with the fetched data
    lv_label_set_text_fmt(ui_lblPnlGrup1SicaklikDeger1, "%d°C", analog_input_1);
    lv_label_set_text_fmt(ui_lblPnlGrup1SicaklikDeger2, "%d°C", analog_input_2);
    lv_label_set_text_fmt(ui_lblGrup1Oran1, "%d%%", analog_input_3);
    lv_label_set_text_fmt(ui_lblGrup1Oran2, "%d%%", analog_input_4);
    //lv_label_set_text(ui_lblBataryaVolt, batarya_volt_str);
    //lv_label_set_text(ui_lblAmper, amper_str);

    // Update the arcs with the fetched data
    lv_arc_set_value(ui_arcGrup1, analog_input_3);
    lv_arc_set_value(ui_arcGrup2, analog_input_4);


    // Check Modbus connection status and update device image
    bool Deviceconnected = get_modbus_connection_status();
    set_device_image(Deviceconnected);

    bool btConnected = get_connection_status();
    set_bluetooth_icon(btConnected);


    char* converted_json_data;
    get_data_json_format(regs_data, 0, &converted_json_data);
    set_converted_json_data(converted_json_data);



    parse_json_data((const char*)get_spp_cmd_buff(), &stream);
    reset_spp_cmd_buff();


    // Ensure create_dynamic_ui is called only once
    static int ui_initialized = 0;
    if (ui_initialized==2) {
        create_dynamic_ui(ui_scrMain);
        ui_initialized = 3;
    }
    if (ui_initialized > 3 && Deviceconnected) {
        for (int i = 0; i < numOfOutputs; i++) {
            set_button_color(btnIO[i], regs_data[OUTPUT_1_INDIS + i], Deviceconnected);
        }
    }
    ui_initialized++;
}

  

// Function to save configuration data to NVS
void save_panel_configuration_to_nvs(int totalOutps, int buffer1[16], int totalSensors, int buffer2[5], int totalDims, int buffer3[4]) {
    // Ensure the values do not exceed the maximum allowed sizes
    if (totalOutps > 16) {
        totalOutps = 16;
    }
    if (totalSensors > 5) {
        totalSensors = 5;
    }
    if (totalDims > 4) {
        totalDims = 4;
    }

    // Save totalOutps to NVS
    nvs_write_int("numOfOutputs", totalOutps);

    // Save buffer1 to NVS
    for (int i = 0; i < 16; i++) {
        if (buffer1[i] < 1 || buffer1[i] > 18) {
            buffer1[i] = 1; // Set to default value if out of range
        }
        char key[16];
        snprintf(key, sizeof(key), "outBuf%d", i);
        nvs_write_int(key, buffer1[i]);
    }

    // Save totalSensors to NVS
    nvs_write_int("numSens", totalSensors);

    // Save buffer2 to NVS
    for (int i = 0; i < 5; i++) {
        if (buffer2[i] < 0 || buffer2[i] > 1) {
            buffer2[i] = 0; // Set to default value if out of range
        }
        char key[16];
        snprintf(key, sizeof(key), "sensBuf%d", i);
        nvs_write_int(key, buffer2[i]);
    }

    // Save totalDims to NVS
    nvs_write_int("numDims", totalDims);

    // Save buffer3 to NVS
    for (int i = 0; i < 4; i++) {
        if (buffer3[i] < 0 || buffer3[i] > 8) {
            buffer3[i] = 0; // Set to default value if out of range
        }
        char key[16];
        snprintf(key, sizeof(key), "dimsBuf%d", i);
        nvs_write_int(key, buffer3[i]);
    }
}

void save_theme_configuration_to_nvs(int16_t* themeType, uint16_t* wallpaperEnabled, uint16_t* wallpaperTimeIndex){


    //Save themeType to NVS
    nvs_write_int("thmTyp", panelThemeType);

    //Save wallpaperEnabled to NVS
    nvs_write_int("wallpEn", panelWallpaperEnable);

    //Save wallpaperTimeIndex to NVS
    nvs_write_int("wllTimI", panelWallpaperTime);


}


// Debug function to print parameters to the screen
void debug_print_configuration(int totalOutpts, int buffer1[16], int totalSensors, int buffer2[5], int totalDims, int buffer3[4]) {
    ESP_LOGI("DEBUG", "totalOutpts: %d", totalOutpts);
    for (int i = 0; i < 16; i++) {
        ESP_LOGI("DEBUG", "buffer1[%d]: %d", i, buffer1[i]);
    }
    ESP_LOGI("DEBUG", "totalSensors: %d", totalSensors);
    for (int i = 0; i < 5; i++) {
        ESP_LOGI("DEBUG", "buffer2[%d]: %d", i, buffer2[i]);
    }
    ESP_LOGI("DEBUG", "totalDims: %d", totalSensors);
    for (int i = 0; i < 4; i++) {
        ESP_LOGI("DEBUG", "buffer3[%d]: %d", i, buffer3[i]);
    }
}

// Function to read configuration data from NVS
void load_panel_configuration_from_nvs(int *totalOutpts, int buffer1[16], int *totalSensors, int buffer2[5], int *totalDims, int buffer3[4]) {
    // Read totalOutpts from NVS
    if (nvs_read_int("numOfOutputs", totalOutpts) != ESP_OK || *totalOutpts < 0 || *totalOutpts > 16) {
        *totalOutpts = 4; // Set to default value if out of range
    }

    // Read buffer1 from NVS
    for (int i = 0; i < 16; i++) {
        char key[16];
        snprintf(key, sizeof(key), "outBuf%d", i);
        if (nvs_read_int(key, &buffer1[i]) != ESP_OK || buffer1[i] < 1 || buffer1[i] > 18) {
            buffer1[i] = 1; // Set to default value if out of range
        }
    }

    // Read totalSensors from NVS
    if (nvs_read_int("numSens", totalSensors) != ESP_OK || *totalSensors < 0 || *totalSensors > 5) {
        *totalSensors = 1; // Set to default value if out of range
    }

    // Read buffer2 from NVS
    for (int i = 0; i < 5; i++) {
        char key[16];
        snprintf(key, sizeof(key), "sensBuf%d", i);
        if (nvs_read_int(key, &buffer2[i]) != ESP_OK || buffer2[i] < 0 || buffer2[i] > 1) {
            buffer2[i] = 1; // Set to default value if out of range
        }
    }

    // Read totalSensors from NVS
    if (nvs_read_int("numDims", totalDims) != ESP_OK || *totalDims < 0 || *totalDims > 4) {
        *totalDims = 1; // Set to default value if out of range
    }

    // Read buffer3 from NVS
    for (int i = 0; i < 4; i++) {
        char key[16];
        snprintf(key, sizeof(key), "dimsBuf%d", i);
        if (nvs_read_int(key, &buffer3[i]) != ESP_OK || buffer3[i] < 1 || buffer3[i] > 8) {
            buffer3[i] = 1; // Set to default value if out of range
        }
    }

}



// Load the theme settings from NVS
void load_theme_configuration_from_nvs(int* themeType, int* wallpaperEnabled, int* wallpaperTimeIndex) {

    // Read themeType from NVS
    if (nvs_read_int("thmTyp", themeType) != ESP_OK || *themeType < 0 || *themeType > 1) {
        *themeType = 1; // Set to default value if out of range
    }


    // Read wallpaperEnabled from NVS
    if (nvs_read_int("wallpEn", wallpaperEnabled) != ESP_OK || *wallpaperEnabled < 0 || *wallpaperEnabled > 1) {
        *wallpaperEnabled = 1; // Set to default value if out of range
    }

    // Read wallpaperEnabled from NVS
    if (nvs_read_int("wllTimI", wallpaperTimeIndex) != ESP_OK || *wallpaperTimeIndex < 0 || *wallpaperTimeIndex > 600) {
        *wallpaperTimeIndex = 30; // Set to default value if out of range
    }


    // Log the loaded configuration
    ESP_LOGI(TAG, "Loaded Theme Configuration: Theme=%d, WallpaperEnabled=%d, WallpaperTimeIndex=%d",
             *themeType, *wallpaperEnabled, *wallpaperTimeIndex);

}



// Function to check switches and get corresponding dropdown values
void check_switches_and_get_dropdown_values() {
    // Reset the outputsBuffer
    memset(outputsBuffer, 0, sizeof(outputsBuffer));

    numOfOutputs = 0; // Initialize numOfOutputs
    // Apply numOfOutputs and outputsBuffer to swO1-swO16 and cbxO1-cbxO16
    lv_obj_t* switches[16] = {ui_swO1, ui_swO2, ui_swO3, ui_swO4, ui_swO5, ui_swO6, ui_swO7, ui_swO8, ui_swO9, ui_swO10, ui_swO11, ui_swO12, ui_swO13, ui_swO14, ui_swO15, ui_swO16};
    lv_obj_t* dropdowns[16] = {ui_cbxO1, ui_cbxO2, ui_cbxO3, ui_cbxO4, ui_cbxO5, ui_cbxO6, ui_cbxO7, ui_cbxO8, ui_cbxO9, ui_cbxO10, ui_cbxO11, ui_cbxO12, ui_cbxO13, ui_cbxO14, ui_cbxO15, ui_cbxO16};


    for (int i = 0; i < 16; i++) {
        if (lv_obj_has_state(switches[i], LV_STATE_CHECKED)) {
            outputsBuffer[i] = 1 + lv_dropdown_get_selected(dropdowns[i]);
            numOfOutputs++; // Increment numOfOutputs for each checked switch
            ESP_LOGI("SWITCH_CHECK", "Switch %d is checked. Dropdown value index: %d", i + 1, outputsBuffer[i]);
        } else {
            outputsBuffer[i] = 0; // Indicate that the switch is not checked
        }
    }

}


// Function to check switches and get corresponding dropdown values
void check_switches_and_get_dropdown_values_for_dims() {
    // Reset the outputsBuffer
    memset(dimsBuffer, 0, sizeof(dimsBuffer));

    numOfDims = 0; // Initialize numOfOutputs
    // Apply numOfOutputs and outputsBuffer to swO1-swO16 and cbxO1-cbxO16
    lv_obj_t* dimcheckboxes[4] = {ui_swDim1, ui_swDim2, ui_swDim3, ui_swDim4};
    lv_obj_t* dimdropdowns[4] = {ui_cbxDim1, ui_cbxDim2, ui_cbxDim3, ui_cbxDim4};

    for (int i = 0; i < 4; i++) {
        if (lv_obj_has_state(dimcheckboxes[i], LV_STATE_CHECKED)) {
            dimsBuffer[i] = 1 + lv_dropdown_get_selected(dimdropdowns[i]);
            numOfDims++; // Increment numOfOutputs for each checked switch
            ESP_LOGI("SWITCH_CHECK", "Switch %d is checked. Dropdown value index: %d", i + 1, dimsBuffer[i]);
        } else {
            dimsBuffer[i] = 0; // Indicate that the switch is not checked
        }
    }

}


// Function to check the state of the first 5 switches and update sensorsBuffer
void check_sensors_and_update_buffer() {
    // Reset the sensorsBuffer
    memset(sensorsBuffer, 0, sizeof(sensorsBuffer));

    numOfSensors = 0; // Initialize numOfSensors

    lv_obj_t* switches[5] = {ui_Checkbox1, ui_Checkbox2, ui_Checkbox3, ui_Checkbox4, ui_Checkbox5};

    for (int i = 0; i < 5; i++) {
        if (lv_obj_has_state(switches[i], LV_STATE_CHECKED)) {
            sensorsBuffer[i] = 1; // Indicate that the switch is checked
            numOfSensors++; // Increment numOfSensors for each checked switch
        } else {
            sensorsBuffer[i] = 0; // Indicate that the switch is not checked
        }
        ESP_LOGI("SWITCH_CHECK", "Switch %d is checked. Value: %d", i + 1, sensorsBuffer[i]);
    }

    ESP_LOGI("SWITCH_CHECK", "Total number of checked switches: %d", numOfSensors);
}

void save_panel_settings()
{
    check_switches_and_get_dropdown_values();
    check_sensors_and_update_buffer();
    check_switches_and_get_dropdown_values_for_dims();
    save_panel_configuration_to_nvs(numOfOutputs, outputsBuffer, numOfSensors, sensorsBuffer, numOfDims, dimsBuffer);
    // Save the panel settings to NVS
    ESP_LOGI(TAG, "##### Panel Settings Saved Successfully! #####");
}

void save_theme_settings()
{
    uint16_t selected_index = 0;
    static char selected_text[32];  // Buffer to store text

    selected_index = lv_roller_get_selected(ui_rlrTime);
    // Get the selected item text
    lv_roller_get_selected_str(ui_rlrTime, selected_text, sizeof(selected_text));
    // Check if switch is enabled (ON)
    if (lv_obj_has_state(ui_swEnableWallpaper, LV_STATE_CHECKED)) {
        // Get the selected roller item index
        panelWallpaperEnable = 1;  // Enable wallpaper
        // Print the selected roller item
        ESP_LOGI(TAG, "Wallpaper Enabled, Selected Time: %s-----index = %d", selected_text, selected_index);
    } else {
        panelWallpaperEnable = 0;  // Disable wallpaper
        ESP_LOGI(TAG, "Wallpaper Disabled");
    }

    // Set panelWallpaperTime based on the selected index
    switch (selected_index) {
        case 0: { panelWallpaperTime = 30;  break;}
        case 1: { panelWallpaperTime = 60;  break;}
        case 2: { panelWallpaperTime = 120; break;}
        case 3: { panelWallpaperTime = 300; break;}
        case 4: { panelWallpaperTime = 600; break;}
        default:
            {
                ESP_LOGW(TAG, "Invalid roller index: %d, setting default to 30", selected_index);
                panelWallpaperTime = 30;  // Default if index is out of range
                break;
            }
    }
    ESP_LOGI(TAG, "Wallpaper Enabled, Selected Time: %s-----index = %d panelThemeType =%d panelWallpaperEnable =%d panelWallpaperTime =%d ", selected_text, selected_index,
             panelThemeType, panelWallpaperEnable, panelWallpaperTime);
    save_theme_configuration_to_nvs(panelThemeType, panelWallpaperEnable, panelWallpaperTime);
}


void apply_theme_settings()
{
    // Apply theme enabled status to the switch
    if (panelWallpaperEnable) {
        lv_obj_add_state(ui_swEnableWallpaper, LV_STATE_CHECKED);
        lv_obj_clear_flag(ui_rlrTime, LV_OBJ_FLAG_HIDDEN);     /// Flags
        lv_obj_clear_flag(ui_lblRolllerTime, LV_OBJ_FLAG_HIDDEN);     /// Flags
    } else {
        lv_obj_clear_state(ui_swEnableWallpaper, LV_STATE_CHECKED);
        lv_obj_add_flag(ui_rlrTime, LV_OBJ_FLAG_HIDDEN);     /// Flags
        lv_obj_add_flag(ui_lblRolllerTime, LV_OBJ_FLAG_HIDDEN);     /// Flags
    }

    // Map panelWallpaperTime to the roller index
    uint16_t roller_index = 0;

    switch (panelWallpaperTime) {
        case 30:  roller_index = 0; break;
        case 60:  roller_index = 1; break;
        case 120: roller_index = 2; break;
        case 300: roller_index = 3; break;
        case 600: roller_index = 4; break;
        default:
            ESP_LOGW(TAG, "Invalid panelWallpaperTime: %d, setting default to 30s", panelWallpaperTime);
            roller_index = 0;  // Default to first option
            break;
    }

    // Set roller selection
    lv_roller_set_selected(ui_rlrTime, roller_index, LV_ANIM_OFF);

    // Log applied settings
    ESP_LOGI(TAG, " ############################Applied Theme Settings:panelThemeType = %d panelWallpaperEnable=%d, WallpaperTimeIndex=%d",panelThemeType, panelWallpaperEnable, roller_index);
}



// Callback function for color changes
 void color_wheel_event_cb() {
    lv_color_t selected_color = lv_colorwheel_get_rgb(ui_Colorwheel1); // Get selected color
    // Apply selected color to the panel background
    lv_obj_set_style_bg_color(ui_btnRGBColor, selected_color, LV_PART_MAIN | LV_STATE_DEFAULT);
}





void display_manager_init() {
  
    waveshare_esp32_s3_rgb_lcd_init(); // Initialize the Waveshare ESP32-S3 RGB LCD 
     wavesahre_rgb_lcd_bl_on();  //Turn on the screen backlight 
    // wavesahre_rgb_lcd_bl_off(); //Turn off the screen backlight 
    
    lv_timer_t * updateScreentimer = lv_timer_create(timer_updateTimer_callback, 1000, NULL);
    lv_timer_t * wallpaperTimer = lv_timer_create(wallpaper_update_timer_callback, 1000, NULL);
    lv_timer_t * initTim = lv_timer_create(init_timer, 200, NULL);
    


     load_panel_configuration_from_nvs(&numOfOutputs, outputsBuffer, &numOfSensors, sensorsBuffer, &numOfDims, dimsBuffer);
     load_theme_configuration_from_nvs(&panelThemeType, &panelWallpaperEnable, &panelWallpaperTime);

            // Debug log the loaded configuration
    ESP_LOGW(TAG, "Loaded Panel Configuration:");
    ESP_LOGW(TAG, "numOfOutputs: %d", numOfOutputs);
    for (int i = 0; i < 16; i++) {
        ESP_LOGW(TAG, "outputsBuffer[%d]: %d", i, outputsBuffer[i]);
    }
    ESP_LOGW(TAG, "numOfSensors: %d", numOfSensors);
    for (int i = 0; i < 5; i++) {
        ESP_LOGW(TAG, "sensorsBuffer[%d]: %d", i, sensorsBuffer[i]);
    }
    ESP_LOGW(TAG, "numOfDims: %d", numOfDims);
    for (int i = 0; i < 4; i++) {
        ESP_LOGW(TAG, "dimsBuffer[%d]: %d", i, dimsBuffer[i]);
    }
    ESP_LOGW(TAG, "panelThemeType: %d", panelThemeType);
    ESP_LOGW(TAG, "panelWallpaperEnable: %d", panelWallpaperEnable);
    ESP_LOGW(TAG, "panelWallpaperTime: %d", panelWallpaperTime);

   


    ESP_LOGI(TAG, "Display LVGL Scatter Chart");
    // Lock the mutex due to the LVGL APIs are not thread-safe
    if (lvgl_port_lock(-1)) {
         //example_lvgl_demo_ui(disp);
        //lv_demo_widgets();
        // lv_demo_benchmark();
        // lv_demo_music();
        // lv_demo_stress();
        // Release the mutex
        ui_init();
        lv_scr_load(ui_scrInit);
        lvgl_port_unlock();
    }
}