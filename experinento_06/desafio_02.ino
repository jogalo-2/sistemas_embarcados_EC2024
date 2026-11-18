#include <Arduino.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_timer.h"

#define LED1 2
#define LED2 4

TaskHandle_t task1Handle = NULL;
TaskHandle_t task2Handle = NULL;

void vTask1(void *pvParameters);
void vTask2(void *pvParameters);

uint32_t tempo_ms() {
  return (uint32_t)(esp_timer_get_time() / 1000);
}

void setup() {
  Serial.begin(115200);

  BaseType_t resultado1 = xTaskCreate(
    vTask1, "TASK1",
    configMINIMAL_STACK_SIZE + 1024,
    NULL, 2, &task1Handle
  );

  BaseType_t resultado2 = xTaskCreate(
    vTask2, "TASK2",
    configMINIMAL_STACK_SIZE + 1024,
    NULL, 1, &task2Handle
  );

  Serial.printf("TASK1: %s\n",
    resultado1 == pdPASS ? "Criada" : "Erro");

  Serial.printf("TASK2: %s\n",
    resultado2 == pdPASS ? "Criada" : "Erro");
}

void loop() {
  vTaskDelay(pdMS_TO_TICKS(3000));
}

void vTask1(void *pvParameters) {
  pinMode(LED1, OUTPUT);

  while (1) {
    digitalWrite(LED1, HIGH);
    Serial.printf("LED1 ON - Tempo: %u ms\n", tempo_ms());

    vTaskDelay(pdMS_TO_TICKS(300));

    digitalWrite(LED1, LOW);
    Serial.printf("LED1 OFF - Tempo: %u ms\n", tempo_ms());

    vTaskDelay(pdMS_TO_TICKS(300));
  }
}

void vTask2(void *pvParameters) {
  pinMode(LED2, OUTPUT);

  while (1) {
    digitalWrite(LED2, HIGH);
    Serial.printf("LED2 ON - Tempo: %u ms\n", tempo_ms());

    vTaskDelay(pdMS_TO_TICKS(100));

    digitalWrite(LED2, LOW);
    Serial.printf("LED2 OFF - Tempo: %u ms\n", tempo_ms());

    vTaskDelay(pdMS_TO_TICKS(100));
  }
}