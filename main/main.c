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
SemaphoreHandle_t i2cMutex = NULL;

#define LCDADDR1 0x27
#define LCDADDR2 0x26

// code to streamline displaying

void lcd_disp(char *str, uint8_t lcd) {
    lcd_clear(lcd);
    lcd_put_cursor(lcd, 0, 0);
    lcd_send_string(lcd, str);
}

// measures from HCSR04, puts in queue 

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
        vTaskDelay(300 / portTICK_PERIOD_MS);
    }
}

// Not really a perfect implementation. If you think about it, depending on whether we recieve the semaphore,
// we could lose data since we would take the queue number, not display it, and then take another number from queue.
// Perhaps we should make it wait till it can display it? but then again that delays the queue and puts them out of sync.
// Maybe the LCD task should just run really fast. Will think about.

void LCD_task1(void *lcd_addr) {
    uint32_t val;
    int lcd = (uint32_t)lcd_addr;
    char str[20];
    

    while(1) {
        str[0] = '\0';
        if (xQueueReceive(queueHandle, &val, portMAX_DELAY) == pdPASS) {
            snprintf(str, sizeof(str), "Dist: %ld", val);
            if(xSemaphoreTake(i2cMutex, portMAX_DELAY) == pdTRUE) {
                lcd_disp(str, lcd);
                xSemaphoreGive(i2cMutex);
                vTaskDelay(300 / portTICK_PERIOD_MS);
            }
        }
    }
}

// Following code from previous 3 task tests

void LCD_task2(void *lcd_addr) {
    int i=0;   // iteration counter
    char numst[20];  // place to hold string to print
    
    int lcd = (uint32_t)lcd_addr;
    char buffer[20];
    lcd_put_cursor(lcd, 0, 0);   // Set cursor position to   row, column
    sprintf(buffer, "Works: 0x%x", lcd);
    lcd_send_string(lcd, buffer);
    xSemaphoreGive(i2cMutex);
    while (1) {
        if (xSemaphoreTake(i2cMutex, portMAX_DELAY) == pdTRUE) {
            /* We were able to obtain the semaphore and can now access the
//                 shared resource. */
            lcd_put_cursor(lcd, 1, 0);

            sprintf(numst, "N: %04d", i++);

            lcd_send_string(lcd, numst);     // Display the count (numst)
            //usleep(3*d100ms);

            /* We have finished accessing the shared resource. Release the
            semaphore. */
            xSemaphoreGive(i2cMutex);
            vTaskDelay(pdMS_TO_TICKS(1000));
        }
    }
}

void app_main(void)
{
    i2c_master_init();
    i2cMutex = xSemaphoreCreateBinary();

    // Online example of how to handle queue, first parameter # of spots in queue, second size of data

    queueHandle = xQueueCreate(15, sizeof(uint32_t));
    if (queueHandle == NULL) {
        printf("Queue creation failed\n");
        return;
    }

    // Lazy, initialize in app main
    lcd_init(LCDADDR1);
    lcd_init(LCDADDR2);

    xTaskCreate(hcsr04_task, "HSRC04 task", 2048, NULL, 4, NULL);
    xTaskCreate(LCD_task1, "LCD_task1", 2048,  (void*)LCDADDR1, 4, NULL);
    xTaskCreate(LCD_task2, "LCD_task2", 2048,  (void*)LCDADDR2, 4, NULL);
}
