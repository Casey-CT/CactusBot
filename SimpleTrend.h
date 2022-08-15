/*
 * Class for tracking a trend, across an array of float values.
 * Calculates variables for linear trend equation using the currently held values.
 */
#pragma once

class SimpleTrend {
  private:
    float slope; // Slope of trend line equation
    float yIntercept; // Y intercept of trend line equation

    /* Using a vector would be convenient, but the standard libraries aren't available for program size reasons.
     * There is an arduino version of parts of the standard library, found here: https://github.com/mike-matera/ArduinoSTL
     * There is another vector specific library, found here: https://github.com/janelia-arduino/Vector
     */
    float * values;
    unsigned int valuesLength = 0;
    unsigned int maxSize = 5; // Number of data points used to calculate the trend line

  protected:
    void recalculate();
  
  public:
    // Initialise values array
    SimpleTrend();
    SimpleTrend(const float initialValue);
    SimpleTrend(const float initialValue, const unsigned int maxSize);

    // Destroy values array
    ~SimpleTrend(); 
    
    // Adds a new datapoint, and recalculates equation
    void addDatapoint(const float dataPoint);

    // Using the calculated slope and y intercept, get y value for given x
    float predictValue(const float x) const;

    // Predicts a value for a given x, and returns true if it is within given parameters
    bool validatePredictedValue(const float x, const float min, const float max) const;

    // Getters and Setters
    float getSlope() const;
    float getYIntercept() const;

    const float* const getValues() const;
    unsigned int getValuesLength() const;
    
    unsigned int getMaxSize() const;
    void setMaxSize(const unsigned int maxSize);
};
