/*
  Tarefa avaliativa 1 - Efeito de aproximacao dos LEDs
  Placa: Arduino Mega 2560

  Ordem fisica, da esquerda para a direita:
  LED 1 = D7, LED 2 = D8, LED 3 = D9,
  LED 4 = D10, LED 5 = D11 e LED 6 = D12.

  Cada LED deve ser ligado em serie com um resistor de 220 a 330 ohms.
*/

const byte PINOS_LEDS[6] = {7, 8, 9, 10, 11, 12};

// 1 = aceso; 0 = apagado.
// A ultima etapa retorna para a primeira, completando o afastamento.
const byte SEQUENCIA[4][6] = {
  {1, 0, 0, 0, 0, 1},  // extremidades: LEDs 1 e 6
  {0, 1, 0, 0, 1, 0},  // aproximacao: LEDs 2 e 5
  {0, 0, 1, 1, 0, 0},  // centro: LEDs 3 e 4
  {0, 1, 0, 0, 1, 0}   // afastamento: LEDs 2 e 5
};

const unsigned long INTERVALO_MS = 400;

byte etapaAtual = 0;
unsigned long instanteDaUltimaMudanca = 0;

void aplicarEtapa(byte etapa) {
  for (byte i = 0; i < 6; i++) {
    digitalWrite(PINOS_LEDS[i], SEQUENCIA[etapa][i]);
  }
}

void setup() {
  for (byte i = 0; i < 6; i++) {
    pinMode(PINOS_LEDS[i], OUTPUT);
  }

  // O sistema inicia exatamente como pede o enunciado:
  // com os LEDs das duas extremidades acesos.
  aplicarEtapa(etapaAtual);
  instanteDaUltimaMudanca = millis();
}

void loop() {
  unsigned long agora = millis();

  if (agora - instanteDaUltimaMudanca >= INTERVALO_MS) {
    instanteDaUltimaMudanca = agora;
    etapaAtual = (etapaAtual + 1) % 4;
    aplicarEtapa(etapaAtual);
  }
}
