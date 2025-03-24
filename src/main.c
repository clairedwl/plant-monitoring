#include <string.h>
#include <stdio.h>
#include <stdint.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <driver/gpio.h>
#include "driver/adc.h"
#include "dht11.h"
#include "esp_log.h"
#include "nvs_flash.h"
#include "esp_wifi.h"
#include "esp_system.h"
#include "esp_event.h"
#include "esp_mac.h"
#include "esp_http_client.h"


#define WIFI_SSID "Livebox-23B0"   // AP SSID
#define WIFI_PASS "*******" // AP Password
#define URL "http://192.168.1.15:5000"

#define LED_PIN 8
#define capt_pin GPIO_NUM_10
#define RAW_DRY 4095           // Valeur mesurée en sol sec
#define RAW_WET 1650           // Valeur mesurée dans l'eau
#define RAW_DRY_SOIL 3200      // Valeur mesurée dans sol sec
#define PERCENTAGE_DRY_SOIL 37 // en pourcentage

static const char *TAG = "wifi_connect";
esp_ip4_addr_t static_addr;

// Event handler for Wi-Fi events
static void wifi_event_handler(void *arg, esp_event_base_t event_base, int32_t event_id, void *event_data)
{
    ESP_LOGI(TAG, "Event received: base=%s, id=%ld", event_base, (long)event_id);

    if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_START)
    {
        ESP_LOGI(TAG, "Wi-Fi started");
        esp_wifi_connect();
    }
    else if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_DISCONNECTED)
    {
        ESP_LOGI(TAG, "Wi-Fi disconnected, reconnecting...");
        esp_wifi_connect();
    }
    else if (event_base == IP_EVENT && event_id == IP_EVENT_STA_GOT_IP)
    {
        ip_event_got_ip_t *event = (ip_event_got_ip_t *)event_data;
        static_addr = event->ip_info.ip;
        ESP_LOGI(TAG, "Connected! Got IP: " IPSTR, IP2STR(&event->ip_info.ip));
    }
}

void wifi_init_sta(void)
{
    printf("Starting Wi-Fi\n");

    ESP_ERROR_CHECK(nvs_flash_init());
    ESP_ERROR_CHECK(esp_netif_init());
    ESP_ERROR_CHECK(esp_event_loop_create_default());

    esp_netif_create_default_wifi_sta();

    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    ESP_ERROR_CHECK(esp_wifi_init(&cfg));

    ESP_ERROR_CHECK(esp_event_handler_register(WIFI_EVENT, ESP_EVENT_ANY_ID, &wifi_event_handler, NULL));
    ESP_ERROR_CHECK(esp_event_handler_register(IP_EVENT, IP_EVENT_STA_GOT_IP, &wifi_event_handler, NULL));

    wifi_config_t wifi_config = {
        .sta = {
            .ssid = WIFI_SSID,
            .password = WIFI_PASS,
            .threshold.authmode = WIFI_AUTH_WPA2_PSK,
        },
    };

    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA));
    ESP_ERROR_CHECK(esp_wifi_set_config(WIFI_IF_STA, &wifi_config));
    ESP_ERROR_CHECK(esp_wifi_start());

    printf("Connecting to SSID: %s...", WIFI_SSID);
}

void print_wifi_mac_address()
{
    uint8_t mac[6];
    esp_wifi_get_mac(ESP_IF_WIFI_STA, mac);
    printf("Wi-Fi MAC Address: %02X:%02X:%02X:%02X:%02X:%02X",
             mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);
}

void send_hello_world()
{
    esp_http_client_config_t config = {
        .url = URL,
    };

    esp_http_client_handle_t client = esp_http_client_init(&config);

    // Set the POST data
    const char *post_data = "{\"message\": \"Hello World\"}";
    esp_http_client_set_method(client, HTTP_METHOD_POST);
    esp_http_client_set_header(client, "Content-Type", "application/json");
    esp_http_client_set_post_field(client, post_data, strlen(post_data));

    // Perform the POST request
    esp_err_t err = esp_http_client_perform(client);
    if (err == ESP_OK)
    {
      printf("ici");
      
        ESP_LOGI("HTTP_CLIENT", "HTTP POST Status = %d, content_length = %lld",
                 esp_http_client_get_status_code(client),
                 esp_http_client_get_content_length(client));
    }
    else
    {
        printf("la");
        ESP_LOGE("HTTP_CLIENT", "HTTP POST request failed: %s", esp_err_to_name(err));
    }

    // Clean up
    esp_http_client_cleanup(client);
}


void send_to_server(char *data)
{
    esp_http_client_config_t config = {
        .url = URL,
    };

    esp_http_client_handle_t client = esp_http_client_init(&config);

    // Set the POST data
    const char *post_data = data;
    esp_http_client_set_method(client, HTTP_METHOD_POST);
    esp_http_client_set_header(client, "Content-Type", "application/json");
    esp_http_client_set_post_field(client, post_data, strlen(post_data));

    // Perform the POST request
    esp_err_t err = esp_http_client_perform(client);
    if (err == ESP_OK)
    {
    
      
        ESP_LOGI("HTTP_CLIENT", "HTTP POST Status = %d, content_length = %lld",
                 esp_http_client_get_status_code(client),
                 esp_http_client_get_content_length(client));
    }
    else
    {
        
        ESP_LOGE("HTTP_CLIENT", "HTTP POST request failed: %s", esp_err_to_name(err));
    }

    // Clean up
    esp_http_client_cleanup(client);
}



void led_blink(void *pvParams)
{
    esp_rom_gpio_pad_select_gpio(LED_PIN);
    gpio_set_direction(LED_PIN, GPIO_MODE_OUTPUT);
    while (1)
    {
        gpio_set_level(LED_PIN, 0);
        vTaskDelay(1000 / portTICK_PERIOD_MS);
        gpio_set_level(LED_PIN, 1);
        vTaskDelay(1000 / portTICK_PERIOD_MS);
        // printf("hello world✨\n");
    }
}

void humidity_task()
{
    DHT11_init(capt_pin);
    while (1)
    {
        printf("Humidity is %d\n", DHT11_read().humidity);
        printf("Temperature is %d \n", DHT11_read().temperature);
        printf("Status code is %d\n", DHT11_read().status);
        vTaskDelay(1000 / portTICK_PERIOD_MS);
    }
}

void moisture_task()
{
    adc1_config_width(ADC_WIDTH_BIT_12);
    adc1_config_channel_atten(ADC1_CHANNEL_0, ADC_ATTEN_DB_11);
    int raw_value = 0;
    int moisturelvl = 0;
    char data_moisture[100];

    while (1)
    {
        raw_value = adc1_get_raw(ADC1_CHANNEL_0);
        moisturelvl = (RAW_DRY - raw_value) * 100 / (RAW_DRY - RAW_WET); // transforme la valeur analogique en %
        // Limiter les valeurs entre 0 et 100
        if (moisturelvl > 100)
            moisturelvl = 100;

        if (moisturelvl < 0)
            moisturelvl = 0;

        printf(" soil moisture : %d%% raw: %d  \n", moisturelvl, raw_value);


        if (moisturelvl < PERCENTAGE_DRY_SOIL)
        {
            printf("{\"moisture\":{\"value\":\"%d\",\"status\":\"1\"}}", moisturelvl);
            
            sprintf(data_moisture, "{\"moisture\":{\"value\":\"%d\",\"status\":\"1\"}}", moisturelvl);
            send_to_server(data_moisture);
        }
        else
        {
            printf("{\"moisture\":{\"value\":\"%d\",\"status\":\"0\"}}", moisturelvl);
            sprintf(data_moisture, "{\"moisture\":{\"value\":\"%d\",\"status\":\"0\"}}", moisturelvl);
            send_to_server(data_moisture);
        }
        vTaskDelay(1000 / portTICK_PERIOD_MS);
    }
}

void app_main()
{
    // Initialize NVS
    esp_err_t ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND)
    {
        ESP_ERROR_CHECK(nvs_flash_erase());
        ret = nvs_flash_init();
    }
    ESP_ERROR_CHECK(ret);

    // Initialize Wi-Fi
    wifi_init_sta();

    // Print Wi-Fi MAC address
    print_wifi_mac_address();

    // Wait for Wi-Fi to connect
    vTaskDelay(5000 / portTICK_PERIOD_MS);

    // Send "Hello World" to the Raspberry Pi
    send_hello_world();
    
    // Create tasks
    xTaskCreate(&led_blink, "LED_BLINK", 2048, NULL, 5, NULL);
   // xTaskCreate(&humidity_task, "humidity", 2048, NULL, 1, NULL);
    xTaskCreate(&moisture_task, "moisture", 8048, NULL, 1, NULL);
}