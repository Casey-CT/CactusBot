#include "SimpleTrend.h"

#include <Arduino.h>

#define FLOAT_SIZE 4 //Size of a float in bytes

SimpleTrend::SimpleTrend() {
  slope = 0.0;
  yIntercept = 0.0;
}

SimpleTrend::SimpleTrend(const float initialValue) {
  values = new float[1]{initialValue};
  valuesLength = 1;

  recalculate();
}

SimpleTrend::SimpleTrend(const float initialValue, const unsigned int maxSize): SimpleTrend(initialValue) {
  setMaxSize(maxSize);
}

SimpleTrend::~SimpleTrend() {
  delete[] values;
}

/*
 * Trend Line calculations: https://classroom.synonym.com/f-value-statistics-6039.html
 * 
 * y = mx + b, where m is the slope of the line, and b is the y intercept of the line.
 * For a dataset of length n:
 * m = (a - b)/(c - d)
 * b = (e - f)/n
 * 
 * Where:
 * a = n * ((x1 * y1) + (x2 * y2) + ...(xn * yn))
 * b = (x1 + x2 + ...xn) * (y1 + y2 + ...yn)
 * c = n * (x1^2 + x2^2 + ...xn^2)
 * d = (x1 + x2 + ...xn)^2
 * 
 * e = y1 + y2 + ...yn
 * f = m * (x1 + x2 + ...xn)
 */ 
void SimpleTrend::recalculate() {
  float a = 0.0;
  float c = 0.0;
  float xTotal = 0.0;
  float yTotal = 0.0;

  // Indexing the dataset from 1
  for (int i = 0; i < valuesLength; i++) {
    a += ((i + 1) * values[i]);
    c += sq(i + 1);
    xTotal += i + 1;
    yTotal += values[i];
  }

  a *= valuesLength;
  float b = xTotal * yTotal;
  c *= valuesLength;
  float d = sq(xTotal);

  // Update slope
  slope = (a - b)/(c - d);

  float f = slope * xTotal;

  // Update yIntercept
  yIntercept = (yTotal - f) / valuesLength;
}

void SimpleTrend::addDatapoint(const float dataPoint) {
  // Initialize array if there are no values yet
  if (valuesLength == 0) {
    values = new float[1]{dataPoint};
  }
  // If current length of values array is less than the maxSize, extend array.
  else if (valuesLength < maxSize) {
    float * temp = new float[valuesLength + 1];
    memcpy(temp, values, FLOAT_SIZE * valuesLength); // Using memcpy as std::copy isn't available

    temp[valuesLength] = dataPoint;
    valuesLength++;

    delete[] values;
    values = temp;
  }
  // Otherwise, shift elements to the left one place, discarding the original first element, then replace the last element with the new datapoint.
  else {
    for (int i = 1; i < valuesLength; i++) {
      values[i - 1] = values[i];
    }

    values[valuesLength - 1] = dataPoint;
  }

  // Recalculate the slope and y intercept with the value set
  recalculate();
}

float SimpleTrend::predictValue(const float x) const {
  return (slope * x) + yIntercept;
}

bool SimpleTrend::validatePredictedValue(const float x, const float min, const float max) const {
  float result = predictValue(x);
  return result >= min && result <= max;
}

float SimpleTrend::getSlope() const {
  return slope;
}

float SimpleTrend::getYIntercept() const {
  return yIntercept;
}

const float* const SimpleTrend::getValues() const {
  return values;
};

unsigned int SimpleTrend::getValuesLength() const {
  return valuesLength;
};

unsigned int SimpleTrend::getMaxSize() const {
  return maxSize;
}

// Set a new value for the maximum size of the values array.
// If the array is longer than the new max size, shorten the values array, cutting off the earliest data first.
void SimpleTrend::setMaxSize(const unsigned int maxSize) {
  if (valuesLength > maxSize) {
    float * temp;
    temp = new float[maxSize];

    memcpy(temp, values + (valuesLength - maxSize), FLOAT_SIZE * maxSize); // Using memcpy as std::copy isn't available

    delete[] values;
    values = temp;
    valuesLength = maxSize;
  }

  this->maxSize = maxSize;
}
