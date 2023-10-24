#include <Arduino.h>

void setup(){
Serial.begin(15200);
pinMode(16,OUTPUT);

pinMode(25,OUTPUT);
digitalWrite(25,HIGH);

}

void loop(){
digitalWrite(16,HIGH);
delay(3000);
digitalWrite(16,LOW);
delay(3000);

}