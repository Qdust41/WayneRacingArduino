#include <SD.h>
#include <SPI.h>
#include <Eventually.h>
#include <mcp2515.h>
#include <ThermocoupleCounter.h>


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
void addrow() {
  if (mcp2515.readMessage(&canMsg) == MCP2515::ERROR_OK) { // reads CAN messages
    total_data = 0;
    for (int i = 0; i<canMsg.can_dlc; i++)  {  // collects data for that message id
      total_data += int(canMsg.data[i]);
    }
    switch(canMsg.can_id){ // picks where to display it
    case(0x0F6):
      id1_data = total_data/canMsg.can_dlc;
      break;
    case(0x036): 
      id2_data = total_data/canMsg.can_dlc;
      break;
    default:
      Serial.print("Id: ");
      Serial.println(canMsg.can_id);
      break;
    }
  }
  
  //live updates the temp of thermocoupler
  if (dbFile) { //prints rows
    dbFile.print(millis());
    dbFile.print(",");
    dbFile.print(analogRead(A0)); //RRDamper
    dbFile.print(",");
    dbFile.print(analogRead(A1)); //RLDamper
    dbFile.print(",");
    dbFile.print(analogRead(A2)); //FR Damper
    dbFile.print(",");
    dbFile.print(analogRead(A3)); //FL Damper
    dbFile.print(",");
    dbFile.print(analogRead(A4)); //Steering Sensor
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
    dbFile.print(id2_data);
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
    dbFile.println(analogRead(A11));
      
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
    dbFile.println("timestamp,RR Damper,RL Damper,FR Damper,FL Damper,Steering,A5,A6,A7,A8,A9,A10,A11,A12,A13,ID1,ID2,F1,F2,F3,F4,F5,F6,F7,F8,F9,F10,F11,F12,2F1,2F2,2F3,2F4,2F5,2F6,2F7,2F8,2F9,2F10,2F11,2F12");
    Serial.println("Headers written to SD card.\nPlease wait until data is written");
  } else {
    Serial.println("Failed to open file for writing.");
    while (1) {}; //Should be unrechable but just in case again
  }

  digitalWrite(10, LOW);
  

  mgr.addListener(new EvtTimeListener(25, true, (EvtAction)addrow));
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
