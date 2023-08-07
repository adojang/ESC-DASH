/*
The Pololu MP6550 DC Motor Controller is a compact, single-channel motor driver that can control one brushed DC motor. It is designed to work with a wide range of input voltages and has a peak output current of 2.5A.
 
Here's an example code for controlling a DC motor using an Arduino and the Pololu MP6550 DC Motor Controller:
 
In this example, we define the direction (DIR) and pulse-width modulation (PWM) pins connected to the Arduino. The setMotorSpeed function controls the motor speed and direction by setting the DIR pin and sending a PWM signal to the PWM pin.
 
The loop function runs the motor at half speed forward, stops the motor, runs the motor at half speed in reverse, and stops the motor again. This sequence repeats indefinitely.
 
To use this code, connect the Pololu MP6550 DC Motor Controller to an Arduino as follows:
 
DIR_pin (Arduino) -> DIR (Pololu MP6550)
PWM_pin (Arduino) -> PWM (Pololu MP6550)
GND (Arduino) -> GND (Pololu MP6550)
VM (Pololu MP6550) -> Power supply positive terminal
GND (Pololu MP6550) -> Power supply negative terminal
OUTA (Pololu MP6550) -> Motor terminal A
OUTB (Pololu MP6550) -> Motor terminal B
Upload the code to the Arduino and observe the motor's behavior.
*/
 
#include <Arduino.h>
 
const int PWM_pin1 = 4; // Direction Pin
const int PWM_pin2 = 5; // PWM Pin
 
void setup() {
  Serial.begin(115200);

    pinMode(PWM_pin1, OUTPUT);
  pinMode(PWM_pin2, OUTPUT);
  ledcAttachPin(PWM_pin1, 0);
  ledcAttachPin(PWM_pin2, 1);
  ledcSetup(0, 1000, 8);
  ledcSetup(1, 1000, 8);
  ledcWrite(0, 0);
  ledcWrite(1, 0);
}
 
 
void loop() {
  int speed;
 
  // Run the motor at half speed forward
     Serial.println("forwardservo");
  ledcWrite(0, 200);
  ledcWrite(1, 0);
  delay(2000);

  ledcWrite(0, 0);
  ledcWrite(1, 0);
  delay(1000);
 
    Serial.println("reverseservo");
  ledcWrite(0, 0);
  ledcWrite(1, 200);
  delay(2000);

  ledcWrite(0, 0);
  ledcWrite(1, 0);
  delay(1000);
}