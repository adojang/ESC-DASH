/* This example shows how to use continuous mode to take
range measurements with the VL6180X. It is based on
vl53l0x_ContinuousRanging_Example.c from the VL53L0X API.
The range readings are in units of mm. */

#include <Wire.h>
#include <VL53L0X.h>

VL53L0X sensor;

const int outPin = 15;
int covered = 0;

void setup()
{
  Serial.begin(9600);
  Wire.begin();

  sensor.init();
  sensor.setTimeout(500);

  pinMode(outPin, OUTPUT);


  sensor.startContinuous();
}

void loop()
{
  // Serial.print(sensor.readRangeContinuousMillimeters());
  if (sensor.timeoutOccurred()) { Serial.print(" TIMEOUT"); }

  // Serial.println();

  if (sensor.readRangeContinuousMillimeters()<100) {
    Serial.println("Covered");
    covered = HIGH;
    digitalWrite(outPin, HIGH);
  }
  else {
    Serial.println("Uncovered");
    covered = LOW;
    digitalWrite(outPin, LOW);
  }
  delay(250);
}