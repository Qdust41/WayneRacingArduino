#include <SD.h>
#include <SPI.h>
#include <Eventually.h>
#include <mcp2515.h>
#include <mcp2515_defs.h>
#include <ThermocoupleCounter.h>
#include <SoftwareSerial.h>
#include <genieArduino.h>

unsigned long currentTimeMillis = millis();
float currentTimeSeconds = currentTimeMillis / 1000.0;
struct can_frame canMsg;
MCP2515 mcp2515(10);

Genie genie;
SoftwareSerial DisplaySerial(10, 11);

// RPM Gauge Settings
#define TICKS_PER_1000_RPM 5  // Adjust ticks per 1000 RPM
#define RED_RPM_THRESHOLD 8000  // RPM where gauge turns red
#define SHIFT_RPM_THRESHOLD 9000  // RPM where shift alert triggers

// Flashing LED Variables
bool shiftAlertState = false;
bool sensorFailureState = false;
unsigned long lastBlinkTime = 0;
const int blinkInterval = 500; // 500ms blink rate
int data = 0;
int oilPressure =0;

ThermocoupleCounter tempSensor(3, 4, 5, 11); //clkPin, csPin, doPin, counterClkPin

int total_data = 0;
int id1_data = 0;
int id2_data = 0;

const int chipSelect = 53; // This is the MISO pin on the SD card thing and which pin it connects to on arduino
char filename[15];
EvtManager mgr(true); // true to manage memory

// Sets up dbFile as our 'global' variable for our SD card file
File dbFile;

void displayUpdate(){
  long unsigned int canId;
  unsigned char len = 0;
  unsigned char buf[8];

  if (mcp2515.readMessage(&canMsg) == MCP2515::ERROR_OK) {
    for (int i = 0; i<canMsg.can_dlc; i++)  {  // print the data
      data += int(canMsg.data[i]);
    }
    // Decode RPM
    if (canMsg.can_id == 0x123) {  // Replace with actual EL129 CAN ID for RPM
      int rpm = data/canMsg.can_dlc;
      Serial.print("RPM: "); Serial.println(rpm);
      genie.WriteObject(GENIE_OBJ_LED_DIGITS, 0, rpm);  // Update RPM display

      // Update RPM Gauge
      int gaugeValue = (rpm / 1000) * TICKS_PER_1000_RPM;
      genie.WriteObject(GENIE_OBJ_GAUGE, 0, gaugeValue);

      // Change gauge color based on RPM
      if (rpm >= RED_RPM_THRESHOLD) {
        genie.WriteObject(GENIE_OBJ_USER_LED, 0, 1); // Turn gauge RED
      } else {
        genie.WriteObject(GENIE_OBJ_USER_LED, 0, 0); // Keep gauge GREEN
      }

      // Shift alert logic
      if (rpm >= SHIFT_RPM_THRESHOLD) {
        shiftAlertState = true;
      } else {
        shiftAlertState = false;
      }
    }

    // Decode Oil Temp
    if (canMsg.can_id == 0x456) {  
      int oilTemp = data/canMsg.can_dlc;
      Serial.print("Oil Temp: "); Serial.println(oilTemp);
      genie.WriteObject(GENIE_OBJ_LED_DIGITS, 1, oilTemp);
    }

    // Decode Oil Pressure
    if (canMsg.can_id == 0x789) {  
      oilPressure = data/canMsg.can_dlc;
      Serial.print("Oil Pressure: "); Serial.println(oilPressure);
      genie.WriteObject(GENIE_OBJ_LED_DIGITS, 2, oilPressure);
    }

    // Decode Fuel Pressure
    if (canMsg.can_id == 0xABC) {  
      int fuelPressure = data/canMsg.can_dlc;
      Serial.print("Fuel Pressure: "); Serial.println(fuelPressure);
      genie.WriteObject(GENIE_OBJ_LED_DIGITS, 3, fuelPressure);
    }

    // Decode Coolant Temp In (°C)
    if (canMsg.can_id == 0xDEF) {  
      int coolantTempIn = data/canMsg.can_dlc;
      Serial.print("Coolant Temp In (°C): "); Serial.println(coolantTempIn);
      genie.WriteObject(GENIE_OBJ_LED_DIGITS, 4, coolantTempIn);
    }

    // Decode Coolant Temp Out (°C)
    if (canMsg.can_id == 0x101) {  
      int coolantTempOut = data/canMsg.can_dlc;
      Serial.print("Coolant Temp Out (°C): "); Serial.println(coolantTempOut);
      genie.WriteObject(GENIE_OBJ_LED_DIGITS, 5, coolantTempOut);
    }

    // Decode Gear Position (1-digit display)
    if (canMsg.can_id == 0x202) {  
      int gearPosition = data/canMsg.can_dlc;
      Serial.print("Gear Position: "); Serial.println(gearPosition);
      genie.WriteObject(GENIE_OBJ_LED_DIGITS, 6, gearPosition);
    }


    // Sensor failure detection (example: if oil pressure reads 255, consider it an error)
    if (oilPressure == 255) {  
      sensorFailureState = true;
    } else {
      sensorFailureState = false;
    }
  }

  // Handle Flashing Alerts
  handleFlashingAlerts();

  genie.DoEvents();
  delay(100);
}

// Function to handle flashing LED indicators
void handleFlashingAlerts() {
  unsigned long currentTime = millis();

  if (currentTime - lastBlinkTime >= blinkInterval) {
    lastBlinkTime = currentTime;

    // Flash Shift Alert LED (Primitive Circle 1)
    if (shiftAlertState) {
      static bool shiftLedState = false;
      shiftLedState = !shiftLedState;
      genie.WriteObject(GENIE_OBJ_USER_LED, 1, shiftLedState);
    } else {
      genie.WriteObject(GENIE_OBJ_USER_LED, 1, 0);
    }

    // Flash Sensor Failure Alert LED (Primitive Circle 2)
    if (sensorFailureState) {
      static bool sensorLedState = false;
      sensorLedState = !sensorLedState;
      genie.WriteObject(GENIE_OBJ_USER_LED, 2, sensorLedState);
    } else {
      genie.WriteObject(GENIE_OBJ_USER_LED, 2, 0);
    }
  }
}

// Slightly messy code to add rows to the CSV file
bool addrow() {
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
  int temp = 0;
  int iternal = 0;
  temp, iternal = tempSensor.update();
  
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
    
    displayUpdate();
  
  } else {
    // This should be unrechable but just in case for logging to the serial output
    Serial.println("Issue adding row to file");
  }
  return true;
}

void setup() {
  // Setup for the SD card
  pinMode(10, OUTPUT);
  tempSensor.begin();
  mcp2515.reset();
  mcp2515.setBitrate(CAN_500KBPS, MCP_8MHZ);
  mcp2515.setNormalMode();
  DisplaySerial.begin(9600); // UART to display
  genie.Begin(DisplaySerial);
  genie.WriteContrast(15);
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
