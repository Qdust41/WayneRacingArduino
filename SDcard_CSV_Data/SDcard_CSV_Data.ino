#include <SD.h>
#include <SPI.h>

const int chipSelect = 10; // This is the MISO pin on the SD card thing and which pin it connects to on arduino

// Sets up dbFile as our 'global' variable for our SD card file
File dbFile;

void setup() {
  Serial.begin(115200);
  if (!SD.begin(chipSelect)) {
    Serial.println("SD card initialization failed!");
    return;
  }
  Serial.println("SD card initialized.");

  // Open or create the file
  dbFile = SD.open("data.csv", FILE_WRITE);
  if (dbFile) {
    // Write a header if needed
    dbFile.println("timestamp,sensor1,sensor2");
    // Write a row of data
    dbFile.print(millis());
    dbFile.print(",");
    dbFile.print(analogRead(A0));
    dbFile.print(",");
    dbFile.println(analogRead(A1));

    Serial.println("Data written to SD card.");
  } else {
    Serial.println("Failed to open file for writing.");
  }
}

void loop() {
  // Yes i made a loop in a loop that runs once; no i dont care.
  for (int i = 0; i <= 100; i++) {
    dbFile.print(millis());
    dbFile.print(",");
    dbFile.print(analogRead(A0));
    dbFile.print(",");
    dbFile.println(analogRead(A1)); // Using a println makes a new line after so we can iterate through each time and just append a row every time (for CSV file types)
  }
  dbFile.close();
  return; // just to close the loop bc i dont want to test it a shitton just want to see if this works tbh  
}
