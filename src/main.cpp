/*
  Arduino Bike Power Meter

  created 5 Apr 2021
*/
#include "HX711.h" //This library can be obtained here http://librarymanager/All#Avia_HX711
#include <Arduino.h>
#include <Arduino_LSM9DS1.h>
#include <LiquidCrystal_I2C.h>
#include <MovingAverage.h>
#include <math.h>

#define calibration_factor 33710.0 // This value is obtained using the SparkFun_HX711_Calibration sketch
#define zero_factor -1050185 // This large value is obtained using the SparkFun_HX711_Calibration sketch

#define LOADCELL_DOUT_PIN 3
#define LOADCELL_SCK_PIN 2
#define GYRO_READINGS 10
#define TORQUE_READINGS 10
#define GRAVITY_CONSTANT 9.81
#define CRANK_LENGTH 0.1725

LiquidCrystal_I2C lcd(0x27,20,4); // set the LCD address to 0x3F for a 16 chars and 2 line display

MovingAverage<float> gyroAvg(GYRO_READINGS);
MovingAverage<float> torqueAvg(TORQUE_READINGS);
HX711 scale;
    
void setup() {
  lcd.init();
  lcd.clear();
  lcd.backlight(); // Make sure backlight is on

  lcd.setCursor(2, 0); // Set cursor to character 2 on line 0
  lcd.print("Initialising");

  if (!IMU.begin()) {
    lcd.clear();
    lcd.setCursor(2, 0);
    lcd.print("Failed to initialize IMU!");
    while (1);
  }

  scale.begin(LOADCELL_DOUT_PIN, LOADCELL_SCK_PIN);
  scale.set_scale(calibration_factor); // This value is obtained by using the SparkFun_HX711_Calibration sketch
  scale.set_offset(zero_factor); // Zero out the scale using a previously known zero_factor
}

unsigned long lastTransmitTime;
void loop() {
  if (IMU.gyroscopeAvailable()) {
    float x, y, z;
    IMU.readGyroscope(x, y, z);
    float gyroReading = gyroAvg.push(z).get();
    float scaleReading = scale.get_units();
    float torqueReading = torqueAvg.push(scaleReading * CRANK_LENGTH * GRAVITY_CONSTANT).get();
    float power = round(torqueReading * (gyroReading * 0.01745));
    if (millis() - lastTransmitTime > 1000) {
      lcd.clear();
      lcd.setCursor(0, 0);
      lcd.print("Cadence: " + String(int(abs(round(gyroReading / 6)))) + "RPM");
      lcd.setCursor(0, 1);
      lcd.print("Power: " + String(int(power) * -1) + " W");
      lastTransmitTime = millis();
    }
    delay(10);
  }
}
