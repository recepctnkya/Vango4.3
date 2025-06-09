#ifndef HEXNET_WIFI_H
#define HEXNET_WIFI_H


#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/event_groups.h"
#include "esp_system.h"
#include "esp_wifi.h"
#include "esp_event.h"
#include "esp_log.h"
#include "nvs_flash.h"

#include "lwip/err.h"
#include "lwip/sys.h"
#include "esp_netif_sntp.h"
#include "lwip/ip_addr.h"
#include "esp_sntp.h"



#ifndef INET6_ADDRSTRLEN
#define INET6_ADDRSTRLEN 48


#define WIFI_CONNECTED_BIT BIT0
#define WIFI_FAIL_BIT      BIT1


#define EXAMPLE_ESP_WIFI_SSID      "Inteno-4035"
#define EXAMPLE_ESP_WIFI_PASS      "MTWBXRPQ34DU4Z"
#define EXAMPLE_ESP_MAXIMUM_RETRY  5


#define ESP_WIFI_SAE_MODE 1
#define EXAMPLE_H2E_IDENTIFIER ""

#define ESP_WIFI_SCAN_AUTH_MODE_THRESHOLD 1

#endif

static void obtain_time(void);
void time_sync_notification_cb(struct timeval *tv);
void time_protocotol_main(void);
static void print_servers(void);
static void obtain_time(void);
static void event_handler(void* arg, esp_event_base_t event_base, int32_t event_id, void* event_data);
void wifi_init_sta(void);
void wifi_ini(void);
#endif // HEXNET_WIFI_H
