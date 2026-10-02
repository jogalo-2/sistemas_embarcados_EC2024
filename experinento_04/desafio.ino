#include <Servo.h>
int open = 90;

int close = 0;

int distance = 0;

double cm = 0;

int n = 0;

int angle = 0;

Servo servo1; // Cria um objeto servo


long readUltrassonicdistance(int triggerPin, int echoPin)

{

  pinMode(triggerPin, OUTPUT);

  digitalWrite(triggerPin, LOW);

  delayMicroseconds(1);

  digitalWrite(triggerPin, HIGH);

  delayMicroseconds(3);

  digitalWrite(triggerPin, LOW);

  pinMode(echoPin, INPUT);

  return pulseIn(echoPin, HIGH);

} //envia sinal no sensor e reconhece a volta dele após ir ao objeto e voltar



void calculaDistancia(){

  distance = 20;

  cm = 0.017233 * readUltrassonicdistance(12, 11);

 

} // calcula distância do objeto ao sensor

void portaofechado()
{
  angle = close;

  servo1.write(angle);

  digitalWrite(6, LOW);  // verde apagado
  digitalWrite(3, HIGH); // vermelho aceso

  noTone(10);
}

void setup(){
    
  Serial.begin(9600);

  pinMode(3, OUTPUT);

  servo1.attach(5);

  pinMode(6, OUTPUT);

  pinMode(10, OUTPUT);

  portaofechado();
} // define pinos do arduino a serem usados



void loop(){
    
  calculaDistancia();
    if (cm >= distance){
        
        delay(3000);
        calculaDistancia();

        if(angle == open){
            angle = close;

        }
        servo1.write(angle);
      digitalWrite(6, LOW);

      digitalWrite(3, HIGH);

      noTone(10);

      calculaDistancia();

    } 

    if(cm < distance){
        if(angle == close){
            angle = open;
        }
        servo1.write(angle);
      digitalWrite(6, HIGH);
      digitalWrite(3, LOW);

      tone(10, 100);

      delay(300);

      noTone(10);


      calculaDistancia();

    }

}
