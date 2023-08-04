/* Kernal*/
#include <Arduino.h>
#include <config.h>
#include "encode.h"
/* ESP-DASH */
void readPin(uint8_t pin);

uint8_t reading = 0;

int pin17 = 0;
int pin18 = 0;
int pin19 = 0;
int pin21 = 0;
int pin22 = 0;
int pin23 = 0;
int pin25 = 0;
int pin26 = 0;
int pin27 = 0;
int pin32 = 0;
int pin33 = 0;
int pin34 = 0;
int pin35 = 0;

void setup() 
{
  Serial.begin(115200);
//LED 2 is the blue onboard light of the ESP32
  pinMode(2, OUTPUT);
  digitalWrite(2,LOW);

  pinMode(17,INPUT_PULLDOWN);
  pinMode(18,INPUT_PULLDOWN);
  pinMode(19,INPUT_PULLDOWN);
  pinMode(21,INPUT_PULLDOWN);
  pinMode(22,INPUT_PULLDOWN);
  pinMode(23,INPUT_PULLDOWN);
  pinMode(25,INPUT_PULLDOWN);
  pinMode(26,INPUT_PULLDOWN);
  pinMode(27,INPUT_PULLDOWN);
  pinMode(32,INPUT_PULLDOWN);
  pinMode(33,INPUT_PULLDOWN);
  pinMode(34,INPUT);
  pinMode(35,INPUT);
}

void loop() 
{
if (pin17 == 1 && pin18 == 1 && pin19 == 1 && pin21 == 1 && pin22 == 1 && pin23 == 1 && pin25 == 1 
&& pin26 == 1 && pin27 == 1 && pin32 == 1 && pin33 == 1 && pin34 == 1 && pin35 == 1)
{
  printf("Puzzle Complete!\n");
  delay(100);
}

readPin(17);
readPin(18);
readPin(19);
readPin(21);
readPin(22);
readPin(23);
readPin(25);
readPin(26);
readPin(27);
readPin(32);
readPin(33);
readPin(34);
readPin(35);
}

void readPin(uint8_t pin)
{
  for (int i = 0; i < 20; i++)
  {
    reading = reading + digitalRead(pin);
    delay(10);
  }

    if (reading > 19)
    {
      Serial.printf("Pin %d: 1 \n", pin);
      switch (pin)
      {
      case 17:
        pin17 = 1;
        break;
      case 18:
        pin18 = 1;
        break;
      case 19:
        pin19 = 1;
        break;
      case 21:
        pin21 = 1;
        break;
      case 22:
        pin22 = 1;
        break;
      case 23:
        pin23 = 1;
        break;
      case 25:
        pin25 = 1;
        break;
      case 26:
        pin26 = 1;
        break;
      case 27:
        pin27 = 1;
        break;
      case 32:
        pin32 = 1;
        break;
      case 33:
        pin33 = 1;
        break;
      case 34:
        pin34 = 1;
        break;
      case 35:
        pin35 = 1;
        break;
      default:
        break;
      }
    }
    else
    {
      Serial.printf("Pin %d: 0 \n", pin);
      switch (pin)
      {
      case 17:
        pin17 = 0;
        break;
      case 18:
        pin18 = 0;
        break;
      case 19:
        pin19 = 0;
        break;
      case 21:
        pin21 = 0;
        break;
      case 22:
        pin22 = 0;
        break;
      case 23:
        pin23 = 0;
        break;
      case 25:
        pin25 = 0;
        break;
      case 26:
        pin26 = 0;
        break;
      case 27:
        pin27 = 0;
        break;
      case 32:
        pin32 = 0;
        break;
      case 33:
        pin33 = 0;
        break;
      case 34:
        pin34 = 0;
        break;
      case 35:
        pin35 = 0;
        break;
      default:
        break;
      }
    }
    reading = 0;
}