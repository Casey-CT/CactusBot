#include "WaterLevelSensor.h"

//#include "SensorTimer.h"

#include <Arduino.h>

#define PI_VAL 3.1415926535

WaterLevelSensor::WaterLevelSensor(const int waterOn, const int waterIn, const float scaleFactor, const unsigned int fillHeight, const unsigned int readCount, const float minPercent, const float containerRadius, const unsigned long interval): SensorTimer(interval) {
  this->waterOn = waterOn;
  this->waterIn = waterIn;

  this->scaleFactor = scaleFactor;

  this->fillHeight = fillHeight;
  this->readCount = readCount;

  this->minPercent = minPercent;
  this->containerRadius = containerRadius;
}

void WaterLevelSensor::init() {
  pinMode(waterOn, OUTPUT);
  takeReading();
}

void WaterLevelSensor::takeReading() {
  // Power sensor via pin, then take a number of readings, then average the results
  digitalWrite(waterOn, HIGH);
  int result = 0;
  for (unsigned int i = 0; i < readCount; i++) {
    result += analogRead(waterIn);
    delay(1);
  }
  digitalWrite(waterOn, LOW);

  result /= readCount;

  latestReading = result;
}

// Get the current fill of the container, in percentage
float WaterLevelSensor::getFillPercent() const {
  float toFloat = static_cast<float>(latestReading);
  float power = (toFloat / scaleFactor);
  float percent = pow(10, power);
  return percent;
}

// Get total fill of container, in ml
float WaterLevelSensor::getFillTotal() const {
  return PI_VAL * containerRadius * fillHeight;
}

// Get current fill of container, in ml
float WaterLevelSensor::getFillAmount() const {
  return getFillPercent() * getFillTotal();
}

bool WaterLevelSensor::isEmpty() const {
  return getFillPercent() < minPercent;
}
