#include <SD.h>
#include <SPI.h>
#include <Eventually.h>
#include <mcp2515.h>
#include <mcp2515_defs.h>

unsigned long currentTimeMillis = millis();
float currentTimeSeconds = currentTimeMillis / 1000.0;
struct can_frame canMsg;
MCP2515 mcp2515(10);

int total_data = 0;
int id1_data = 0;
int id2_data = 0;

const int chipSelect = 53; // This is the MISO pin on the SD card thing and which pin it connects to on arduino
char filename[15];
EvtManager mgr(true); // true to manage memory

// Sets up dbFile as our 'global' variable for our SD card file
File dbFile;

// Slightly messy code to add rows to the CSV file
bool addrow() {
  if (mcp2515.readMessage(&canMsg) == MCP2515::ERROR_OK) {
    String id = String(canMsg.can_id, HEX);
    total_data = 0;
    for (int i = 0; i<canMsg.can_dlc; i++)  {  // print the data
      total_data += int(canMsg.data[i]);
    }
    if(id == "f6"){
      id1_data = total_data/canMsg.can_dlc;
    }else{
      id2_data = total_data/canMsg.can_dlc;
    }
  }
  if (dbFile) {
    dbFile.print(millis());
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
    dbFile.print(analogRead(A5)); 
    dbFile.print(",");
    dbFile.print(analogRead(A6)); 
    dbFile.print(",");
    dbFile.print(analogRead(A7)); 
    dbFile.print(",");
    dbFile.print(analogRead(A8)); 
    dbFile.print(",");
    dbFile.print(analogRead(A9)); 
    dbFile.print(",");
    dbFile.print(analogRead(A10)); 
    dbFile.print(",");
    dbFile.print(analogRead(A11)); 
    dbFile.print(",");
    dbFile.print(analogRead(A12)); 
    dbFile.print(",");
    dbFile.print(analogRead(A13)); 
    dbFile.print(",");
    dbFile.print(id1_data); 
    dbFile.print(",");
    dbFile.println(id2_data);
  } else {
    // This should be unrechable but just in case for logging to the serial output
    Serial.println("Issue adding row to file");
  }
  return true;
}

void setup() {
  // Setup for the SD card
  pinMode(10, OUTPUT);
  mcp2515.reset();
  mcp2515.setBitrate(CAN_500KBPS, MCP_8MHZ);
  mcp2515.setNormalMode();
  Serial.begin(500000);
  if (!SD.begin(chipSelect)) {
    Serial.println("SD card initialization failed!");
    while (1) {}; // Stops if there is no SD card
  }
  Serial.println("SD card initialized.");

  // Open or create the file
  generateFilename();
  dbFile = SD.open(filename, FILE_WRITE);
  if (dbFile) {
    // Write the CSV header lines
    dbFile.println("timestamp,A0,A1,A2,A3,A4,A5,A6,A7,A8,A9,A10,A11,A12,A13,ID1,ID2");
    Serial.println("Headers written to SD card.\nPlease wait until data is written");
  } else {
    Serial.println("Failed to open file for writing.");
    while (1) {}; //Should be unrechable but just in case again
  }

  // Setup for the event handler (this is in case we opt for some event based system instead)
  pinMode(A0, INPUT);
  pinMode(A1, INPUT);
  pinMode(A2, INPUT);
  pinMode(A3, INPUT);
  pinMode(A4, INPUT);
  pinMode(A5, INPUT);
  pinMode(A6, INPUT);
  pinMode(A7, INPUT);
  pinMode(A8, INPUT);
  pinMode(A9, INPUT);
  pinMode(A10, INPUT);
  pinMode(A11, INPUT);
  pinMode(A12, INPUT);
  pinMode(A13, INPUT);
  pinMode(A14, INPUT);
  pinMode(A15, INPUT);

  digitalWrite(10, LOW);
  

  mgr.addListener(new EvtTimeListener(1, true, (EvtAction)addrow));
}

void loop() {
  mgr.loopIteration();

  Serial.print(millis());
  Serial.println("ms");

  if (millis() > 30000) { // We will need to determine a better way to know how long to collect data for or customize it on a per test basis
    dbFile.close();
    Serial.println("Data written to SD card; safe to remove");
    while (1) {}; // PLEASE JUST STOP THE LOOP I HATE IT HERE 
  }

}

void generateFilename() {
  int fileNumber = 0;
  bool fileExists = true;

  // Generate a unique filename
  while (fileExists) {
    sprintf(filename, "data%03d.csv", fileNumber);
    if (!SD.exists(filename)) {
      fileExists = false;
    } else {
      fileNumber++;
    }
  }
}
