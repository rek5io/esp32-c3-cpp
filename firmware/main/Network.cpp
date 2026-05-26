#include <stdio.h>
#include <string.h>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "esp_wifi.h"
#include "esp_event.h"
#include "esp_log.h"
#include "nvs_flash.h"

#include "esp_netif.h"

#include "esp_http_server.h"
#include "builtinfiles.h"
static const char *TAG = "WEB_SERVER";

static unsigned char  g_ssid[32] = "                               ";
static unsigned char  g_password[64] = "                                                               ";

// ==========================
// ROOT HANDLER
// ==========================

static esp_err_t root_get_handler(httpd_req_t *req)
{
    httpd_resp_set_type(req, "text/html");
    httpd_resp_send(req, html_page, HTTPD_RESP_USE_STRLEN);

    return ESP_OK;
}


// ==========================
// API HANDLER
// ==========================

static esp_err_t api_handler(httpd_req_t *req)
{
    ESP_LOGI(TAG, "API CALL");

    const char *response =
        "{ \"status\": \"ok\", \"device\": \"ESP32-C3\" }";

    httpd_resp_set_type(req, "application/json");
    httpd_resp_send(req, response, HTTPD_RESP_USE_STRLEN);

    return ESP_OK;
}


// ==========================
// START WEB SERVER
// ==========================

static httpd_handle_t start_webserver(void)
{
    httpd_config_t config = HTTPD_DEFAULT_CONFIG();

    httpd_handle_t server = NULL;

    if (httpd_start(&server, &config) == ESP_OK)
    {
        httpd_uri_t root = {
            .uri      = "/",
            .method   = HTTP_GET,
            .handler  = root_get_handler,
            .user_ctx = NULL
        };

        httpd_register_uri_handler(server, &root);

        httpd_uri_t api = {
            .uri      = "/api/led",
            .method   = HTTP_GET,
            .handler  = api_handler,
            .user_ctx = NULL
        };

        httpd_register_uri_handler(server, &api);

        ESP_LOGI(TAG, "Web server started");
    }

    return server;
}


// ==========================
// WIFI EVENT HANDLER
// ==========================

static void wifi_event_handler(
    void* arg,
    esp_event_base_t event_base,
    int32_t event_id,
    void* event_data)
{
    if (event_base == WIFI_EVENT &&
        event_id == WIFI_EVENT_STA_START)
    {
        esp_wifi_connect();
    }

    if (event_base == IP_EVENT &&
        event_id == IP_EVENT_STA_GOT_IP)
    {
        ip_event_got_ip_t* event =
            (ip_event_got_ip_t*) event_data;

        ESP_LOGI(TAG,
                 "Got IP: " IPSTR,
                 IP2STR(&event->ip_info.ip));

        start_webserver();
    }
}


// ==========================
// WIFI INIT
// ==========================

static void wifi_init(void)
{
    esp_netif_init();

    esp_event_loop_create_default();

    esp_netif_create_default_wifi_sta();

    wifi_init_config_t cfg =
        WIFI_INIT_CONFIG_DEFAULT();

    esp_wifi_init(&cfg);

    esp_event_handler_instance_register(
        WIFI_EVENT,
        ESP_EVENT_ANY_ID,
        &wifi_event_handler,
        NULL,
        NULL);

    esp_event_handler_instance_register(
        IP_EVENT,
        IP_EVENT_STA_GOT_IP,
        &wifi_event_handler,
        NULL,
        NULL);

    wifi_config_t wifi_config;
    strncpy(reinterpret_cast<char*>(wifi_config.sta.ssid),
        reinterpret_cast<const char*>(g_ssid),
        sizeof(wifi_config.sta.ssid));
    strncpy(reinterpret_cast<char*>(wifi_config.sta.password),
        reinterpret_cast<const char*>(g_password),
        sizeof(wifi_config.sta.password));

    esp_wifi_set_mode(WIFI_MODE_STA);

    esp_wifi_set_config(
        WIFI_IF_STA,
        &wifi_config);

    esp_wifi_start();
}
// ========================================
// WIFI AP INIT
// ========================================

static void wifi_init_softap(void)
{
    esp_netif_init();

    esp_event_loop_create_default();

    esp_netif_create_default_wifi_ap();

    wifi_init_config_t cfg =
        WIFI_INIT_CONFIG_DEFAULT();

    esp_wifi_init(&cfg);

    wifi_config_t wifi_config;
        strncpy(reinterpret_cast<char*>(wifi_config.ap.ssid),
        "ESP32-C3-AP",
        sizeof(wifi_config.ap.ssid));
        wifi_config.ap.ssid_len = strlen("ESP32-C3-AP");
        strncpy(reinterpret_cast<char*>(wifi_config.ap.password),
        "12345678",
        sizeof(wifi_config.ap.password));
        wifi_config.ap.max_connection = 4;
        wifi_config.ap.channel = 5;
        wifi_config.ap.authmode = WIFI_AUTH_WPA2_PSK;

    const char* ap_password = "12345678";
    strcpy((char*)wifi_config.ap.password, ap_password);

    if (strlen(ap_password) == 0)
     {
         wifi_config.ap.authmode =
             WIFI_AUTH_OPEN;
     }

    esp_wifi_set_mode(WIFI_MODE_AP);

    esp_wifi_set_config(
        WIFI_IF_AP,
        &wifi_config);

    esp_wifi_start();

    ESP_LOGI(TAG, "WiFi AP started");
    ESP_LOGI(TAG, "SSID: ESP32-C3-AP");
    ESP_LOGI(TAG, "Password: 12345678");
    ESP_LOGI(TAG, "IP: 192.168.4.1");
}


