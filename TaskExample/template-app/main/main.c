#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

TaskHandle_t taskHandle[2];

void TaskCode(void *parameters)
{
  uint8_t taskNumber = (uint8_t) parameters;
  printf("Task started\n");
  for(;;) 
  {
    printf("Task %d\n", taskNumber);
    vTaskDelay(1000 / portTICK_PERIOD_MS);
  }
  vTaskDelete(taskHandle);
}

void app_main(void)
{
  xTaskCreate(TaskCode, "task1", 2048, (void *) 1, 1, &taskHandle[0]);
  vTaskDelay(2000 / portTICK_PERIOD_MS);
  xTaskCreate(TaskCode, "task2", 2048, (void *) 2, 1, &taskHandle[1]);
  vTaskDelay(2000 / portTICK_PERIOD_MS);
  vTaskDelete(taskHandle[0]);
}