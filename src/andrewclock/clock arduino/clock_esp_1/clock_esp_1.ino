/* This example shows how to use continuous mode to take
range measurements with the VL6180X. It is based on
vl53l0x_ContinuousRanging_Example.c from the VL53L0X API.
The range readings are in units of mm. */

#include <Wire.h>
#include <VL53L0X.h>
#include <ESP32_Servo.h>

VL53L0X sensor;
Servo myservo;

const int inPin = 15;
int servoPin = 13;

int sens1 = 0; 
int sens2 = 0; 
int sequence = 0;
unsigned long t = 0;
int servoPos = 0;

void getReadings() {
  
  sens2 = digitalRead(inPin);

  if (sensor.timeoutOccurred()) { Serial.print(" TIMEOUT"); }

  if (sensor.readRangeContinuousMillimeters()<100) {
    sens1 = HIGH;
  }
  else {
    sens1 = LOW;
  }

  Serial.print("sensor 1: ");
  Serial.print(sens1);
  Serial.print(", sensor 2: ");
  Serial.println(sens2);
  Serial.println(sensor.readRangeContinuousMillimeters());
}

void activateServo(){
  Serial.println("Servo activated");
  myservo.attach(servoPin);
  for (servoPos = 0; servoPos >=-360; servoPos -= 1) { 
    myservo.write(servoPos);             
    delay(15);                      
  }
  myservo.detach();     

}

void printSequence() {
  Serial.print("Sequence: ");
  Serial.println(sequence);
}

void setup()
{
  Serial.begin(9600);
  Wire.begin();

  sensor.init();
  sensor.setTimeout(500);
  sensor.startContinuous();
  
  pinMode(inPin, INPUT_PULLDOWN);

}

void loop()
{
  getReadings();
  printSequence();

  //Sense1 is where the hole will be.
  //Sense2 is where the hole is covered.

  if ((sequence==0) && (sens1==HIGH) and (sens2==HIGH)){ // both are initially open.
    sequence=1;
  }
  
  if ((sequence==1) and (sens1==LOW) and (sens2==HIGH)) { // correct holes covered, trigger
    sequence=0;
    activateServo();
  } 
  else if ((sequence==1) and (sens1==LOW) and (sens2==LOW)) { // both are open
    sequence=0;
  }
  printSequence();


  delay(500);
}






