#include <driver/gpio.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include "driver/adc.h"
#include "esp_adc_cal.h"

#include "dht11.h"

#define LED_PIN 8
#define capt_pin GPIO_NUM_10
#define RAW_DRY 4095           // Valeur mesurée en sol sec
#define RAW_WET 1650           // Valeur mesurée dans l'eau
#define RAW_DRY_SOIL 3200      // Valeur mesurée dans sol sec
#define PERCENTAGE_DRY_SOIL 37 // en pourcentage
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
    while (1)
    {
        raw_value = adc1_get_raw(ADC1_CHANNEL_0);
        moisturelvl = (RAW_DRY - raw_value) * 100 / (RAW_DRY - RAW_WET); // transforme la valeur analogique en %
        // Limiter les valeurs entre 0 et 100
        if (moisturelvl > 100)
        {
            moisturelvl = 100;
        }
        if (moisturelvl < 0)
        {
            moisturelvl = 0;
        }

        printf(" soil moisture : %d%% raw: %d  \n", moisturelvl, raw_value);
        vTaskDelay(1000 / portTICK_PERIOD_MS);

        if (moisturelvl < PERCENTAGE_DRY_SOIL)
        {
            printf("{\"moisture\":{\"value\":\"%d\",\"status\":\"1\"}}", moisturelvl); // besoin d'eau
        }
        else
        {
            printf("{\"moisture\":{\"value\":\"%d\",\"status\":\"o\"}}", moisturelvl);
        }
    }
}

void app_main()
{
    xTaskCreate(&led_blink, "LED_BLINK", 1024, NULL, 5, NULL);
    xTaskCreate(&humidity_task, "humidity", 2048, NULL, 1, NULL);
    xTaskCreate(&moisture_task, "moisture", 2048, NULL, 1, NULL);
}