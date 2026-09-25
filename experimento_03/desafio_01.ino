// Desafio 1: frequência do piezo em função do ângulo
// Piezo no pino 8

const int piezo = 8;

void setup() {
  pinMode(piezo, OUTPUT);
  Serial.begin(9600);
  Serial.println("Angulo (graus) | Frequencia (Hz)");
}

void loop() {
  for (int angulo = 0; angulo <= 360; angulo += 30) {
    float radianos = angulo * PI / 180.0;
    int frequencia = 2000 + 1000 * sin(radianos);

    Serial.print(angulo);
    Serial.print(" | ");
    Serial.println(frequencia);

    tone(piezo, frequencia);
    delay(200);
  }

  noTone(piezo);
  delay(2000);
}
