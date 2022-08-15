/*
 * Class for checking the water level, and detecting the amount of liquid remaining in the container
 * Sensor readings are on a logarithmic scale as the water level increases.
 */
#pragma once

#include "SensorTimer.h"

class WaterLevelSensor: public SensorTimer<int> {
  private:
    int waterOn; // Digital pin, used to power on the sensor
    int waterIn; // Analog pin, used to take a reading from the sensor

    float scaleFactor; // Factor by which to scale the logarithmic equation representing the sensor output

    unsigned int fillHeight; // Length of the 'sensor' part of the sensor, in mm
    unsigned int readCount; // Number of times sensor is read, then averaged out to take a reading

    float minPercent; // Container fill percentage, below which the container is considered 'empty'

    float containerRadius; // Radius of container, in mm

  public:
    WaterLevelSensor(const int waterOn, const int waterIn, const float scaleFactor, const unsigned int fillHeight, const unsigned int readCount, const float minPercent, const float containerRadius, const unsigned long interval);

    void init() override;
    void takeReading() override;

    float getFillPercent() const; // Get the current fill of the container, in percentage
    float getFillTotal() const; // Get total fill of container, in ml
    float getFillAmount() const; // Get current fill of container, in ml

    bool isEmpty() const; // Returns true if container fill is below the minimum percentage
};
