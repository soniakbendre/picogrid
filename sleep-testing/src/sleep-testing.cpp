/* 
 * Project eLFL
 * Author: Sonia Bendre, Manushri Muthukumaran, Sukriti Somvanshi
 * Adapted from code written by: Maitreyee Marathe
 * Date: 5/29/2025
 * For comprehensive documentation and examples, please visit:
 * https://docs.particle.io/firmware/best-practices/firmware-template/
 */

// Include Particle Device OS APIs
#include "Particle.h"
#include "SdFat.h"


// Let Device OS manage the connection to the Particle Cloud
SYSTEM_MODE(AUTOMATIC);

// Run the application and system concurrently in separate threads
SYSTEM_THREAD(ENABLED);

// Show system, cloud connectivity, and application logs over USB
// View logs with CLI using 'particle serial monitor --follow'
SerialLogHandler logHandler(LOG_LEVEL_INFO);

// Create an SD object
SdFat sd;
SdFile myFile;

// Chip select pin
const int chipSelect = A5;
const int tempSensorPin = A1;
const int currentSensorPin = A0;
const int voltageSensorPin = A2;

// Define sensor constants
const float sensitivity = 2.66;       // mA/mV
const float battRefV = 1650.0;      // mV @ 0 current
const float lowV = 11.5;            // Low voltage threshold (V)
const float highV = 12.0;           // High voltage threshold (V)
const int avgSamples = 10;           // Number of sensor readings to average
const int delayTime = 2;             // ms between samples
const int logTime = 5000;            // ms between logs

// Define runtime variables
int currentValue = 0;
int temperatureValue = 0;
int voltageValue = 0;


// define stop function
int stop(String command) {
  if(command.equalsIgnoreCase("stop"))
  {
      Serial.println("Closing file...");
      myFile.close();
      Serial.println("File closed. Device will stop logging.");
      // Optionally, you can also stop the device from running further
      // by entering a deep sleep or resetting the device.
      Particle.disconnect();
      return 0; // Return 0 to indicate success
  }
  return -1;
}

// define sleep function
int sleepp(String command) {
  if(command.equalsIgnoreCase("sleep"))
  {
      Serial.println("Device going to sleep...");
  // Close the file if it's open
  if (myFile.isOpen()) {
      Serial.println("Device sleeping.");
      myFile.close();
      return 0;
  }
  
  // Put the device to sleep
  System.sleep(SLEEP_MODE_DEEP, 60); // Sleep for 60 seconds
}
  return -1;
}

//define wakeup function
int wakeup(String command) {
  if(command.equalsIgnoreCase("wakeup"))
  {
  // Reinitialize the SD card and file
  if (!sd.begin(chipSelect, SD_SCK_MHZ(4))) {
      Serial.println("SD card initialization failed!");
      return 0;
  }

  if (!myFile.open("eLFL_log.csv", O_WRITE | O_APPEND)) {
      Serial.println("Error opening file");
      return 0;
  }

  Serial.println("Device woke up and file is ready for writing.");
}
return -1;
}


void setup() {
  Serial.begin(); // Start serial monitor
  delay(1000); // Give time for Serial to start

  // bring chipselect pin high
  pinMode(chipSelect, OUTPUT);
  digitalWrite(chipSelect, HIGH);

  // set temp sensor pin as input
  pinMode(tempSensorPin, INPUT);

  // set current sensor pin as input
  pinMode(currentSensorPin, INPUT);

  // set voltage sensor pin as input
  pinMode(voltageSensorPin, INPUT);

  Time.zone(-5); // Set your local time zone (e.g. CST = -5, (for now I have set)  adjust as needed)
  waitUntil(Particle.connected); // Ensure time is synced

  // Initialize the SD card
  if (!sd.begin(chipSelect, SD_SCK_MHZ(4))) {
      Serial.println("SD card initialization failed!");
      return;
  }

  Serial.println("SD card initialized.");

  // Open file for writing
  if (!myFile.open("eLFL_log.csv", O_WRITE | O_CREAT | O_APPEND)) {
      Serial.println("Error opening file");
      return;
  }

  // Write header to the file if it's empty
  if (myFile.fileSize() == 0) {
      myFile.println("Time,Current(mA),Temp,Voltage(V)");
  }
  // declare cloud stop function
  Particle.function("stop", stop);

  // declare cloud sleep function
  Particle.function("sleep", sleepp);

  // declare cloud wakeup function
  Particle.function("wakeup", wakeup);
}


void loop() {   
  currentValue = 0;
  temperatureValue = 0;
  voltageValue = 0;
 
  for (int i = 0; i < avgSamples; i++) {
    currentValue += analogRead(currentSensorPin);
    temperatureValue += analogRead(tempSensorPin);
    voltageValue += analogRead(voltageSensorPin);
    delay(delayTime);
  }
 
  currentValue /= avgSamples;
  temperatureValue /= avgSamples;
  voltageValue /= avgSamples;
  
  // Physical conversions
  // Battery current
  float battmV = currentValue * 0.805;  // 3.3V / 4095 * 1000
  float current = (battmV - battRefV) * sensitivity;
  
  // Battery voltage (via voltage divider)
  float adcVoltage = voltageValue * 3.3 / 4095.0; // ADC voltage
  float voltage = adcVoltage * 5.5;               // Apply gain from voltage divider

  float temperature = ((((temperatureValue * 3.3) / 4095.0) * 1000) - 500.0)/10; // Convert analog reading to temperature

  String timestamp = Time.format(Time.now(), "%Y-%m-%d %H:%M:%S");
 
  String dataLine = timestamp + "," + String(current, 2) + 
                    "," + String(temperature, 2) + 
                    "," + String(voltage, 2);

  myFile.println(dataLine);
  Serial.println(dataLine); // Send to OpenLog
 
  delay(logTime - (delayTime * avgSamples));

  // sleep for 1 minute, then wake up
  // System.sleep(SLEEP_MODE_DEEP, 60); // Sleep for 60 seconds
}