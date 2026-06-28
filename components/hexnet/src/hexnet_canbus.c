#include "hexnet_canbus.h"
#include <inttypes.h>
#include <stdio.h>
#include <stdlib.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"
#include "freertos/semphr.h"
#include "esp_err.h"
#include "esp_log.h"
#include "driver/twai.h"
#include <esp_timer.h>



#define GPIO_OUTPUT_PIN_SEL  ((1ULL<<20) | (1ULL<<19))

/* --------------------- Definitions and static variables ------------------ */
//Example Configuration
#define TX_GPIO_NUM             20
#define RX_GPIO_NUM             19
// #define TX_GPIO_NUM             15
// #define RX_GPIO_NUM             16
#define EXAMPLE_TAG             "TWAI Master"

static bool driver_installed = false;
#define POLLING_RATE_MS 1000

// Error recovery tracking variables
static uint32_t error_recovery_count = 0; // Error recovery counter
static uint32_t last_successful_tx = 0;   // Last successful transmission timestamp
static uint32_t last_health_check = 0;    // Last health check timestamp
static uint32_t last_recovery_attempt = 0; // Last recovery attempt timestamp

// Forward declarations
static void check_and_recover_from_errors(void);
static void monitor_can_health(void);
static bool is_panel_cfg_frame(uint32_t id);
static void send_can_response_frame(const twai_message_t *rx_message);
static void handle_panel_cfg_frame(const twai_message_t *message);
static uint32_t panel_cfg_expected_mask(int count);
extern void save_panel_configuration_to_nvs(int totalOutps, int buffer1[16],
                                            int totalSensors, int buffer2[5],
                                            int totalDims, int buffer3[4]);

static const twai_timing_config_t t_config = TWAI_TIMING_CONFIG_500KBITS();
static const twai_filter_config_t f_config = TWAI_FILTER_CONFIG_ACCEPT_ALL();
static const twai_general_config_t g_config = TWAI_GENERAL_CONFIG_DEFAULT(TX_GPIO_NUM, RX_GPIO_NUM, TWAI_MODE_NORMAL);




#define FRAME_1_ID  0x100
#define FRAME_2_ID  0x200
#define FRAME_3_ID  0x300
#define PANEL_CFG_TOTALS_ID 0x400
#define PANEL_CFG_OUTPUT_ID 0x401
#define PANEL_CFG_SENSOR_ID 0x402
#define PANEL_CFG_DIM_ID 0x403
#define DISPLAY_RESET_CMD_ID 0x404
#define PANEL_CFG_MAX_OUTPUTS 16
#define PANEL_CFG_MAX_SENSORS 5
#define PANEL_CFG_MAX_DIMS 4
#define PANEL_CFG_RESTART_DELAY_MS 250

static const char *TAG = "CAN_HANDLER";

// Global variables to store received data
uint16_t voltage = 0;
uint16_t outputs = 0;
uint16_t inputs = 0;
uint8_t analog_inputs[5] = {0};
uint8_t dimmable_outputs[4] = {0};
uint8_t rgb_values[4] = {0};
uint8_t rgb_enabled = 0; // RGB'nin aktif olup olmadığını tutar
uint8_t canbusConnection = 0;
uint8_t sensorTemp = 0;
uint8_t sensorHum = 0;

static bool panel_cfg_totals_received = false;
static int panel_cfg_total_outputs = 0;
static int panel_cfg_total_sensors = 0;
static int panel_cfg_total_dims = 0;
static int panel_cfg_outputs[PANEL_CFG_MAX_OUTPUTS] = {0};
static int panel_cfg_sensors[PANEL_CFG_MAX_SENSORS] = {0};
static int panel_cfg_dims[PANEL_CFG_MAX_DIMS] = {0};
static uint32_t panel_cfg_output_received_mask = 0;
static uint32_t panel_cfg_sensor_received_mask = 0;
static uint32_t panel_cfg_dim_received_mask = 0;


// Getter Functions
uint16_t get_voltage() {
    return voltage;
}

uint16_t get_outputs() {
    return outputs;
}

uint16_t get_inputs() {
    return inputs;
}
uint8_t get_sensorTemp() {
    return sensorTemp;
}
uint8_t get_sensorHum() {
    return sensorHum;
}

uint8_t get_analog_input(uint8_t index) {
    if (index < 5) {
        return analog_inputs[index];
    }
    return 0;  // Hatalı index
}

uint8_t get_dimmable_output(uint8_t index) {
    if (index < 4) {
        return dimmable_outputs[index];
    }
    return 0;  // Hatalı index
}

uint8_t get_rgb_value(uint8_t index) {
    if (index < 4) {
        return rgb_values[index];
    }
    return 0;  // Hatalı index
}


uint8_t get_canbus_connection_status() {
    return canbusConnection;
}

// Error recovery function
static void check_and_recover_from_errors() {
    uint32_t current_time = esp_timer_get_time() / 1000;
    
    // Prevent excessive recovery attempts (max once every 10 seconds)
    if (current_time - last_recovery_attempt < 10000) {
        return;
    }
    last_recovery_attempt = current_time;
    
    twai_status_info_t status;
    twai_get_status_info(&status);
    
    // Check for Bus-Off condition
    if (status.state == TWAI_STATE_BUS_OFF) {
        ESP_LOGW(TAG, "CAN Bus-Off detected! Attempting recovery...");
        error_recovery_count++;
        
        // Stop and restart the driver
        twai_stop();
        vTaskDelay(pdMS_TO_TICKS(100));
        
        if (twai_start() == ESP_OK) {
            ESP_LOGI(TAG, "CAN driver restarted successfully (recovery #%"PRIu32")", error_recovery_count);
        } else {
            ESP_LOGE(TAG, "Failed to restart CAN driver (recovery #%"PRIu32")", error_recovery_count);
        }
    }
    
    // Check for high error counts
    if (status.tx_error_counter > 100 || status.rx_error_counter > 100) {
        ESP_LOGW(TAG, "High error counts detected");
        
        // Reset error counters
        twai_initiate_recovery();
        ESP_LOGI(TAG, "CAN error recovery initiated");
    }
}

// Watchdog monitoring function
static void monitor_can_health() {
    uint32_t current_time = esp_timer_get_time() / 1000;
    
    // Only check health every 5 seconds to prevent excessive calls
    if (current_time - last_health_check < 5000) {
        return;
    }
    last_health_check = current_time;
    
    // Check if we haven't had a successful transmission in 10 seconds
    if (last_successful_tx > 0 && (current_time - last_successful_tx) > 10000) {
        ESP_LOGW(TAG, "No successful CAN transmission");
        check_and_recover_from_errors();
    }
}

// Watchdog task to monitor CAN bus health
void can_watchdog_task(void *pvParameter) {
    uint32_t last_status_check = 0;
    uint32_t consecutive_failures = 0;
    
    while(1) {
        uint32_t current_time = esp_timer_get_time() / 1000;
        
        // Check CAN status every 5 seconds
        if (current_time - last_status_check >= 5000) {
            twai_status_info_t status;
            twai_get_status_info(&status);
            
            //ESP_LOGI(TAG, "CAN Status check");
            
            // Check for problematic states
            if (status.state == TWAI_STATE_BUS_OFF) {
                ESP_LOGE(TAG, "CAN Bus-Off state detected by watchdog!");
                check_and_recover_from_errors();
                consecutive_failures++;
            } else if (status.state == TWAI_STATE_RECOVERING) {
                ESP_LOGW(TAG, "CAN in recovery state");
            } else if (status.state == TWAI_STATE_RUNNING) {
                consecutive_failures = 0; // Reset failure counter on successful state
            }
            
            // If we have too many consecutive failures, try a full restart
            if (consecutive_failures >= 3) {
                ESP_LOGE(TAG, "Too many CAN failures, restarting");
                
                // Stop and uninstall driver
                twai_stop();
                vTaskDelay(pdMS_TO_TICKS(1000));
                twai_driver_uninstall();
                vTaskDelay(pdMS_TO_TICKS(1000));
                
                // Reinitialize
                twai_ini();
                if (driver_installed) {
                    ESP_LOGI(TAG, "CAN driver fully restarted successfully");
                    consecutive_failures = 0;
                } else {
                    ESP_LOGE(TAG, "Failed to restart CAN driver");
                }
            }
            
            last_status_check = current_time;
        }
        
        vTaskDelay(pdMS_TO_TICKS(1000)); // Check every second
    }
}

// Function to send a CAN frame
void send_can_frame(uint32_t id, uint8_t *data) {
    twai_message_t message = {0};
    message.identifier = id;
    message.rtr = 0;  // Data frame
    message.data_length_code = 8;  // Max CAN data length is 8 bytes
    memcpy(message.data, data, 8);

    // Send the message over the CAN bus
    esp_err_t res = twai_transmit(&message, pdMS_TO_TICKS(100));  // 100 ms timeout
    if (res != ESP_OK) {
        ESP_LOGE(TAG, "Failed to send frame ID: 0x%03X, error: 0x%x", (unsigned int)id, res);
        // Check if we need to recover from errors
        check_and_recover_from_errors();
    } else {
        last_successful_tx = esp_timer_get_time() / 1000; // Update successful TX timestamp
    }
}

static bool is_panel_cfg_frame(uint32_t id) {
    return id == PANEL_CFG_TOTALS_ID ||
           id == PANEL_CFG_OUTPUT_ID ||
           id == PANEL_CFG_SENSOR_ID ||
           id == PANEL_CFG_DIM_ID    ||
           id == DISPLAY_RESET_CMD_ID;
}

static void send_can_response_frame(const twai_message_t *rx_message) {
    twai_message_t response = {0};
    uint8_t dlc = rx_message->data_length_code;

    if (dlc > 8) {
        dlc = 8;
    }

    response.identifier = rx_message->identifier;
    response.rtr = 0;
    response.data_length_code = dlc;
    memcpy(response.data, rx_message->data, dlc);

    esp_err_t res = twai_transmit(&response, pdMS_TO_TICKS(100));
    if (res != ESP_OK) {
        ESP_LOGE(TAG, "Failed to send response frame ID: 0x%03X, error: 0x%x",
                 (unsigned int)response.identifier, res);
        check_and_recover_from_errors();
    } else {
        last_successful_tx = esp_timer_get_time() / 1000;
    }
}

static uint32_t panel_cfg_expected_mask(int count) {
    if (count <= 0) {
        return 0;
    }

    return (1U << count) - 1U;
}

static void handle_panel_cfg_frame(const twai_message_t *message) {
    switch (message->identifier) {
        case PANEL_CFG_TOTALS_ID:
            if (message->data_length_code < 3) {
                ESP_LOGW(TAG, "CFG totals frame ignored: DLC %u", (unsigned int)message->data_length_code);
                return;
            }

            panel_cfg_total_outputs = (message->data[0] > PANEL_CFG_MAX_OUTPUTS) ? PANEL_CFG_MAX_OUTPUTS : message->data[0];
            panel_cfg_total_sensors = (message->data[1] > PANEL_CFG_MAX_SENSORS) ? PANEL_CFG_MAX_SENSORS : message->data[1];
            panel_cfg_total_dims = (message->data[2] > PANEL_CFG_MAX_DIMS) ? PANEL_CFG_MAX_DIMS : message->data[2];
            memset(panel_cfg_outputs, 0, sizeof(panel_cfg_outputs));
            memset(panel_cfg_sensors, 0, sizeof(panel_cfg_sensors));
            memset(panel_cfg_dims, 0, sizeof(panel_cfg_dims));
            panel_cfg_output_received_mask = 0;
            panel_cfg_sensor_received_mask = 0;
            panel_cfg_dim_received_mask = 0;
            panel_cfg_totals_received = true;
            ESP_LOGI(TAG, "CFG TOTALS O:%d S:%d D:%d",
                     panel_cfg_total_outputs, panel_cfg_total_sensors, panel_cfg_total_dims);
            break;

        case PANEL_CFG_OUTPUT_ID:
            if (!panel_cfg_totals_received || message->data_length_code < 2) {
                ESP_LOGW(TAG, "CFG output frame ignored");
                return;
            }

            {
                int index = message->data[0];
                int value = message->data[1];

                if (index >= 1 && index <= panel_cfg_total_outputs) {
                    panel_cfg_outputs[index - 1] = value;
                    panel_cfg_output_received_mask |= (1U << (index - 1));
                }
                printf("CFG Output[%d] = %d\n", index - 1, value);
            }
            break;

        case PANEL_CFG_SENSOR_ID:
            if (!panel_cfg_totals_received || message->data_length_code < 2) {
                ESP_LOGW(TAG, "CFG sensor frame ignored");
                return;
            }

            {
                int index = message->data[0];
                int value = message->data[1];

                if (index >= 1 && index <= panel_cfg_total_sensors) {
                    panel_cfg_sensors[index - 1] = value;
                    panel_cfg_sensor_received_mask |= (1U << (index - 1));
                }
                printf("CFG Sensor[%d] = %d\n", index - 1, value);
            }
            break;

        case PANEL_CFG_DIM_ID:
            if (!panel_cfg_totals_received || message->data_length_code < 2) {
                ESP_LOGW(TAG, "CFG dim frame ignored");
                return;
            }

            {
                int index = message->data[0];
                int value = message->data[1];

                if (index >= 1 && index <= panel_cfg_total_dims) {
                    panel_cfg_dims[index - 1] = value;
                    panel_cfg_dim_received_mask |= (1U << (index - 1));
                }
                printf("CFG Dim[%d] = %d\n", index - 1, value);
            }
            break;

        case DISPLAY_RESET_CMD_ID:
            if (!panel_cfg_totals_received) {
                ESP_LOGW(TAG, "DISPLAY_RESET_CMD_ID ignored: no panel config totals received");
                return;
            }

            if (panel_cfg_output_received_mask != panel_cfg_expected_mask(panel_cfg_total_outputs) ||
                panel_cfg_sensor_received_mask != panel_cfg_expected_mask(panel_cfg_total_sensors) ||
                panel_cfg_dim_received_mask != panel_cfg_expected_mask(panel_cfg_total_dims)) {
                ESP_LOGW(TAG, "DISPLAY_RESET_CMD_ID received before all panel config items; saving received data");
                ESP_LOGW(TAG, "Masks O:0x%04" PRIX32 "/0x%04" PRIX32 " S:0x%04" PRIX32 "/0x%04" PRIX32 " D:0x%04" PRIX32 "/0x%04" PRIX32,
                         panel_cfg_output_received_mask, panel_cfg_expected_mask(panel_cfg_total_outputs),
                         panel_cfg_sensor_received_mask, panel_cfg_expected_mask(panel_cfg_total_sensors),
                         panel_cfg_dim_received_mask, panel_cfg_expected_mask(panel_cfg_total_dims));
            }

            save_panel_configuration_to_nvs(panel_cfg_total_outputs, panel_cfg_outputs,
                                            panel_cfg_total_sensors, panel_cfg_sensors,
                                            panel_cfg_total_dims, panel_cfg_dims);
            ESP_LOGW(TAG, "Panel config saved from CAN reset command; restarting display");
            vTaskDelay(pdMS_TO_TICKS(PANEL_CFG_RESTART_DELAY_MS));
            esp_restart();
            break;

        default:
            break;
    }
}

void handle_rx_message(twai_message_t message) {
    if (is_panel_cfg_frame(message.identifier)) {
        printf("Received frame with ID 0x%03X, data: %02X %02X %02X %02X %02X %02X %02X %02X\n",
               (unsigned int)message.identifier,
               message.data[0], message.data[1], message.data[2], message.data[3],
               message.data[4], message.data[5], message.data[6], message.data[7]);
        send_can_response_frame(&message);
        handle_panel_cfg_frame(&message);
        return;
    }
    
    switch (message.identifier) {
        case FRAME_1_ID:
            // Assign to global variables
            voltage = (message.data[0] << 8) | message.data[1];
            outputs = (message.data[2] << 8) | message.data[3];
            inputs = (message.data[4] << 8) | message.data[5];
            break;
        case FRAME_2_ID:
            // Assign to global arrays
            for (int i = 0; i < 4; i++) {
                analog_inputs[i] = message.data[i];
                // Assign to global RGB array
                dimmable_outputs[i] = message.data[i + 4];

            }

            break;
        case FRAME_3_ID:
            rgb_values[0] = message.data[0];
            rgb_values[1] = message.data[1];
            rgb_values[2] = message.data[2];
            rgb_values[3] = message.data[3];
            sensorTemp = message.data[4];
            sensorHum = message.data[5];
            break;
        default:
            break;
    }

}

int twaiCounter = 0;
void twai_task(void *pvParameter)
{
    twai_message_t message;
    
    while(1)
    {
        if (!driver_installed) {
            // Driver not installed
            vTaskDelay(pdMS_TO_TICKS(1000));
            return;
        }
        
        // Check if alert happened with timeout
        uint32_t alerts_triggered;
        esp_err_t alert_result = twai_read_alerts(&alerts_triggered, pdMS_TO_TICKS(100));
        
        if (alert_result == ESP_OK) {
            twai_status_info_t twaistatus;
            twai_get_status_info(&twaistatus);

            // Handle alerts
            if (alerts_triggered & TWAI_ALERT_ERR_PASS) {
                ESP_LOGW(TAG,"Alert: TWAI controller has become error passive.");
                check_and_recover_from_errors();
            }
            
            if (alerts_triggered & TWAI_ALERT_BUS_ERROR) {
                ESP_LOGW(TAG,"Alert: A (Bit, Stuff, CRC, Form, ACK) error has occurred on the bus.");
                ESP_LOGW(TAG,"Bus error count: %"PRIu32, twaistatus.bus_error_count);
                check_and_recover_from_errors();
            }

            if (alerts_triggered & TWAI_ALERT_RX_QUEUE_FULL) {
                //ESP_LOGW(TAG,"RX queue full, clearing");
                
                // Clear the queue aggressively to prevent further issues
                int cleared_count = 0;
                while (twai_receive(&message, 0) == ESP_OK) {
                    cleared_count++;
                    // Process the message if it's important, otherwise discard
                    if (message.identifier == FRAME_1_ID || message.identifier == FRAME_2_ID || message.identifier == FRAME_3_ID ||
                        is_panel_cfg_frame(message.identifier)) {
                        handle_rx_message(message);
                    }
                }
            }

            // Check if message is received
            if (alerts_triggered & TWAI_ALERT_RX_DATA) {     
                if (twai_receive(&message, 0) == ESP_OK) {
                   handle_rx_message(message);
                   canbusConnection = 1;
                }
            }
        } else if (alert_result == ESP_ERR_TIMEOUT) {
            // Timeout is normal, continue monitoring
        } else {
            ESP_LOGE(TAG, "Error reading alerts: 0x%x", alert_result);
        }
        
        // Proactive queue monitoring - check queue level and clear if getting full
        twai_status_info_t status;
        twai_get_status_info(&status);
        if (status.msgs_to_rx > 3) { // If more than 3 messages queued
            //ESP_LOGW(TAG, "RX queue full, clearing");
            int cleared_count = 0;
            while (twai_receive(&message, 0) == ESP_OK && cleared_count < 5) { // Clear up to 5 messages
                cleared_count++;
                if (message.identifier == FRAME_1_ID || message.identifier == FRAME_2_ID || message.identifier == FRAME_3_ID ||
                    is_panel_cfg_frame(message.identifier)) {
                    handle_rx_message(message);
                }
            }
        }
        
        // Update connection status based on recent activity
        if (alerts_triggered & TWAI_ALERT_RX_DATA) {
            canbusConnection = 1;
        } else {
            // Only set to 0 if we haven't had activity for a while
            static uint32_t last_activity = 0;
            uint32_t current_time = esp_timer_get_time() / 1000;
            if (current_time - last_activity > 5000) { // 5 seconds without activity
                canbusConnection = 0;
            }
            if (alerts_triggered & TWAI_ALERT_RX_DATA) {
                last_activity = current_time;
            }
        }
        
        vTaskDelay(pdMS_TO_TICKS(100)); // 100ms delay for stable operation

        if(twaiCounter == 100) {    
            twaiCounter = 0;
        }
        twaiCounter++;
    }
}

void twai_ini(void)
{
    esp_err_t ret;
    
    // Install TWAI driver
    ret = twai_driver_install(&g_config, &t_config, &f_config);
    if (ret == ESP_OK) {
        ESP_LOGI(TAG,"Driver installed");
    } else {
        ESP_LOGE(TAG,"Failed to install driver: 0x%x", ret);
        return;
    }
    
    // Start TWAI driver
    ret = twai_start();
    if (ret == ESP_OK) {
        ESP_LOGI(TAG,"Driver started");
    } else {
        ESP_LOGE(TAG,"Failed to start driver: 0x%x", ret);
        twai_driver_uninstall();
        return;
    }

    // Reconfigure alerts to detect frame receive, Bus-Off error and RX queue full states
    uint32_t alerts_to_enable = TWAI_ALERT_RX_DATA | TWAI_ALERT_ERR_PASS | TWAI_ALERT_BUS_ERROR | 
                               TWAI_ALERT_RX_QUEUE_FULL | TWAI_ALERT_TX_FAILED | TWAI_ALERT_TX_SUCCESS;
    ret = twai_reconfigure_alerts(alerts_to_enable, NULL);
    if (ret == ESP_OK) {
        ESP_LOGI(TAG,"CAN Alerts reconfigured");
    } else {
        ESP_LOGE(TAG,"Failed to reconfigure alerts: 0x%x", ret);
        twai_stop();
        twai_driver_uninstall();
        return;
    }

    // Initialize monitoring variables
    last_successful_tx = esp_timer_get_time() / 1000;
    error_recovery_count = 0;
    last_health_check = 0;
    last_recovery_attempt = 0;

    // TWAI driver is now successfully installed and started
    driver_installed = true;
    ESP_LOGI(TAG,"CAN bus initialization completed successfully");
 
    //define twai task
    xTaskCreate(twai_task, "twai_task", 4096, NULL, 5, NULL);
    
    //define watchdog task
    xTaskCreate(can_watchdog_task, "can_watchdog", 3072, NULL, 3, NULL);
}
