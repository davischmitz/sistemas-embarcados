#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/gpio.h"

#define LED_PLACA 2

void app_main(void)
{
  gpio_reset_pin(LED_PLACA);
  gpio_set_direction(LED_PLACA, GPIO_MODE_OUTPUT);

  while(1) 
  {
    printf("🐉 Ligando LED\n");
    gpio_set_level(LED_PLACA, 1);
    vTaskDelay(1000 / portTICK_PERIOD_MS);

    printf("🦔 Desligando LED\n");
    gpio_set_level(LED_PLACA, 0);
    vTaskDelay(1000 / portTICK_PERIOD_MS);
  }
}