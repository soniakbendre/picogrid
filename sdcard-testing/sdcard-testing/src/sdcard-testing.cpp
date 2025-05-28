/* 
 * Project myProject
 * Author: Your Name
 * Date: 
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

// Temperature conversion factor for a specific sensor, e.g., TMP36
  //converting from 10 mv per degree with 500 mV offset to degrees
const float tempConvFactor = (3.3 / 4095.0) * 100.0 - 0.5;

void setup() {
    Serial.begin(9600);
    delay(1000); // Give time for Serial to start


    // bring chipselect pin high
    pinMode(chipSelect, OUTPUT);
    digitalWrite(chipSelect, HIGH);

    // set temp sensor pin as input
    pinMode(tempSensorPin, INPUT);

    // Initialize the SD card
    if (!sd.begin(chipSelect, SD_SCK_MHZ(4))) {
        Serial.println("SD card initialization failed!");
        return;
    }


    Serial.println("SD card initialized.");


    // Open file for writing
    if (!myFile.open("temperature.csv", O_WRITE | O_CREAT | O_TRUNC)) {
        Serial.println("Error opening file");
        return;
    }

    myFile.println("Temperature (C)"); // Write header to the file

    // declare cloud stop function
    Particle.function("stop", stop);
}


void loop() {
    // write timestamp and temperature to the file
    time_t timestamp = Time.now();
    Time.format(timestamp, TIME_FORMAT_DEFAULT); // Sat Jan 10 08:22:04 2004 , same as Time.timeStr()

    Time.zone(-6.00);  // setup a time zone, which is part of the ISO8601 format
    formatted_time = Time.format(timestamp, TIME_FORMAT_ISO8601_FULL); 

    float temperature = analogRead(tempSensorPin) * tempConvFactor; // Convert analog reading to temperature
    myFile.print(formatted_time); // Write timestamp to the file
    myFile.print(",");
    myFile.println(temperature);
    myFile.flush(); // Ensure data is written to the file

    Serial.print("Timestamp: ");
    Serial.print(timestamp);
    Serial.print(", Temperature: ");
    Serial.println(temperature);
    
    // wait half a second before the next reading
    delay(500); 

    // If user types stop, close the file
    if (Serial.available()) {
        String command = Serial.readStringUntil('\n');
        if (command.equalsIgnoreCase("stop")) {
            Serial.println("Closing file...");
            myFile.close();
            return;
        }
    }
}
