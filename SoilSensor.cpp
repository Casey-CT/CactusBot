#include "SoilSensor.h"
#include "SensorTimer.h"

#include <Arduino.h>

SoilSensor::SoilSensor(const int soilOn, const int soilIn, const int sensorMin, const int sensorMax, const unsigned long interval): SensorTimer(interval) {
  this->soilOn = soilOn;
  this->soilIn = soilIn;
  
  this->sensorMin = sensorMin;
  this->sensorMax = sensorMax;

  calculateMoisturePercentage();
}

/* 
 * Soil moisure sensor uses a digital pin for power, which needs to be set to OUTPUT, then set to LOW, so the sensor starts turned OFF. 
 * To avoid the sensor becoming oxidised faster
 */
void SoilSensor::init() {
  pinMode(soilOn, OUTPUT);
  digitalWrite(soilOn, LOW);
  takeReading();
}

// TODO: Test this sensor type further.
// Check the sensor in soil at various levels of moisture.
// This function may require alteration, likely similar to the reading of the water level sensor.
void SoilSensor::takeReading() {
  digitalWrite(soilOn, HIGH);
  int result = analogRead(soilIn);
  digitalWrite(soilOn, LOW);

  latestReading = result;

  calculateMoisturePercentage();
}

float SoilSensor::calculateMoisturePercentage() {
  float val = static_cast<float>(latestReading) - static_cast<float>(sensorMin);
  float max = static_cast<float>(sensorMax) - static_cast<float>(sensorMin);

  moisturePercentage = val / max;
  return moisturePercentage;
}

// Getters and Setters
int SoilSensor::getSensorMin() const {
  return sensorMin;
}

void SoilSensor::setSensorMin(const int sensorMin) {
  this->sensorMin = sensorMin;
}

int SoilSensor::getSensorMax() const {
  return sensorMax;
}

void SoilSensor::setSensorMax(const int sensorMax) {
  this->sensorMax = sensorMax;
}

float SoilSensor::getMoisturePercentage() const {
  return moisturePercentage;
}
