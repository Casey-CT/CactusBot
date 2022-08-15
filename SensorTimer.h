/*
 * Base class for reading a sensor at a given interval, for reading a sensor at a set interval.
 * 
 * Given that any individual sensor can be read in different ways, (digital vs analog, etc),
 * the implementation for this base class will only handle the timer.
 */
#pragma once

#include <Arduino.h>

template <class T>
class SensorTimer {
  private:
    unsigned long prevReadingMillis; // Time, in millis since arduino was started, that the last reading was taken
    unsigned long interval; // Interval time in millis, after which to perform the next reading
  protected:
    T latestReading;
    virtual void takeReading();
  public:
    SensorTimer(const unsigned long interval);

    // Sets up arduino pins, etc, and perform the initial reading.
    virtual void init();

    // Increment timer, updating reading and returning true if increment has passed, or force is set to true.
    bool tick(const bool force = false);
    bool tick(T& reading, const bool force); // Latest reading as output parameter

    const T getLatestReading() const;

    unsigned long getPrevReadingMillis() const;
    
    unsigned long getInterval() const;
    void setInterval(const unsigned long interval);
};

#include <Arduino.h>

template <class T>
SensorTimer<T>::SensorTimer(const unsigned long interval) {
  prevReadingMillis = 0;
  latestReading = T();

  this->interval = interval;
}

template <class T>
void SensorTimer<T>::init() {}

template <class T>
bool SensorTimer<T>::tick(const bool force) {
  unsigned long currentMillis = millis();
  bool shouldUpdate = (currentMillis - prevReadingMillis >= interval) || force;
  
  if (shouldUpdate) {
    prevReadingMillis = currentMillis;
    takeReading();
  }

  return shouldUpdate;
}

template <class T>
bool SensorTimer<T>::tick(T& reading, const bool force) {
  bool result = tick(force);
  reading = latestReading;
  return result;
}

template <class T>
void SensorTimer<T>::takeReading() {}

template <class T>
const T SensorTimer<T>::getLatestReading() const {
  return latestReading;
}

template <class T>
unsigned long SensorTimer<T>::getPrevReadingMillis() const {
  return prevReadingMillis;
}

template <class T>
unsigned long SensorTimer<T>::getInterval() const {
  return interval;
}

template <class T>
void SensorTimer<T>::setInterval(const unsigned long interval) {
  this->interval = interval;
}
