#include <driver/gpio.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>


#include "dht11.h"


#define LED_PIN 8
#define capt_pin GPIO_NUM_0


void led_blink(void *pvParams) {
       esp_rom_gpio_pad_select_gpio(LED_PIN);
     gpio_set_direction (LED_PIN,GPIO_MODE_OUTPUT);
    while (1) {
        gpio_set_level(LED_PIN,0);
        vTaskDelay(1000/portTICK_PERIOD_MS);
        gpio_set_level(LED_PIN,1);
        vTaskDelay(1000/portTICK_PERIOD_MS);
        printf("hello world✨\n");

    }
}

void humidity_task(){
   
    DHT11_init(capt_pin);
    while(1) {
       
        printf("Humidity is %d\n", DHT11_read().humidity);
        printf("Temperature is %d \n", DHT11_read().temperature);
        printf("Status code is %d\n", DHT11_read().status);
        vTaskDelay(1000/portTICK_PERIOD_MS);
    }
}

void app_main() {
    xTaskCreate(&led_blink,"LED_BLINK",1024,NULL,5,NULL);
    xTaskCreate(&humidity_task, "humidity",2048,NULL,1,NULL);
}