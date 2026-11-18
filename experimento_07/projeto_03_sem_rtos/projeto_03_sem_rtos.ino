/*************************************************
 * Experimento 7 - Uso dos nucleos do ESP32
 * Projeto 3 (roteiro: "Experimento 4 - Versao 2")
 * Comparacao SEM FreeRTOS (programacao sequencial)
 *
 * Mesmo comportamento do Projeto 2, mas em um unico
 * loop, sem multitarefa real (temporizacao por millis()).
 *************************************************/

#define LED1 2
#define LED2 4

unsigned long t1 = 0;
unsigned long t2 = 0;

const unsigned long periodoLED1 = 500;
const unsigned long periodoLED2 = 300;

void setup() {

  Serial.begin(115200);

  pinMode(LED1, OUTPUT);
  pinMode(LED2, OUTPUT);

  digitalWrite(LED1, LOW);
  digitalWrite(LED2, LOW);

  t1 = millis();
  t2 = millis();

  Serial.println("Sistema iniciado - Sem RTOS");
}

void loop() {

  unsigned long agora = millis();

  // Controle temporal do LED1
  if (agora - t1 >= periodoLED1) {

    unsigned long intervalo = agora - t1;

    digitalWrite(LED1, !digitalRead(LED1));

    Serial.printf(
      "LED1 | Esperado: %lums | Observado: %lums\n",
      periodoLED1, intervalo
    );

    t1 = agora;
  }

  // Controle temporal do LED2
  if (agora - t2 >= periodoLED2) {

    unsigned long intervalo = agora - t2;

    digitalWrite(LED2, !digitalRead(LED2));

    Serial.printf(
      "LED2 | Esperado: %lums | Observado: %lums\n",
      periodoLED2, intervalo
    );

    t2 = agora;
  }
}
