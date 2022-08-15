/*
 * Class for taking readings from a soil moisture sensor, and returning a percentage moisture level of the soil.
 */
#pragma once

#include "SensorTimer.h"

class SoilSensor: public SensorTimer<int> {
  private:
    int soilOn; // Digital arduino pin for activating sensor
    int soilIn; // Analog arduino pin for reading sensor data in
  
    // Sensor readings at maximum and minimum moisture levels
    int sensorMin;
    int sensorMax;

  protected:
    // Soil moisture level, based on last reading
    float moisturePercentage;
    float calculateMoisturePercentage();
    
  public:
    SoilSensor(const int soilOn, const int soilIn, const int sensorMin, const int sensorMax, const unsigned long interval);

    void init() override;
    void takeReading() override;

    // Getters and Setters
    int getSensorMin() const;
    void setSensorMin(const int sensorMin);
    int getSensorMax() const;
    void setSensorMax(const int sensorMax);
    
    float getMinPercent() const;
    void setMinPercent(const float minPercent);
    float getMaxPercent() const;
    void setMaxPercent(const float maxPercent);

    float getMoisturePercentage() const;
};
