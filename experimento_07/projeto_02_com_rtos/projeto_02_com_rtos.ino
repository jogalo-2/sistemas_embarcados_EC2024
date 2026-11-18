/*************************************************
 * Experimento 7 - Uso dos nucleos do ESP32
 * Projeto 2 (roteiro: "Experimento 4 - Versao 1")
 * Comparacao COM FreeRTOS (xTaskCreatePinnedToCore)
 *
 * Dois LEDs piscando simultaneamente, cada tarefa
 * em um nucleo. O intervalo real entre execucoes e
 * medido com esp_timer_get_time() (microssegundos).
 *************************************************/

#include <Arduino.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_timer.h"

#define LED1 2
#define LED2 4

TaskHandle_t handleLED1 = NULL;
TaskHandle_t handleLED2 = NULL;

/* Tarefa do LED1 - periodo 500 ms - Nucleo 0 */
void tarefaLED1(void *pvParameters) {

  int64_t tempoAnterior = esp_timer_get_time();

  while (1) {

    int64_t tempoAtual = esp_timer_get_time();
    int64_t intervalo = tempoAtual - tempoAnterior;

    digitalWrite(LED1, !digitalRead(LED1));

    Serial.printf(
      "LED1 | Core: %d | Tempo: %lldus | Intervalo: %lldus | Esperado: 500000 us\n",
      xPortGetCoreID(), tempoAtual, intervalo
    );

    tempoAnterior = tempoAtual;

    vTaskDelay(pdMS_TO_TICKS(500));
  }
}

/* Tarefa do LED2 - periodo 300 ms - Nucleo 1 */
void tarefaLED2(void *pvParameters) {

  int64_t tempoAnterior = esp_timer_get_time();

  while (1) {

    int64_t tempoAtual = esp_timer_get_time();
    int64_t intervalo = tempoAtual - tempoAnterior;

    digitalWrite(LED2, !digitalRead(LED2));

    Serial.printf(
      "LED2 | Core: %d | Tempo: %lldus | Intervalo: %lldus | Esperado: 300000 us\n",
      xPortGetCoreID(), tempoAtual, intervalo
    );

    tempoAnterior = tempoAtual;

    vTaskDelay(pdMS_TO_TICKS(300));
  }
}

void setup() {

  Serial.begin(115200);

  pinMode(LED1, OUTPUT);
  pinMode(LED2, OUTPUT);

  digitalWrite(LED1, LOW);
  digitalWrite(LED2, LOW);

  xTaskCreatePinnedToCore(
    tarefaLED1, "LED1", 4096, NULL, 1, &handleLED1, 0
  );

  xTaskCreatePinnedToCore(
    tarefaLED2, "LED2", 4096, NULL, 1, &handleLED2, 1
  );
}

void loop() {
  vTaskDelay(pdMS_TO_TICKS(1000));
}
