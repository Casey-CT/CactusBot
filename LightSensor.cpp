#include "LightSensor.h"

#include "SensorTimer.h"
#include "SimpleTrend.h"

#include <Arduino.h>

// Set pin for use with sensor
LightSensor::LightSensor(const int lightIn, const unsigned long interval): SensorTimer(interval), SimpleTrend() {
  this->lightIn = lightIn;
}

// Take an initial reading, so trend data is initialised
void LightSensor::init() {
  takeReading();
}

void LightSensor::takeReading() {
  latestReading = analogRead(lightIn);
  addDatapoint(latestReading);
}
