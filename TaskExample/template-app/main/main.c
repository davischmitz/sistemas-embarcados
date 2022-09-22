#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/semphr.h"

TaskHandle_t taskHandle[2];
SemaphoreHandle_t semaphore = NULL;

uint32_t criticalValue = 0;

void incrementCriticalValue() {
  if (criticalValue < 1000) {
    criticalValue++;
  } else {
    criticalValue = 0;
  }
}

void TaskCode(void * parameters) {
  uint8_t taskNumber = (uint8_t) parameters;
  printf("Task started\n");
  for (;;) {
    if (semaphore != NULL) {
      if (xSemaphoreTake(semaphore, 10)) {
        printf("Task %d\n", taskNumber);
        incrementCriticalValue();
        printf("Critical Value %d\n", criticalValue);
        xSemaphoreGive(semaphore);
      } else {
        printf("Semaphore take failed by task %d\n", taskNumber);
      }
      vTaskDelay(1000 / portTICK_PERIOD_MS);
    }
  }
  vTaskDelete(taskHandle);
}

void app_main(void) {
  semaphore = xSemaphoreCreateMutex();
  xSemaphoreGive(semaphore);

  xTaskCreate(TaskCode, "task1", 2048, (void * ) 1, 1, & taskHandle[0]);
  vTaskDelay(2000 / portTICK_PERIOD_MS);
  xTaskCreate(TaskCode, "task2", 2048, (void * ) 2, 1, & taskHandle[1]);
  vTaskDelay(2000 / portTICK_PERIOD_MS);
  vTaskDelete(taskHandle[0]);
}