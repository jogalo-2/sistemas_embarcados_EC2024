// Desafio 2: alarme quando a luminosidade estiver baixa
// LDR entre 5 V e A0; resistor de 10 kOhm entre A0 e GND
// LED no pino 9 e piezo no pino 8

const int ldr = A0;
const int led = 9;
const int piezo = 8;
const int limiar = 500;

void setup() {
  pinMode(led, OUTPUT);
  pinMode(piezo, OUTPUT);
  Serial.begin(9600);
}

void loop() {
  int luminosidade = analogRead(ldr);
  Serial.println(luminosidade);

  if (luminosidade < limiar) {
    digitalWrite(led, HIGH);
    tone(piezo, 1000);
    delay(250);

    digitalWrite(led, LOW);
    noTone(piezo);
    delay(250);
  } else {
    digitalWrite(led, LOW);
    noTone(piezo);
    delay(100);
  }
}
