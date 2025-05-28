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


void setup() {
    Serial.begin(9600);
    delay(1000); // Give time for Serial to start


    // bring chipselect pin high
    pinMode(chipSelect, OUTPUT);
    digitalWrite(chipSelect, HIGH);


    if (!sd.begin(chipSelect, SD_SCK_MHZ(4))) {
        Serial.println("SD card initialization failed!");
        return;
    }


    Serial.println("SD card initialized.");


    // Open file for writing
    if (!myFile.open("hello.csv", O_WRITE | O_CREAT | O_TRUNC)) {
        Serial.println("Error opening file");
        return;
    }


    myFile.println("Hello World");


    myFile.close();
    Serial.println("Wrote 'Hello World' to hello.csv");
}


void loop() {
    // Nothing here
}
