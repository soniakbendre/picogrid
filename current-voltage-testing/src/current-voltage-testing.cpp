
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

// Include SD interfacing library
#include "SdFat.h"


// Let Device OS manage the connection to the Particle Cloud
SYSTEM_MODE(AUTOMATIC);

// Run the application and system concurrently in separate threads
SYSTEM_THREAD(ENABLED);

// Show system, cloud connectivity, and application logs over USB
// View logs with CLI using 'particle serial monitor --follow'
SerialLogHandler logHandler(LOG_LEVEL_INFO);

// Create an SD object ("sd" and "myFile" are programmer's choice names)
SdFat sd;
SdFile myFile;

// These pins correspond to the Particle Boron hardware pins being used.

// A5 is the chip select pin for the SD card
const int chipSelect = A5;

const int tempSensorPin = A1;
const int currentSensorPin = A0;
const int voltageSensorPin = A2;

// Define sensor constants
const float sensitivity = 2.66;       // mA/mV
const float battRefV = 1650.0;      // mV @ 0 current (half of 3300mV reference voltage)

// future improvements: send message when outside of voltage thresholds
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
      Serial.end(); // Close the Serial connection
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
  System.sleep(SLEEP_MODE_DEEP, 60); // Sleep for 60 seconds in deep sleep (disconnects from the cellular network)
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
  
  if (!myFile.open("eLFL_log.csv", O_WRITE | O_CREAT | O_APPEND)) {
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

  // Number of hours behind UTC
  Time.zone(-5); // Set your local time zone (e.g. CST = -5, (for now I have set)  adjust as needed)

  if (!Particle.connected()) {
    Particle.connect(); // Ensure connection is attempted
    waitUntil(Particle.connected); // Then wait for it
}

  // Initialize the SD card with the specified chip select pin and clock speed 4 MHz
  if (!sd.begin(chipSelect, SD_SCK_MHZ(4))) {
      Serial.println("SD card initialization failed!");
  }
  else {
      Serial.println("SD card initialized successfully.");
      
      // Open file for writing
      if (myFile.isOpen()) {
      myFile.close();
      }
      // Open the file in write mode, create it if it doesn't exist, and append to it
      if (!myFile.open("eLFL_log.csv", O_WRITE | O_CREAT | O_APPEND)) {
        Serial.println("Error opening file");
      }
      else
      {
        // if the file is successfully opened and empty, write the header
          if(myFile.fileSize() == 0)
            myFile.println("Time,Current(mA),Temp(C),Voltage(V)"); // Write header to the file
      } 
  }

  Serial.println("Time,Current(mA),Temp(C),Voltage(V)");

  // declare cloud stop function
  Particle.function("stop", stop);

  // declare cloud sleep function
  Particle.function("sleep", sleepp);

  // declare cloud wakeup function
  Particle.function("wakeup", wakeup);
}


void loop() {   
  unsigned long startTime = millis();

  while (millis() - startTime < 60000) { // Stay awake for 60 seconds
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
  if(myFile.isOpen())
  {
    myFile.println(dataLine);
  }
  Serial.println(dataLine); // Send to OpenLog
  Particle.publish("eLFL_log", dataLine, PRIVATE); // publish dataLine to Particle Cloud
  delay(logTime - (delayTime * avgSamples));
  }

  if(myFile.isOpen())
  {
    myFile.flush();
    myFile.close(); // Close the file after writing
  }
  Serial.println("Sleeping for 5 minutes...");
  delay(200); // Let serial finish printing

  // Put the device to sleep for 5 minutes (300s in line 228) using System.sleep() which is a Particle Device OS API
  SystemSleepConfiguration config;
  config.mode(SystemSleepMode::STOP)
      .duration(300s)
      .network(NETWORK_INTERFACE_CELLULAR);

  System.sleep(config);

  // reinitialize SD card
  if (!sd.begin(chipSelect, SD_SCK_MHZ(4))) {
        Serial.println("SD card initialization failed!");
    }
  else{
      Serial.println("SD card initialized successfully.");
  if (!myFile.open("eLFL_log.csv", O_WRITE | O_CREAT | O_APPEND)) {
    Serial.println("Error opening file");
  }
}

}


