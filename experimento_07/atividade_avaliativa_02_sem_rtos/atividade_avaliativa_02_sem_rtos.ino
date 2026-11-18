/*************************************************
 * Experimento 7 - Uso dos nucleos do ESP32
 * ATIVIDADE AVALIATIVA 2 - Versao SEM FreeRTOS
 *
 * Mesma aplicacao do Projeto 3 (loop sequencial com
 * temporizacao por tempo decorrido), mas medindo em
 * microssegundos (micros()) e com as mesmas
 * estatisticas da versao COM RTOS, para comparacao.
 *
 * LED1 (GPIO 2) - 500 ms
 * LED2 (GPIO 4) - 300 ms
 *
 * CARGA_MS: simula um processamento bloqueante que
 * ocorre a cada PERIODO_CARGA_MS dentro do loop.
 * Como tudo roda em sequencia, enquanto a carga
 * executa nenhum LED pode ser atendido.
 *************************************************/

#include <math.h>

#define LED1 2
#define LED2 4

#define PERIODO_LED1_MS 500
#define PERIODO_LED2_MS 300

#define CARGA_MS          0     // duracao da carga bloqueante (0 = sem carga)
#define PERIODO_CARGA_MS  100

#define RESUMO_A_CADA 20

typedef struct {
  const char *nome;
  uint8_t pino;
  uint32_t periodoMs;
  unsigned long tAnterior;   // us (micros)
  uint32_t amostras;
  long minI, maxI;
  double soma, somaQuad;
} Estat;

Estat e1 = { "LED1", LED1, PERIODO_LED1_MS, 0, 0, 0x7FFFFFFF, 0, 0, 0 };
Estat e2 = { "LED2", LED2, PERIODO_LED2_MS, 0, 0, 0x7FFFFFFF, 0, 0, 0 };

unsigned long tCarga = 0;

/* Verifica se o periodo venceu; se sim, executa a "tarefa" */
void verificar(Estat *e) {
  unsigned long agora = micros();
  unsigned long esperado = e->periodoMs * 1000UL;

  if (agora - e->tAnterior < esperado) return;   // ainda nao venceu

  long intervalo = (long)(agora - e->tAnterior);
  e->tAnterior = agora;

  digitalWrite(e->pino, !digitalRead(e->pino));

  e->amostras++;
  if (intervalo < e->minI) e->minI = intervalo;
  if (intervalo > e->maxI) e->maxI = intervalo;
  e->soma += intervalo;
  e->somaQuad += (double)intervalo * intervalo;

  Serial.printf("SEM RTOS | %s | Core %d | Esperado: %lu us | Observado: %ld us | Desvio: %+ld us\n",
                e->nome, xPortGetCoreID(), esperado, intervalo, intervalo - (long)esperado);

  if (e->amostras >= RESUMO_A_CADA) {
    double media = e->soma / e->amostras;
    double var = e->somaQuad / e->amostras - media * media;
    Serial.printf(">> RESUMO SEM RTOS | %s | n=%u | Min: %ld | Max: %ld | Media: %.1f | DP: %.1f | Jitter p-p: %ld us\n",
                  e->nome, (unsigned)e->amostras, e->minI, e->maxI,
                  media, var > 0 ? sqrt(var) : 0.0, e->maxI - e->minI);
    e->amostras = 0; e->minI = 0x7FFFFFFF; e->maxI = 0; e->soma = 0; e->somaQuad = 0;
  }
}

void setup() {
  Serial.begin(115200);
  delay(500);

  pinMode(LED1, OUTPUT);
  pinMode(LED2, OUTPUT);
  digitalWrite(LED1, LOW);
  digitalWrite(LED2, LOW);

  e1.tAnterior = micros();
  e2.tAnterior = micros();
  tCarga = millis();

  Serial.printf("Sistema iniciado - SEM RTOS (loop sequencial) | Carga: %d ms a cada %d ms\n",
                CARGA_MS, PERIODO_CARGA_MS);
}

void loop() {
  verificar(&e1);   // "tarefa" do LED1
  verificar(&e2);   // "tarefa" do LED2

  /* Carga bloqueante: durante esse tempo o loop fica preso */
  if (CARGA_MS > 0 && millis() - tCarga >= PERIODO_CARGA_MS) {
    tCarga = millis();
    unsigned long inicio = micros();
    while (micros() - inicio < CARGA_MS * 1000UL) {
      /* processamento simulado */
    }
  }
}
