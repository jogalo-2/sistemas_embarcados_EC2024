/*************************************************
 * Experimento 7 - Uso dos nucleos do ESP32
 * ATIVIDADE AVALIATIVA 2 - Versao COM FreeRTOS
 *
 * Mesma aplicacao do Projeto 2 (dois LEDs, cada um
 * em uma tarefa e em um nucleo), acrescida de
 * estatisticas de regularidade (min, max, media,
 * desvio padrao e jitter) para comparacao direta
 * com a versao SEM RTOS (mesmo formato de saida).
 *
 * LED1 (GPIO 2) - 500 ms - Nucleo 0
 * LED2 (GPIO 4) - 300 ms - Nucleo 1
 *
 * CARGA_MS: simula um processamento bloqueante
 * (ex.: leitura de sensor / calculo) que ocorre a
 * cada PERIODO_CARGA_MS. Com 0 nao ha carga.
 * Teste com 0 e depois com, por exemplo, 50 ms.
 *************************************************/

#include <Arduino.h>
#include <math.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/semphr.h"
#include "esp_timer.h"

#define LED1 2
#define LED2 4

#define PERIODO_LED1_MS 500
#define PERIODO_LED2_MS 300

#define CARGA_MS          0     // duracao da carga bloqueante (0 = sem carga)
#define PERIODO_CARGA_MS  100   // a cada quanto tempo a carga ocorre

#define RESUMO_A_CADA 20

typedef struct {
  const char *nome;
  uint8_t pino;
  uint32_t periodoMs;
  int64_t tAnterior;
  uint32_t amostras;
  int64_t minI, maxI;
  double soma, somaQuad;
} Estat;

Estat e1 = { "LED1", LED1, PERIODO_LED1_MS, 0, 0, INT64_MAX, 0, 0, 0 };
Estat e2 = { "LED2", LED2, PERIODO_LED2_MS, 0, 0, INT64_MAX, 0, 0, 0 };

TaskHandle_t handleLED1 = NULL;
TaskHandle_t handleLED2 = NULL;
TaskHandle_t handleCarga = NULL;
SemaphoreHandle_t mutexSerial;

/* Registra uma ativacao e imprime intervalo / resumo */
void registrar(Estat *e, int64_t agora) {
  if (e->tAnterior == 0) { e->tAnterior = agora; return; }

  int64_t intervalo = agora - e->tAnterior;
  int64_t esperado = (int64_t)e->periodoMs * 1000LL;
  e->tAnterior = agora;

  e->amostras++;
  if (intervalo < e->minI) e->minI = intervalo;
  if (intervalo > e->maxI) e->maxI = intervalo;
  e->soma += intervalo;
  e->somaQuad += (double)intervalo * intervalo;

  xSemaphoreTake(mutexSerial, portMAX_DELAY);
  Serial.printf("COM RTOS | %s | Core %d | Esperado: %lld us | Observado: %lld us | Desvio: %+lld us\n",
                e->nome, xPortGetCoreID(), (long long)esperado,
                (long long)intervalo, (long long)(intervalo - esperado));

  if (e->amostras >= RESUMO_A_CADA) {
    double media = e->soma / e->amostras;
    double var = e->somaQuad / e->amostras - media * media;
    Serial.printf(">> RESUMO COM RTOS | %s | n=%u | Min: %lld | Max: %lld | Media: %.1f | DP: %.1f | Jitter p-p: %lld us\n",
                  e->nome, (unsigned)e->amostras, (long long)e->minI, (long long)e->maxI,
                  media, var > 0 ? sqrt(var) : 0.0, (long long)(e->maxI - e->minI));
    e->amostras = 0; e->minI = INT64_MAX; e->maxI = 0; e->soma = 0; e->somaQuad = 0;
  }
  xSemaphoreGive(mutexSerial);
}

void tarefaLED1(void *pv) {
  while (1) {
    int64_t agora = esp_timer_get_time();
    digitalWrite(LED1, !digitalRead(LED1));
    registrar(&e1, agora);
    vTaskDelay(pdMS_TO_TICKS(PERIODO_LED1_MS));
  }
}

void tarefaLED2(void *pv) {
  while (1) {
    int64_t agora = esp_timer_get_time();
    digitalWrite(LED2, !digitalRead(LED2));
    registrar(&e2, agora);
    vTaskDelay(pdMS_TO_TICKS(PERIODO_LED2_MS));
  }
}

/* Carga bloqueante (espera ocupada) no nucleo 1.
 * Por ser uma tarefa separada, o escalonador pode
 * intercalar/preemptar e o LED2 continua no horario. */
void tarefaCarga(void *pv) {
  while (1) {
    int64_t inicio = esp_timer_get_time();
    while (esp_timer_get_time() - inicio < (int64_t)CARGA_MS * 1000LL) {
      /* processamento simulado */
    }
    vTaskDelay(pdMS_TO_TICKS(PERIODO_CARGA_MS - CARGA_MS > 0 ? PERIODO_CARGA_MS - CARGA_MS : 1));
  }
}

void setup() {
  Serial.begin(115200);
  delay(500);

  pinMode(LED1, OUTPUT);
  pinMode(LED2, OUTPUT);
  digitalWrite(LED1, LOW);
  digitalWrite(LED2, LOW);

  mutexSerial = xSemaphoreCreateMutex();

  Serial.printf("Sistema iniciado - COM RTOS (dois nucleos) | Carga: %d ms a cada %d ms\n",
                CARGA_MS, PERIODO_CARGA_MS);

  xTaskCreatePinnedToCore(tarefaLED1, "LED1", 4096, NULL, 1, &handleLED1, 0);
  xTaskCreatePinnedToCore(tarefaLED2, "LED2", 4096, NULL, 1, &handleLED2, 1);
  if (CARGA_MS > 0) {
    xTaskCreatePinnedToCore(tarefaCarga, "CARGA", 4096, NULL, 1, &handleCarga, 1);
  }
}

void loop() {
  vTaskDelay(pdMS_TO_TICKS(1000));
}
