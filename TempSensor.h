/*
 * Class for taking readings from a DHT11 temperature sensor.
 * Uses the Adafruit DHT11 library: https://github.com/adafruit/DHT-sensor-library
 */
#pragma once

#include "SensorTimer.h"
#include "SimpleTrend.h"

#include <DHT.h>

class TempSensor: public SensorTimer<float>, public SimpleTrend {
  private:
    int tempPin; // Digital pin

    DHT* dht;

  public:
    TempSensor(const int tempPin, const unsigned long interval);
    ~TempSensor(); // delete dht object

    void init() override;
    void takeReading() override;
};
