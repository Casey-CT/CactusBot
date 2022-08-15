#include "TempSensor.h"

#include "SensorTimer.h"
#include "SimpleTrend.h"

#include <Arduino.h>
#include <DHT.h>

TempSensor::TempSensor(const int tempPin, const unsigned long interval): SensorTimer(interval), SimpleTrend() {
  this->tempPin = tempPin;
  dht = new DHT(this->tempPin, DHT11);
}

TempSensor::~TempSensor() {
  delete dht;
}

void TempSensor::init() {
  dht->begin();
  takeReading();
}

void TempSensor::takeReading() {
  float reading = dht->readTemperature(false, false); // Reading temperature in celsius
  latestReading = reading;
  addDatapoint(reading);
}
