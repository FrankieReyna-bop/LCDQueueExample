#include <stdio.h>
#include <inttypes.h>
#include "sdkconfig.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/gpio.h"
#include "driver/i2c.h"
#include "esp_chip_info.h"
#include "esp_flash.h"
#include "esp_log.h"
#include "esp_system.h"
#include "i2c_lcd.h"
#include "unistd.h"
#include "hcsr04_driver.h"

QueueHandle_t queueHandle;

#define LCDADDR 0x27

void lcd_disp(char *str) {
    lcd_clear(LCDADDR);
    lcd_put_cursor(LCDADDR, 0, 0);
    lcd_send_string(LCDADDR, str);
}


void hcsr04_task(void *pvParameters)
{
    esp_err_t return_value = ESP_OK;
    (void) UltrasonicInit();

    
    // create variable which stores the measured distance
    static uint32_t afstand = 0;

    while (1) {
        return_value = UltrasonicMeasure(100, &afstand);
        UltrasonicAssert(return_value);
        if (return_value == ESP_OK) {
            printf ("Afstand: %ld\n", afstand);
            xQueueSend(queueHandle, &afstand, portMAX_DELAY);
        }    
        // 0,5 second delay before starting new measurement
        vTaskDelay(500 / portTICK_PERIOD_MS);
    }
}

void lcd_task(void *pvParameters) {
    uint32_t val;
    char str[20];
    lcd_init(LCDADDR);
    while(1) {
        str[0] = '\0';
        if (xQueueReceive(queueHandle, &val, portMAX_DELAY) == pdPASS) {
            snprintf(str, sizeof(str), "Dist: %ld", val);
            lcd_disp(str);
        }
        vTaskDelay(500 / portTICK_PERIOD_MS);
    }
}

void app_main(void)
{
    i2c_master_init();
    // Create measurement task
    queueHandle = xQueueCreate(10, sizeof(uint32_t));
    if (queueHandle == NULL) {
        // Failed to create the queue
        printf("Queue creation failed\n");
        return;
    }
    

    xTaskCreate(hcsr04_task, "HSRC04 task", 2048, NULL, 4, NULL);
    xTaskCreate(lcd_task, "LCD_task", 2048, NULL, 4, NULL);
}
