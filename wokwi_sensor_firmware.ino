/*
   wokwi_sensor_firmware.ino

   ESP32 + DHT22 environmental sensor simulation, built for Wokwi.
   Outputs one CSV line per reading over the serial monitor:

       reading_id,time_min,temperature_c,humidity_pct

   -----------------------------------------------------------------
   A note on how this file was built:

   Unlike data_pipeline.py, which was verified field-by-field against
   your actual published 402-row dataset, there is no surviving copy
   of the original firmware or its raw serial output to check this
   against, only the report's description of what it did: realistic,
   time-varying temperature and humidity readings, drifting from
   about 21 C to 29 C and 47% to 33% humidity over roughly 400
   readings, with reading-to-reading noise on top of that trend.

   This is a from-scratch reconstruction matching that description: a
   slow drift toward a moving target value, plus small random noise
   each reading, which is a standard way to make a Wokwi virtual DHT22
   (which otherwise just reports a fixed value) produce a believable
   changing environment for a demo. Treat this file as a faithful
   *behavioral* match, not a recovered original.
   -----------------------------------------------------------------
*/

#include <DHT.h>

#define DHTPIN 4
#define DHTTYPE DHT22

DHT dht(DHTPIN, DHTTYPE);

// Drift model: temperature trends upward, humidity trends downward,
// matching the environmental degradation described in the report.
float currentTemp = 21.0;
float currentHumidity = 47.0;
const float TEMP_TARGET = 29.0;
const float HUMIDITY_TARGET = 33.0;
const int TOTAL_READINGS = 402;

int readingId = 0;
unsigned long startTime = 0;

float randomNoise(float magnitude) {
  // Small symmetric noise, +/- magnitude, one decimal place.
  long steps = (long)(magnitude * 20);
  long r = random(-steps, steps + 1);
  return r / 10.0;
}

void setup() {
  Serial.begin(115200);
  dht.begin();
  randomSeed(analogRead(0));
  startTime = millis();

  // CSV header, matches what data_pipeline.py expects as input.
  Serial.println("reading_id,time_min,temperature_c,humidity_pct");
}

void loop() {
  if (readingId >= TOTAL_READINGS) {
    return;  // monitoring window complete
  }

  // Step the drift target slowly across the whole session so the trend
  // is smooth over ~400 readings, not a straight line reading to reading.
  float progress = (float)readingId / TOTAL_READINGS;
  float driftTemp = 21.0 + (TEMP_TARGET - 21.0) * progress;
  float driftHumidity = 47.0 + (HUMIDITY_TARGET - 47.0) * progress;

  // Blend the slow drift target with the sensor's actual reading (in
  // Wokwi, the DHT22's own value, adjustable on its virtual slider) and
  // add small reading-to-reading noise on top.
  float sensorTemp = dht.readTemperature();
  float sensorHumidity = dht.readHumidity();
  if (isnan(sensorTemp)) sensorTemp = driftTemp;
  if (isnan(sensorHumidity)) sensorHumidity = driftHumidity;

  currentTemp = (driftTemp * 0.8) + (sensorTemp * 0.2) + randomNoise(0.9);
  currentHumidity = (driftHumidity * 0.8) + (sensorHumidity * 0.2) + randomNoise(1.2);

  float timeMinutes = (millis() - startTime) / 60000.0;

  Serial.print(readingId);
  Serial.print(",");
  Serial.print(timeMinutes, 2);
  Serial.print(",");
  Serial.print(currentTemp, 1);
  Serial.print(",");
  Serial.println(currentHumidity, 1);

  readingId++;
  delay(1000);  // one reading per second in simulated time
}
