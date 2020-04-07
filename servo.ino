#include <Servo.h>

Servo myServo;

const int ipin1 = A0;
const int ipin2 = A2;

const int pin2 = 9;

void setup() {
  // put your setup code here, to run once:
  myServo.attach(pin2);
  Serial.begin(9600);
}

int prevState = -2;
int state = -1;
boolean triggered = false;
unsigned long timer=0;

void loop() {
  // put your main code here, to run repeatedly:
  int pot1 = analogRead(ipin1);
  int pot2 = analogRead(ipin2);
  Serial.print(pot1);
  Serial.print(" ");
  Serial.print(pot2);
  Serial.print(" ");
  Serial.print(state);
  Serial.print(" ");
  Serial.print(prevState);
  Serial.print(" ");
  Serial.print(triggered);
  Serial.print(" ");
  Serial.print(timer);
    Serial.print(" ");
  Serial.print(millis());
  Serial.println();

  if(prevState != state)
  {
    prevState = state;
    timer = millis();
    triggered=true;
  }
  if(triggered && millis()-timer<5000) {myServo.write(80); Serial.println("ROR");}
  else {triggered=false; myServo.write(0);}

  if(pot1 > 600 && pot2 > 600) {state = 1;}
  if(pot1 > 600 && pot2 < 600) {state = 2;}
  if(pot1 < 600 && pot2 > 600) {state = 3;}
  if(pot1 < 600 && pot2 < 600) {state = 4;}

    delay(15);
}
