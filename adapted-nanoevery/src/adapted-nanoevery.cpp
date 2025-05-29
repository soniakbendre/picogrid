// electric Little Free Library - Particle Boron Version
// Author: Adapted by Sukriti 
// Platform: Particle Boron
// Logs battery + temperature data via Serial1 to OpenLog or SD system
 
#include "Particle.h"
 
// -------- Pin Definitions --------
const int temperature_pin = A0;
const int iLoad_pin = A1;     // Not used right now
const int iBatt_pin = A2;
const int vBatt_pin = A3;
const int door_pin = A4;      // Replace with D2/D3 if needed
const int red_pin = D2;
const int green_pin = D3;
 
// -------- Sensor Constants --------
const float sensitivity = 2.5;       // mA/mV
const float Vref_Batt = 2480.0;      // mV @ 0 current
const float v_low = 11.5;            // Low voltage threshold (V)
const float v_high = 12.0;           // High voltage threshold (V)
const int avgSamples = 10;           // Number of sensor readings to average
const int delayTime = 2;             // ms between samples
const int logTime = 5000;            // ms between logs
 
// -------- Runtime Variables --------
int iBatt_value = 0;
int temperature_value = 0;
int vBatt_value = 0;
 
void setup() {
  Serial.begin(9600);     // USB serial
  Serial1.begin(9600);    // TX/RX for OpenLog
 
  pinMode(temperature_pin, INPUT);
  pinMode(iLoad_pin, INPUT);
  pinMode(iBatt_pin, INPUT);
  pinMode(vBatt_pin, INPUT);
  pinMode(door_pin, INPUT);
  pinMode(red_pin, OUTPUT);
  pinMode(green_pin, OUTPUT);
 
  Time.zone(-5); // Set your local time zone (e.g. CST = -5, (for now I have set)  adjust as needed)
  waitUntil(Particle.connected); // Ensure time is synced
}
 
void loop() {
  // ---------- Sensor Averaging ----------
  iBatt_value = 0;
  temperature_value = 0;
  vBatt_value = 0;
 
  for (int i = 0; i < avgSamples; i++) {
    iBatt_value += analogRead(iBatt_pin);
    temperature_value += analogRead(temperature_pin);
    vBatt_value += analogRead(vBatt_pin);
    delay(delayTime);
  }
 
  iBatt_value /= avgSamples;
  temperature_value /= avgSamples;
  vBatt_value /= avgSamples;
 
  // ---------- Convert to Physical Units ----------
 
  // Battery current
  float batt_voltage_mV = iBatt_value * 0.805;  // 3.3V / 4095 * 1000
  float iBatt = (batt_voltage_mV - Vref_Batt) * sensitivity;
 
  // Temperature (LM35 style sensor)
  float temp_voltage = temperature_value * 3.3 / 4095.0; // in volts
  float temperature = (temp_voltage - 0.5) * 100.0;       // in °C
 
  // Battery voltage (via voltage divider)
  float adc_voltage = vBatt_value * 3.3 / 4095.0; // ADC voltage
  float vBatt = adc_voltage * 5.5;               // Apply gain from voltage divider
 
  // ---------- LED Indicator Logic ----------
  if (vBatt >= v_high) {
    digitalWrite(green_pin, HIGH);
    digitalWrite(red_pin, LOW);
  } else if (vBatt <= v_low) {
    digitalWrite(green_pin, LOW);
    digitalWrite(red_pin, HIGH);
  }
 
  // ---------- Time Logging ----------
  String timestamp = Time.format(Time.now(), "%Y-%m-%d %H:%M:%S");
 
  String dataLine = timestamp + ", " +
                    "Battery Current (mA): " + String(iBatt, 2) + ", " +
                    "Temperature (C): " + String(temperature, 2) + ", " +
                    "Battery Voltage (V): " + String(vBatt, 2);
 
  Serial.println(dataLine);
  Serial1.println(dataLine); // Send to OpenLog
 
  delay(logTime - (delayTime * avgSamples));
}