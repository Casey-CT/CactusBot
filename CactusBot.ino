#include "WaterLevelSensor.h"
#include "SoilSensor.h"
#include "TempSensor.h"
#include "LightSensor.h"

// TODO: Number returned by millis() will overflow back to 0 after ~50 days.
// https://www.arduino.cc/reference/en/language/functions/time/millis/
// Implement a reset for that point

// TODO: Implement a safety check, to end a current watering if it has been going for too long

/*
 * Common constants
 */
const unsigned long SENSOR_INTERVAL = 1800000; // Check interval for each sensor, in milliseconds
const unsigned int TREND_LOOKAHEAD = 5; // Number of sensor checking intervals to predict ahead, when checking that trended data will be within acceptable parameters


/*
 * WATER LEVEL SENSOR
 * 
 * Used to track the contents of a water container, used to dispense water to the plant.
 * 
 * The part of the sensor that takes readings is 42mm long.
 * When calibrating my sensor, I got the following values from analogRead():
 * 0% submerged   - 0
 * 25% submerged  - 110
 * 50% submerged  - 135
 * 75% submerged  - 145
 * 100% submerged - 150
 * 
 * This equates APPROXIMATELY to f(x) = 75 * log10(x)
 */
const int WATER_ON = 2; // Digital pin, used to power on the sensor
const int WATER_IN = A0; // Analog pin, used to take a reading from the sensor
const float SCALE_FACTOR = 75; // Factor by which to scale the logarithmic equation representing the sensor output
const unsigned int FILL_HEIGHT = 42; // Length of the 'sensor' part of the sensor, in mm
const unsigned int READ_COUNT = 25; // Number of times sensor is read, then averaged out to take a reading
const float MIN_FILL_PERCENT = 10.0; // Container fill percentage, below which the container is considered 'empty'
const float CONTAINER_RADIUS = 5.0; // Radius of container, in mm

WaterLevelSensor waterSensor (WATER_ON, WATER_IN, SCALE_FACTOR, FILL_HEIGHT, READ_COUNT, MIN_FILL_PERCENT, CONTAINER_RADIUS, SENSOR_INTERVAL);


/*
 * SOIL MOISTURE SENSOR 
 * 
 * Used to take readings of the moisture level of the soil, one of the factors used to trigger a watering.
 * If the moisture level exceeds the maximum, then any watering in progress will be stopped.
 * 
 * My sensor returned a value of 0 when completely dry, and a value of ~640 when completely submerged in water.
 * This sensor will require testing in actual soil of various moisture levels.
 */
const int SOIL_ON = 4; // Digital arduino pin for activating sensor
const int SOIL_IN = A1; // Analog arduino pin for reading sensor data in
const int SOIL_READING_MIN = 0; // Sensor reading at minimum level
const int SOIL_READING_MAX = 640; // Sensor reading at maximum level
const float SOIL_MOISTURE_MIN = 15.0; // Soil moisture percentage level, below which to trigger a watering
const float SOIL_MOISTURE_MAX = 35.0; // Soil moisture percentage level, above which to stop watering

SoilSensor soilSensor (SOIL_ON, SOIL_IN, SOIL_READING_MIN, SOIL_READING_MAX, SENSOR_INTERVAL);


/*
 * TEMPERATURE SENSOR
 * 
 * Readings will be used to predict the temperature in TREND_LOOKAHEAD intervals.
 * The latest reading, and the predicted temperature being above the minimum level will be used as a factor to trigger a watering.
 * 
 * Using a DHT11 temperature and humidity sensor.
 * Wiring diagram: https://www.circuitbasics.com/how-to-set-up-the-dht11-humidity-sensor-on-an-arduino/
 */
const int TEMP_PIN = 7; // Digital arduino pin used by sensor
const float MIN_TEMP = 12.0; // Minimum temperature, in celsius, to allow a watering to occur

TempSensor tempSensor (TEMP_PIN, SENSOR_INTERVAL);


/*
 * LIGHT LEVEL SENSOR
 * 
 * Readings will be used to predict the light level in TREND_LOOKAHEAD intervals.
 * The latest reading, and the predicted light level being above the minimum level will be used as a factor to trigger a watering.
 * 
 * Using an XC4446 light sensor. The analog signal out is not linear, but this sensor is being used simply to detect whether enough light will be present,
 * so this isn't being taken into account.
 * 
 * My sensor return values of around ~500 in a well-lit room, and ~70 in a mostly dark room (with computer screen illumination)
 */
const int LIGHT_IN = A2; // Analog arduino pin used to retrieve signal
const int MIN_LIGHT = 350; // Returned reading considered well-lit enough to trigger a watering

LightSensor lightSensor (LIGHT_IN, SENSOR_INTERVAL);


/*
 * SOLENOID VALVE/WATERING
 * 
 * Connected to the water container, used to pipe water into the plant.
 * Connected via a relay, activated by a digital pin.
 * The valve will open when the pin is set to HIGH.
 * 
 * When a watering is triggered, a set amount of water will be dispensed onto the plant.
 * To allow the water to soak into the soil (and reach the soil sensor), minimum time between waterings will be set.
 * The level of water in the container, and level of moisture in the soil will be checked constantly while watering.
 * If either the set amount of water is dispensed, or the soil moisture sensor reaches the maximum moisture level, the watering will be stopped.
 */
const int SOL_PIN = 8;
const unsigned long WATERING_INTERVAL = 3600000; // Minimum time, in milliseconds, between waterings. Water at most, once an hour
const float WATERING_AMOUNT = 10.0; // Water amount, in ml, to dispense to the plant on a single watering.

bool isWatering = false; // Currently watering?
unsigned long prevWatering = 0; // Time, in milliseconds, that last watering took place.
float remainingWatering = 0.0; // Remaining water to dispense to plant on the current watering.
float fillAmount = 0.0; // Previous fill of water container in ml. Will be compared to latest water level to calculate dispensed water.

/*
 * RBG INDICATOR LED
 * 
 * RGB LED used to indicate the watering state.
 * Will blink blue while watering, and blink red if the water container is empty.
 * Uses 3 Digital PWM pins as output.
 */
const int R_PIN = 9;
const int G_PIN = 10;
const int B_PIN = 11;
const unsigned long BLINK_INTERVAL = 1000; // Time, in milliseconds between LED to blinks
unsigned long blinkTime = 0; // Time, in milliseconds of previous blink
bool ledOn = false;


/*
 * STATUS Update
 * 
 * Send latest readings and watering state over the serial port once per minute.
 */
const unsigned long STATUS_INTERVAL = 60000;
unsigned long statusTime = 0;

/*
 * Prints the following values to the serial port, each seperated by a '|' character. Floats will be printed to two decimal places.
 * Soil moisture percentage, temperature, light level, water fill amount, water fill percentage,
 * 'W' or 'NW' if currently watering or not watering, prevWatering and remainingWatering.
 */
void statusUpdate() {
  Serial.println(String(soilSensor.getMoisturePercentage(), 2) + '|' +
                 String(tempSensor.getLatestReading(), 2) + '|' +
                 String(lightSensor.getLatestReading()) + '|' +
                 String(waterSensor.getFillAmount(), 2) + '|' +
                 String(waterSensor.getFillPercent(), 2) + '|' +
                 (isWatering ? "W" : "NW") + '|' +
                 String(prevWatering) + '|' +
                 String(remainingWatering, 2));
}

void setup() {
  // Initialise sensors
  waterSensor.init();
  soilSensor.init();
  tempSensor.init();
  lightSensor.init();

  // Setup solenoid valve relay pin, and set to LOW
  pinMode(SOL_PIN, OUTPUT);
  digitalWrite(SOL_PIN, LOW);

  // Make sure RBG LED is turned off
  analogWrite(R_PIN, 0);
  analogWrite(G_PIN, 0);
  analogWrite(B_PIN, 0);

  // Open Serial Port, and send initial status update
  Serial.begin(9600);
  statusUpdate();
}

void loop() {
  // ----- Update each of the sensors -----
  waterSensor.tick();
  soilSensor.tick();
  tempSensor.tick();
  lightSensor.tick();


  // ----- Watering -----
  if (isWatering) {
    // --- Update Watering in Progress ---
    // Force soil moisture and water level to update
    waterSensor.tick(true);
    soilSensor.tick(true);

    float currentFillAmount = waterSensor.getFillAmount();

    // If previous fillAmount is greater than current fill amount, remove the difference from remainingWater
    if ((!waterSensor.isEmpty()) && fillAmount > currentFillAmount) {
      remainingWatering -= (fillAmount - currentFillAmount);
    }
    
    // Open the solenoid valve if the water container is not empty
    digitalWrite(SOL_PIN, waterSensor.isEmpty() ? LOW : HIGH);

    // Update the tracked fillAmount
    fillAmount = currentFillAmount;

    // --- Finish Watering ---
    // Watering has finished when there is no water left to dispense, or the soil sensor is reading the maximum desired moisture level
    if (remainingWatering <= 0.0 || soilSensor.getMoisturePercentage() >= SOIL_MOISTURE_MAX) {
      // Set the prev watering flag to current millis
      // Reset each of the watering variables
      // Close the solenoid valve
      prevWatering = millis();
      remainingWatering = 0.0;
      fillAmount = 0.0;
      isWatering = false;
      digitalWrite(SOL_PIN, LOW);
    }
    
  } 
  // --- Trigger Watering ---
  // Trigger a watering if:
  //  - Latest soil sensor value is below minimum percentage
  //  - Temperature, and forward projected temperature are above minimum
  //  - Light level, and forward projected light level are above minimum
  //  - Watering interval has passed since last watering
  else if (soilSensor.getMoisturePercentage() <= SOIL_MOISTURE_MIN &&
           tempSensor.getLatestReading() >= MIN_TEMP && tempSensor.predictValue(tempSensor.getValuesLength() + TREND_LOOKAHEAD) >= MIN_TEMP &&
           lightSensor.getLatestReading() >= MIN_LIGHT && lightSensor.predictValue(lightSensor.getValuesLength() + TREND_LOOKAHEAD) >= MIN_LIGHT &&
           millis() - prevWatering >= WATERING_INTERVAL) {
    // Force water level sensor update, and set fillAmount using current value
    // Set isWatering flag to true, prevWatering to current millis and remainingWatering to total watering amount
    // Open the solenoid valve
    waterSensor.tick(true);
    fillAmount = waterSensor.getFillAmount();
    isWatering = true;
    prevWatering = millis();
    remainingWatering = WATERING_AMOUNT;
    digitalWrite(SOL_PIN, HIGH);
  }


  // ----- Indicator LED -----
  unsigned long currentMillis = millis();
  int r = 0;
  int g = 0;
  int b = 0;

  // Blink the LED red if the water container is empty
  if (waterSensor.isEmpty()) {
    r = 255;
  }
  // Blink the LED blue if currently watering
  else if (isWatering) {
    b = 255;
  }
  // Otherwise, turn the LED off
  else {
    blinkTime = millis();
    ledOn = false;
  }

  if (currentMillis - blinkTime >= BLINK_INTERVAL) {
    blinkTime = currentMillis;
    ledOn = !ledOn;
  }

  analogWrite(R_PIN, ledOn ? r : 0);
  analogWrite(G_PIN, ledOn ? g : 0);
  analogWrite(B_PIN, ledOn ? b : 0);

  // ----- Status Update -----
  currentMillis = millis();
  if (currentMillis - statusTime > STATUS_INTERVAL) {
    statusTime = currentMillis;
    statusUpdate();
  }

  delay(100); // Run ~10 times per second
}
