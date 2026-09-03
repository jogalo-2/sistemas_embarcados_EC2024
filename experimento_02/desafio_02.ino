/*
  Tarefa avaliativa 2 - Sistema de sinalizacao com estados
  Placa: Arduino Mega 2560

  LED verde: D8
  LED vermelho: D9
  Botao: entre D2 e GND (INPUT_PULLUP)

  Cada LED deve ser ligado em serie com um resistor de 220 a 330 ohms.
*/

const byte LED_VERDE = 8;
const byte LED_VERMELHO = 9;
const byte BOTAO = 2;

const unsigned long TEMPO_LENTO_MS = 500;      // ciclo completo = 1 s = 1 Hz
const unsigned long TEMPO_RAPIDO_MS = 125;     // ciclo completo = 250 ms = 4 Hz
const unsigned long TEMPO_ALTERNADO_MS = 250;  // troca de LED a cada 250 ms
const unsigned long ANTIRRUIDO_MS = 50;

enum Estado {
  NORMAL = 0,
  SINALIZACAO_LENTA = 1,
  SINALIZACAO_RAPIDA = 2,
  SINALIZACAO_ALTERNADA = 3
};

Estado estadoAtual = NORMAL;

bool faseLigada = true;
unsigned long instanteDaUltimaTroca = 0;

bool ultimaLeituraDoBotao = HIGH;
bool estadoEstavelDoBotao = HIGH;
unsigned long instanteDaMudancaDoBotao = 0;

void mostrarEstadoNoSerial() {
  Serial.print("Estado atual: ");

  switch (estadoAtual) {
    case NORMAL:
      Serial.println("0 - operacao normal");
      break;
    case SINALIZACAO_LENTA:
      Serial.println("1 - atencao, sinalizacao lenta (1 Hz)");
      break;
    case SINALIZACAO_RAPIDA:
      Serial.println("2 - alerta, sinalizacao rapida (4 Hz)");
      break;
    case SINALIZACAO_ALTERNADA:
      Serial.println("3 - falha/manutencao, LEDs alternados");
      break;
  }
}

void entrarNoEstado(Estado novoEstado) {
  estadoAtual = novoEstado;
  faseLigada = true;
  instanteDaUltimaTroca = millis();

  // Define imediatamente a aparencia inicial de cada estado.
  switch (estadoAtual) {
    case NORMAL:
      digitalWrite(LED_VERDE, HIGH);
      digitalWrite(LED_VERMELHO, LOW);
      break;
    case SINALIZACAO_LENTA:
      digitalWrite(LED_VERDE, HIGH);
      digitalWrite(LED_VERMELHO, LOW);
      break;
    case SINALIZACAO_RAPIDA:
      digitalWrite(LED_VERDE, LOW);
      digitalWrite(LED_VERMELHO, HIGH);
      break;
    case SINALIZACAO_ALTERNADA:
      digitalWrite(LED_VERDE, HIGH);
      digitalWrite(LED_VERMELHO, LOW);
      break;
  }

  mostrarEstadoNoSerial();
}

void sinalizarInicializacao() {
  // Tres piscadas simultaneas indicam que o sistema esta pronto.
  for (byte i = 0; i < 3; i++) {
    digitalWrite(LED_VERDE, HIGH);
    digitalWrite(LED_VERMELHO, HIGH);
    delay(150);
    digitalWrite(LED_VERDE, LOW);
    digitalWrite(LED_VERMELHO, LOW);
    delay(150);
  }
}

void lerBotao() {
  bool leituraAtual = digitalRead(BOTAO);

  if (leituraAtual != ultimaLeituraDoBotao) {
    instanteDaMudancaDoBotao = millis();
    ultimaLeituraDoBotao = leituraAtual;
  }

  if (millis() - instanteDaMudancaDoBotao >= ANTIRRUIDO_MS &&
      leituraAtual != estadoEstavelDoBotao) {
    estadoEstavelDoBotao = leituraAtual;

    // Com INPUT_PULLUP, LOW significa que o botao foi pressionado.
    if (estadoEstavelDoBotao == LOW) {
      byte proximo = (static_cast<byte>(estadoAtual) + 1) % 4;
      entrarNoEstado(static_cast<Estado>(proximo));
    }
  }
}

void atualizarSinalizacao() {
  unsigned long agora = millis();

  switch (estadoAtual) {
    case NORMAL:
      // Saidas fixas; nada precisa ser atualizado.
      break;

    case SINALIZACAO_LENTA:
      if (agora - instanteDaUltimaTroca >= TEMPO_LENTO_MS) {
        instanteDaUltimaTroca = agora;
        faseLigada = !faseLigada;
        digitalWrite(LED_VERDE, faseLigada);
      }
      break;

    case SINALIZACAO_RAPIDA:
      if (agora - instanteDaUltimaTroca >= TEMPO_RAPIDO_MS) {
        instanteDaUltimaTroca = agora;
        faseLigada = !faseLigada;
        digitalWrite(LED_VERMELHO, faseLigada);
      }
      break;

    case SINALIZACAO_ALTERNADA:
      if (agora - instanteDaUltimaTroca >= TEMPO_ALTERNADO_MS) {
        instanteDaUltimaTroca = agora;
        faseLigada = !faseLigada;
        digitalWrite(LED_VERDE, faseLigada);
        digitalWrite(LED_VERMELHO, !faseLigada);
      }
      break;
  }
}

void setup() {
  pinMode(LED_VERDE, OUTPUT);
  pinMode(LED_VERMELHO, OUTPUT);
  pinMode(BOTAO, INPUT_PULLUP);

  Serial.begin(9600);
  sinalizarInicializacao();
  entrarNoEstado(NORMAL);
}

void loop() {
  lerBotao();
  atualizarSinalizacao();
}
