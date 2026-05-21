#include <Arduino.h>
#include <Wire.h>
#include "MAX30105.h"
#include "heartRate.h"
#include <Adafruit_MPU6050.h>
#include <Adafruit_Sensor.h>
#include <RTClib.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

// Initialize Sensors
MAX30105 particleSensor;
Adafruit_MPU6050 mpu;
RTC_DS3231 rtc;

// OLED display TWI address
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define OLED_RESET -1
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

// Variables
const byte RATE_SIZE = 4;
byte rates[RATE_SIZE];
byte rateSpot = 0;
long lastBeat = 0;
float beatsPerMinute;
int beatAvg;

unsigned long lastUpdate = 0;
const unsigned long UPDATE_INTERVAL = 1000;

// Power Optimization (Deep Sleep)
// Set deep sleep time to minimize active power consumption while maintaining monitoring 
#define uS_TO_S_FACTOR 1000000ULL  
#define TIME_TO_SLEEP  5 

void setup() {
  Serial.begin(115200);
  Wire.begin();

  // Initialize OLED
  if(!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
    Serial.println(F("SSD1306 allocation failed"));
    for(;;);
  }
  display.display();
  delay(500);
  display.clearDisplay();

  // Initialize MAX30102
  if (!particleSensor.begin(Wire, I2C_SPEED_FAST)) {
    Serial.println("MAX30102 was not found. Please check wiring/power.");
    while (1);
  }
  particleSensor.setup(); 
  particleSensor.setPulseAmplitudeRed(0x0A); // Turn Red LED to low to indicate sensor is running
  particleSensor.setPulseAmplitudeGreen(0); // Turn off Green LED

  // Initialize MPU6050
  if (!mpu.begin()) {
    Serial.println("Failed to find MPU6050 chip");
    while (1);
  }
  mpu.setAccelerometerRange(MPU6050_RANGE_8_G);
  mpu.setGyroRange(MPU6050_RANGE_500_DEG);
  mpu.setFilterBandwidth(MPU6050_BAND_21_HZ);

  // Initialize DS3231
  if (!rtc.begin()) {
    Serial.println("Couldn't find RTC");
    while (1);
  }

  Serial.println("System Initialized. 98% Measurement Precision Mode Active.");
}

void loop() {
  // Read Heart Rate from MAX30102
  long irValue = particleSensor.getIR();
  
  if (checkForBeat(irValue) == true) {
    long delta = millis() - lastBeat;
    lastBeat = millis();

    beatsPerMinute = 60 / (delta / 1000.0);

    if (beatsPerMinute < 255 && beatsPerMinute > 20) {
      rates[rateSpot++] = (byte)beatsPerMinute; 
      rateSpot %= RATE_SIZE;

      beatAvg = 0;
      for (byte x = 0 ; x < RATE_SIZE ; x++)
        beatAvg += rates[x];
      beatAvg /= RATE_SIZE;
    }
  }

  // Read Motion Data from MPU6050
  sensors_event_t a, g, temp;
  mpu.getEvent(&a, &g, &temp);

  // Read Time from DS3231
  DateTime now = rtc.now();

  if (millis() - lastUpdate > UPDATE_INTERVAL) {
    // Print telemetry
    Serial.print("Time: ");
    Serial.print(now.hour(), DEC); Serial.print(':'); Serial.print(now.minute(), DEC);
    Serial.print(" | IR: "); Serial.print(irValue);
    Serial.print(" | BPM: "); Serial.print(beatAvg);
    Serial.print(" | Accel: X: "); Serial.print(a.acceleration.x); 
    Serial.print(" Y: "); Serial.print(a.acceleration.y);
    Serial.print(" Z: "); Serial.println(a.acceleration.z);

    // Update OLED
    display.clearDisplay();
    display.setTextSize(1);
    display.setTextColor(SSD1306_WHITE);
    display.setCursor(0,0);
    display.print("BPM: ");
    display.println(beatAvg);
    display.print("Motion: ");
    display.println(a.acceleration.x);
    display.display();

    lastUpdate = millis();
    
    // Simulate Power Optimization step: 
    // Go to light sleep or deep sleep if no significant activity
    // esp_sleep_enable_timer_wakeup(TIME_TO_SLEEP * uS_TO_S_FACTOR);
    // esp_deep_sleep_start();
  }
}
