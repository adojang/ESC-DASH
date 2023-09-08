/*
  --------------------------------------------------------------------------
  Escape Room Template
  Adriaan van Wijk
  22 May 2023

  This code is part of a multi-node project which involes Esc Rooms in Tygervallei,
  South Africa.

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

#include <EscCore.h>


void setup() {
  Serial.begin(115200);
  
  pinMode(25, OUTPUT);
  digitalWrite(25,LOW);


}

int counter = 376;

void loop() {
  while(counter < 1000){
Serial.printf("Successful Tests: %d\n", counter);
digitalWrite(25,HIGH);
delay(3000);
digitalWrite(25,LOW);
delay(3000);
counter += 1;
Serial.printf("Sensor: %d\n",digitalRead(18));
  }
digitalWrite(25,HIGH);
Serial.printf("All Tests Completed Successfully");



}

//300 tests ran successfully!