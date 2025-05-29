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
    String formatted_time = Time.format(timestamp, TIME_FORMAT_DEFAULT); 

    float temperature = ((((analogRead(tempSensorPin) * 3.3) / 4095.0) * 1000) - 500.0)/10; // Convert analog reading to temperature
    myFile.print(formatted_time); // Write timestamp to the file
    myFile.print(",");
    myFile.println(temperature);
    myFile.flush(); // Ensure data is written to the file

    Serial.print("Timestamp: ");
    Serial.print(formatted_time);
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

