/*************************************************
 * Experimento 7 - Uso dos nucleos do ESP32
 * ATIVIDADE AVALIATIVA 1
 * Monitoramento temporal de tarefas em nucleos
 * distintos (xTaskCreatePinnedToCore)
 *
 * TASK1: LED vermelho (GPIO 2) - Nucleo 0 - 500 ms
 * TASK2: LED amarelo  (GPIO 4) - Nucleo 1 - 300 ms
 *
 * A cada execucao, cada tarefa registra:
 *  - instante de ativacao (esp_timer_get_time, em us)
 *  - nucleo de execucao (xPortGetCoreID)
 *  - periodo esperado x intervalo observado
 *  - desvio (observado - esperado)
 *  - jitter ciclo a ciclo |T(k) - T(k-1)|
 * A cada RESUMO_A_CADA execucoes imprime estatisticas:
 *  minimo, maximo, media, desvio padrao, jitter
 *  pico a pico (max - min) e maior atraso.
 *************************************************/

#include <Arduino.h>
#include <math.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/semphr.h"
#include "esp_timer.h"

/* ---------- Configuracoes ---------- */
#define LedVerm 2
#define LedAmar 4

#define PERIODO_VERM_MS 500
#define PERIODO_AMAR_MS 300

/* Numero de execucoes entre cada resumo estatistico */
#define RESUMO_A_CADA 20

/* 0 = vTaskDelay (igual ao experimento original: o periodo
 *     real = tempo de execucao da tarefa + atraso)
 * 1 = vTaskDelayUntil (periodo absoluto, sem acumular deriva) */
#define USAR_DELAY_UNTIL 0

/* ---------- Estrutura de monitoramento ---------- */
typedef struct {
  const char *nome;        // identificacao da tarefa
  uint8_t pino;            // GPIO do LED
  uint32_t periodoMs;      // periodo esperado (ms)
  int64_t tAnterior;       // instante da ativacao anterior (us)
  int64_t intervaloAnt;    // intervalo anterior (us) -> jitter ciclo a ciclo
  uint32_t ativacoes;      // numero total de ativacoes
  uint32_t amostras;       // intervalos acumulados no resumo atual
  int64_t minIntervalo;    // menor intervalo observado (us)
  int64_t maxIntervalo;    // maior intervalo observado (us)
  double soma;             // soma dos intervalos (media)
  double somaQuad;         // soma dos quadrados (desvio padrao)
} MonitorTarefa;

MonitorTarefa monVerm = { "TASK1 LED VERMELHO", LedVerm, PERIODO_VERM_MS, 0, 0, 0, 0, INT64_MAX, 0, 0, 0 };
MonitorTarefa monAmar = { "TASK2 LED AMARELO ", LedAmar, PERIODO_AMAR_MS, 0, 0, 0, 0, INT64_MAX, 0, 0, 0 };

/* Handles das tarefas */
TaskHandle_t tarefa1 = NULL;
TaskHandle_t tarefa2 = NULL;

/* Mutex que protege a Serial (evita mensagens misturadas,
 * ja que as duas tarefas rodam ao mesmo tempo em nucleos diferentes) */
SemaphoreHandle_t mutexSerial = NULL;

/* Prototipos */
void blinkVermelho(void *pvParameter);
void blinkAmarelo(void *pvParameter);
void registrarAtivacao(MonitorTarefa *m, int64_t agora);
void zerarEstatisticas(MonitorTarefa *m);

/*************************************************
 * SETUP
 *************************************************/
void setup() {
  Serial.begin(115200);
  delay(500);

  pinMode(LedVerm, OUTPUT);
  pinMode(LedAmar, OUTPUT);
  digitalWrite(LedVerm, LOW);
  digitalWrite(LedAmar, LOW);

  mutexSerial = xSemaphoreCreateMutex();

  Serial.println();
  Serial.println("=== Experimento 7 - Atividade Avaliativa 1 ===");
  Serial.printf("Modo de espera: %s\n",
                USAR_DELAY_UNTIL ? "vTaskDelayUntil" : "vTaskDelay");

  BaseType_t r1 = xTaskCreatePinnedToCore(
    blinkVermelho, "tarefa1", 4096, &monVerm, 1, &tarefa1, 0);

  BaseType_t r2 = xTaskCreatePinnedToCore(
    blinkAmarelo, "tarefa2", 4096, &monAmar, 2, &tarefa2, 1);

  Serial.println(r1 == pdPASS ? "TASK1 criada com sucesso!" : "Erro ao criar TASK1!");
  Serial.println(r2 == pdPASS ? "TASK2 criada com sucesso!" : "Erro ao criar TASK2!");
}

/*************************************************
 * LOOP - nao faz nada, apenas libera a CPU
 *************************************************/
void loop() {
  vTaskDelay(pdMS_TO_TICKS(1000));
}

/*************************************************
 * TASK1 - LED VERMELHO | GPIO 2 | Nucleo 0 | 500 ms
 *************************************************/
void blinkVermelho(void *pvParameter) {
  MonitorTarefa *m = (MonitorTarefa *)pvParameter;
#if USAR_DELAY_UNTIL
  TickType_t ultimoWake = xTaskGetTickCount();
#endif

  while (1) {
    /* 1) Captura o instante de ativacao ANTES de qualquer
     *    processamento, para nao distorcer a medida */
    int64_t agora = esp_timer_get_time();

    /* 2) Executa a funcao da tarefa: inverte o LED */
    digitalWrite(m->pino, !digitalRead(m->pino));

    /* 3) Calcula e imprime as metricas temporais */
    registrarAtivacao(m, agora);

    /* 4) Aguarda o proximo periodo */
#if USAR_DELAY_UNTIL
    vTaskDelayUntil(&ultimoWake, pdMS_TO_TICKS(m->periodoMs));
#else
    vTaskDelay(pdMS_TO_TICKS(m->periodoMs));
#endif
  }
}

/*************************************************
 * TASK2 - LED AMARELO | GPIO 4 | Nucleo 1 | 300 ms
 *************************************************/
void blinkAmarelo(void *pvParameter) {
  MonitorTarefa *m = (MonitorTarefa *)pvParameter;
#if USAR_DELAY_UNTIL
  TickType_t ultimoWake = xTaskGetTickCount();
#endif

  while (1) {
    int64_t agora = esp_timer_get_time();

    digitalWrite(m->pino, !digitalRead(m->pino));

    registrarAtivacao(m, agora);

#if USAR_DELAY_UNTIL
    vTaskDelayUntil(&ultimoWake, pdMS_TO_TICKS(m->periodoMs));
#else
    vTaskDelay(pdMS_TO_TICKS(m->periodoMs));
#endif
  }
}

/*************************************************
 * Calcula intervalo, desvio, jitter e estatisticas
 * e envia para o Monitor Serial
 *************************************************/
void registrarAtivacao(MonitorTarefa *m, int64_t agora) {
  m->ativacoes++;
  int nucleo = xPortGetCoreID();
  int64_t esperadoUs = (int64_t)m->periodoMs * 1000LL;

  /* Primeira ativacao: ainda nao existe intervalo anterior */
  if (m->tAnterior == 0) {
    m->tAnterior = agora;
    if (xSemaphoreTake(mutexSerial, portMAX_DELAY) == pdTRUE) {
      Serial.printf("[%10.6f s] %s | Nucleo %d | Primeira ativacao\n",
                    agora / 1e6, m->nome, nucleo);
      xSemaphoreGive(mutexSerial);
    }
    return;
  }

  int64_t intervalo = agora - m->tAnterior;          // periodo observado
  int64_t desvio    = intervalo - esperadoUs;        // atraso (+) / adiantamento (-)
  int64_t jitterCiclo = (m->intervaloAnt == 0) ? 0 : llabs(intervalo - m->intervaloAnt);

  m->tAnterior = agora;
  m->intervaloAnt = intervalo;

  /* Acumula estatisticas */
  m->amostras++;
  if (intervalo < m->minIntervalo) m->minIntervalo = intervalo;
  if (intervalo > m->maxIntervalo) m->maxIntervalo = intervalo;
  m->soma     += (double)intervalo;
  m->somaQuad += (double)intervalo * (double)intervalo;

  if (xSemaphoreTake(mutexSerial, portMAX_DELAY) == pdTRUE) {
    Serial.printf(
      "[%10.6f s] %s | Nucleo %d | Esperado: %lld us | Observado: %lld us | Desvio: %+lld us | Jitter: %lld us\n",
      agora / 1e6, m->nome, nucleo,
      (long long)esperadoUs, (long long)intervalo,
      (long long)desvio, (long long)jitterCiclo);

    /* Resumo estatistico periodico */
    if (m->amostras >= RESUMO_A_CADA) {
      double media = m->soma / m->amostras;
      double var = (m->somaQuad / m->amostras) - (media * media);
      double dp = var > 0 ? sqrt(var) : 0;
      int64_t jitterPP = m->maxIntervalo - m->minIntervalo;
      int64_t piorAtraso = m->maxIntervalo - esperadoUs;

      Serial.println("--------------------------------------------------------------------------");
      Serial.printf("RESUMO %s | Nucleo %d | ultimas %u execucoes (total %u)\n",
                    m->nome, nucleo, (unsigned)m->amostras, (unsigned)m->ativacoes);
      Serial.printf("  Esperado: %lld us | Min: %lld us | Max: %lld us | Media: %.1f us\n",
                    (long long)esperadoUs, (long long)m->minIntervalo,
                    (long long)m->maxIntervalo, media);
      Serial.printf("  Desvio padrao: %.1f us | Jitter pico-a-pico: %lld us | Pior atraso: %+lld us\n",
                    dp, (long long)jitterPP, (long long)piorAtraso);
      Serial.printf("  Situacao: %s\n",
                    (llabs(piorAtraso) <= 1000 && jitterPP <= 1000)
                      ? "periodo RESPEITADO (variacao <= 1 tick / 1 ms)"
                      : "ATRASO/JITTER acima de 1 ms");
      Serial.println("--------------------------------------------------------------------------");
      zerarEstatisticas(m);
    }
    xSemaphoreGive(mutexSerial);
  }
}

void zerarEstatisticas(MonitorTarefa *m) {
  m->amostras = 0;
  m->minIntervalo = INT64_MAX;
  m->maxIntervalo = 0;
  m->soma = 0;
  m->somaQuad = 0;
}
