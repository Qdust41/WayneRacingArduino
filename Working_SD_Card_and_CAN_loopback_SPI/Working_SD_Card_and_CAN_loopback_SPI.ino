#include <SPI.h>
#include <mcp_can.h>
#include <SD.h>

const int chipSelect = 10;
const int potentiometerPins[] = {A0, A1, A2, A3};
const int numPotentiometers = 4;
const int threshold = 10; // Threshold for detecting a disconnected potentiometer
File dataFile;
char filename[15];

// CAN TX Variables
unsigned long prevTX = 0;                                        // Variable to store last execution time
const unsigned int invlTX = 1000;                                // One second interval constant
byte data[] = {0xAA, 0x55, 0x01, 0x10, 0xFF, 0x12, 0x34, 0x56};  // Generic CAN data to send

// CAN RX Variables
long unsigned int rxId;
unsigned char len;
unsigned char rxBuf[8];

// Serial Output String Buffer
char msgString[128];

// CAN0 INT and CS
#define CAN0_INT 2                              // Set INT to pin 2
MCP_CAN CAN0(53);

void setup() {
  // put your setup code here, to run once:
  pinMode(10,OUTPUT);
  SPI.begin();
  Serial.begin(500000);
  // Initialize the SD card
  if (!SD.begin(chipSelect)) {
    Serial.println("Card failed, or not present");
    // don't do anything more:
    exit(0);                                    // ??? while(1) is a while true loop for some reason???
  }
  Serial.println("Card initialized.");
  generateFilename();

  // Open the file. Note that only one file can be open at a time,
  // so you have to close this one before opening another. !!!
  dataFile = SD.open(filename, FILE_WRITE);

  // if the file opened okay, write to it:
  if (dataFile) {
    Serial.print("Writing to ");
    Serial.println(filename);
    dataFile.print("Time(s)");
    dataFile.print(", RR Damper Voltage(V)");
    dataFile.print(", RL Damper Voltage(V)");
    dataFile.print(", FR Damper Voltage(V)");
    dataFile.print(", FL Damper Voltage(V)");
    dataFile.println();
    Serial.println("Header written.");
  } else {
    // if the file didn't open, print an error:
    Serial.println("error opening file");
  }

  //CAN COMMUNICATIONS SET UP
  if(CAN0.begin(MCP_ANY, CAN_500KBPS, MCP_16MHZ) == CAN_OK)
    Serial.println("MCP2515 Initialized Successfully!");
  else
    Serial.println("Error Initializing MCP2515...");
  
  // Since we do not set NORMAL mode, we are in loopback mode by default.
  //CAN0.setMode(MCP_NORMAL);

  pinMode(2, INPUT);                           
  
  Serial.println("MCP2515 Library Loopback Example...");
}

void loop() {
  // put your main code here, to run repeatedly:

  digitalWrite(10, LOW);
  if (dataFile) {
    // Read the potentiometer values
    float voltages[numPotentiometers];
    for (int i = 0; i < numPotentiometers; i++) {
      int sensorValue = analogRead(potentiometerPins[i]);
      if (sensorValue < threshold) {
        voltages[i] = 0; // Set to zero if not connected
      } else {
        voltages[i] = sensorValue * (5.0 / 1023.0); // Convert to voltage
      }
    }

    unsigned long currentTimeMillis = millis();
    float currentTimeSeconds = currentTimeMillis / 1000.0; // Convert to seconds

    // Write the time and the sensor voltages to the file
    dataFile.print(currentTimeSeconds, 3); // Print time in seconds with 3 decimal places
    for (int i = 0; i < numPotentiometers; i++) {
      dataFile.print(", ");
      dataFile.print(voltages[i], 3); // Print voltage with 3 decimal places
    }
    dataFile.println();

    // Flush the data to ensure it is written to the file
    dataFile.flush();
  } else {
    // If the file isn't open, print an error:
    Serial.println("error writing to file");
  }

  // Reduce the delay to sample more data per second
  delay(1); // Increased sampling rate by reducing delay
  digitalWrite(10, HIGH);

  digitalWrite(53, LOW);
  
  if(!digitalRead(CAN0_INT))                          // If CAN0_INT pin is low, read receive buffer
  {
    CAN0.readMsgBuf(&rxId, &len, rxBuf);              // Read data: len = data length, buf = data byte(s)
    
    if((rxId & 0x80000000) == 0x80000000)             // Determine if ID is standard (11 bits) or extended (29 bits)
      sprintf(msgString, "Extended ID: 0x%.8lX  DLC: %1d  Data:", (rxId & 0x1FFFFFFF), len);
    else
      sprintf(msgString, "Standard ID: 0x%.3lX       DLC: %1d  Data:", rxId, len);
  
    Serial.print(msgString);
  
    if((rxId & 0x40000000) == 0x40000000){            // Determine if message is a remote request frame.
      sprintf(msgString, " REMOTE REQUEST FRAME");
      Serial.print(msgString);
    } else {
      for(byte i = 0; i<len; i++){
        sprintf(msgString, " 0x%.2X", rxBuf[i]);
        Serial.print(msgString);
      }
    }
        
    Serial.println();
  }
  
  if(millis() - prevTX >= invlTX){                    // Send this at a one second interval. 
    prevTX = millis();
    byte sndStat = CAN0.sendMsgBuf(0x100, 8, data);
    
    if(sndStat == CAN_OK)
      Serial.println("Message Sent Successfully!");
    else
      Serial.println("Error Sending Message...");

  }
  digitalWrite(53, HIGH);

}

void generateFilename() {
  int fileNumber = 0;
  bool fileExists = true;

  // Generate a unique filename
  while (fileExists) {
    sprintf(filename, "Run%03d.txt", fileNumber);
    if (!SD.exists(filename)) {
      fileExists = false;
    } else {
      fileNumber++;
    }
  }
}

