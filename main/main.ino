#include <SPI.h>
#include <SD.h>
#include <mcp2515.h>
#include <genieArduino.h>

const int chipSelect = 53;
struct can_frame canMsg;
MCP2515 mcp2515(10);

char filename[15];
File dbFile;
 
//Set Up for display and Arduino
//Some of these are being changed because their pins are in use
#define DISP_TX_PIN   16    //TX2 ON MEGA
#define DISP_RX_PIN   17    //RX2 ON MEGA
#define DISP_RST_PIN   5    //D4 ON MEGA (RESET PIN)
#define DISP_BAUD    115200   //BAUD RATE OF DISPLAY
#define DISP_SERIAL  Serial2

//Index for Display Elements
#define IDX_COOL_IN       0
#define IDX_COOL_OUT      1
#define IDX_FUEL_PRESSURE 2
#define IDX_OIL_TEMP      3
#define IDX_OIL_PRESSURE  4
#define IDX_GEAR          5
#define IDX_RPM           6

//Can IDs For Data
#define CAN_IDX_RPM         0x123
#define CAN_IDX_GEAR        0x000
#define CAN_IDX_COOL_IN     0x001
#define CAN_IDX_COOL_OUT    0x002
#define CAN_IDX_FUEL_PRESS  0x003
#define CAN_IDX_OIL_TEMP    0x004
#define CAN_IDX_OIL_PRESS   0x005



//For Flashing Alerts
int stateShiftAlert = 0;
int stateSensorAlert = 0;

bool shiftAlertState = false;
bool sensorFailureState = false;

unsigned long lastShiftBlinkTimeOff = 0;
const int blinkIntervalOff = 200;
unsigned long lastShiftBlinkTimeOn = 0;
const int blinkIntervalOn = 100;

unsigned long lastSensorBlinkTimeOff = 0;
unsigned long lastSensorBlinkTimeOn = 0;


//last value for sensors
int rpm = 0;
int lastCoolIn = 0;
int lastCoolOut = 0;
int lastFuelPress = 0;
int lastOilTemp = 0;
int lastOilPress = 0;

Genie genie;
 
void setup() {
  delay(1000);

  //sd card prep function
  prep_sdcard();

  //reset
  pinMode(DISP_RST_PIN, OUTPUT);
  digitalWrite(DISP_RST_PIN, LOW);
  delay(50);
  digitalWrite(DISP_RST_PIN, HIGH);
  delay(5000);
 
  //Baud Rate Setup For Everything
  DISP_SERIAL.begin(DISP_BAUD);
  Serial.begin(115200);
  genie.Begin(DISP_SERIAL);

  //Pin Set Up for SD card and CAN Bus
  pinMode(10, OUTPUT);
  pinMode(53, OUTPUT);
  digitalWrite(10, HIGH);
  digitalWrite(53, HIGH);

  //CAN Set Up
  mcp2515.reset();
  mcp2515.setBitrate(CAN_500KBPS, MCP_8MHZ);
  mcp2515.setNormalMode();

}

int sensor_data = 0; // This is the value for the sensor defined by its ID header and then the average across the 8 points of value
void loop() {
  digitalWrite(10, LOW);

  sensor_decoding();

  fail_alerts();

  genie.DoEvents();

  if (millis() > 20000) { // We will need to determine a better way to know how long to collect data for or customize it on a per test basis
    dbFile.close();
    Serial.println("Data written to SD card; safe to remove");
    while (1) {}; // PLEASE JUST STOP THE LOOP I HATE IT HERE 
  }

  digitalWrite(10, HIGH);
}


void sensor_decoding() {
  sensor_data = 0;
  if (mcp2515.readMessage(&canMsg) == MCP2515::ERROR_OK) {
    Serial.println(canMsg.can_id);
    for (int i = 0; i<canMsg.can_dlc; i++)  {  // print the data
      sensor_data += int(canMsg.data[i]);
    }   
  }
  sensor_data /= canMsg.can_dlc;

  switch (canMsg.can_id) {
    case (CAN_IDX_RPM):
      rpm = sensor_data;
      genie.WriteObject(GENIE_OBJ_LED_DIGITS, IDX_RPM, rpm);
      break;
    case (CAN_IDX_GEAR):
      genie.WriteObject(GENIE_OBJ_LED_DIGITS, IDX_GEAR, sensor_data);
      break;
    case (CAN_IDX_COOL_IN):
      lastCoolIn = sensor_data;
      genie.WriteObject(GENIE_OBJ_LED_DIGITS, IDX_COOL_IN, sensor_data);
      break;
    case (CAN_IDX_COOL_OUT):
      lastCoolOut = sensor_data;
      genie.WriteObject(GENIE_OBJ_LED_DIGITS, IDX_COOL_OUT, sensor_data);
      break;
    case (CAN_IDX_FUEL_PRESS):
      lastFuelPress = sensor_data;
      genie.WriteObject(GENIE_OBJ_LED_DIGITS, IDX_FUEL_PRESSURE, sensor_data);
      break;
    case (CAN_IDX_OIL_TEMP):
      lastOilTemp = sensor_data;
      genie.WriteObject(GENIE_OBJ_LED_DIGITS, IDX_OIL_TEMP, sensor_data);
      break;
    case (CAN_IDX_OIL_PRESS):
      lastOilPress = sensor_data;
      genie.WriteObject(GENIE_OBJ_LED_DIGITS, IDX_OIL_PRESSURE, sensor_data);
      break;
    default:
      // Needs to be changed
      Serial.println("Default case used something broken");
      break;
    }
}

void fail_alerts() {

  //rpm alerts
  if(rpm >= 11000){
    shiftAlertState = true;
  }else{
    shiftAlertState = false;
  }

  //Gear Shift Alert
  if(shiftAlertState == true){
    if(millis() - lastShiftBlinkTimeOn >= blinkIntervalOn){
      lastShiftBlinkTimeOn = millis();
      stateShiftAlert = 1;
      genie.WriteObject(GENIE_OBJ_ILED, 0, stateShiftAlert);
    }
    if(millis() - lastShiftBlinkTimeOff >= blinkIntervalOff){
      lastShiftBlinkTimeOff = millis();
      stateShiftAlert = 0;
      genie.WriteObject(GENIE_OBJ_ILED, 0, stateShiftAlert);
    }
  }else if(stateShiftAlert != 0){
    stateShiftAlert = 0;
    genie.WriteObject(GENIE_OBJ_ILED, 0, stateShiftAlert);
  }

  //Sensor Failure
  if(lastFuelPress >= 100 || lastOilPress >= 100 || lastOilPress <= 0){
    sensorFailureState = true;
  }else{
    sensorFailureState = false;
  }
  
  //Checking if the sensor did fail
  if(sensorFailureState == true){
    if(millis() - lastSensorBlinkTimeOn >= blinkIntervalOn){
      lastSensorBlinkTimeOn = millis();
      stateSensorAlert = 1;
      genie.WriteObject(GENIE_OBJ_ILED, 1, stateSensorAlert);
    }
    if(millis() - lastSensorBlinkTimeOff >= blinkIntervalOff){
      lastSensorBlinkTimeOff = millis();
      stateSensorAlert = 0;
      genie.WriteObject(GENIE_OBJ_ILED, 1, stateSensorAlert);
    }
  }else if(stateSensorAlert != 0){
    stateSensorAlert = 0;
    genie.WriteObject(GENIE_OBJ_ILED, 1, stateSensorAlert);
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

void prep_sdcard() {
  if (!SD.begin(chipSelect)) {
    Serial.println("SD card initialization failed!");
  }
  Serial.println("SD card initialized.");
  generateFilename();
  dbFile = SD.open(filename, FILE_WRITE);
  if (dbFile) {
    // Write the CSV header lines
    dbFile.println("ms,RR Damper,RL Damper,FR Damper,FL Damper,Steering,");
  } else {
    Serial.println("Failed to open file for writing.");
  }
}

void write_sdcard() {
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
    dbFile.println(analogRead(A4)); //Steering Sensor
  } else {
    Serial.println("error writing row to file");
  }
}

 