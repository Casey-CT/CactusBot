/*
 * Class for taking readings from a photoresistor light sensor.
 */
#pragma once

#include "SensorTimer.h"
#include "SimpleTrend.h"

class LightSensor: public SensorTimer<int>, public SimpleTrend {
  private:
    int lightIn; // Analog input pin
    
  public:
    LightSensor(const int lightIn, const unsigned long interval);

    void init() override;
    void takeReading() override;
};
