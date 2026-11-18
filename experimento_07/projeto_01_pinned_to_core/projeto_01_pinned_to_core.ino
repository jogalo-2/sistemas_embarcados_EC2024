/*************************************************
 * Experimento 7 - Uso dos nucleos do ESP32
 * Projeto 1 (roteiro: "Experimento 3")
 * Funcao xTaskCreatePinnedToCore
 *
 * TASK1: Piscar LED vermelho - Nucleo 0
 * TASK2: Piscar LED amarelo  - Nucleo 1
 *
 * Objetivo:
 * Demonstrar a execucao de tarefas em dois
 * nucleos do ESP32 utilizando FreeRTOS.
 *************************************************/

/* Biblioteca do Arduino */
#include <Arduino.h>

/* Bibliotecas FreeRTOS */
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

/* Mapeamento dos pinos */
#define LedVerm 2
#define LedAmar 4

/* Handles das tarefas */
TaskHandle_t tarefa1 = NULL;
TaskHandle_t tarefa2 = NULL;

/* Prototipos das tarefas */
void blinkVermelho(void *pvParameter);
void blinkAmarelo(void *pvParameter);

/*************************************************
 * FUNCAO SETUP
 *************************************************/
void setup() {

  /* Inicializa a comunicacao serial */
  Serial.begin(115200);

  /* Configura os pinos dos LEDs como saida */
  pinMode(LedVerm, OUTPUT);
  pinMode(LedAmar, OUTPUT);

  /* Inicializa os LEDs desligados */
  digitalWrite(LedVerm, LOW);
  digitalWrite(LedAmar, LOW);

  /* Criacao da TASK1
     Funcao: blinkVermelho | Nome: tarefa1
     Pilha: 4096 bytes | Prioridade: 1 | Nucleo: 0 */
  BaseType_t resultado1 = xTaskCreatePinnedToCore(
    blinkVermelho,
    "tarefa1",
    4096,
    NULL,
    1,
    &tarefa1,
    0
  );

  /* Criacao da TASK2
     Funcao: blinkAmarelo | Nome: tarefa2
     Pilha: 4096 bytes | Prioridade: 2 | Nucleo: 1 */
  BaseType_t resultado2 = xTaskCreatePinnedToCore(
    blinkAmarelo,
    "tarefa2",
    4096,
    NULL,
    2,
    &tarefa2,
    1
  );

  /* Verifica se a TASK1 foi criada */
  if (resultado1 == pdPASS) {
    Serial.println("TASK1 criada com sucesso!");
  } else {
    Serial.println("Erro ao criar TASK1!");
  }

  /* Verifica se a TASK2 foi criada */
  if (resultado2 == pdPASS) {
    Serial.println("TASK2 criada com sucesso!");
  } else {
    Serial.println("Erro ao criar TASK2!");
  }
}

/*************************************************
 * FUNCAO LOOP
 *************************************************/
void loop() {
  /* Libera a CPU por 1 segundo */
  vTaskDelay(pdMS_TO_TICKS(1000));
}

/*************************************************
 * TASK1 - CONTROLE DO LED VERMELHO
 * GPIO: 2 | Prioridade: 1 | Nucleo: 0 | Intervalo: 500 ms
 *************************************************/
void blinkVermelho(void *pvParameter) {
  while (1) {
    /* Inverte o estado do LED vermelho */
    digitalWrite(LedVerm, !digitalRead(LedVerm));

    /* Exibe o nucleo de execucao */
    Serial.printf("LED VERMELHO - Nucleo: %d\n", xPortGetCoreID());

    /* Aguarda 500 ms */
    vTaskDelay(pdMS_TO_TICKS(500));
  }
}

/*************************************************
 * TASK2 - CONTROLE DO LED AMARELO
 * GPIO: 4 | Prioridade: 2 | Nucleo: 1 | Intervalo: 300 ms
 *************************************************/
void blinkAmarelo(void *pvParameter) {
  while (1) {
    /* Inverte o estado do LED amarelo */
    digitalWrite(LedAmar, !digitalRead(LedAmar));

    /* Exibe o nucleo de execucao */
    Serial.printf("LED AMARELO - Nucleo: %d\n", xPortGetCoreID());

    /* Aguarda 300 ms */
    vTaskDelay(pdMS_TO_TICKS(300));
  }
}
