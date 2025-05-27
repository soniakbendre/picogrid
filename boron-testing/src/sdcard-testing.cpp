#include "Particle.h"
#include "SdFat.h"

SYSTEM_THREAD(ENABLED); // Explicitly enable system threading

// Create an SD object
SdFat sd;
SdFile myFile;

// Chip select pin
const int chipSelect = D5;

void setup() {
    Serial.begin(9600);
    delay(1000); // Give time for Serial to start

    // bring chipselect pin high

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
