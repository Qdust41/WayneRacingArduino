#include <SD.h>
#include <SPI.h>
#include <Eventually.h>

const int chipSelect = 10; // This is the MISO pin on the SD card thing and which pin it connects to on arduino

EvtManager mgr(true); // true to manage memory

// Sets up dbFile as our 'global' variable for our SD card file
File dbFile;

bool addrow() {
  if (dbFile) {
    dbFile.print(millis() - 240);
    dbFile.print(",");
    dbFile.print(analogRead(A0));
    dbFile.print(",");
    dbFile.print(analogRead(A1));
    dbFile.print(",");
    dbFile.print(analogRead(A2));
    dbFile.print(",");
    dbFile.print(analogRead(A3));
    dbFile.print(",");
    dbFile.print(analogRead(A4));
    dbFile.print(",");
    dbFile.println(analogRead(A5)); 
  } else {
    Serial.println("Issue adding row to file");
  }
  return true;
}

void setup() {
  // Setup for the SD card
  Serial.begin(115200);
  if (!SD.begin(chipSelect)) {
    Serial.println("SD card initialization failed!");
    return;
  }
  Serial.println("SD card initialized.");

  // Open or create the file
  dbFile = SD.open("data.csv", FILE_WRITE);
  if (dbFile) {
    // Write a header
    dbFile.println("timestamp,A0,A1,A2,A3,A4,A5");
    Serial.println("Columns written to SD card. Please wait until data is written");
  } else {
    Serial.println("Failed to open file for writing.");
    return;
  }

  // Setup for the event handler
  pinMode(A0, INPUT);
  pinMode(A1, INPUT);
  pinMode(A2, INPUT);
  pinMode(A3, INPUT);
  pinMode(A4, INPUT);
  pinMode(A5, INPUT);

  mgr.addListener(new EvtTimeListener(500, true, (EvtAction)addrow));
}

void loop() {
  
  mgr.loopIteration();

  if ((millis() - 240) > 10000) {
    dbFile.close();
    Serial.println("Data written to SD card; safe to remove");
    while (1) {}; // PLEASE JUST FUCKING STOP THE LOOP I HATE IT HERE
  }

}
