#include <Arduino.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#define LED1 2
#define LED2 4

TaskHandle_t task1Handle = NULL;
TaskHandle_t task2Handle = NULL;
TaskHandle_t task3Handle = NULL;

void vTask1(void *pvParameters);
void vTask2(void *pvParameters);
void vTask3(void *pvParameters);

void setup() {
  Serial.begin(9600);

  BaseType_t resultado1 = xTaskCreate(
    vTask1, "TASK1",
    configMINIMAL_STACK_SIZE + 1024,
    NULL, 1, &task1Handle
  );

  BaseType_t resultado2 = xTaskCreate(
    vTask2, "TASK2",
    configMINIMAL_STACK_SIZE + 1024,
    NULL, 2, &task2Handle
  );

  BaseType_t resultado3 = xTaskCreate(
    vTask3, "TASK3",
    configMINIMAL_STACK_SIZE + 1024,
    NULL, 3, &task3Handle
  );

  Serial.printf("TASK1: %s\n",
    resultado1 == pdPASS ? "Criada" : "Erro");

  Serial.printf("TASK2: %s\n",
    resultado2 == pdPASS ? "Criada" : "Erro");

  Serial.printf("TASK3: %s\n",
    resultado3 == pdPASS ? "Criada" : "Erro");
}

void loop() {
  vTaskDelay(pdMS_TO_TICKS(3000));
}

void vTask1(void *pvParameters) {
  pinMode(LED1, OUTPUT);

  while (1) {
    digitalWrite(LED1, !digitalRead(LED1));

    Serial.println("TASK1 - LED GPIO 2 alternou");

    vTaskDelay(pdMS_TO_TICKS(200));
  }
}

void vTask2(void *pvParameters) {
  int cont = 0;

  while (1) {
    Serial.print("TASK2 - Contador: ");
    Serial.println(cont++);

    vTaskDelay(pdMS_TO_TICKS(1000));
  }
}

void vTask3(void *pvParameters) {
  pinMode(LED2, OUTPUT);

  while (1) {
    digitalWrite(LED2, !digitalRead(LED2));

    Serial.println("TASK3 - LED GPIO 4 alternou");

    vTaskDelay(pdMS_TO_TICKS(500));
  }
}