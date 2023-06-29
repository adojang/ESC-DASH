/*
  --------------------------------------------------------------------------
  Escape Room Template
  Adriaan van Wijk
  22 May 2023

  This code controls 4 relays which will arm and disarm electromagnetic locks.

  Copyright [2023] [Proxonics (Pty) Ltd]

  Licensed under the Apache License, Version 2.0 (the "License");
  you may not use this file except in compliance with the License.
  You may obtain a copy of the License at

  http://www.apache.org/licenses/LICENSE-2.0

  Unless required by applicable law or agreed to in writing, software
  distributed under the License is distributed on an "AS IS" BASIS,
  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
  See the License for the specific language governing permissions and
  limitations under the License.
  --------------------------------------------------------------------------
*/

#define NAME "atticmaster2"
#define MACAD 0x04 // Refer to Table in Conventions


/* Kernal*/
#include <Arduino.h>
#include <config.h>
#include <encode.h>
#include <AsyncTimer.h>

AsyncTimer asynctimer(35);


bool DOORTOUCH = false;
bool doorlocked = false;

int readingcounter = 0;

float currentValue = 0.0;
float previousEMA = 0.0;
float prevousEMA2 = 0.0;
float smoothingFactor = 0.8;  // Adjust this value based on your application
//0.8 600-3400 floating, 4000 touching

float emaFilter(float currentValue, float previousEMA, float smoothingFactor) {
  return (currentValue * smoothingFactor) + (previousEMA * (1 - smoothingFactor));
}

void getTouch(){

  int reading = analogRead(34);
  float filteredValue = emaFilter(reading, previousEMA, smoothingFactor);
  previousEMA = filteredValue;
    Serial.println(previousEMA);


  if(filteredValue > 3800){
    Serial.printf("ABOVE THRESHOLD\n\n\n\n");
    readingcounter +=80; // trigger in 2 ticks.
  }


    if (readingcounter >= 100) {
      readingcounter = 0;
      Serial.printf("TRIGGERD THRESHOLD\n");
      Serial.println("Human Chain Touch Detected!");
      DOORTOUCH = true;
    }
    readingcounter -= 5;

    if (readingcounter < 0) readingcounter = 0;
}






void setup() {
  Serial.begin(115200);
  //HumanTouch Pins
  pinMode(34, INPUT_PULLDOWN);
  pinMode(15, OUTPUT);
  digitalWrite(15,HIGH);
  pinMode(5,OUTPUT);
  pinMode(5,LOW);


  asynctimer.setInterval([]() {getTouch();},  250);

}



void loop() {
if(DOORTOUCH == true)
{    
    Serial.println("Write HIGH");
    digitalWrite(5,HIGH);
    DOORTOUCH = false;
    delay(500);
}
else{
    digitalWrite(5,LOW);
    }







  asynctimer.handle();

}