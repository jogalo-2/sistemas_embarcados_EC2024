// Desafio 3: letras A, B, C, D e E em código Morse
// LED no pino 9 e piezo no pino 8

const int led = 9;
const int piezo = 8;
const int unidade = 200;

const char* letras[] = {"A", "B", "C", "D", "E"};
const char* codigos[] = {".-", "-...", "-.-.", "-..", "."};

void sinal(int duracao) {
  digitalWrite(led, HIGH);
  tone(piezo, 1000);
  delay(duracao);

  digitalWrite(led, LOW);
  noTone(piezo);
  delay(unidade);
}

void setup() {
  pinMode(led, OUTPUT);
  pinMode(piezo, OUTPUT);
  Serial.begin(9600);
}

void loop() {
  for (int letra = 0; letra < 5; letra++) {
    Serial.println(letras[letra]);

    for (int simbolo = 0; codigos[letra][simbolo] != '\0'; simbolo++) {
      if (codigos[letra][simbolo] == '.') {
        sinal(unidade);
      } else {
        sinal(3 * unidade);
      }
    }

    delay(2 * unidade);
  }

  delay(4 * unidade);
}
